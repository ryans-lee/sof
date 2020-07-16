/* SPDX-License-Identifier: GPL-2.0-or-later
 *  Copyright (c) 2020 Maxim Integrated
 */

#ifndef __SOF_AUDIO_DSM_H__
#define __SOF_AUDIO_DSM_H__

#include <sof/platform.h>
#include <sof/audio/component.h>

//#define DSM_BYPASS
#define USE_DSM_LIB
//#define DEBUG_ROUTE_IV_TO_OUTPUT

#define SZ_PROC_BUF 240
#define SZ_BUFFER (SZ_PROC_BUF * 2)
#define SZ_IV_BUFFER (SZ_PROC_BUF * 4)

#ifdef DEBUG_ROUTE_IV_TO_OUTPUT
#define IV_BACKUP_BUF_SZ	SZ_PROC_BUF * 5
#define LEFT_V_TO_RIGHT	1
#define LEFT_I_TO_RIGHT		2
#define RIGHT_V_TO_LEFT	3
#define RIGHT_I_TO_LEFT		4
#define ROUTE_TYPE	LEFT_V_TO_RIGHT
#endif

struct sof_dsm_struct_t {
	short buf_ff[SZ_BUFFER];
	short buf_ff_out[SZ_BUFFER];
	short stage[SZ_BUFFER];
	
	int buf_ff32[SZ_BUFFER];
	int buf_ff_out32[SZ_BUFFER];
	int stage32[SZ_BUFFER];

	short buf_fb[SZ_IV_BUFFER];
	int buf_fb32[SZ_IV_BUFFER];

	int ff_avail;
	int ff_rdy;
	int fb_avail;

	bool init;
	/* Debug purpose */
	int seq_ff;
	int seq_fb;
	bool toggle;
	int seq;
	bool tone_gen_toggle0;
	bool tone_gen_toggle1;
	int tone_gen_seq0;
	int tone_gen_seq1;
	#ifdef DEBUG_ROUTE_IV_TO_OUTPUT
	int iv_wptr;
	int iv_rptr;
	short iv_backup[IV_BACKUP_BUF_SZ];
	#endif

	short iData[SZ_BUFFER];
	short vData[SZ_BUFFER];
};

void sof_dsm_onoff(struct comp_dev *dev, int on);
void sof_dsm_create(struct sof_dsm_struct_t *dsmHandle,
	int ch_id, struct comp_dev *dev);
void sof_dsm_destroy(struct sof_dsm_struct_t *dsmHandle,
	struct comp_dev *dev);
void dsm_ff_process(struct sof_dsm_struct_t *dsmHandle,
	short *in, int nSamples, int szSamples, struct comp_dev *dev);
void dsm_fb_process(struct sof_dsm_struct_t *dsmHandle,
	short *in, int nSamples, int szSamples, struct comp_dev *dev);
void sof_dsm_ff_process_32(struct sof_dsm_struct_t *sofDsmHandle,
	void *in, void *out, int nSamples, int szSample, struct comp_dev *dev);
void sof_dsm_ff_process(struct sof_dsm_struct_t *dsmHandle,
	void *in, void *out, int nSamples, int szSample, struct comp_dev *dev);
void sof_dsm_fb_process_32(struct sof_dsm_struct_t *sofDsmHandle, void *in,
	int nSamples, int szSample, struct comp_dev *dev);
void sof_dsm_fb_process(struct sof_dsm_struct_t *sofDsmHandle, void *in,
	int nSamples, int szSample, struct comp_dev *dev);
/* DSM Library Parameters */
#define MAX_CHANNELS 2

#endif

