/* SPDX-License-Identifier: GPL-2.0-or-later
 *  Copyright (c) 2020 Maxim Integrated
 */

#ifndef __SOF_AUDIO_DSM_H__
#define __SOF_AUDIO_DSM_H__

#include <sof/platform.h>
#include <sof/audio/component.h>

#define SUPPORT_VARIABLE_FRAME_SZ

#define MAX_CHANNELS 2
#define SOF_FRM_SZ	48	// 48 samples per frame
#define SOF_FF_BUF_SZ	(SOF_FRM_SZ << 1)
#define SOF_FB_BUF_SZ	(SOF_FRM_SZ << 2)
#define DSM_FRM_SZ	240	// 240 samples per frame
#define DSM_FF_BUF_SZ	(DSM_FRM_SZ << 1)
#define DSM_FB_BUF_SZ	(DSM_FRM_SZ << 2)

#ifdef SUPPORT_VARIABLE_FRAME_SZ
#define SOF_FF_BUF_DB_SZ	(SOF_FF_BUF_SZ << 1)
#define SOF_FB_BUF_DB_SZ	(SOF_FB_BUF_SZ << 1)
#define DSM_FF_BUF_DB_SZ	(DSM_FF_BUF_SZ << 1)
#define DSM_FB_BUF_DB_SZ	(DSM_FB_BUF_SZ << 1)
#else
#define SOF_FF_BUF_DB_SZ	SOF_FF_BUF_SZ
#define SOF_FB_BUF_DB_SZ	SOF_FB_BUF_SZ
#define DSM_FF_BUF_DB_SZ	DSM_FF_BUF_SZ
#define DSM_FB_BUF_DB_SZ	DSM_FB_BUF_SZ
#endif

#define BUILD_TIME ("[DSM] FW VER : "__DATE__" "__TIME__)

union sof_dsm_buf {
	int16_t *buf16;
	int32_t *buf32;
};
struct sof_dsm_prep_ff_buf_struct_t {
	int32_t *buf;
	int avail;
};
struct sof_dsm_prep_fb_buf_struct_t {
	int32_t *buf;
	int avail;
	int rdy;
};
struct sof_dsm_buf_struct_t {
	/* buffer : sof -> sof dsm ff */
	int32_t *sof_a_frame_in;
	/* buffer : sof <- sof dsm ff */
	int32_t *sof_a_frame_out;
	/* buffer : sof -> sof dsm fb */
	int32_t *sof_a_frame_iv;
	/* buffer : sof dsm ff stage */
	int32_t *stage;
	/* buffer : sof dsm fb stage */
	int32_t *stage_fb;
	/* buffer : sof dsm -> dsm ff in */
	int16_t *input;
	/* buffer : sof dsm <- dsm ff out */
	int16_t *output;
	/* buffer : sof dsm -> dsm fb voltage */
	int16_t *voltage;
	/* buffer : sof dsm -> dsm fb current */
	int16_t *current;
	/* buffer : sof dsm ff in : variable length -> fixed length */
	struct sof_dsm_prep_ff_buf_struct_t ff;
	/* buffer : sof dsm ff out : variable length <- fixed length */
	struct sof_dsm_prep_ff_buf_struct_t ff_out;
	/* buffer : sof dsm fb : variable length -> fixed length */
	struct sof_dsm_prep_fb_buf_struct_t fb;
};
struct sof_dsm_struct_t {
	void *dsmhandle;
	int delayedsamples[MAX_CHANNELS << 2];
	int circularbuffersize[MAX_CHANNELS << 2];
	int ff_fr_sz_samples;
	int fb_fr_sz_samples;
	int channelmask;
	int nchannels;
	int ifsamples;
	int ibsamples;
	int ofsamples;
	bool dsm_init;
	struct sof_dsm_buf_struct_t buf;
	int seq_sof_in;
	int seq_ff;
	int seq_fb;
};

/* Interface functions */
int sof_dsm_inf_reset(struct sof_dsm_struct_t *hsof_dsm, struct comp_dev *dev);
int sof_dsm_inf_create(struct sof_dsm_struct_t *hsof_dsm, struct comp_dev *dev);
int sof_dsm_inf_ff_copy(struct comp_dev *dev, uint32_t frames,
			struct comp_buffer *source,
			struct comp_buffer *sink, int8_t *chan_map,
			struct sof_dsm_struct_t *hsof_dsm);
int sof_dsm_inf_fb_copy(struct comp_dev *dev, uint32_t frames,
			struct comp_buffer *source,
			struct comp_buffer *sink, int8_t *chan_map,
			struct sof_dsm_struct_t *hsof_dsm);
/* DSM Integration functions */
int sof_dsm_get_memory_size(struct sof_dsm_struct_t *hsof_dsm, struct comp_dev *dev);
int sof_dsm_create(struct sof_dsm_struct_t *hsof_dsm, struct comp_dev *dev);
void sof_dsm_ff_process(struct sof_dsm_struct_t *hsof_dsm, struct comp_dev *dev,
			void *in, void *out, int nsamples, int szsample);
void sof_dsm_fb_process(struct sof_dsm_struct_t *hsof_dsm, struct comp_dev *dev,
			void *in, int nsamples, int szsample);
#endif
