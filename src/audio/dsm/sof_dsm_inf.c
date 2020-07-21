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

static void *_wrap_buf_ptr(void *ptr, const struct audio_stream *buffer)
{
	if (ptr >= buffer->end_addr)
		ptr = (char *)buffer->addr +
			((char *)ptr - (char *)buffer->end_addr);
	return ptr;
}
int sof_dsm_inf_create(struct sof_dsm_struct_t *hSof, struct comp_dev *dev)
{
	return sof_dsm_create(hSof, dev);
}
static int sof_dsm_inf_get_buffer(int32_t *buf, uint32_t frames,
	struct audio_stream *stream, int num_channel)
{
	int x, y;
	union sof_dsm_buf input, output;

	input.buf16 = (int16_t *) stream->r_ptr;
	input.buf32 = (int32_t *) stream->r_ptr;
	output.buf16 = (int16_t *) buf;
	output.buf32 = (int32_t *) buf;

	switch (stream->frame_fmt) {
	case SOF_IPC_FRAME_S16_LE:
		for (x = 0 ; x < frames ; x++)            {
			for (y = 0 ; y < num_channel ; y++)            {
				input.buf16 = (int16_t *) _wrap_buf_ptr(
					input.buf16,
					stream);
				output.buf16[num_channel * x + y]
					= *input.buf16;
				input.buf16++;
			}
		}
		break;
	case SOF_IPC_FRAME_S24_4LE:
	case SOF_IPC_FRAME_S32_LE:
		for (x = 0 ; x < frames ; x++)            {
			for (y = 0 ; y < num_channel ; y++)            {
				input.buf32 = (int32_t *) _wrap_buf_ptr(
					input.buf32,
					stream);
				output.buf32[num_channel * x + y]
					= *input.buf32;
				input.buf32++;
			}
		}
		break;
	default:
		return -EINVAL;
	}
	return 0;
}
static int sof_dsm_inf_put_buffer(int32_t *buf, uint32_t frames,
	struct audio_stream *stream, int num_channel, int num_slot,
	char mask, struct comp_dev *dev)
{
	int x, y, offset;
	char value;
	union sof_dsm_buf input, output;
	const char pow_2[8] = {1, 2, 4, 8, 16, 32, 64, 128};

	input.buf16 = (int16_t *) buf;
	input.buf32 = (int32_t *) buf;
	output.buf16 = (int16_t *) stream->w_ptr;
	output.buf32 = (int32_t *) stream->w_ptr;

	//comp_info(dev, "[DSM] buf:%p", (uintptr_t) buf);

	switch (stream->frame_fmt) {
	case SOF_IPC_FRAME_S16_LE:
		for (x = 0 ; x < frames ; x++)           {
			for (y = 0 ; y < num_slot ; y++)            {
				if (y == 0)
					offset = 0;
				output.buf16 = (int16_t *)_wrap_buf_ptr(
					output.buf16,
					stream);
				value = pow_2[y] & mask;
				*output.buf16 =
					value > 0 ? input.buf16[num_channel * x
					+ (offset++)] : 0;
				output.buf16++;
			}
		}
		break;
	case SOF_IPC_FRAME_S24_4LE:
	case SOF_IPC_FRAME_S32_LE:
		for (x = 0 ; x < frames ; x++)            {
			for (y = 0 ; y < num_slot ; y++)            {
				if (y == 0)
					offset = 0;
				output.buf32 = (int32_t *)_wrap_buf_ptr(
					output.buf32,
					stream);
				value = pow_2[y] & mask;
				*output.buf32 =
					value > 0 ? input.buf32[num_channel * x
					+ (offset++)] : 0;
				output.buf32++;
			}
		}
		break;
	default:
		return -EINVAL;
	}
	return 0;
}

int sof_dsm_inf_ff_copy(struct comp_dev *dev, uint32_t frames,
			     struct comp_buffer *source,
			     struct comp_buffer *sink, int8_t *chan_map,
			     struct sof_dsm_struct_t *hSof)
{
	int ret;

	if (!hSof->dsm_init)
		return -EINVAL;

	hSof->seq_sof_in++;

	if (frames != 48)	{
		comp_err(dev, "[DSM] FF: Unexpected frame size : %d", frames);
		return -EINVAL;
	}

	ret = sof_dsm_inf_get_buffer(hSof->buf.sof_a_frame_in,
		frames, &(source->stream), 2);
	if (ret != 0)
		goto err;

	switch (source->stream.frame_fmt) {
	case SOF_IPC_FRAME_S16_LE:
		sof_dsm_ff_process(hSof, dev,
			hSof->buf.sof_a_frame_in,
			hSof->buf.sof_a_frame_out,
			frames * 2, sizeof(int16_t));
		break;
	case SOF_IPC_FRAME_S24_4LE:
	case SOF_IPC_FRAME_S32_LE:
		sof_dsm_ff_process(hSof, dev,
			hSof->buf.sof_a_frame_in,
			hSof->buf.sof_a_frame_out,
			frames * 2, sizeof(int32_t));
		break;
	default:
		ret = -EINVAL;
		goto err;
	}

	ret = sof_dsm_inf_put_buffer(hSof->buf.sof_a_frame_out,
		frames, &sink->stream, 2, 4, 0x3, dev);
	if (ret != 0)
		goto err;

	return 0;
err:
	comp_err(dev, "[DSM] Not supported frame format");
	return ret;
}
int sof_dsm_inf_fb_copy(struct comp_dev *dev, uint32_t frames,
			     struct comp_buffer *source,
			     struct comp_buffer *sink, int8_t *chan_map,
			     struct sof_dsm_struct_t *hSof)
{
	int ret;

	if (!hSof->dsm_init)
		return -EINVAL;

	if (frames != 48)	{
		comp_err(dev, "[DSM] FB: Unexpected frame size : %d", frames);
		return -EINVAL;
	}

	ret = sof_dsm_inf_get_buffer(hSof->buf.sof_a_frame_iv,
		frames, &(source->stream), 4);
	if (ret != 0)
		goto err;

	switch (source->stream.frame_fmt) {
	case SOF_IPC_FRAME_S16_LE:
		sof_dsm_fb_process(hSof, dev, hSof->buf.sof_a_frame_iv,
			frames * 4, sizeof(int16_t));
		break;
	case SOF_IPC_FRAME_S24_4LE:
	case SOF_IPC_FRAME_S32_LE:
		sof_dsm_fb_process(hSof, dev, hSof->buf.sof_a_frame_iv,
			frames * 4, sizeof(int32_t));
		break;
	default:
		ret = -EINVAL;
		goto err;
	}
	return 0;
err:
	comp_err(dev, "[DSM] Not supported frame format : %d",
		source->stream.frame_fmt);
	return ret;
}

