/* SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright(c) 2020 Maxim Integrated All rights reserved.
 *
 * Author: Ryan Lee <ryans.lee@maximintegrated.com>
 */

#ifndef __SOF_AUDIO_SMART_AMP_MAXIM_H__
#define __SOF_AUDIO_SMART_AMP_MAXIM_H__

#include <sof/platform.h>
#include <sof/audio/component.h>

#define SMART_AMP_MAX_STREAM_CHAN   8

/** IPC blob types */
#define SOF_SMART_AMP_CONFIG	0
#define SOF_SMART_AMP_MODEL     1

struct smart_amp_model_data {
	uint32_t data_size;
	void *data;
	uint32_t crc;
	uint32_t data_pos;
};

typedef int(*smart_amp_proc)(struct comp_dev *dev,
			     const struct audio_stream *source,
			     const struct audio_stream *sink, uint32_t frames,
			     int8_t *chan_map, bool is_feedback);

struct sof_smart_amp_config {
	uint32_t size;
	uint32_t feedback_channels;
	int8_t source_ch_map[PLATFORM_MAX_CHANNELS];
	int8_t feedback_ch_map[PLATFORM_MAX_CHANNELS];
};

#endif /* __SOF_AUDIO_SMART_AMP_H__ */
