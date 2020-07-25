/* SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright(c) 2020 Maxim Integrated All rights reserved.
 *
 * Author: Ryan Lee <ryans.lee@maximintegrated.com>
 */
#ifndef _DSM_API_PUBLIC_H_
#define _DSM_API_PUBLIC_H_

#define DSM_SET_MONO_CMD_ID(cmd_id)       (((cmd_id) & 0x00FFFFFF))
#define DSM_SET_STEREO_CMD_ID(cmd_id)     (((cmd_id) & 0x00FFFFFF) | 0x03000000)
#define DSM_SET_LEFT_CMD_ID(cmd_id)       (((cmd_id) & 0x00FFFFFF) | 0x01000000)
#define DSM_SET_RIGHT_CMD_ID(cmd_id)      (((cmd_id) & 0x00FFFFFF) | 0x02000000)

#if (MAX_CHANNELS == 2)
#define DSM_SET_CMD_ID(cmd_id) DSM_SET_STEREO_CMD_ID(cmd_id)
#else
#define DSM_SET_CMD_ID(cmd_id) DSM_SET_MONO_CMD_ID(cmd_id)
#endif

struct dsm_api_memory_size_ext_t {
	int isamplingrate;
	int ichannels;
	int *ipcircbuffersizebytes;
	int omemsizerequestedbytes;
	int numeqfilters;
};

struct dsm_api_init_ext_t {
	int isamplingrate;
	int ichannels;
	int off_framesizesamples;
	int ofb_framesizesamples;
	int *ipcircbuffersizebytes;
	int *ipdelayedsamples;
	int isamplebitwidth;
};

enum DSM_API_MESSAGE {
	DSM_API_OK = 0,
	DSM_API_MSG_NULL_MODULE_HANDLER = 1 << 1,
	DSM_API_MSG_NULL_PARAM_POINTER = 1 << 2,
	DSM_API_MSG_NULL_INPUT_BUFFER_POINTER = 1 << 3,
	DSM_API_MSG_NULL_OUTPUT_BUFFER_POINTER = 1 << 4,
	DSM_API_MSG_INVALID_CMD_ID = 1 << 5,
	DSM_API_MSG_INVALID_PARAM = 1 << 6,
	DSM_API_MSG_INVALID_PARAMS_NUM = 1 << 7,
	DSM_API_MSG_INVALID_SAMPLING_RATE = 1 << 8,
	DSM_API_MSG_NOT_IMPLEMENTED = 1 << 9,
	DSM_API_MSG_INVALID_MEMORY = 1 << 10,
	DSM_API_MSG_ZERO_I = 1 << 11,
	DSM_API_MSG_ZERO_V = 1 << 12,
	DSM_API_MSG_MIN_RDC_BEYOND_THRESHOLD = 1 << 13,
	DSM_API_MSG_MAX_RDC_BEYOND_THRESHOLD = 1 << 14,
	DSM_API_MISMATCHED_SETGET_CMD = 1 << 15,
	DSM_API_MSG_IV_DATA_WARNING = 1 << 16,
	DSM_API_MSG_COIL_TEMPERATURE_WARNING = 1 << 17,
	DSM_API_MSG_EXCURSION_WARNING = 1 << 18,
	DSM_API_MSG_WRONG_COMMAND_TYPE = 1 << 19,
	DSM_API_MSG_COMMAND_OBSOLETE = 1 << 20,
	DSM_API_MSG_INSUFFICIENT_INPUT_DATA = 1 << 21,
	DSM_API_MSG_FF_NOT_START = 1 << 22,
	DSM_API_MSG_INVALID
};

enum DSM_API_MESSAGE dsm_api_get_mem(struct dsm_api_memory_size_ext_t *iopmmemparam,
				     int iparamsize);
enum DSM_API_MESSAGE dsm_api_init(void *ipmodulehandler,
				  struct dsm_api_init_ext_t *iopparamstruct,
				  int iparamsize);
enum DSM_API_MESSAGE dsm_api_ff_process(void *ipmodulehandler,
					int channelmask,
					short *ibufferorg,
					int *ipnrsamples,
					short *obufferorg,
					int *opnrsamples);
enum DSM_API_MESSAGE dsm_api_fb_process(void *ipmodulehandler,
					int ichannelmask,
					short *icurrbuffer,
					short *ivoltbuffer,
					int *iopnrsamples);
enum DSM_API_MESSAGE dsm_api_set_params(void *ipmodulehandler,
					int icommandnumber, void *ipparamsbuffer);
enum DSM_API_MESSAGE dsm_api_get_params(void *ipmodulehandler,
					int icommandnum, void *opparams);

#endif

