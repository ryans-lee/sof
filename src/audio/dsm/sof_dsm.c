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

#include "dsm_api_public.h"

#ifdef PRINT_ADAPTIVE_LOG
static int sof_dsm_get_adaptive_apram(struct sof_dsm_struct_t *hsof_dsm,
				      struct comp_dev *dev)
{
	enum DSM_API_MESSAGE retcode;
	int cmdblock[1 + MAX_CHANNELS];
	int excur[2], temp[2], dc_res[2], fc[2], q[2];

	if (hsof_dsm->seq_fb % 200 != 0)
		return 0;

	cmdblock[0] = DSM_SET_CMD_ID(20);
	retcode = dsm_api_get_params(hsof_dsm->dsmhandle, 1, (void *)cmdblock);
	if (retcode != DSM_API_OK)
		goto err;
	excur[0] = cmdblock[1];
	excur[1] = cmdblock[2];

	cmdblock[0] = DSM_SET_CMD_ID(19);
	retcode = dsm_api_get_params(hsof_dsm->dsmhandle, 1, (void *)cmdblock);
	if (retcode != DSM_API_OK)
		goto err;
	temp[0] = cmdblock[1];
	temp[1] = cmdblock[2];

	cmdblock[0] = DSM_SET_CMD_ID(18);
	retcode = dsm_api_get_params(hsof_dsm->dsmhandle, 1, (void *)cmdblock);
	if (retcode != DSM_API_OK)
		goto err;
	dc_res[0] = cmdblock[1];
	dc_res[1] = cmdblock[2];

	cmdblock[0] = DSM_SET_CMD_ID(16);
	retcode = dsm_api_get_params(hsof_dsm->dsmhandle, 1, (void *)cmdblock);
	if (retcode != DSM_API_OK)
		goto err;
	fc[0] = cmdblock[1];
	fc[1] = cmdblock[2];

	cmdblock[0] = DSM_SET_CMD_ID(17);
	retcode = dsm_api_get_params(hsof_dsm->dsmhandle, 1, (void *)cmdblock);
	if (retcode != DSM_API_OK)
		goto err;
	q[0] = cmdblock[1];
	q[1] = cmdblock[2];

	comp_dbg(dev, "[DSM] excursion(Q27) :%08x, %08x",
		 excur[0], excur[1]);
	comp_dbg(dev,
		 "[DSM] temp(Q19) :%08x, %08x, dc_res(Q27) :%08x, %08x",
		 temp[0], temp[1], dc_res[0], dc_res[1]);
	comp_dbg(dev,
		 "[DSM] fc(Q9) :%08x, %08x, q(Q29) :%08x, %08x",
		 fc[0], fc[1], q[0], q[1]);

	return 0;
err:
	comp_err(dev, "[DSM] Get adaptive parameters. error:%d", retcode);
	return retcode;
}
#endif
static int sof_dsm_set_default_param(struct sof_dsm_struct_t *hsof_dsm,
				     struct comp_dev *dev)
{
	enum DSM_API_MESSAGE retcode;
	int value[MAX_CHANNELS * 2 + 1];

	value[0] = DSM_SET_CMD_ID(111);
	value[1] = 0;
	retcode = dsm_api_set_params(hsof_dsm->dsmhandle, 1, value);
	if (retcode != DSM_API_OK)
		goto err;

	value[0] = DSM_SET_CMD_ID(104);
	value[1] = 0;
	retcode = dsm_api_set_params(hsof_dsm->dsmhandle, 1, value);
	if (retcode != DSM_API_OK)
		goto err;

	value[0] = DSM_SET_CMD_ID(70);
	value[1] = 0;	/* Smart pilot tone off */
	retcode = dsm_api_set_params(hsof_dsm->dsmhandle, 1, value);
	if (retcode != DSM_API_OK)
		goto err;

	value[0] = DSM_SET_CMD_ID(198);
	value[1] = 0;	/* Debuzzer off */
	retcode = dsm_api_set_params(hsof_dsm->dsmhandle, 1, value);
	if (retcode != DSM_API_OK)
		goto err;

	value[0] = DSM_SET_CMD_ID(36);
	value[1] = 0;	/* EQ off */
	retcode = dsm_api_set_params(hsof_dsm->dsmhandle, 1, value);
	if (retcode != DSM_API_OK)
		goto err;

	value[0] = DSM_SET_CMD_ID(146);
	value[1] = 0;
	retcode = dsm_api_set_params(hsof_dsm->dsmhandle, 1, value);
	if (retcode != DSM_API_OK)
		goto err;

	value[0] = DSM_SET_CMD_ID(30);
	value[1] = 1;
	retcode = dsm_api_set_params(hsof_dsm->dsmhandle, 1, value);
	if (retcode != DSM_API_OK)
		goto err;

	value[0] = DSM_SET_CMD_ID(31);
	value[1] = 1;
	retcode = dsm_api_set_params(hsof_dsm->dsmhandle, 1, value);
	if (retcode != DSM_API_OK)
		goto err;

	value[0] = DSM_SET_CMD_ID(1);
	value[1] = 1;
	retcode = dsm_api_set_params(hsof_dsm->dsmhandle, 1, value);
	if (retcode != DSM_API_OK)
		goto err;

	return 0;
err:
	comp_err(dev, "[DSM] Set default parameters ID:%x value:%x, error:%d",
		 value[0], value[1], retcode);
	return retcode;
}

int sof_dsm_get_memory_size(struct sof_dsm_struct_t *hsof_dsm, struct comp_dev *dev)
{
	enum DSM_API_MESSAGE retcode;
	struct dsm_api_memory_size_ext_t memsize;
	int *circularbuffersize = hsof_dsm->circularbuffersize;

	memsize.ichannels = 2;
	memsize.ipcircbuffersizebytes = circularbuffersize;
	memsize.isamplingrate = 48000;
	memsize.omemsizerequestedbytes = 0;
	memsize.numeqfilters = 8;
	retcode = dsm_api_get_mem(&memsize, sizeof(struct dsm_api_memory_size_ext_t));
	if (retcode != DSM_API_OK)
		return 0;

	return memsize.omemsizerequestedbytes;
}

int sof_dsm_create(struct sof_dsm_struct_t *hsof_dsm, struct comp_dev *dev)
{
	int *circularbuffersize = hsof_dsm->circularbuffersize;
	int *delayedsamples = hsof_dsm->delayedsamples;
	struct dsm_api_init_ext_t initparam;
	enum DSM_API_MESSAGE retcode;

	comp_dbg(dev, BUILD_TIME);

	if (!hsof_dsm->dsm_init) {
		initparam.isamplebitwidth       = 16;
		initparam.ichannels             = 2;
		initparam.ipcircbuffersizebytes = circularbuffersize;
		initparam.ipdelayedsamples      = delayedsamples;
		initparam.isamplingrate         = 48000;

		retcode = dsm_api_init(hsof_dsm->dsmhandle, &initparam,
				       sizeof(struct dsm_api_init_ext_t));
		if (retcode != DSM_API_OK) {
			goto exit;
		} else	{
			hsof_dsm->ff_fr_sz_samples =
				initparam.off_framesizesamples;
			hsof_dsm->fb_fr_sz_samples =
				initparam.ofb_framesizesamples;
			hsof_dsm->channelmask = 0;
			hsof_dsm->nchannels = initparam.ichannels;
			hsof_dsm->ifsamples = hsof_dsm->ff_fr_sz_samples
				* initparam.ichannels;
			hsof_dsm->ibsamples = hsof_dsm->fb_fr_sz_samples
				* initparam.ichannels;

			hsof_dsm->dsm_init = true;
		}

		sof_dsm_set_default_param(hsof_dsm, dev);

		comp_dbg(dev, "[DSM] Initialization completed. (sof:%p, dsm:%p)",
			 (uintptr_t)hsof_dsm,
			 (uintptr_t)hsof_dsm->dsmhandle);
	} else {
		comp_dbg(dev, "[DSM] Re-initialization.");
	}
	return 0;
exit:
	comp_err(dev, "[DSM] Initialization failed. ret:%d", retcode);
	return (int)retcode;
}

void sof_dsm_ff_process(struct sof_dsm_struct_t *hsof_dsm, struct comp_dev *dev,
			void *in, void *out, int nsamples, int szsample)
{
	enum DSM_API_MESSAGE retcode;
	union sof_dsm_buf buf, buf_out;
	int16_t *input = (int16_t *)hsof_dsm->buf.input;
	int16_t *output = (int16_t *)hsof_dsm->buf.output;
	int16_t *stage = (int16_t *)hsof_dsm->buf.stage;
	int *w_ptr = &hsof_dsm->buf.ff.avail;
	int *r_ptr = &hsof_dsm->buf.ff_out.avail;
	bool is_16bit = szsample == 2 ? true : false;
	int remain;
	int x;

	if (!hsof_dsm->dsm_init)
		return;

	buf.buf16 = (int16_t *)hsof_dsm->buf.ff.buf;
	buf.buf32 = (int32_t *)hsof_dsm->buf.ff.buf;
	buf_out.buf16 = (int16_t *)hsof_dsm->buf.ff_out.buf;
	buf_out.buf32 = (int32_t *)hsof_dsm->buf.ff_out.buf;
	//comp_info(dev, "[DSM] FF in frames:%d", nsamples);
	if (*w_ptr + nsamples <= DSM_FF_BUF_DB_SZ) {
		if (is_16bit)
			memcpy_s(&buf.buf16[*w_ptr], nsamples * szsample,
				 in, nsamples * szsample);
		else
			memcpy_s(&buf.buf32[*w_ptr], nsamples * szsample,
				 in, nsamples * szsample);
		*w_ptr += nsamples;
	} else {
		comp_err(dev,
			 "[DSM] FF process buffer overflow. (w_ptr : %d + %d > %d)",
			 *w_ptr, nsamples, DSM_FF_BUF_DB_SZ);
		return;
	}

	if (*w_ptr >= DSM_FF_BUF_SZ) {
		/* Do FF processing */
		//comp_info(dev, "[DSM] FF PROCESS ++ %d", nsamples >> 1);
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

		remain = (*w_ptr - DSM_FF_BUF_SZ);
		if (remain)	{
			/* memmove is not available. */
			if (is_16bit) {
				memcpy_s(stage, remain * szsample,
					 &buf.buf16[DSM_FF_BUF_SZ],
					 remain * szsample);
				memcpy_s(buf.buf16, remain * szsample,
					 stage, remain * szsample);
			} else {
				memcpy_s(stage, remain * szsample,
					 &buf.buf32[DSM_FF_BUF_SZ],
					 remain * szsample);
				memcpy_s(buf.buf32, remain * szsample,
					 stage, remain * szsample);
			}
		}
		*w_ptr -= DSM_FF_BUF_SZ;

		hsof_dsm->ifsamples = hsof_dsm->nchannels * hsof_dsm->ff_fr_sz_samples;
		retcode = dsm_api_ff_process(hsof_dsm->dsmhandle, hsof_dsm->channelmask,
					     input, &hsof_dsm->ifsamples,
					     output, &hsof_dsm->ofsamples);

		for (x = 0; x < DSM_FRM_SZ; x++) {
			/* Buffer re-ordering LR/LR/LR */
			if (is_16bit) {
				buf_out.buf16[*r_ptr + 2 * x] = (output[x]);
				buf_out.buf16[*r_ptr + 2 * x + 1] =
					(output[x + DSM_FRM_SZ]);
			} else {
				buf_out.buf32[*r_ptr + 2 * x] = (output[x] << 16);
				buf_out.buf32[*r_ptr + 2 * x + 1] =
					(output[x + DSM_FRM_SZ] << 16);
			}
		}

		*r_ptr += DSM_FF_BUF_SZ;

		hsof_dsm->seq_ff++;
		//comp_info(dev, "[DSM] FF PROCESS --");
	}

	/* Output buffer preparation */
	if (*r_ptr >= nsamples) {
		memcpy_s(out, nsamples * szsample,
			 buf_out.buf16, nsamples * szsample);
		remain = (*r_ptr - nsamples);
		if (remain)	{
			/* memmove is not available. */
			if (is_16bit) {
				memcpy_s(stage, remain * szsample,
					 &buf_out.buf16[nsamples],
					 remain * szsample);
				memcpy_s(buf_out.buf16, remain * szsample,
					 stage, remain * szsample);
			} else {
				memcpy_s(stage, remain * szsample,
					 &buf_out.buf32[nsamples],
					 remain * szsample);
				memcpy_s(buf_out.buf32, remain * szsample,
					 stage, remain * szsample);
			}
		}
		*r_ptr -= nsamples;
	} else {
		memset(out, 0, nsamples * szsample);
		comp_err(dev,
			 "[DSM] DSM FF process underrun. r_ptr : %d",
			 *r_ptr);
	}
}

void sof_dsm_fb_process(struct sof_dsm_struct_t *hsof_dsm, struct comp_dev *dev,
			void *in, int nsamples, int szsample)
{
	enum DSM_API_MESSAGE retcode;
	union sof_dsm_buf buf;
	int *w_ptr = &hsof_dsm->buf.fb.avail;
	int16_t *stage = (int16_t *)hsof_dsm->buf.stage_fb;
	int16_t *v = hsof_dsm->buf.voltage;
	int16_t *i = hsof_dsm->buf.current;
	bool is_16bit = szsample == 2 ? true : false;
	int remain;
	int x;

	buf.buf16 = (int16_t *)hsof_dsm->buf.fb.buf;
	buf.buf32 = (int32_t *)hsof_dsm->buf.fb.buf;

	if (!hsof_dsm->dsm_init)
		return;

	if (*w_ptr + nsamples <= DSM_FB_BUF_DB_SZ) {
		if (is_16bit)
			memcpy_s(&buf.buf16[*w_ptr], nsamples * szsample,
				 in, nsamples * szsample);
		else
			memcpy_s(&buf.buf32[*w_ptr], nsamples * szsample,
				 in, nsamples * szsample);
		*w_ptr += nsamples;
	} else {
		comp_err(dev, "[DSM] DSM FB process overflow. w_ptr : %d",
			 *w_ptr);
		return;
	}
	if (*w_ptr >= DSM_FB_BUF_SZ) {
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

		remain = (*w_ptr - DSM_FB_BUF_SZ);
		if (remain)	{
			/* memmove is not available. */
			if (is_16bit) {
				memcpy_s(stage, remain * szsample,
					 &buf.buf16[DSM_FB_BUF_SZ],
					 remain * szsample);
				memcpy_s(buf.buf16, remain * szsample,
					 stage, remain * szsample);
			} else {
				memcpy_s(stage, remain * szsample,
					 &buf.buf32[DSM_FB_BUF_SZ],
					 remain * szsample);
				memcpy_s(buf.buf32, remain * szsample,
					 stage, remain * szsample);
			}
		}
		*w_ptr -= DSM_FB_BUF_SZ;

		hsof_dsm->ibsamples = hsof_dsm->fb_fr_sz_samples * hsof_dsm->nchannels;
		hsof_dsm->channelmask = 0;

		retcode = dsm_api_fb_process(hsof_dsm->dsmhandle,
					     hsof_dsm->channelmask,
					     i, v, &hsof_dsm->ibsamples);

		#ifdef PRINT_ADAPTIVE_LOG
		sof_dsm_get_adaptive_apram(hsof_dsm, dev);
		#endif

		hsof_dsm->seq_fb++;
	}
}
