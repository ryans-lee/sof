// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright (c) 2020, Maxim Integrated

#include <sof/audio/sof_dsm.h>
#include <sof/audio/smart_amp.h>
#include <sof/audio/component.h>
#include <sof/trace/trace.h>
#include <sof/drivers/ipc.h>
#include <sof/ut.h>

#include <sof/audio/channel_map.h>
#include <sof/trace/trace.h>
#include <user/trace.h>
#include <sof/bit.h>
#include <sof/common.h>
#include <ipc/channel_map.h>
#include <stdint.h>
#include <stdlib.h>

#include "dsm_api.h"
#include "dsm_api_types.h"

#ifdef USE_DSM_LIB
static void *dsmHandle;
static char _buf_dsm_static[50000];
static int delayedSamples[MAX_CHANNELS*4];
static dsm_api_init_ext_t sInitParam;
static int ffFrameSizeSamples, fbFrameSizeSamples;
static dsm_api_memory_size_ext_t   sMemSize;
int circularBufferSize[MAX_CHANNELS*4];
static int channelMask;
static int iFSamples;//, iBSamples;
static int oFSamples;
#else
#define SZ_S200 120
static const short sin_200[] = {
	0x0000, 0x035A, 0x06B3, 0x0A0B, 0x0D61,
	0x10B5, 0x1406, 0x1753, 0x1A9D, 0x1DE1,
	0x2121, 0x245A, 0x278E, 0x2ABA, 0x2DDF,
	0x30FB, 0x3410, 0x371B, 0x3A1C, 0x3D13,
	0x3FFF, 0x42E1, 0x45B6, 0x487F, 0x4B3C,
	0x4DEB, 0x508D, 0x5320, 0x55A5, 0x581B,
	0x5A82, 0x5CD8, 0x5F1F, 0x6154, 0x6379,
	0x658C, 0x678D, 0x697C, 0x6B59, 0x6D22,
	0x6ED9, 0x707C, 0x720C, 0x7387, 0x74EE,
	0x7641, 0x777F, 0x78A8, 0x79BB, 0x7ABA,
	0x7BA2, 0x7C76, 0x7D33, 0x7DDA, 0x7E6C,
	0x7EE7, 0x7F4B, 0x7F9A, 0x7FD2, 0x7FF4,
	0x7FFF, 0x7FF4, 0x7FD2, 0x7F9A, 0x7F4C,
	0x7EE7, 0x7E6C, 0x7DDA, 0x7D33, 0x7C76,
	0x7BA2, 0x7ABA, 0x79BB, 0x78A8, 0x777F,
	0x7641, 0x74EE, 0x7387, 0x720C, 0x707C,
	0x6ED9, 0x6D22, 0x6B59, 0x697C, 0x678D,
	0x658C, 0x6379, 0x6154, 0x5F1F, 0x5CD8,
	0x5A82, 0x581B, 0x55A5, 0x5320, 0x508D,
	0x4DEB, 0x4B3C, 0x487F, 0x45B6, 0x42E1,
	0x4000, 0x3D13, 0x3A1C, 0x371B, 0x3410,
	0x30FB, 0x2DDF, 0x2ABA, 0x278E, 0x245A,
	0x2121, 0x1DE1, 0x1A9D, 0x1753, 0x1406,
	0x10B5, 0x0D61, 0x0A0B, 0x06B3, 0x035A
};

#define SZ_S1K 24
static const short sin_1k[] = {
	0x0000, 0x10B5, 0x2121, 0x30FB, 0x3FFF,
	0x4DEB, 0x5A82, 0x658C, 0x6ED9, 0x7641,
	0x7BA2, 0x7EE7, 0x7FFF, 0x7EE7, 0x7BA2,
	0x7641, 0x6ED9, 0x658C, 0x5A82, 0x4DEB,
	0x4000, 0x30FB, 0x2121, 0x10B5};

static short sof_dsm_test_sin_gen(struct sof_dsm_struct_t* sofDsmHandle, int tone_type)
{
	short value;

	if (tone_type == 0) {
		value =
			sin_200[sofDsmHandle->tone_gen_seq0 % SZ_S200] >> 1;
		sofDsmHandle->tone_gen_seq0++;

		if (!sofDsmHandle->tone_gen_toggle0)
			value *= -1;

		if (sofDsmHandle->tone_gen_seq0 % SZ_S200 == 0) {
			if (sofDsmHandle->tone_gen_toggle0)
				sofDsmHandle->tone_gen_toggle0 = false;
			else
				sofDsmHandle->tone_gen_toggle0 = true;
		}
	} else {
		value =
			sin_1k[sofDsmHandle->tone_gen_seq1 % SZ_S1K] >> 1;
		sofDsmHandle->tone_gen_seq1++;

		if (!sofDsmHandle->tone_gen_toggle1)
			value *= -1;

		if (sofDsmHandle->tone_gen_seq1 % SZ_S1K == 0) {
			if (sofDsmHandle->tone_gen_toggle1)
				sofDsmHandle->tone_gen_toggle1 = false;
			else
				sofDsmHandle->tone_gen_toggle1 = true;
		}
	}

	return value;
}
#endif
static void sof_dsm_print_params(void *handle, struct comp_dev *dev)
{
	DSM_API_MESSAGE retCode;
	int cmdBlock[1+MAX_CHANNELS];
	int x;
	#define NUM_SERIES	11
	int print_series[NUM_SERIES] = {16,17,18,19,20,28,29,30,31,111,26};

	for (x = 0; x < NUM_SERIES ; x++) {
	       	cmdBlock[0] = DSM_SET_CMD_ID(print_series[x]);
		retCode = DSM_API_Get_Params(handle,
			1, (void *)cmdBlock);
		if (retCode == DSM_API_OK)
			comp_info(dev, "[RYAN] PARAM ID:%d, L:%d, R:%d",
				print_series[x], cmdBlock[1],cmdBlock[2]);
	}
}

void sof_dsm_onoff(struct comp_dev *dev, int on)
{
	DSM_API_MESSAGE retCode;
	int value[4*2+1];
	int cmdBlock[1+MAX_CHANNELS];

	value[0] = DSM_SET_CMD_ID(DSM_API_SETGET_ENABLE);
	value[1] = on;
	retCode = DSM_API_Set_Params(dsmHandle, 1, value);

	if (retCode == DSM_API_OK)	{
	       	cmdBlock[0] = DSM_SET_CMD_ID(DSM_API_SETGET_ENABLE);
		retCode = DSM_API_Get_Params(dsmHandle,
			1, (void *)cmdBlock);
		if (retCode == DSM_API_OK)
			comp_info(dev, "[RYAN] sof_dsm_onoff = L:%d, R:%d <- %d)",
				cmdBlock[1], cmdBlock[2], on);
		else
			goto exit;

	} else
		goto exit;
	return;
exit:
	comp_info(dev, "[RYAN] sof_dsm_onoff failed : %d", retCode);
}

void sof_dsm_create(struct sof_dsm_struct_t *sofDsmHandle,
	int ch_id, struct comp_dev *dev)
{
#ifdef USE_DSM_LIB
	int x;

	comp_info(dev, "[RYAN] FW VER : 01JUL2020 #59");
	comp_info(dev, "[RYAN] sof_dsm_create. ex:%d, ch_id:%d",
		sofDsmHandle->init, ch_id);

	if (!sofDsmHandle->init)	{
		DSM_API_MESSAGE retCode;

		sMemSize.iChannels              = 2;
		sMemSize.ipCircBufferSizeBytes  =    circularBufferSize;
		sMemSize.iSamplingRate          = 48000;
		sMemSize.oMemSizeRequestedBytes = 0;
		sMemSize.numEQFilters           = 8;
		retCode = DSM_API_get_memory_size_ext(&sMemSize,
			sizeof(dsm_api_memory_size_ext_t));
		if (retCode != DSM_API_OK)
			comp_info(dev,
				"DSM get memory Failure (retCode:%d)", retCode);
		else
			comp_info(dev,
				"DSM get memory OK. (retCode:%d)", retCode);

		dsmHandle = (void *) &_buf_dsm_static[0];	// 44272
		memset(dsmHandle, 0, 50000);

		sInitParam.iSampleBitWidth       = 16;
		sInitParam.iChannels             = 2;
		sInitParam.ipCircBufferSizeBytes = circularBufferSize;
		sInitParam.ipDelayedSamples      = delayedSamples;
		sInitParam.iSamplingRate         = 48000;

		comp_info(dev,
			"[RYAN] DSM_API_Init +++ sz:%d dsmHandle=0x%x %d, %d)",
			sMemSize.oMemSizeRequestedBytes, (int) dsmHandle,
			sInitParam.oFF_FrameSizeSamples,
			sInitParam.oFB_FrameSizeSamples);

		retCode = DSM_API_Init_ext(dsmHandle,
			&sInitParam,
			sizeof(dsm_api_init_ext_t)
		);
		if (retCode != DSM_API_OK) {
			comp_info(dev, "[RYAN] DSM_API_Init failure (retCode:%d)",
				(int) retCode);
			sofDsmHandle->init = 0;
		} else	{
			sofDsmHandle->init = 1;

			ffFrameSizeSamples = sInitParam.oFF_FrameSizeSamples;
			fbFrameSizeSamples = sInitParam.oFB_FrameSizeSamples;
			channelMask = 0;

			comp_info(dev,
				"[RYAN] DSM_API_Init     SizeSamples:(ff:%d, fb:%d)",
				ffFrameSizeSamples,
				fbFrameSizeSamples);
			comp_info(dev,
				"[RYAN] DSM_API_Init --- sz:%d dsmHandle=0x%x  %d, %d",
				sMemSize.oMemSizeRequestedBytes,
				(int) dsmHandle,
				sInitParam.oFF_FrameSizeSamples,
				sInitParam.oFB_FrameSizeSamples);
		}
		/* Read / Write parameters */
		int cmdBlock[1+MAX_CHANNELS];

		for (x = DSM_API_SETGET_ENABLE;
			x <= DSM_API_GET_EXCURSION ; x++) {

			if (x == DSM_API_INIT_F_Q_FILTERS)
				continue;

		       	cmdBlock[0] = DSM_SET_CMD_ID(x);
			retCode = DSM_API_Get_Params(dsmHandle,
				1, (void *)cmdBlock);
			if (retCode != DSM_API_OK)
				comp_info(dev,
					"[RYAN] DSM_API_Get_Params() failed: id:%d error code = %i",
					x, (int)retCode);
			else
				comp_info(dev, "[RYAN] DSM_API_Get_Params(ID:%d) (L:%x, R:%x)",
					x, cmdBlock[1], cmdBlock[2]);
		}
		int value[4*2+1];

		#if 0
		for (x = DSM_API_SETGET_CLIP_ENABLE;
			x <= DSM_API_SETGET_THERMAL_ENABLE ; x++)	{
			value[0] = DSM_SET_CMD_ID(x);
			value[1] = 0;
			retCode = DSM_API_Set_Params((void *)dsmHandle,
				1, value);
			if (retCode != DSM_API_OK)
				comp_info(dev,
					"[RYAN] DSM_API_Set_Params() failed: id:%d error code = %i",
					x, (int)retCode);
		}
		#endif
#if 1	// 200625
		value[0] = DSM_SET_CMD_ID(DSM_API_SETGET_CLIP_ENABLE);
		value[1] = 0;
		retCode = DSM_API_Set_Params((void *)dsmHandle, 1, value);
		if (retCode != DSM_API_OK)
			comp_info(dev,
				"[RYAN] DSM_API_Set_Params() failed: id:%d error code = %i",
				DSM_API_SETGET_ENABLE_LINKWITZ_EQ,
				(int)retCode);
		value[0] = DSM_SET_CMD_ID(DSM_API_SETGET_EXC_ENABLE);
		value[1] = 0;
		retCode = DSM_API_Set_Params((void *)dsmHandle, 1, value);
		if (retCode != DSM_API_OK)
			comp_info(dev,
				"[RYAN] DSM_API_Set_Params() failed: id:%d error code = %i",
				DSM_API_SETGET_ENABLE_LINKWITZ_EQ,
				(int)retCode);
		value[0] = DSM_SET_CMD_ID(DSM_API_SETGET_THERMAL_ENABLE);
		value[1] = 0;
		retCode = DSM_API_Set_Params((void *)dsmHandle, 1, value);
		if (retCode != DSM_API_OK)
			comp_info(dev,
				"[RYAN] DSM_API_Set_Params() failed: id:%d error code = %i",
				DSM_API_SETGET_ENABLE_LINKWITZ_EQ,
				(int)retCode);
#endif		
		value[0] = DSM_SET_CMD_ID(DSM_API_SETGET_ENABLE_LINKWITZ_EQ);
		value[1] = 0;
		retCode = DSM_API_Set_Params((void *)dsmHandle, 1, value);
		if (retCode != DSM_API_OK)
			comp_info(dev,
				"[RYAN] DSM_API_Set_Params() failed: id:%d error code = %i",
				DSM_API_SETGET_ENABLE_LINKWITZ_EQ,
				(int)retCode);

		value[0] = DSM_SET_CMD_ID(DSM_API_SETGET_ENABLE_SMART_PT);
		value[1] = 0;
		retCode = DSM_API_Set_Params((void *)dsmHandle, 1, value);
		if (retCode != DSM_API_OK)
			comp_info(dev,
				"[RYAN] DSM_API_Set_Params() failed: id:%d error code = %i",
				DSM_API_SETGET_ENABLE_SMART_PT, (int)retCode);

		value[0] = DSM_SET_CMD_ID(DSM_API_SETGET_ENABLE_LOGGING);
		value[1] = 0;
		retCode = DSM_API_Set_Params((void *)dsmHandle, 1, value);
		if (retCode != DSM_API_OK)
			comp_info(dev,
				"[RYAN] DSM_API_Set_Params() failed: id:%d error code = %i",
				DSM_API_SETGET_ENABLE_LOGGING, (int)retCode);

		value[0] = DSM_SET_CMD_ID(DSM_API_SETGET_DEBUZZER_ENABLE);
		value[1] = 0;
		retCode = DSM_API_Set_Params((void *)dsmHandle, 1, value);
		if (retCode != DSM_API_OK)
			comp_info(dev,
				"[RYAN] DSM_API_Set_Params() failed: id:%d error code = %i",
				DSM_API_SETGET_DEBUZZER_ENABLE, (int)retCode);

		value[0] = DSM_SET_CMD_ID(DSM_API_SETGET_EQ_BAND_ENABLE);
		value[1] = 0;
		retCode = DSM_API_Set_Params((void *)dsmHandle, 1, value);
		if (retCode != DSM_API_OK)
			comp_info(dev,
				"[RYAN] DSM_API_Set_Params() failed: id:%d error code = %i",
				DSM_API_SETGET_EQ_BAND_ENABLE, (int)retCode);
	}
#else
	comp_info(dev, "[RYAN] DSM test initialized. ex:%d, ch_id:%d",
		sofDsmHandle->init, ch_id);
	if (!sofDsmHandle->init) {
		memset((void *) sofDsmHandle, 0, sizeof(struct sof_dsm_struct_t));
		sofDsmHandle->init = 1;
	}
#endif
}

void sof_dsm_destroy(struct sof_dsm_struct_t *sofDsmHandle, struct comp_dev *dev)
{
	comp_info(dev, "[RYAN] DSM de-initialized. ex:%d", sofDsmHandle->init);
	sofDsmHandle->init = 0;
}

#ifndef USE_DSM_LIB
void dsm_ff_process(struct sof_dsm_struct_t *sofDsmHandle, short *input,
	int nSamples, int szSamples, struct comp_dev *dev)
{
	int x;
	int nSamples_per_channel = (nSamples >> 1);

	if (sofDsmHandle->seq_ff % 200 == 0)
		comp_info(dev, "[RYAN] DSM FF. nSamples:%d (%d, %d), initialized:%d",
			nSamples, input[0], input[1], sofDsmHandle->init);

	if (sofDsmHandle->seq_ff % 1000 == 0)	{
		if (sofDsmHandle->toggle == false)
			sofDsmHandle->toggle = true;
		else
			sofDsmHandle->toggle = false;
	}

	for (x = 0 ; x < nSamples_per_channel; x++)	{
		input[x] = sof_dsm_test_sin_gen(sofDsmHandle, 1);
		input[x + nSamples_per_channel] = sof_dsm_test_sin_gen(sofDsmHandle, 0);
	}
	sofDsmHandle->seq_ff++;
}
#endif

void dsm_fb_process(struct sof_dsm_struct_t *sofDsmHandle, short *input,
	int nSamples, int szSamples, struct comp_dev *dev)
{
	if (sofDsmHandle->seq_fb % 200 == 0)
		comp_info(dev, "[RYAN] DSM FB nSamples:%d (%d, %d), initialized:%d",
			nSamples, input[0], input[1], sofDsmHandle->init);
	sofDsmHandle->seq_fb++;
}

static void dsm_test_rms_print32_raw(int *input, int nSamples, int *rms_left, int *rms_right)
{
	int x;
	long long left, right;

	left = 0;
	right = 0;

	for (x = 0 ; x < nSamples ; x++)	{
		left += (input[2* x] > 0 ? input[2* x] : -input[2* x]);
		right += (input[2* x + 1] > 0 ? input[2* x + 1] : -input[2* x + 1]);
	}
	*rms_left = (left >> 16);
	*rms_right = (right >> 16);
}

static void dsm_test_rms_print(short *input, int nSamples, int *rms_left, int *rms_right)
{
	int x;
	int righty;

	*rms_left = 0;
	*rms_right = 0;

	for (x = 0 ; x < nSamples ; x++)	{
		*rms_left += (input[x] > 0 ? input[x] : -input[x]);
		righty = x + nSamples;
		*rms_right += (input[righty] > 0 ? input[righty] : -input[righty]);
	}
}

void sof_dsm_ff_process_32(struct sof_dsm_struct_t *sofDsmHandle, void *in, void *out,
	int nSamples, int szSample, struct comp_dev *dev)
{
	int *buf = sofDsmHandle->buf_ff;
	int *obuf = sofDsmHandle->buf_ff_out;
	int *stage = sofDsmHandle->stage;
	short *input = (short *)sofDsmHandle->proc;
	short *output = (short *)sofDsmHandle->proc2;
	int *wrPtr = &(sofDsmHandle->ff_avail);
	int *rdPtr = &(sofDsmHandle->ff_rdy);
	int remain;
	int x;
	int rms_left, rms_right, rms_left_out, rms_right_out;

	if (*wrPtr + nSamples <= SZ_BUFFER) {
		memcpy_s(&buf[*wrPtr], nSamples * szSample,
			in, nSamples * szSample);
		*wrPtr += nSamples;
	}
	else {
		comp_info(dev,
			"[RYAN] DSM FF process overflow. rdPtr : %d",
			*wrPtr);
		return;
	}


	if (*wrPtr >= SZ_BUFFER) {
		/* Do FF processing */
		if (sofDsmHandle->init) {
			if (sofDsmHandle->seq % 200 == 0 || sofDsmHandle->seq < 20) {
				dsm_test_rms_print32_raw(buf, SZ_PROC_BUF, &rms_left, &rms_right);
				comp_info(dev,
					"[RYAN] DSM FF reference (%d, %d)",
					rms_left, rms_right);
			}
			for (x = 0; x < SZ_PROC_BUF; x++) {
				input[x] = (buf[2 * (*wrPtr - SZ_BUFFER + x)] >> 16);
				input[x + SZ_PROC_BUF] = (buf[2 * (*wrPtr - SZ_BUFFER + x) + 1] >> 16);

			}

			if (sofDsmHandle->seq % 200 == 0 || sofDsmHandle->seq < 20)
				dsm_test_rms_print(input, SZ_PROC_BUF, &rms_left, &rms_right);

			#ifdef USE_DSM_LIB
			DSM_API_MESSAGE retCode;

			iFSamples = ffFrameSizeSamples*sInitParam.iChannels;

			if (sofDsmHandle->seq % 200 == 0 || sofDsmHandle->seq < 20)
				comp_info(dev, "[RYAN] DSM FF +++ . nSamples:%d, FrameSize:%d, ch:%d, seq:%d",
					iFSamples, ffFrameSizeSamples,
					sInitParam.iChannels, sofDsmHandle->seq);

#if 0
			retCode = DSM_API_FF_process(
				(void *)dsmHandle,
				channelMask, input, &iFSamples,
				output, &oFSamples);
#else
			for (x = 0; x < SZ_PROC_BUF; x++) {
				output[2 * x] = input[2 * x];
				output[2* x + 1] = input[2* x + 1];
			}
#endif
			#else
			dsm_ff_process(
				(struct sof_dsm_struct_t *) sofDsmHandle,
				tempBuf, SZ_BUFFER, 2, dev);
			#endif
			/* process done */

			if (sofDsmHandle->seq % 200 == 0 || sofDsmHandle->seq < 20) {
				sof_dsm_print_params((void *)dsmHandle, dev);

				dsm_test_rms_print(output, SZ_PROC_BUF, &rms_left_out, &rms_right_out);
				comp_info(dev,
					"[RYAN] DSM FF RMS CHECK. Left %d->%d, Right %d -> %d",
					rms_left, rms_left_out, rms_right, rms_right_out);
			}

			if (sofDsmHandle->seq % 200 == 0 || sofDsmHandle->seq < 20)
				comp_info(dev,
					"[RYAN] DSM FF    . ffFrameSizeSamples:%d, chMask:%d, iFSampes:%d, oFSamples:%d",
					ffFrameSizeSamples,
					channelMask, iFSamples, oFSamples);
				comp_info(dev,
					"[RYAN] DSM FF ---. retCode:%d ([0]%d, [1]%d)",
					retCode,
					output[0], output[1]);

			/* Re-ordering back */
			for (x = 0; x < SZ_PROC_BUF; x++) {
				obuf[2 * (*rdPtr + x)] = (output[x] << 16);
				obuf[2 * (*rdPtr + x) + 1] = (output[x + SZ_PROC_BUF] << 16);
			}

		} else {
			comp_info(dev,
				"[RYAN] DSM FF process skipped. %d, seq:%d",
				sofDsmHandle->init, sofDsmHandle->seq);
		}

		*rdPtr += SZ_BUFFER;	// *rdPtr sized buffer is ready.

		/* Do Process here */
		/* Process done */
		remain = (*wrPtr - SZ_BUFFER);

		if (remain)	{
			//memmove(&buf[0], &buf[SZ_PROC_BUF],
			//	(SZ_BUFFER - SZ_PROC_BUF) * szSample);
			memcpy_s(&stage[0], remain * szSample,
				&buf[SZ_BUFFER], remain * szSample);
			//memset(&buf[SZ_PROC_BUF], 0, remain * szSample);
			memcpy_s(&buf[0], remain * szSample,
				&stage[0], remain * szSample);
		}

		*wrPtr -= SZ_BUFFER;
		sofDsmHandle->seq++;
	}	

	/* Output buffer preparation */
	if (*rdPtr >= nSamples) {
		memcpy_s(out, nSamples * szSample,
			obuf, nSamples * szSample);
		remain = (*rdPtr - nSamples);

		if (remain)	{
			//memmove(&obuf[0], &obuf[nSamples],
				//(*rdPtr - nSamples) * szSample);
			memcpy_s(&stage[0], remain * szSample,
				&obuf[nSamples], remain * szSample);
			//memset(&obuf[nSamples], 0, remain * szSample);
			memcpy_s(&obuf[0], remain * szSample,
				&stage[0], remain * szSample);
		}
		*rdPtr -= nSamples;
	} else {
		memset(out, 0, nSamples * szSample);
		comp_info(dev,
			"[RYAN] DSM FF process underrun. rdPtr : %d",
			*rdPtr);
	}
}

void sof_dsm_ff_process(struct sof_dsm_struct_t *sofDsmHandle, void *in, void *out,
	int nSamples, int szSample, struct comp_dev *dev)
{
	short *buf = (short *)sofDsmHandle->buf_ff;
	short *obuf = (short *)sofDsmHandle->buf_ff_out;
	short *stage = (short *)sofDsmHandle->stage;
	short *input = (short *)sofDsmHandle->proc;
	short *output = (short *)sofDsmHandle->proc2;
	int *wrPtr = &(sofDsmHandle->ff_avail);
	int *rdPtr = &(sofDsmHandle->ff_rdy);
	int remain;
	int x;

	if (*wrPtr + nSamples <= SZ_BUFFER) {
		/* 16bit process only */
		memcpy_s(&buf[*wrPtr], nSamples * szSample,
			in, nSamples * szSample);
		*wrPtr += nSamples;
	}
	else {
		comp_info(dev,
			"[RYAN] DSM FF process overflow. rdPtr : %d",
			*wrPtr);
		return;
	}


	if (*wrPtr >= SZ_BUFFER) {
		/* Do FF processing */
		if (sofDsmHandle->init) {
			for (x = 0; x < SZ_PROC_BUF; x++) {
				input[x] = buf[2 * (*wrPtr-SZ_BUFFER+x)];
				input[x + SZ_PROC_BUF] = buf[2 * (*wrPtr-SZ_BUFFER+x) + 1];
			}

			#ifdef USE_DSM_LIB
			DSM_API_MESSAGE retCode;

			iFSamples = ffFrameSizeSamples*sInitParam.iChannels;

			if (sofDsmHandle->seq % 200 == 0 || sofDsmHandle->seq < 20)
				comp_info(dev, "[RYAN] DSM FF +++ . nSamples:%d, ch:%d,(%d) seq:%d",
				iFSamples, sInitParam.iChannels,
				input[0], sofDsmHandle->seq);

			channelMask = 0;

			retCode = DSM_API_FF_process(
				(void *)dsmHandle,
				channelMask, input, &iFSamples,
				output, &oFSamples);

			if (sofDsmHandle->seq % 200 == 0 || sofDsmHandle->seq < 20)
				comp_info(dev,
					"[RYAN] DSM FF    . ffFrameSizeSamples:%d, chMask:%d, iFSampes:%d, oFSamples:%d",
					ffFrameSizeSamples,
					channelMask, iFSamples, oFSamples);
				comp_info(dev,
					"[RYAN] DSM FF ---. retCode:%d ([0]%d, [1]%d)",
					retCode,
					output[0], output[1]);
			#else
			dsm_ff_process(
				(struct sof_dsm_struct_t *) sofDsmHandle,
				input, SZ_BUFFER, 2, dev);
			#endif

			/* Re-ordering back */
			for (x = 0; x < SZ_PROC_BUF; x++) {
				obuf[2 * (*rdPtr + x)] = output[x];
				obuf[2 * (*rdPtr + x) + 1] = output[x + SZ_PROC_BUF];
			}

		} else {
			if (sofDsmHandle->seq % 200 == 0 || sofDsmHandle->seq < 20)
				comp_info(dev,
					"[RYAN] DSM FF process skipped. %d, seq:%d",
					sofDsmHandle->init, sofDsmHandle->seq);
		}

		*rdPtr += SZ_BUFFER;	// *rdPtr sized buffer is ready.

		/* Do Process here */
		/* Process done */
		remain = (*wrPtr - SZ_BUFFER);

		if (remain)	{
			//memmove(&buf[0], &buf[SZ_PROC_BUF],
			//	(SZ_BUFFER - SZ_PROC_BUF) * szSample);
			memcpy_s(&stage[0], remain * szSample,
				&buf[SZ_BUFFER], remain * szSample);
			//memset(&buf[SZ_PROC_BUF], 0, remain * szSample);
			memcpy_s(&buf[0], remain * szSample,
				&stage[0], remain * szSample);
		}

		*wrPtr -= SZ_BUFFER;
		sofDsmHandle->seq++;
	}	

	/* Output buffer preparation */
	if (*rdPtr >= nSamples) {
		memcpy_s(out, nSamples * szSample,
			obuf, nSamples * szSample);
		remain = (*rdPtr - nSamples);

		if (remain)	{
			//memmove(&obuf[0], &obuf[nSamples],
				//(*rdPtr - nSamples) * szSample);
			memcpy_s(&stage[0], remain * szSample,
				&obuf[nSamples], remain * szSample);
			//memset(&obuf[nSamples], 0, remain * szSample);
			memcpy_s(&obuf[0], remain * szSample,
				&stage[0], remain * szSample);
		}
		*rdPtr -= nSamples;
	} else {
		memset(out, 0, nSamples * szSample);
		comp_info(dev,
			"[RYAN] DSM FF process underrun. rdPtr : %d",
			*rdPtr);
	}
}

