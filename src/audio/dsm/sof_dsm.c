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

int sof_dsm_create(struct sof_dsm_struct_t *hsof_dsm, struct comp_dev *dev)
{
	if (!hsof_dsm->dsm_init) {
		/* DSM will be initialized here. */
		hsof_dsm->dsm_init = true;
	} else {
		comp_dbg(dev, "[DSM] Re-initialization.");
	}
	return 0;
}

void sof_dsm_ff_process(struct sof_dsm_struct_t *hsof_dsm, struct comp_dev *dev,
			void *in, void *out, int nsamples, int szsample)
{
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

		/* Do DSM feed forward processing here. Currently bypassed */
		memcpy_s(output, DSM_FF_BUF_SZ * szsample,
			 input, DSM_FF_BUF_SZ * szsample);

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

		/* Do DSM feedback processing here */

		hsof_dsm->seq_fb++;
	}
}
