// SPDX-License-Identifier: BSD-3-Clause
//
// Copyright(c) 2020 Intel Corporation. All rights reserved.
//
// Author: Bartosz Kokoszko <bartoszx.kokoszko@linux.intel.com>

#include <sof/audio/smart_amp.h>
#include <sof/audio/component.h>
#include <sof/trace/trace.h>
#include <sof/drivers/ipc.h>
#include <sof/ut.h>
#ifdef CONFIG_MAXIM_DSM
#include <sof/audio/sof_dsm.h>
#include "dsm_api.h"
#include "dsm_api_types.h"
#endif
static const struct comp_driver comp_smart_amp;

#ifdef CONFIG_MAXIM_DSM
#define DSM_PROC_SZ	48 * 2
#define DSM_IV_PROC_SZ	48 * 4
static bool dsm_init_done;
static struct sof_dsm_struct_t sofDsmHandle;
static int test_seq;
static int test_toggle;
#endif

/* 167a961e-8ae4-11ea-89f1-000c29ce1635 */
DECLARE_SOF_UUID("smart_amp-test", smart_amp_comp_uuid, 0x167a961e, 0x8ae4,
		 0x11ea, 0x89, 0xf1, 0x00, 0x0c, 0x29, 0xce, 0x16, 0x35);

DECLARE_TR_CTX(smart_amp_comp_tr, SOF_UUID(smart_amp_comp_uuid),
	       LOG_LEVEL_INFO);

#define INITIAL_MODEL_DATA_SIZE	64

struct smart_amp_data {
	struct sof_smart_amp_config config;
	struct smart_amp_model_data model;

	struct comp_buffer *source_buf; /**< stream source buffer */
	struct comp_buffer *feedback_buf; /**< feedback source buffer */
	struct comp_buffer *sink_buf; /**< sink buffer */

	uint32_t in_channels;
	uint32_t out_channels;
#ifdef CONFIG_MAXIM_DSM
	int32_t *dsm_in;
	int32_t *dsm_iv;
	int32_t *dsm_test;
#endif
};

static void free_mem_load(struct smart_amp_data *sad)
{
	if (!sad) {
		comp_cl_err(&comp_smart_amp, "free_mem_load(): invalid sad");
		return;
	}

	if (sad->model.data) {
		rfree(sad->model.data);
		sad->model.data = NULL;
		sad->model.data_size = 0;
		sad->model.crc = 0;
		sad->model.data_pos = 0;
	}
}

static int alloc_mem_load(struct smart_amp_data *sad, uint32_t size)
{
	if (!size)
		return 0;

	if (!sad) {
		comp_cl_err(&comp_smart_amp, "alloc_mem_load(): invalid sad");
		return -EINVAL;
	}

	free_mem_load(sad);

	sad->model.data = rballoc(0, SOF_MEM_CAPS_RAM, size);

	if (!sad->model.data) {
		comp_cl_err(&comp_smart_amp, "alloc_mem_load() alloc failed");
		return -ENOMEM;
	}

	bzero(sad->model.data, size);
	sad->model.data_size = size;
	sad->model.data_pos = 0;

	return 0;
}

static struct comp_dev *smart_amp_new(const struct comp_driver *drv,
				      struct sof_ipc_comp *comp)
{
	struct comp_dev *dev;
	struct sof_ipc_comp_process *sa;
	struct sof_ipc_comp_process *ipc_sa =
		(struct sof_ipc_comp_process *)comp;
	struct smart_amp_data *sad;
	struct sof_smart_amp_config *cfg;
	size_t bs;
	int ret;

	comp_cl_info(&comp_smart_amp, "smart_amp_new()");

	dev = comp_alloc(drv, COMP_SIZE(struct sof_ipc_comp_process));
	if (!dev)
		return NULL;
	dev->drv = drv;

	sa = (struct sof_ipc_comp_process *)&dev->comp;

	assert(!memcpy_s(sa, sizeof(*sa), ipc_sa,
	       sizeof(struct sof_ipc_comp_process)));

	sad = rzalloc(SOF_MEM_ZONE_RUNTIME, 0, SOF_MEM_CAPS_RAM, sizeof(*sad));

	if (!sad) {
		rfree(dev);
		return NULL;
	}

	comp_set_drvdata(dev, sad);

	ret = alloc_mem_load(sad, INITIAL_MODEL_DATA_SIZE);
	if (ret < 0) {
		comp_err(dev, "smart_amp_new(): model data initial failed");
		goto fail;
	}

	cfg = (struct sof_smart_amp_config *)ipc_sa->data;
	bs = ipc_sa->size;

	if ((bs > 0) && (bs < sizeof(struct sof_smart_amp_config))) {
		comp_err(dev, "smart_amp_new(): failed to apply config");

		if (sad)
			rfree(sad);
		rfree(sad);
		return NULL;
	}

	memcpy_s(&sad->config, sizeof(struct sof_smart_amp_config), cfg, bs);

#ifdef CONFIG_MAXIM_DSM
	sad->dsm_in =rzalloc(SOF_MEM_ZONE_RUNTIME, 0,
		SOF_MEM_CAPS_RAM, sizeof(int32_t) * DSM_PROC_SZ);
	if (!sad->dsm_in) {
		rfree(dev);
		return NULL;
	}

	sad->dsm_iv =rzalloc(SOF_MEM_ZONE_RUNTIME, 0,
		SOF_MEM_CAPS_RAM, sizeof(int32_t) * DSM_IV_PROC_SZ);
	if (!sad->dsm_iv) {
		rfree(dev);
		return NULL;
	}

	sad->dsm_test =rzalloc(SOF_MEM_ZONE_RUNTIME, 0,
		SOF_MEM_CAPS_RAM, sizeof(int32_t) * DSM_PROC_SZ);
	if (!sad->dsm_test) {
		rfree(dev);
		return NULL;
	}

	if (!dsm_init_done) {
		comp_info(dev, "[RYAN] smart_amp_new +++ init:%d",
			dsm_init_done);
		memset(&sofDsmHandle, 0, sizeof(struct sof_dsm_struct_t));
		test_seq = 0;
		sof_dsm_create(&sofDsmHandle, 0, dev);
		//sof_dsm_create(&sofDsmHandle1, 1, dev);
		dsm_init_done = 1;
		comp_info(dev, "[RYAN] smart_amp_new --- init:%d",
			dsm_init_done);
	}
#endif

	dev->state = COMP_STATE_READY;

	return dev;

fail:
	if (sad)
		rfree(sad);
	rfree(dev);
	return NULL;
}

static int smart_amp_set_config(struct comp_dev *dev,
				struct sof_ipc_ctrl_data *cdata)
{
	struct smart_amp_data *sad = comp_get_drvdata(dev);
	struct sof_smart_amp_config *cfg;
	size_t bs;

	comp_info(dev, "smart_amp_set_config()");

	/* Copy new config, find size from header */
	cfg = (struct sof_smart_amp_config *)cdata->data->data;
	bs = cfg->size;

	comp_info(dev, "smart_amp_set_config(), blob size = %u", bs);

	if (bs != sizeof(struct sof_smart_amp_config)) {
		comp_err(dev, "smart_amp_set_config(): invalid blob size");
		return -EINVAL;
	}

	memcpy_s(&sad->config, sizeof(struct sof_smart_amp_config),
		cfg, sizeof(struct sof_smart_amp_config));

	return 0;
}

static int smart_amp_get_config(struct comp_dev *dev,
				struct sof_ipc_ctrl_data *cdata, int size)
{
	struct smart_amp_data *sad = comp_get_drvdata(dev);
	size_t bs;
	int ret = 0;

	comp_info(dev, "smart_amp_get_config()");

	// Copy back to user space 
	bs = sad->config.size;
	comp_info(dev, "smart_amp_get_config(): block size: %u", bs);

	if (bs == 0 || bs > size)
		return -EINVAL;

	ret = memcpy_s(cdata->data->data, size, &sad->config, bs);
	assert(!ret);

	cdata->data->abi = SOF_ABI_VERSION;
	cdata->data->size = bs;

	return ret;
}

static int smart_amp_get_model(struct comp_dev *dev,
			       struct sof_ipc_ctrl_data *cdata, int size)
{
	struct smart_amp_data *sad = comp_get_drvdata(dev);
	size_t bs;
	int ret = 0;

	comp_dbg(dev, "smart_amp_get_model() msg_index = %d, num_elems = %d, remaining = %d ",
		 cdata->msg_index, cdata->num_elems,
		 cdata->elems_remaining);

	/* Copy back to user space */
	if (sad->model.data) {
		if (!cdata->msg_index) {
			/* reset copy offset */
			sad->model.data_pos = 0;
			comp_info(dev, "smart_amp_get_model() model data_size = 0x%x",
				  sad->model.data_size);
		}

		bs = cdata->num_elems;
		if (bs > size) {
			comp_err(dev, "smart_amp_get_model(): invalid size %d",
				 bs);
			return -EINVAL;
		}

		ret = memcpy_s(cdata->data->data, size,
			       (char *)sad->model.data + sad->model.data_pos,
			       bs);
		assert(!ret);

		cdata->data->abi = SOF_ABI_VERSION;
		cdata->data->size = sad->model.data_size;
		sad->model.data_pos += bs;

	} else {
		comp_err(dev, "smart_amp_get_model(): !sad->model.data");
		ret = -EINVAL;
	}

	return ret;
}

static int smart_amp_ctrl_get_bin_data(struct comp_dev *dev,
				       struct sof_ipc_ctrl_data *cdata,
				       int size)
{
	int ret = 0;

	switch (cdata->data->type) {
	case SOF_SMART_AMP_CONFIG:
		ret = smart_amp_get_config(dev, cdata, size);
		break;
	case SOF_SMART_AMP_MODEL:
		ret = smart_amp_get_model(dev, cdata, size);
		break;
	default:
		comp_err(dev, "smart_amp_ctrl_get_bin_data(): unknown binary data type");
		break;
	}

	return ret;
}

static int smart_amp_ctrl_get_data(struct comp_dev *dev,
				   struct sof_ipc_ctrl_data *cdata, int size)
{
	int ret = 0;

	comp_info(dev, "smart_amp_ctrl_get_data() size: %d", size);

	switch (cdata->cmd) {
	case SOF_CTRL_CMD_BINARY:
		ret = smart_amp_ctrl_get_bin_data(dev, cdata, size);
		break;
	default:
		comp_err(dev, "smart_amp_ctrl_get_data(): invalid cdata->cmd");
		return -EINVAL;
	}

	return ret;
}

static int smart_amp_set_model(struct comp_dev *dev,
			       struct sof_ipc_ctrl_data *cdata)
{
	struct smart_amp_data *sad = comp_get_drvdata(dev);
	bool done = false;
	int ret = 0;

	comp_dbg(dev, "smart_amp_set_model() msg_index = %d, num_elems = %d, remaining = %d ",
		 cdata->msg_index, cdata->num_elems,
		 cdata->elems_remaining);

	if (!cdata->msg_index) {
		ret = alloc_mem_load(sad, cdata->data->size);
		if (ret < 0)
			return ret;
	}

	if (!sad->model.data) {
		comp_err(dev, "smart_amp_set_model(): buffer not allocated");
		return -EINVAL;
	}

	if (!cdata->elems_remaining) {
		if (cdata->num_elems + sad->model.data_pos <
		    sad->model.data_size) {
			comp_err(dev, "smart_amp_set_model(): not enough data to fill the buffer");
			// TODO: handle this situation

			return -EINVAL;
		}

		done = true;
		comp_info(dev, "smart_amp_set_model() final packet received");
	}

	if (cdata->num_elems >
	    sad->model.data_size - sad->model.data_pos) {
		comp_err(dev, "smart_amp_set_model(): too much data");
		return -EINVAL;
	}

	ret = memcpy_s((char *)sad->model.data + sad->model.data_pos,
		       sad->model.data_size - sad->model.data_pos,
		       cdata->data->data, cdata->num_elems);
	assert(!ret);

	sad->model.data_pos += cdata->num_elems;

	if (done) {
		/* Set model data done, update crc value */
		sad->model.crc = crc32(0, sad->model.data,
				      sad->model.data_size);
		comp_info(dev, "smart_amp_set_model() done, memory_size = 0x%x, crc = 0x%08x",
			  sad->model.data_size, sad->model.crc);
	}

	return 0;
}

static int smart_amp_ctrl_set_bin_data(struct comp_dev *dev,
				       struct sof_ipc_ctrl_data *cdata)
{
	int ret = 0;

	if (dev->state != COMP_STATE_READY) {
		/* It is a valid request but currently this is not
		 * supported during playback/capture. The driver will
		 * re-send data in next resume when idle and the new
		 * configuration will be used when playback/capture
		 * starts.
		 */
		comp_err(dev, "smart_amp_ctrl_set_bin_data(): driver is busy");
		return -EBUSY;
	}

	switch (cdata->data->type) {
	case SOF_SMART_AMP_CONFIG:
		ret = smart_amp_set_config(dev, cdata);
		break;
	case SOF_SMART_AMP_MODEL:
		ret = smart_amp_set_model(dev, cdata);
		break;
	default:
		comp_err(dev, "smart_amp_ctrl_set_bin_data(): unknown binary data type");
		break;
	}

	return ret;
}

static int smart_amp_ctrl_set_data(struct comp_dev *dev,
				   struct sof_ipc_ctrl_data *cdata)
{
	int ret = 0;

	/* Check version from ABI header */
	if (SOF_ABI_VERSION_INCOMPATIBLE(SOF_ABI_VERSION, cdata->data->abi)) {
		comp_err(dev, "smart_amp_ctrl_set_data(): invalid version");
		return -EINVAL;
	}

	switch (cdata->cmd) {
	case SOF_CTRL_CMD_ENUM:
		comp_info(dev, "smart_amp_ctrl_set_data(), SOF_CTRL_CMD_ENUM");
		break;
	case SOF_CTRL_CMD_BINARY:
		comp_info(dev, "smart_amp_ctrl_set_data(), SOF_CTRL_CMD_BINARY");
		ret = smart_amp_ctrl_set_bin_data(dev, cdata);
		break;
	default:
		comp_err(dev, "smart_amp_ctrl_set_data(): invalid cdata->cmd");
		ret = -EINVAL;
		break;
	}

	return ret;
}

/* used to pass standard and bespoke commands (with data) to component */
static int smart_amp_cmd(struct comp_dev *dev, int cmd, void *data,
			 int max_data_size)
{
	struct sof_ipc_ctrl_data *cdata = data;

	comp_info(dev, "smart_amp_cmd()");

	comp_info(dev, "smart_amp_cmd(): cmd: %d", cmd);

	switch (cmd) {
	case COMP_CMD_SET_DATA:
		return smart_amp_ctrl_set_data(dev, cdata);
	case COMP_CMD_GET_DATA:
		return smart_amp_ctrl_get_data(dev, cdata, max_data_size);
	default:
		return -EINVAL;
	}
}

static void smart_amp_free(struct comp_dev *dev)
{
	struct smart_amp_data *sad = comp_get_drvdata(dev);

	comp_info(dev, "smart_amp_free()");

#ifdef CONFIG_MAXIM_DSM
	comp_info(dev, "[RYAN] smart_amp_free init:%d", dsm_init_done);
#endif

	free_mem_load(sad);
	rfree(sad);
	rfree(dev);
}

static int smart_amp_verify_params(struct comp_dev *dev,
				   struct sof_ipc_stream_params *params)
{
	int ret;

	comp_info(dev, "smart_amp_verify_params()");

	ret = comp_verify_params(dev, BUFF_PARAMS_CHANNELS, params);
	if (ret < 0) {
		comp_err(dev, "volume_verify_params() error: comp_verify_params() failed.");
		return ret;
	}

	return 0;
}

static int smart_amp_params(struct comp_dev *dev,
			    struct sof_ipc_stream_params *params)
{
	int err;

	comp_info(dev, "smart_amp_params()");

	err = smart_amp_verify_params(dev, params);
	if (err < 0) {
		comp_err(dev, "smart_amp_params(): pcm params verification failed.");
		return -EINVAL;
	}

	return 0;
}

static int smart_amp_trigger(struct comp_dev *dev, int cmd)
{
	struct smart_amp_data *sad = comp_get_drvdata(dev);
	int ret = 0;

	comp_info(dev, "smart_amp_trigger(), command = %u", cmd);

	ret = comp_set_state(dev, cmd);

	if (ret == COMP_STATUS_STATE_ALREADY_SET)
		ret = PPL_STATUS_PATH_STOP;

	switch (cmd) {
	case COMP_TRIGGER_START:
	case COMP_TRIGGER_RELEASE:
		buffer_zero(sad->feedback_buf);
		break;
	case COMP_TRIGGER_PAUSE:
	case COMP_TRIGGER_STOP:
		break;
	default:
		break;
	}

	return ret;
}

static int smart_amp_process_s16(struct comp_dev *dev,
				 const struct audio_stream *source,
				 const struct audio_stream *sink,
				 uint32_t frames, int8_t *chan_map)
{
	struct smart_amp_data *sad = comp_get_drvdata(dev);
	int16_t *src;
	int16_t *dest;
	uint32_t in_buff_frag = 0;
	uint32_t out_buff_frag = 0;
	int i;
	int j;

	for (i = 0; i < frames; i++) {
		for (j = 0 ; j < sad->out_channels; j++) {
			if (chan_map[j] != -1) {
				src = audio_stream_read_frag_s16(source,
					in_buff_frag + chan_map[j]);
				dest = audio_stream_write_frag_s16(sink,
					out_buff_frag);
				*dest = *src;
			}
			out_buff_frag++;
		}
		in_buff_frag += source->channels;
	}
	return 0;
}

static int smart_amp_process_s32(struct comp_dev *dev,
				 const struct audio_stream *source,
				 const struct audio_stream *sink,
				 uint32_t frames, int8_t *chan_map)
{
	struct smart_amp_data *sad = comp_get_drvdata(dev);
	int32_t *src;
	int32_t *dest;
	uint32_t in_buff_frag = 0;
	uint32_t out_buff_frag = 0;
	int i;
	int j;

	comp_info(dev, "smart_amp_process_s32()");

	for (i = 0; i < frames; i++) {
		for (j = 0 ; j < sad->out_channels; j++) {
			if (chan_map[j] != -1) {
				src = audio_stream_read_frag_s32(source,
					in_buff_frag + chan_map[j]);
				dest = audio_stream_write_frag_s32(sink,
					out_buff_frag);
				*dest = *src;
			}
			out_buff_frag++;
		}
		in_buff_frag += source->channels;
	}
	return 0;
}

static int smart_amp_process(struct comp_dev *dev, uint32_t frames,
			     struct comp_buffer *source,
			     struct comp_buffer *sink, int8_t *chan_map)
{
	int ret = 0;

	switch (source->stream.frame_fmt) {
	case SOF_IPC_FRAME_S16_LE:
		ret = smart_amp_process_s16(dev, &source->stream, &sink->stream,
					    frames, chan_map);
		break;
	case SOF_IPC_FRAME_S24_4LE:
	case SOF_IPC_FRAME_S32_LE:
		ret = smart_amp_process_s32(dev, &source->stream, &sink->stream,
					    frames, chan_map);

		break;
	default:
		comp_err(dev, "smart_amp_process() error: not supported frame format");
		return -EINVAL;
	}

	return ret;
}

static void *wrap_buffer_pointer(void *ptr, const struct audio_stream *buffer) {
	if (ptr >= buffer->end_addr)
		ptr = (char *)buffer->addr + ((char *)ptr - (char *)buffer->end_addr);
	return ptr;
}

//static int test_dsm_onoff;
static int smart_amp_copy(struct comp_dev *dev)
{
	struct smart_amp_data *sad = comp_get_drvdata(dev);
	uint32_t avail_passthrough_frames;
	uint32_t avail_feedback_frames;
	uint32_t avail_frames;
	uint32_t source_bytes;
	uint32_t sink_bytes;
	uint32_t feedback_bytes;
	uint32_t source_flags;
	uint32_t sink_flags;
	uint32_t feedback_flags;
	int ret = 0;

	comp_dbg(dev, "smart_amp_copy()");

	#ifdef CONFIG_MAXIM_DSM
	int x;
	if (test_seq % 200 == 0 || test_seq % 200 == 1
		|| test_seq % 200 == 2 || test_seq < 10)
		comp_info(dev,
			"[RYAN] smart_amp_copy() +++ seq:%d", test_seq);
	#endif

	#ifdef CONFIG_MAXIM_DSM
	if (!dsm_init_done) {
		comp_info(dev, "[RYAN] DSM not initialized. forced init here. num_channels: %d",
			sad->out_channels);
		memset(&sofDsmHandle, 0, sizeof(struct sof_dsm_struct_t));
		test_seq = 0;
		sof_dsm_create(&sofDsmHandle, 0, dev);
		dsm_init_done = 1;
	}
	#endif

	buffer_lock(sad->source_buf, &source_flags);
	buffer_lock(sad->sink_buf, &sink_flags);

	/* available bytes and samples calculation */
	avail_passthrough_frames = audio_stream_avail_frames(
		&sad->source_buf->stream,
		&sad->sink_buf->stream);

	buffer_unlock(sad->source_buf, source_flags);
	buffer_unlock(sad->sink_buf, sink_flags);

	avail_frames = avail_passthrough_frames;

	comp_dbg(dev, "smart_amp_copy(): avail_passthrough_frames: %d",
		avail_passthrough_frames);

	buffer_lock(sad->feedback_buf, &feedback_flags);
	if (sad->feedback_buf->source->state == dev->state) {
		/* feedback */
		avail_feedback_frames = sad->feedback_buf->stream.avail /
			audio_stream_frame_bytes(&sad->feedback_buf->stream);

		avail_frames = MIN(avail_passthrough_frames,
			avail_feedback_frames);

		feedback_bytes = avail_frames *
			audio_stream_frame_bytes(&sad->feedback_buf->stream);

		buffer_unlock(sad->feedback_buf, feedback_flags);

		comp_dbg(dev, "smart_amp_copy(): processing %d feedback bytes",
			feedback_bytes);

		if ( avail_frames != 48 )	{	// Just keep extra caution to avoid wrong memory access.
			comp_info(dev,
				"[RYAN] smart_amp_copy() size warning! size:%d", avail_frames);
			/* do nothing */
		} else {
			int32_t *iv = (int32_t*) sad->feedback_buf->stream.r_ptr;
			if (test_seq % 200 == 0 || test_seq % 200 == 1
				|| test_seq % 200 == 2 || test_seq < 10) {
				comp_info(dev, "[RYAN] smart_amp_copy() FB avail:%d",
						avail_frames);				
			}
			if (0) {
				for (x = 0 ; x < avail_frames ; x++)            {
					/* Copying input CH0 */
					iv = (int32_t *)wrap_buffer_pointer(iv, &sad->feedback_buf->stream);
					sad->dsm_iv[4 * x + 2] = *iv;	// right V
					iv++;
					/* Copying input CH1 */
					iv = (int32_t *)wrap_buffer_pointer(iv, &sad->feedback_buf->stream);
					sad->dsm_iv[4 * x + 3] = *iv;	// right I
					iv++;
					/* Copying input CH2 */
					iv = (int32_t *)wrap_buffer_pointer(iv, &sad->feedback_buf->stream);
					sad->dsm_iv[4 * x] = *iv;		// left V
					iv++;
					/* Copying input CH3 */
					iv = (int32_t *)wrap_buffer_pointer(iv, &sad->feedback_buf->stream);
					sad->dsm_iv[4 * x + 1] = *iv;	// left I
					iv++;
				}
			}
			if (0) {
			sof_dsm_fb_process_32(&sofDsmHandle, sad->dsm_iv,
				avail_frames * 4, sizeof(int32_t), dev);
			if (0)
				smart_amp_process(dev, avail_frames,
					sad->feedback_buf, sad->sink_buf,
					sad->config.feedback_ch_map);
			}
		}
		comp_update_buffer_consume(sad->feedback_buf, feedback_bytes);
	}

	/* bytes calculation */
	buffer_lock(sad->source_buf, &source_flags);
	source_bytes = avail_frames *
		audio_stream_frame_bytes(&sad->source_buf->stream);
	buffer_unlock(sad->source_buf, source_flags);
	
	buffer_lock(sad->sink_buf, &sink_flags);
	sink_bytes = avail_frames *
		audio_stream_frame_bytes(&sad->sink_buf->stream);
	buffer_unlock(sad->sink_buf, sink_flags);

	/* process data */
	#ifdef CONFIG_MAXIM_DSM
	struct comp_buffer *source = sad->source_buf;
	struct comp_buffer *sink = sad->sink_buf;

	if (test_seq % 200 == 0 || test_seq % 200 == 1
		|| test_seq % 200 == 2 || test_seq < 10) {
		comp_info(dev, "[RYAN] smart_amp_copy() avail:%d, source_bytes:%d, sink_bytes:%d, test_toggle:%d",
			avail_frames, source_bytes, sink_bytes,
			test_toggle);

		comp_info(dev, "[RYAN] smart_amp_copy() w_ptr:%p, r_ptr:%p, addr:%p, end_addr:%p",
			(uintptr_t) source->stream.w_ptr,
			(uintptr_t) source->stream.r_ptr,
			(uintptr_t) source->stream.addr,
			(uintptr_t) source->stream.end_addr);

		comp_info(dev, "[RYAN] smart_amp_copy() sink w_ptr:%p, r_ptr:%p, addr:%p, end_addr:%p",
			(uintptr_t) sink->stream.w_ptr,
			(uintptr_t) sink->stream.r_ptr,
			(uintptr_t) sink->stream.addr,
			(uintptr_t) sink->stream.end_addr);
		comp_info(dev, "[RYAN] smart_amp_copy() source source->stream.frame_fmt :%d, avail:%d, free:%d, size:%d",
			source->stream.frame_fmt,
			source->stream.avail,
			source->stream.free,
			source->stream.size);
		comp_info(dev, "[RYAN] smart_amp_copy() sink source->stream.frame_fmt :%d, avail:%d, free:%d, size:%d",
			sink->stream.frame_fmt,
			sink->stream.avail,
			sink->stream.free,
			sink->stream.size);
	}
	#endif

//	smart_amp_process(dev, avail_frames, sad->source_buf, sad->sink_buf,
//			  sad->config.source_ch_map);

	#ifdef CONFIG_MAXIM_DSM
	int32_t *input = (int32_t*) sad->source_buf->stream.r_ptr;
	int32_t *output = (int32_t*) sad->sink_buf->stream.w_ptr;

	if ( avail_frames != 48 )	{	// Just keep extra caution to avoid wrong memory access.
		comp_info(dev,
			"[RYAN] smart_amp_copy() size warning! size:%d", avail_frames);
		/* do nothing */
	} else {	
		for (x = 0 ; x < avail_frames ; x++)            {
			/* Copying input left */
			input = (int32_t *)wrap_buffer_pointer(input, &sad->source_buf->stream);
			sad->dsm_in[2 * x] = *input;
			sad->dsm_test[2 * x] = sad->dsm_in[2 * x];
			input++;

			/* Copying input right */
			input = (int32_t *)wrap_buffer_pointer(input, &sad->source_buf->stream);
			sad->dsm_in[2 * x + 1] = *input;
			sad->dsm_test[2 * x + 1] = sad->dsm_in[2 * x + 1];
			input++;
		}
		#if 1
		#ifndef DSM_BYPASS
		#if 0
		if (test_seq == 0)	{	// In every 2 seconds,
			if (test_dsm_onoff == 0) {
				test_dsm_onoff = 1;
				sof_dsm_onoff(dev, 1);
				comp_info(dev, "[RYAN] MODE : DSM ON test_dsm_onoff:%d, seq:%d",
					test_dsm_onoff,
					sofDsmHandle.seq);
				// on
			} else if (test_dsm_onoff == 1) {
				test_dsm_onoff = 2;
				sof_dsm_onoff(dev, 0);
				comp_info(dev, "[RYAN] MODE : DSM OFF test_dsm_onoff:%d, seq:%d",
					test_dsm_onoff,
					sofDsmHandle.seq);
				// off
			} else {
				// memcpy bypass
				test_dsm_onoff = 0;
				comp_info(dev, "[RYAN] MODE : NO DSM MEMCPY ONLY test_dsm_onoff:%d, seq:%d",
					test_dsm_onoff,
					sofDsmHandle.seq);
			}
		}
		#endif
		sof_dsm_ff_process_32(&sofDsmHandle, sad->dsm_in, sad->dsm_in,
			avail_frames * 2, sizeof(int32_t), dev);
		for (x = 0 ; x < avail_frames ; x++)            {
			sad->dsm_in[2 * x + 1] = sad->dsm_test[2 * x];
		}
		#if 0
		for (x = 0 ; x < avail_frames ; x++)            {
			if (test_dsm_onoff == 1)	// on
				sad->dsm_in[2 * x + 1] = sad->dsm_test[2 * x];
			else if (test_dsm_onoff == 2)	// off
				sad->dsm_in[2 * x + 1] = (int32_t) (sad->dsm_test[2 * x] * 0.5);
			else	{	// bypass
				sad->dsm_in[2 * x] = sad->dsm_test[2 * x];
				sad->dsm_in[2 * x + 1] = sad->dsm_test[2 * x + 1];
			}
			// Copy DSM left input to the right output.
		}
		#endif

		#endif
		#else
		if (test_toggle)	{
			comp_info(dev, "[RYAN] MODE : RIGHT to LEFT, REMOVE RIGHT (%d)", sofDsmHandle.seq);
			for (x = 0 ; x < avail_frames ; x++)            {
				sad->dsm_in[2 * x] = sad->dsm_in[2 * x + 1];
				sad->dsm_in[2 * x + 1] = 0;
			}
		} else
			comp_info(dev, "[RYAN] MODE : BYPASS (%d, %d)", test_seq, test_dsm_onoff);
		#endif

		for (x = 0 ; x < avail_frames ; x++)            {
			/* Proceed CH0 */
			output = (int32_t *)wrap_buffer_pointer(output, &sad->sink_buf->stream);
			*output = sad->dsm_in[2 * x];
			output++;
			/* Proceed CH1 */
			output = (int32_t *)wrap_buffer_pointer(output, &sad->sink_buf->stream);
			*output = sad->dsm_in[2 * x + 1];
			output++;
			/* Proceed CH2 */		
			output = (int32_t *)wrap_buffer_pointer(output, &sad->sink_buf->stream);
			*output = 0;
			output++;
			/* Proceed CH3 */			
			output = (int32_t *)wrap_buffer_pointer(output, &sad->sink_buf->stream);
			*output = 0;
			output++;
		}

	}
	#endif

	/* source/sink buffer pointers update */
	comp_update_buffer_consume(sad->source_buf, source_bytes);
	comp_update_buffer_produce(sad->sink_buf, sink_bytes);

	#ifdef CONFIG_MAXIM_DSM
	if (test_seq % 200 == 0 || test_seq % 200 == 1
		|| test_seq % 200 == 2 || test_seq < 10)
		comp_info(dev,
			"[RYAN] smart_amp_copy() --- seq:%d", test_seq);
	test_seq++;
	if (test_seq >= 2000)	{
		test_seq = 0;
		if (test_toggle)
			test_toggle = 0;
		else
			test_toggle = 1;
	}
	#endif
	return ret;
}

static int smart_amp_reset(struct comp_dev *dev)
{
	comp_info(dev, "smart_amp_reset()");

	comp_set_state(dev, COMP_TRIGGER_RESET);

	return 0;
}

static int smart_amp_prepare(struct comp_dev *dev)
{
	struct sof_ipc_comp_process *ipc_sa =
		(struct sof_ipc_comp_process *)&dev->comp;
	struct smart_amp_data *sad = comp_get_drvdata(dev);
	struct comp_buffer *source_buffer;
	struct list_item *blist;
	int ret;

	(void)ipc_sa;

	comp_info(dev, "smart_amp_prepare()");

	ret = comp_set_state(dev, COMP_TRIGGER_PREPARE);
	if (ret < 0)
		return ret;

	if (ret == COMP_STATUS_STATE_ALREADY_SET)
		return PPL_STATUS_PATH_STOP;

	/* searching for stream and feedback source buffers */
	list_for_item(blist, &dev->bsource_list) {
		source_buffer = container_of(blist, struct comp_buffer,
					     sink_list);

		if (source_buffer->source->comp.type == SOF_COMP_DEMUX)
			sad->feedback_buf = source_buffer;
		else
			sad->source_buf = source_buffer;
	}

	sad->sink_buf = list_first_item(&dev->bsink_list, struct comp_buffer,
					source_list);

	sad->in_channels = sad->source_buf->stream.channels;
	sad->out_channels = sad->sink_buf->stream.channels;

	sad->feedback_buf->stream.channels = sad->config.feedback_channels;
	sad->feedback_buf->stream.frame_fmt = 2;

	return 0;
}

static const struct comp_driver comp_smart_amp = {
	.type = SOF_COMP_SMART_AMP,
	.uid = SOF_UUID(smart_amp_comp_uuid),
	.tctx = &smart_amp_comp_tr,
	.ops = {
		.create = smart_amp_new,
		.free = smart_amp_free,
		.params = smart_amp_params,
		.prepare = smart_amp_prepare,
		.cmd = smart_amp_cmd,
		.trigger = smart_amp_trigger,
		.copy = smart_amp_copy,
		.reset = smart_amp_reset,
	},
};

static SHARED_DATA struct comp_driver_info comp_smart_amp_info = {
	.drv = &comp_smart_amp,
};

static void sys_comp_smart_amp_init(void)
{
	comp_register(platform_shared_get(&comp_smart_amp_info,
					  sizeof(comp_smart_amp_info)));
}

DECLARE_MODULE(sys_comp_smart_amp_init);
