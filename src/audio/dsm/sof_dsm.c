// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright (c) 2020, Maxim Integrated

#include <sof/audio/sof_dsm.h>
#include <sof/audio/smart_amp_test.h>
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

#ifdef PRINT_ADAPTIVE_LOG
static int sof_dsm_get_adaptive_apram(struct sof_dsm_struct_t *hSof,
	struct comp_dev *dev)
{
	DSM_API_MESSAGE retCode;
	int cmdBlock[1+MAX_CHANNELS];
	int excur[2], temp[2], dc_res[2], fc[2], q[2];

	if (hSof->seq_fb % 200 != 0)
		return 0;

	cmdBlock[0] = DSM_SET_CMD_ID(DSM_API_GET_EXCURSION);
	retCode = DSM_API_Get_Params(hSof->dsmHandle,
		1, (void *)cmdBlock);
	if (retCode != DSM_API_OK)
		goto err;
	excur[0] = cmdBlock[1];
	excur[1] = cmdBlock[2];

	cmdBlock[0] = DSM_SET_CMD_ID(DSM_API_GET_ADAPTIVE_COILTEMP);
	retCode = DSM_API_Get_Params(hSof->dsmHandle,
		1, (void *)cmdBlock);
	if (retCode != DSM_API_OK)
		goto err;
	temp[0] = cmdBlock[1];
	temp[1] = cmdBlock[2];

	cmdBlock[0] = DSM_SET_CMD_ID(DSM_API_GET_ADAPTIVE_DC_RES);
	retCode = DSM_API_Get_Params(hSof->dsmHandle,
		1, (void *)cmdBlock);
	if (retCode != DSM_API_OK)
		goto err;
	dc_res[0] = cmdBlock[1];
	dc_res[1] = cmdBlock[2];

	cmdBlock[0] = DSM_SET_CMD_ID(DSM_API_GET_ADAPTIVE_FC);
	retCode = DSM_API_Get_Params(hSof->dsmHandle,
		1, (void *)cmdBlock);
	if (retCode != DSM_API_OK)
		goto err;
	fc[0] = cmdBlock[1];
	fc[1] = cmdBlock[2];

	cmdBlock[0] = DSM_SET_CMD_ID(DSM_API_GET_ADAPTIVE_Q);
	retCode = DSM_API_Get_Params(hSof->dsmHandle,
		1, (void *)cmdBlock);
	if (retCode != DSM_API_OK)
		goto err;
	q[0] = cmdBlock[1];
	q[1] = cmdBlock[2];

	comp_info(dev, "[DSM] excursion(Q27) :%08x, %08x",
		excur[0], excur[1]);
	comp_info(dev,
		"[DSM] temp(Q19) :%08x, %08x, dc_res(Q27) :%08x, %08x",
		temp[0], temp[1], dc_res[0], dc_res[1]);
	comp_info(dev,
		"[DSM] fc(Q9) :%08x, %08x, q(Q29) :%08x, %08x",
		fc[0], fc[1], q[0], q[1]);

	return 0;
err:
	comp_err(dev, "[DSM] Get adaptive parameters. error:%d", retCode);
	return retCode;
}
#endif
static int sof_dsm_set_default_param(struct sof_dsm_struct_t *hSof,
	struct comp_dev *dev)
{
	DSM_API_MESSAGE retCode;
	int value[MAX_CHANNELS*2+1];

	value[0] = DSM_SET_CMD_ID(DSM_API_SETGET_ENABLE_LINKWITZ_EQ);
	value[1] = 0;
	retCode = DSM_API_Set_Params(hSof->dsmHandle, 1, value);
	if (retCode != DSM_API_OK)
		goto err;

	value[0] = DSM_SET_CMD_ID(DSM_API_SETGET_ENABLE_SMART_PT);
	value[1] = 0;
	retCode = DSM_API_Set_Params(hSof->dsmHandle, 1, value);
	if (retCode != DSM_API_OK)
		goto err;

	value[0] = DSM_SET_CMD_ID(DSM_API_SETGET_ENABLE_LOGGING);
	value[1] = 0;	/* Smart pilot tone off */
	retCode = DSM_API_Set_Params(hSof->dsmHandle, 1, value);
	if (retCode != DSM_API_OK)
		goto err;

	value[0] = DSM_SET_CMD_ID(DSM_API_SETGET_DEBUZZER_ENABLE);
	value[1] = 0;	/* Debuzzer off */
	retCode = DSM_API_Set_Params(hSof->dsmHandle, 1, value);
	if (retCode != DSM_API_OK)
		goto err;

	value[0] = DSM_SET_CMD_ID(DSM_API_SETGET_EQ_BAND_ENABLE);
	value[1] = 0;	/* EQ off */
	retCode = DSM_API_Set_Params(hSof->dsmHandle, 1, value);
	if (retCode != DSM_API_OK)
		goto err;

	value[0] = DSM_SET_CMD_ID(DSM_API_SETGET_IV_FORMAT);
	value[1] = 0;
	retCode = DSM_API_Set_Params(hSof->dsmHandle, 1, value);
	if (retCode != DSM_API_OK)
		goto err;

	value[0] = DSM_SET_CMD_ID(DSM_API_SETGET_THERMAL_ENABLE);
	value[1] = 1;
	retCode = DSM_API_Set_Params(hSof->dsmHandle, 1, value);
	if (retCode != DSM_API_OK)
		goto err;

	value[0] = DSM_SET_CMD_ID(DSM_API_SETGET_ENABLE);
	value[1] = 1;
	retCode = DSM_API_Set_Params(hSof->dsmHandle, 1, value);
	if (retCode != DSM_API_OK)
		goto err;

	return 0;
err:
	comp_err(dev, "[DSM] Set default parameters ID:%x value:%x, error:%d",
		value[0], value[1], retCode);
	return retCode;
}

int sof_dsm_get_memory_size(struct sof_dsm_struct_t *hSof, struct comp_dev *dev)
{
	DSM_API_MESSAGE retCode;
	int *circularBufferSize = hSof->circularBufferSize;
	dsm_api_memory_size_ext_t sMemSize;

	sMemSize.iChannels = 2;
	sMemSize.ipCircBufferSizeBytes = circularBufferSize;
	sMemSize.iSamplingRate = 48000;
	sMemSize.oMemSizeRequestedBytes = 0;
	sMemSize.numEQFilters = 8;
	retCode = DSM_API_get_memory_size_ext(&sMemSize,
		sizeof(dsm_api_memory_size_ext_t));
	if (retCode != DSM_API_OK)
		return 0;

	comp_info(dev,
		"[DSM] sof_dsm_get_memory_size = %d",
		sMemSize.oMemSizeRequestedBytes);

	return sMemSize.oMemSizeRequestedBytes;
}
int sof_dsm_create(struct sof_dsm_struct_t *hSof, struct comp_dev *dev)
{
	int *circularBufferSize = hSof->circularBufferSize;
	int *delayedSamples = hSof->delayedSamples;
	dsm_api_init_ext_t sInitParam;
	DSM_API_MESSAGE retCode;

	comp_info(dev, BUILD_TIME);

	if (!hSof->dsm_init)	{
		sInitParam.iSampleBitWidth       = 16;
		sInitParam.iChannels             = 2;
		sInitParam.ipCircBufferSizeBytes = circularBufferSize;
		sInitParam.ipDelayedSamples      = delayedSamples;
		sInitParam.iSamplingRate         = 48000;

		retCode = DSM_API_Init_ext(
			hSof->dsmHandle,
			&sInitParam,
			sizeof(dsm_api_init_ext_t));
		if (retCode != DSM_API_OK)
			goto exit;
		else	{
			hSof->ffFrameSizeSamples =
				sInitParam.oFF_FrameSizeSamples;
			hSof->fbFrameSizeSamples =
				sInitParam.oFB_FrameSizeSamples;
			hSof->channelMask = 0;
			hSof->nChannels = sInitParam.iChannels;
			hSof->iFSamples = hSof->ffFrameSizeSamples
				* sInitParam.iChannels;
			hSof->iBSamples = hSof->fbFrameSizeSamples
				* sInitParam.iChannels;

			hSof->dsm_init = true;
		}

		sof_dsm_set_default_param(hSof, dev);

		comp_info(dev, "[DSM] Initialization completed. (sof:%p, dsm:%p)",
			(uintptr_t) hSof,
			(uintptr_t) hSof->dsmHandle);
	} else
		comp_info(dev, "[DSM] Re-initialization.");
	return 0;
exit:
	comp_err(dev, "[DSM] Initialization failed. ret:%d", retCode);
	return (int) retCode;
}

void sof_dsm_ff_process(struct sof_dsm_struct_t *hSof, struct comp_dev *dev,
	void *in, void *out, int nSamples, int szSample)
{
	DSM_API_MESSAGE retCode;
	union sof_dsm_buf buf, buf_out;
	int16_t *input = (int16_t *) hSof->buf.input;
	int16_t *output = (int16_t *) hSof->buf.output;
	int16_t *stage = (int16_t *) hSof->buf.stage;
	int *wrPtr = &(hSof->buf.ff.avail);
	int *rdPtr = &(hSof->buf.ff_out.avail);
	bool is_16bit = szSample == 2 ? true : false;
	int remain;
	int x;

	if (!hSof->dsm_init)
		return;

	buf.buf16 = (int16_t *) hSof->buf.ff.buf;
	buf.buf32 = (int32_t *) hSof->buf.ff.buf;
	buf_out.buf16 = (int16_t *) hSof->buf.ff_out.buf;
	buf_out.buf32 = (int32_t *) hSof->buf.ff_out.buf;

	if (*wrPtr + nSamples <= DSM_FF_BUF_DB_SZ) {
		if (is_16bit)
			memcpy_s(&buf.buf16[*wrPtr], nSamples * szSample,
				in, nSamples * szSample);
		else
			memcpy_s(&buf.buf32[*wrPtr], nSamples * szSample,
				in, nSamples * szSample);
		*wrPtr += nSamples;
	} else {
		comp_info(dev,
			"[DSM] FF process buffer overflow. (rdPtr : %d > %d)",
			*wrPtr, DSM_FF_BUF_DB_SZ);
		return;
	}

	if (*wrPtr >= DSM_FF_BUF_SZ) {
		/* Do FF processing */
		for (x = 0; x < DSM_FRM_SZ; x++) {
			/* Buffer ordering for DSM process LLL... / RRR... */
			if (is_16bit) {
				input[x] = (buf.buf16[2 * x]);
				input[x + DSM_FRM_SZ] =
					(buf.buf16[2 * x + 1]);
			} else {
				input[x] = (buf.buf32[2 * x] >> 16);
				input[x + DSM_FRM_SZ] =
					(buf.buf32[2 * x + 1] >> 16);
			}
		}

		remain = (*wrPtr - DSM_FF_BUF_SZ);
		if (remain)	{
			/* memmove is not available. */
			if (is_16bit) {
				memcpy_s(stage, remain * szSample,
					&buf.buf16[DSM_FF_BUF_SZ],
					remain * szSample);
				memcpy_s(buf.buf16, remain * szSample,
					stage, remain * szSample);
			} else {
				memcpy_s(stage, remain * szSample,
					&buf.buf32[DSM_FF_BUF_SZ],
					remain * szSample);
				memcpy_s(buf.buf32, remain * szSample,
					stage, remain * szSample);
			}
		}
		*wrPtr -= DSM_FF_BUF_SZ;		

		hSof->iFSamples = hSof->nChannels * hSof->ffFrameSizeSamples;

		retCode = DSM_API_FF_process(
			hSof->dsmHandle,
			hSof->channelMask, input, &hSof->iFSamples,
			output, &hSof->oFSamples);

		for (x = 0; x < DSM_FRM_SZ; x++) {
			/* Buffer re-ordering LR/LR/LR */
			if (is_16bit) {
				buf_out.buf16[*rdPtr + 2 * x] = (output[x]);
				buf_out.buf16[*rdPtr + 2 * x+ 1] =
					(output[x + DSM_FRM_SZ]);
			} else {
				buf_out.buf32[*rdPtr + 2 * x] = (output[x] << 16);
				buf_out.buf32[*rdPtr + 2 * x + 1] =
					(output[x + DSM_FRM_SZ] << 16);
			}
		}

		*rdPtr += DSM_FF_BUF_SZ;

		hSof->seq_ff++;
	}

	/* Output buffer preparation */
	if (*rdPtr >= nSamples) {
		memcpy_s(out, nSamples * szSample,
			buf_out.buf16, nSamples * szSample);
		remain = (*rdPtr - nSamples);
		if (remain)	{
			/* memmove is not available. */
			if (is_16bit) {
				memcpy_s(stage, remain * szSample,
					&buf_out.buf16[nSamples],
					remain * szSample);
				memcpy_s(buf_out.buf16, remain * szSample,
					stage, remain * szSample);
			} else {
				memcpy_s(stage, remain * szSample,
					&buf_out.buf32[nSamples],
					remain * szSample);
				memcpy_s(buf_out.buf32, remain * szSample,
					stage, remain * szSample);
			}
		}
		*rdPtr -= nSamples;
	} else {
		memset(out, 0, nSamples * szSample);
		comp_err(dev,
			"[DSM] DSM FF process underrun. rdPtr : %d",
			*rdPtr);
	}
	memcpy(out, in, nSamples * szSample);
}
void sof_dsm_fb_process(struct sof_dsm_struct_t *hSof, struct comp_dev *dev,
	void *in, int nSamples, int szSample)
{
	DSM_API_MESSAGE retCode;
	union sof_dsm_buf buf;
	int *wrPtr = &(hSof->buf.fb.avail);
	int16_t *stage = (int16_t *) hSof->buf.stage_fb;
	int16_t *v = hSof->buf.voltage;
	int16_t *i = hSof->buf.current;
	bool is_16bit = szSample == 2 ? true : false;
	int remain;
	int x;

	buf.buf16 = (int16_t *) hSof->buf.fb.buf;
	buf.buf32 = (int32_t *) hSof->buf.fb.buf;

	if (!hSof->dsm_init)
		return;

	if (*wrPtr + nSamples <= DSM_FB_BUF_DB_SZ) {
		if (is_16bit)
			memcpy_s(&buf.buf16[*wrPtr], nSamples * szSample,
				in, nSamples * szSample);
		else
			memcpy_s(&buf.buf32[*wrPtr], nSamples * szSample,
				in, nSamples * szSample);
		*wrPtr += nSamples;
	} else {
		comp_err(dev, "[DSM] DSM FB process overflow. wrPtr : %d",
			*wrPtr);
		return;
	}
	if (*wrPtr >= DSM_FB_BUF_SZ) {
		/* Do FB processing */
		for (x = 0; x < DSM_FRM_SZ; x++) {
			/* Buffer ordering for DSM process IVIV -> VV... II...*/
			if (is_16bit) {
				v[x] = buf.buf16[4 * x + 1];
				i[x] = buf.buf16[4 * x];
				v[x + DSM_FRM_SZ] = buf.buf16[4 * x + 3];
				i[x + DSM_FRM_SZ] = buf.buf16[4 * x + 2];
			} else {
				v[x] = (buf.buf32[4 * x + 1] >> 16);
				i[x] = (buf.buf32[4 * x] >> 16);
				v[x + DSM_FRM_SZ] =
					(buf.buf32[4 * x + 3] >> 16);
				i[x + DSM_FRM_SZ] =
					(buf.buf32[4 * x + 2] >> 16);
			}
		}

		remain = (*wrPtr - DSM_FB_BUF_SZ);
		if (remain)	{
			/* memmove is not available. */
			if (is_16bit) {
				memcpy_s(stage, remain * szSample,
					&buf.buf16[DSM_FB_BUF_SZ],
					remain * szSample);
				memcpy_s(buf.buf16, remain * szSample,
					stage, remain * szSample);
			} else {
				memcpy_s(stage, remain * szSample,
					&buf.buf32[DSM_FB_BUF_SZ],
					remain * szSample);
				memcpy_s(buf.buf32, remain * szSample,
					stage, remain * szSample);
			}
		}
		*wrPtr -= DSM_FB_BUF_SZ;

		hSof->iBSamples = hSof->fbFrameSizeSamples * hSof->nChannels;
		hSof->channelMask = 0;

		retCode = DSM_API_FB_process(
			hSof->dsmHandle,
			hSof->channelMask, i, v, &hSof->iBSamples);

		#ifdef PRINT_ADAPTIVE_LOG
		sof_dsm_get_adaptive_apram(hSof, dev);
		#endif

		hSof->seq_fb++;
	}
}

