/*=============================================================================
 #
 # The content of this file or document is CONFIDENTIAL and PROPRIETARY
 # to Maxim Integrated Products.  It is subject to the terms of a
 # License Agreement between Licensee and Maxim Integrated Products.
 # restricting among other things, the use, reproduction, distribution
 # and transfer.  Each of the embodiments, including this information and
 # any derivative work shall retain this copyright notice.
 #
 #============================================================================*/
/*
 *       DSM module interfaces
 * 
 *   In this file are defined all messages, constants and 
 * functions which are used to interface to DSM module.
 * 
 * ********************************************************/
#ifndef _DSM_API_H_
#define _DSM_API_H_

#include "dsm_api_types.h"


#ifdef __cplusplus
extern "C" {
#endif

//#define ENABLE_DSM_LOG_DATA

/***********************************************************
 * 
 *       DSM module interfaces
 *
 *     In this file defines all messages, constants and
 * functions which are used to interface with DSM module.
 * 
 *     The command consists of 32 bits whose format is
 * described as below:
 * 
 *     +----------------------------+------------------+
 *     |  bit31  bit30  ...  bit24  | bit23  ...   bit0|
 *     +----------------------------+------------------+
 *     |    speaker channel mask    |    commandId     |
 *     +----------------------------+------------------+
 *         Table_1:  format of DSM API command
 *    
 *     +-----------+-----------+-----------+-----------+
 *     |   bit31   |   bit30   |   ......  |   bit24   |
 *     +-----------+-----------+-----------+-----------+
 *     | channel_7 | channel_6 |           | channel_0 |
 *     +-----------+-----------+---- ------+-----------+
 *        Table_2:  format of channel mask
 *     
 *    The bits between bit31 and bit24 are called channel mask bits. 
 * The bit with the value of one indicates that the command is applied
 * for the corresponding channel. 
 * 
 *    For instance, in a stereo speaker system, 0x1 represents left 
 * channel, 0x3 represents left and right channels.
 *    A special case: when the channel mask equals to zero, that means 
 * the command will apply for all channels.
 * 
 *    The lower 24 bits contain the command Id.
 * 
 * ********************************************************/

/*********************************************************************
 *       macro DSM_CMD
 *
 * Description:
 *     The macro DSM_CMD is used to merge channel Id with command Id.
 * Direct use of command Id implies that the command will be sent to
 * the left channel.
 *
 *********************************************************************/

#define SAMPLING_RATE_SUPPORTED			((unsigned int)48000)
#define DSM_MAX_STRING_PARAM_SIZE		32	// in bytes
#define	DSM_CMD(chanId, cmdId)			((chanId)|(cmdId))

#if (MAX_CHANNELS==2 )
#define DSM_SET_CMD_ID(cmdId)            DSM_SET_STEREO_CMD_ID(cmdId)
#else
#define DSM_SET_CMD_ID(cmdId)            DSM_SET_MONO_CMD_ID(cmdId)
#endif

#ifndef TO_FIX
#define TO_FIX(a, q)     		        ((int)((a) * ((unsigned long long)1<<(q))))
#endif

#define TO_FIX64(a, q)                 ((long long)((a) * ((unsigned long long)1<<(q))))

#ifndef TO_DBL
#define TO_DBL(a, q)                    (double)((double)((double)(a)/(double)((unsigned long long)1<<(q))))
#endif


#define DSM_TIME_MAX_SIZE               16 //in bytes
#define DSM_DATE_MAX_SIZE               32 //in bytes
#define DSM_VERSION_MAX_SIZE            64
#define DSM_CHIPSET_MAX_SIZE            16 //in bytes

#ifdef ENABLE_DSM_LOG_DATA

#define  LOG_BUFFER_ARRAY_SIZE     10

typedef struct _dsm_logging_buffer {
    /* Refer "byteLogStruct" for mapping below array */
	unsigned char  byteLogArray[4+(LOG_BUFFER_ARRAY_SIZE*2)];
    /* Refer "intLogStruct" for mapping below array */
	unsigned int   intLogArray[4+(LOG_BUFFER_ARRAY_SIZE*2)];
    /* Refer "byteLogStruct" for mapping below array */
    unsigned char  afterProbByteLogArray[LOG_BUFFER_ARRAY_SIZE*2*2];
    /* Refer "intLogStruct" for mapping below array */
    unsigned int afterProbIntLogArray[LOG_BUFFER_ARRAY_SIZE*2*2];
} dsm_logging_buffer_t;

typedef struct _dsm_logging_data {
    /* 0x1 - excursion overshoot, 0x2 - temp overshoot, 0x3 - both excursion and temp overshoot */
	unsigned char  logAvailable[4];
    /* 10 temperature logs for the last 2 seconds when temperature overshoot has been occurred.
     *  2 bytes per each sample. Each sample represents 200ms. */
	unsigned short  last2SecondTemp[10];   /* 20 bytes */
    /* 10 excursion logs for the last 2 seconds when temperature overshoot has been occurred.
     *  2 bytes per each sample. Each sample represents 200ms. */
    unsigned short  last2SecondExcur[10];  /* 20 bytes */
    /* Increase number when temperature overshoot is occurred */
    unsigned int sequenceNumTemp;  /* 4 bytes */
    /* Increase number when excursion overshoot is occurred. */
    unsigned int sequenceNumExcur;  /* 4 bytes */
    /* 10 Rdc logs for the last 2 seconds when temperature overshoot has been occurred.
     *  4 bytes per each sample. Each sample represents 200ms. */
    unsigned int last2SecondRdc[10];  /* 40 bytes */
    /* 10 Rdc logs for the last 2 seconds when temperature overshoot has been occurred.
     *  4 bytes per each sample. Each sample represents 200ms. */
    unsigned int last2SecondFreq[10];  /* 40 bytes */
    /* 10 temperature logs for 2 seconds after temperature overshoot has been occurred.
     *  2 bytes per each sample. Each sample represents 200ms. */
    unsigned short after2SecondTempForTemp[10];  /* 20 bytes */
    /* 10 excursion logs for 2 seconds after temperature overshoot has been occurred.
     *  2 bytes per each sample. Each sample represents 200ms. */
    unsigned short after2SecondExcurForTemp[10]; /* 20 bytes */
    /* 10 temperature logs for 2 seconds after excursion overshoot has been occurred.
     *  2 bytes per each sample. Each sample represents 200ms. */
    unsigned short after2SecondTempForExcur[10];  /* 20 bytes */
    /* 10 excursion logs for 2 seconds after excursion overshoot has been occurred.
     *  2 bytes per each sample. Each sample represents 200ms. */
    unsigned short after2SecondExcurForExcur[10];  /* 20 bytes */
    /* 10 Rdc logs for 2 seconds after temperature overshoot has been occurred.
     *  4 bytes per each sample. Each sample represents 200ms. */
    unsigned int after2SecondRdcForTemp[10];  /* 40 bytes */
    /* 10 Fc logs for 2 seconds after temperature overshoot has been occurred.
     *  4 bytes per each sample. Each sample represents 200ms. */
    unsigned int after2SecondFreqForTemp[10];  /* 40 bytes */
    /* 10 Rdc logs for 2 seconds after excursion overshoot has been occurred.
     *  4 bytes per each sample. Each sample represents 200ms. */
    unsigned int after2SecondRdcForExcur[10];  /* 40 bytes */
    /* 10 Fc logs for 2 seconds after excursion overshoot has been occurred.
     *  4 bytes per each sample. Each sample represents 200ms. */
    unsigned int after2SecondFreqForExcur[10]; /* 40 bytes */
} dsm_logging_data_t;

#endif //ENABLE_DSM_LOG_DATA

/*
 * IV data from the feedback path could be stored as:
 * DSM_IV_FORMAT_DETERLEAVED_16_BIT: I and V are in two separate 16-bit buffers @48KHz
 * DSM_IV_FORMAT_INTERLEAVED_16_BIT: left channel I and V are interleaved in one 16bit buffer@24KHz
 *                                   right channel I and V are interleaved in another 16-bit buffer@24KHz
 * DSM_IV_FORMAT_8_BIT_VOLTAGE: I in one 16-bit buffer@48KHz, V in another 8-bit buffer@48KHz
 */
typedef enum {
	DSM_IV_FORMAT_DEINTERLEAVED_16_BIT = 0,
    DSM_IV_FORMAT_INTERLEAVED_16_BIT,
    DSM_IV_FORMAT_8_BIT_VOLTAGE
} DSM_IV_FORMAT;

/*******************************************************
 * 
 *      Data type DSM_API_SPKER
 *
 * Description:
 *     This data type DSM_API_SPKER defines the speaker 
 * channel Id.
 * 
 * *****************************************************/

enum DSM_API_SMART_PA_MODE
{
    DSM_API_SMART_PA_DISABLE_MODE       = 0,
    DSM_API_SMART_PA_ENABLE_MODE        = 1<<0,
    DSM_API_SMART_PA_BYPASS_ADD_MODE    = 1<<1,
    DSM_API_SMART_PA_PITONE_ONLY_MODE   = 1<<2,  /* added for Rdc calibration mode */
};
#define DSM_API_DISABLE_MODE                DSM_API_SMART_PA_DISABLE_MODE
#define DSM_API_ENABLE_MODE                 DSM_API_SMART_PA_ENABLE_MODE
#define DSM_API_BYPASS_WITH_PILOT_TONE_MODE DSM_API_SMART_PA_BYPASS_ADD_MODE

typedef enum
{
    DSM_API_FEED_FORWARD_MODULE         = 1<<0,
    DSM_API_FEED_BACKWORD_MODULE        = 1<<1,
} DSM_API_DSM_MODULE_ID;

typedef enum {
    DSM_API_MONO_SPKER                  = 0x00000000,//the mono speaker
    DSM_API_STEREO_SPKER                = 0x03000000,//the stereo speakers

    DSM_API_L_CHAN                      = 0x01000000,//the left channel speaker Id
    DSM_API_R_CHAN                      = 0x02000000,//the left channel speaker Id

    DSM_API_CHANNEL_1                   = 0x01000000,
    DSM_API_CHANNEL_2                   = 0x02000000,
    DSM_API_CHANNEL_3                   = 0x04000000,
    DSM_API_CHANNEL_4                   = 0x08000000,
    DSM_API_CHANNEL_5                   = 0x10000000,
    DSM_API_CHANNEL_6                   = 0x20000000,
    DSM_API_CHANNEL_7                   = 0x40000000,
    DSM_API_CHANNEL_8                   = 0x00000000,

    DSM_MAX_SUPPORTED_CHANNELS          = 8
} DSM_API_CHANNEL_ID;

typedef enum{
    DSM_API_FF_PATH                     = 0,
    DSM_API_FB_PATH                     = 1
} DSM_API_PATH_ID;


typedef enum{
    DSM_API_INPUT_CIRC_BUFFER_ID        = 0,
    DSM_API_OUTPUT_CIRC_BUFFER_ID,
    DSM_API_I_CIRC_BUFFER_ID,
    DSM_API_V_CIRC_BUFFER_ID,
} DSM_API_CIRC_BUFFER_ID;


#define DSM_SET_MONO_CMD_ID(cmdId)       ((cmdId&0x00FFFFFF)|DSM_API_MONO_SPKER)
#define DSM_SET_STEREO_CMD_ID(cmdId)     ((cmdId&0x00FFFFFF)|DSM_API_STEREO_SPKER)
#define DSM_SET_LEFT_CMD_ID(cmdId)       ((cmdId&0x00FFFFFF)|DSM_API_L_CHAN)
#define DSM_SET_RIGHT_CMD_ID(cmdId)      ((cmdId&0x00FFFFFF)|DSM_API_R_CHAN)

typedef enum {
    LANDSCAPE_1_RCV_RIGHT = 0,
    LANDSCAPE_2_RCV_LEFT,
    PORTRAIT_ORIENT
} stereo_spk_orient_t;

/*******************************************************
 * 
 *      Data type DSM_API_EQ_BAND
 *
 * Description:
 *     This data type DSM_API_EQ_BAND defines the Equalizer 
 * band Id.
 * 
 * *****************************************************/

typedef enum {
    DSM_API_EQ_BAND_1                   = 1<<0,
    DSM_API_EQ_BAND_2                   = 1<<1,
    DSM_API_EQ_BAND_3                   = 1<<2,
    DSM_API_EQ_BAND_4                   = 1<<3
} DSM_API_EQ_BAND;

/**************************************************
 *      Data type DSM_API_CMD
 *
 * Description:
 *     The data type of DSM_API_CMD is defined
 * as a 16-bit integer. The highest 4-bits contain
 * the channel Id and lower 12-bits the command Id.
 *
 * Notation of valid floating-point range
 * {Va, Vb, Vc}:  valid value is one of the number in the discrete set.
 * {Va, ..., Vc}: valid value is an integer between Va and Vc, including both Va and Vc.
 * [Vl, Vr]:      valid value is between Vl and Vr, including Vl and Vr.
 * [Vl, Vr):      valid value is equal to or bigger than Vl but less than Vr.
 *
 * ************************************************/

    //The following messages are read/write
#define    DSM_API_GET_MAXIMUM_CMD_ID                  0    //32-bits data, Q0.                  valid floating-point range = [0, 0x7FFFFFFF]
#define    DSM_API_SETGET_ENABLE                       1    //32-bits data, Q0.                  valid floating-point range = {DSM_API_SMART_PA_DISABLE_MODE, DSM_API_SMART_PA_ENABLE_MODE, DSM_API_SMART_PA_BYPASS_ADD_MODE}
#define    DSM_API_SETGET_COILTEMP_THRESHOLD           2    //32-bits data, Q19, in C degrees,   valid floating-point range = [0, 4096)
#define    DSM_API_SETGET_XCL_THRESHOLD                3    //32-bits data, Q27, in millimeters, valid floating-point range = [0, 16)
#define    DSM_API_SETGET_LIMITERS_RELTIME             4    //32-bits data, Q30, in seconds,     valid floating-point range = [0, 2)
#define    DSM_API_SETGET_MAKEUP_GAIN                  5    //32-bits data, Q29, in %,           valid floating-point range = [0, 4)
#define    DSM_API_SETGET_RDC_AT_ROOMTEMP              6    //32-bits data, Q27,                 valid floating-point range = [0, 16)
#define    DSM_API_SETGET_COPPER_CONSTANT              7    //32-bits data, Q30,                 valid floating-point range = [0, 2)
#define    DSM_API_SETGET_COLDTEMP                     8    //32-bits data, Q19,                 valid floating-point range = [0, 4096)
#define    DSM_API_SETGET_PITONE_GAIN                  9    //32-bits data, Q31, in %,           valid floating-point range = [0, 1)
#define    DSM_API_SETGET_LEAD_RESISTANCE              10   //32-bits data, Q27,                 valid floating-point range = [0, 16)
#define    DSM_API_SETGET_HPCUTOFF_FREQ                11   //32-bits data, Q9, in Hz,           valid floating-point range = [0, 4194304)
#define    DSM_API_SETGET_LFX_GAIN                     12   //32-bits data, Q30,                 valid floating-point range = (0, 1]

    //for impedance model updating
#define    DSM_API_SETGET_REF_FC                       13   //32-bits data, Q9, in Hz,           valid floating-point range = [0, 4194304)
#define    DSM_API_SETGET_REF_Q                        14   //32-bits data, Q29,                 valid floating-point range = [0, 4)
#define    DSM_API_INIT_F_Q_FILTERS                    15   //32-bits data, Q0,                  valid floating-point range = {0, 1}

    //The following messages are read only
#define    DSM_API_GET_ADAPTIVE_FC                     16   //32-bits data, Q9, in Hz,           valid floating-point range = [0, 4194304)
#define    DSM_API_GET_ADAPTIVE_Q                      17   //32-bits data, Q29,                 valid floating-point range = [0, 4)
#define    DSM_API_GET_ADAPTIVE_DC_RES                 18   //32-bits data, Q27,                 valid floating-point range = [0, 16)
#define    DSM_API_GET_ADAPTIVE_COILTEMP               19   //32-bits data, Q19,                 valid floating-point range = [0, 4096)
#define    DSM_API_GET_EXCURSION                       20   //32-bits data, Q27,                 valid floating-point range = [0, 16)

    //The following 4 commands are used the buffers used for the following commands must be allocated
    //by the caller function. The maxim buffer size is 4K bytes.
#define    DSM_API_GET_PCM_INPUT_DATA                  21   //the buffer will be filled with 16-bit input PCM data to DSM module.
#define    DSM_API_GET_IV_DATA                         22   //the buffer will be filled with I and V data.
#define    DSM_API_GET_PCM_AND_IV_DATA                 23   //the buffer will be filled with PCM, I and V data. All data are in 16-bit integer.
#define    DSM_API_GET_PCM_OUTPUT_DATA                 24   //the buffer will be filled with 16-bit output PCM data of DSM module.

#define    DSM_API_SETGET_INTERN_DEBUG                 25   //32-bits data, Q0,                  This command is used for internal debugging.

#define    DSM_API_SETGET_VLIMIT                       26   //32-bits data, Q27, in percentage.  valid floating-point range = [0, 16)

#define    DSM_API_SETGET_NEW_CONTROLS                 27   //32-bits data, Q0,                  valid floating-point range = {dsm_control_flag_t}

#define    DSM_API_SETGET_PILOT_ENABLE                 28   //32-bits data, Q0,                  valid floating-point range = {0, 1}
#define    DSM_API_SETGET_CLIP_ENABLE                  29   //32-bits data, Q0,                  valid floating-point range = {0, 1}
#define    DSM_API_SETGET_EXC_ENABLE                   30   //32-bits data, Q0,                  valid floating-point range = {0, 1}
#define    DSM_API_SETGET_THERMAL_ENABLE               31   //32-bits data, Q0,                  valid floating-point range = {0, 1}

#define    DSM_API_SETGET_EQ_BAND_FC                   32   //32-bits data, Q9,  in Hz,          valid floating-point range = [0, 4194304)
#define    DSM_API_SETGET_EQ_BAND_Q                    33   //32-bits data, Q26,                 valid floating-point range = [0, 32)
#define    DSM_API_SETGET_EQ_BAND_ATTENUATION_DB       34   //32-bits data, Q20, in db,          valid floating-point range = [-2048, 2048)
#define    DSM_API_SETGET_TARGET_EQ_BAND_ID            35   //32-bits data, Q0,                  valid floating-point range = {1, 2, 3, ..., 32}
#define    DSM_API_SETGET_EQ_BAND_ENABLE               36   //32-bits data, Q0.                  Bit field definitions: bit0:band1,...,bit31:band32, valid floating-point range = {0,...,0xFFFFFFFF}
#define    DSM_API_GET_EQ_BAND_COEFFICIENTS            37   //32-bits data, Q0.                  The array address of structure biqcoeff_t, the number of structures in the array is 8 by default.
                                                        //                                   Set value to 0 to disable the retrieval of coefficients of EQ filters.

    // ZIMP - impedance filter coefficients, for power from voltage
#define    DSM_API_GET_ZIMP_A1                         38   //32-bits data, Q28,                 valid floating-point range = [0, 8)
#define    DSM_API_GET_ZIMP_A2                         39   //32-bits data, Q28,                 valid floating-point range = [0, 8)
#define    DSM_API_GET_ZIMP_B0                         40   //32-bits data, Q28,                 valid floating-point range = [0, 8)
#define    DSM_API_GET_ZIMP_B1                         41   //32-bits data, Q28,                 valid floating-point range = [0, 8)
#define    DSM_API_GET_ZIMP_B2                         42   //32-bits data, Q28,                 valid floating-point range = [0, 8)

    // EQ Biquad coefficients	
#define    DSM_API_SETGET_EQ1_A1                       43   //obsolete,  command 37 will return coefficients of all EQ bands
#define    DSM_API_SETGET_EQ1_A2                       44   //obsolete,  command 37 will return coefficients of all EQ bands
#define    DSM_API_SETGET_EQ1_B0                       45   //obsolete,  command 37 will return coefficients of all EQ bands
#define    DSM_API_SETGET_EQ1_B1                       46   //obsolete,  command 37 will return coefficients of all EQ bands
#define    DSM_API_SETGET_EQ1_B2                       47   //obsolete,  command 37 will return coefficients of all EQ bands

#define    DSM_API_SETGET_FC_GUARD_BAND                48   //32-bit data, Q9,  Fc guard band in Hz. Valid range = [0, 4194304)
#define    DSM_API_SETGET_PROJECT_ID                   49   //32-bit data, Q0
#define    DSM_API_SETGET_INF_VERSION                  50   //32-bit data, Q0

#define    DSM_API_SETGET_PPR_XOVER_FREQ_Hz            51   //32-bit data, Q9, the cross-over frequency of PPR, in Hz. valid floating-point range = [0, 4194304)
#define    DSM_API_SETGET_ENABLE_PPR                   52   //32-bit data, Q0, enable/disable PPR, valid floating-point range = {0,1}

#define    DSM_API_SETGET_RDC_SCALING                  53   //32-bit data, Q25, the scaling of Rdc, valid floating-point range = [0, 64)

#define    DSM_API_SETGET_MBDRC_TARGET_SUBBAND_ID      54   //32-bit data, Q0, multi-band DRC channel ID. It is a control command to updates DRC parameters (Th, ratio, aTime,rTime,f1,f2).
                                                        //1 = sub-band1; 2 = sub-band2; 3 = sub-band3, valid floating-point range = {1, 2, 3}
#define    DSM_API_SETGET_MBDRC_ENABLE                 55   //32-bit data, Q0,  enable/disable a sub-band of DRC.
                                                        //Bit field definitions: bit0:DRC band1, bit1: DRC subband2, bit3:DRC subband3, valid floating-point range = {0, 1, 2, 3, ..., 7}
#define    DSM_API_SETGET_DRC_TRHESHOLD                56   //32-bit data, Q20, DRC threshold in dB, valid floating-point range = (-120.0, 0.0]
#define    DSM_API_SETGET_DRC_RATIO                    57   //32-bit data, Q20, DRC compression ratio,valid floating-point range = [1.0, 1000.0]
#define    DSM_API_SETGET_DRC_ATTACK                   58   //32-bit data, Q20, DRC attack time in seconds, valid floating-point range = (0.0, 5.0)
#define    DSM_API_SETGET_DRC_RELEASE                  59   //32-bit data, Q20, DRC release time in seconds, valid floating-point range = (0.0, 5.0)
#define    DSM_API_SETGET_MBDRC_CUTOFF_F1              60   //32-bit data, Q0,  DRC low-middle subband cutoff frequency in Hz. valid floating-point range = (200, 1000)
#define    DSM_API_SETGET_MBDRC_CUTOFF_F2              61   //32-bit data, Q0,  DRC middle-high subband cutoff frequency in Hz. valid floating-point range =(1000, 4000)
#define    DSM_API_SETGET_MBDRC_DEBUG_MODE             62   //32-bit data, Q0, DRC internal debug mode. valid floating-point range =[0, 1]

#define    DSM_API_SETGET_TRAJECTORY                   63   //32-bits data, Q0,                  valid floating-point range = {0, 1}
#define    DSM_API_SETGET_GUARD_BEEN_MEAN_SCALE        64   //32-bits data, Q30,                 valid floating-point range = [0, 2.0)

#define    DSM_API_SET_UPDATE_DELAY                    65   //32-bits data, Q0,                  valid floating-point range = {0, 1}
#define    DSM_API_SETGET_DELAY                        66   //32-bits data, Q0,                  valid floating-point range = [0, 0x7FFFFFFF]
#define    DSM_API_GET_MAXIMUM_DELAY                   67   //32-bits data, Q0,                  valid floating-point range = [0, 0x7FFFFFFF]
#define    DSM_API_SETGET_ENABLE_FAST_FC               68   //32-bits data, Q0,                  valid floating-point range = {0, 1}
   
#define    DSM_API_SETGET_ORM_QUALIFY_THRESH           69   //32-bits data, Q30, valid floating-point range = [-2, 2)
 
#define    DSM_API_SETGET_ENABLE_LOGGING               70   //32-bits data, Q0,                  valid floating-point range = {0, 1}

#define    DSM_API_SETGET_SPEAKER_PARAM_LFX_A1         71   //32-bits data, Q28,                 valid floating-point range = [0, 8)
#define    DSM_API_SETGET_SPEAKER_PARAM_LFX_A2         72   //32-bits data, Q28,                 valid floating-point range = [0, 8)
#define    DSM_API_SETGET_SPEAKER_PARAM_LFX_B0         73   //32-bits data, Q28,                 valid floating-point range = [0, 8)
#define    DSM_API_SETGET_SPEAKER_PARAM_LFX_B1         74   //32-bits data, Q28,                 valid floating-point range = [0, 8)
#define    DSM_API_SETGET_SPEAKER_PARAM_LFX_B2         75   //32-bits data, Q28,                 valid floating-point range = [0, 8)

#define    DSM_API_SETGET_SPEAKER_PARAM_TCTH1          76   //32-bits data, Q20,                 valid floating-point range = [0, 2048)
#define    DSM_API_SETGET_SPEAKER_PARAM_TCTH2          77   //32-bits data, Q20,                 valid floating-point range = [0, 2048)
#define    DSM_API_SETGET_SPEAKER_PARAM_RTH1           78   //32-bits data, Q22,                 valid floating-point range = [0, 512)
#define    DSM_API_SETGET_SPEAKER_PARAM_RTH2           79   //32-bits data, Q22,                 valid floating-point range = [0, 512)

#define    DSM_API_SETGET_SPEAKER_PARAM_ADMIT_A1       80   //32-bits data, Q28,                 valid floating-point range = [0, 8)
#define    DSM_API_SETGET_SPEAKER_PARAM_ADMIT_A2       81   //32-bits data, Q28,                 valid floating-point range = [0, 8)
#define    DSM_API_SETGET_SPEAKER_PARAM_ADMIT_B0       82   //32-bits data, Q28,                 valid floating-point range = [0, 8)
#define    DSM_API_SETGET_SPEAKER_PARAM_ADMIT_B1       83   //32-bits data, Q28,                 valid floating-point range = [0, 8)
#define    DSM_API_SETGET_SPEAKER_PARAM_ADMIT_B2       84   //32-bits data, Q28,                 valid floating-point range = [0, 8)

#define    DSM_API_SETGET_SPEAKER_PARAM_UPDATE         85   //unsigned 32-bit, Q0,               valid floating-point range = {0, 1}

#define    DSM_API_SETGET_READBACK_EQ_BAND_ID          86   //obsolete

#define    DSM_API_SETGET_GENERATE_SINE_WAVE           87   //32-bit, Q0,                        valid floating-point range = {0, 1}

    // special APIs
#define    DSM_API_GET_FIRMWARE_BUILD_TIME             88   //C string,                          valid maximum length of C string = DSM_TIME_MAX_SIZE
#define    DSM_API_GET_FIRMWARE_BUILD_DATE             89   //C string,                          valid maximum length of C string = DSM_DATE_MAX_SIZE
#define    DSM_API_GET_FIRMWARE_VERSION                90   //C string,                          valid maximum length of C string = DSM_VERSION_MAX_SIZE
#define    DSM_API_GET_CHIPSET_MODEL                   91   //C string,                          valid maximum length of C string = DSM_CHIPSET_MAX_SIZE
#define    DSM_API_GET_ENDIAN                          92   //32-bits data, Q0,                  valid value = {0xdeadbeef}

#define    DSM_API_SETGET_SINE_WAVE_GAIN               93   //32-bits data, Q15,                 valid floating-point range = [0, 65536)

#define    DSM_API_SETGET_VIRTUAL_V_ENABLE             94   //32-bits data, Q0,                  valid floating-point range = {0, 1}
#define    DSM_API_SETGET_FORCED_VIRTUAL_V             95   //32-bits data, Q0,                  valid floating-point range = {0, 1}

#define    DSM_API_SETGET_I_SHIFT_BITS                 96   //32-bits data, Q0,                  valid floating-point range = [0, 15]

#define    DSM_API_SETGET_EXC_FUNC_GAIN_ADJUSTED       97   //32-bits data, Q0,                  valid floating-point range = {0, 1, 2}. 2=no gain update

#define    DSM_API_SETGET_FADE_IN_TIME_MS              98   //32-bits data, Q0,                  valid floating-point range = [0, 0x7FFFFFFF]
#define    DSM_API_SETGET_FADE_OUT_TIME_MS             99   //32-bits data, Q0,                  valid floating-point range = [0, 0x7FFFFFFF]

    //This command takes effect only after 
    //DSM_API_FADE_OUT_TIME_MS
#define    DSM_API_SETGET_FADE_OUT_MUTE_TIME_MS        100  //32-bits data, Q0,                  valid floating-point range = [0, 0x7FFFFFFF]

#define    DSM_API_SETGET_FADE_IN_ENABLE               101  //32-bits data, Q0,                  valid floating-point range = {0, 1}
#define    DSM_API_SETGET_FADE_OUT_ENABLE              102  //32-bits data, Q0,                  valid floating-point range = {0, 1}

#define    DSM_API_SETGET_ENABLE_MULTICHAN_LINKING     103  //32-bits data, Q0,                  valid floating-point range = {0, 1}
    
#define    DSM_API_SETGET_ENABLE_SMART_PT              104  //32-bits data, Q0,                  valid floating-point range = {0, 1}
#define    DSM_API_SETGET_PILOTTONE_SILENCE_THRESHOLD  105  //32-bits data, Q0,                  valid floating-point range = [0, 0x7FFFFFFF]
#define    DSM_API_SETGET_SILENCE_PILOTTONE_GAIN_Q31   106  //32-bits data, Q31,                 valid floating-point range = [0, 1.0)
#define    DSM_API_SETGET_SILENCE_FRAMES               107  //32-bits data, Q0,                  valid floating-point range = [-1, 0x7FFFFFFF], -1 indicates infinite.
#define    DSM_API_SETGET_PILOTTONE_TRANSITION_FRAMES  108  //32-bits data, Q0,                  valid floating-point range = [-1, 0x7FFFFFFF], -1 indicates infinite.

#define    DSM_API_SETGET_PILOTTONE_SMALL_SIGNAL_THRESHOLD    109  //32-bits data, Q0,                 valid floating-point range = [0, 0x7FFFFFFF]
#define    DSM_API_SETGET_SMALL_SIGNAL_PILOTTONE_GAIN_Q31     110  //32-bits data, Q31,                 valid floating-point range = [0, 1.0)

#define    DSM_API_SETGET_ENABLE_LINKWITZ_EQ           111  //32-bits data, Q0,                 valid floating-point range = {0, 1}

#define    DSM_API_SETGET_CHANNEL_MASK                 112  //32-bits data, Q0,                 valid floating-point range = [0, 0x00FF]
#define    DSM_API_SETGET_ENABLE_FF_FB_MODULES         113  //32-bits data, Q0: bit0=FF module, bit1=FB module,valid floating-point range = {0, ..., 3}

#define    DSM_API_SETGET_ENABLE_CROSSOVER             114  //32-bits data, Q0,                 valid floating-point range = {0, 1}

#define    DSM_API_SETGET_AUX_EQ_BAND_FC               115  //32-bits data, Q9, in Hz,          valid floating-point range = [0, 4194304)
#define    DSM_API_SETGET_AUX_EQ_BAND_Q                116  //32-bits data, Q29,                valid floating-point range = [0, 4)
#define    DSM_API_SETGET_AUX_EQ_BAND_ATTENUATION_DB   117  //32-bits data, Q20, in db,         valid floating-point range = [-32, 32]
#define    DSM_API_SET_EQ_AUX_BAND_COEFF_UPDATE        118  //32-bits data, Q0,                 valid floating-point range = {0, 1}
#define    DSM_API_SETGET_AUX_EQ_BAND_ENABLE           119  //32-bits data, Q0. Bit field definitions: bit0:band1,bit1:band2,bit2:band3,bit3:band4, valid floating-point range = {0, ..., 15}

#define    DSM_API_SETGET_ENABLE_AUX_CROSSOVER         120  //32-bits data, Q0,                 valid floating-point range = {0, 1}

#define    DSM_API_SETGET_ENABLE_AUX1_DELAYED_SAMPLES  121  //32-bits data, Q0,                 valid floating-point range = [0, 0x7FFFFFFF]
#define    DSM_API_SETGET_ENABLE_AUX2_DELAYED_SAMPLES  122  //32-bits data, Q0,                 valid floating-point range = [0, 0x7FFFFFFF]
#define    DSM_API_SETGET_ENABLE_AUX3_DELAYED_SAMPLES  123  //32-bits data, Q0,                 valid floating-point range = [0, 0x7FFFFFFF]
#define    DSM_API_SETGET_ENABLE_AUX4_DELAYED_SAMPLES  124  //32-bits data, Q0,                 valid floating-point range = [0, 0x7FFFFFFF]

#define    DSM_API_SETGET_SPEECH_GUARD_BINS            125  //32-bits data, Q0,                 valid floating-point range = [0, 0x7FFFFFFF]
#define    DSM_API_SETGET_MEAN_SPEECH_THRESHOLD        126  //32-bits data, Q31,                valid floating-point range = [0, 1)

#define    DSM_API_SETGET_HPCUTOFF_FREQ_AUX            127  //32-bits data, Q9,                 valid floating-point range = [0, 4194304)
#define    DSM_API_SETGET_AUX_HP_FILTER_ENABLE         128  //32-bits data, Q0,                 valid floating-point range = {0, 1}

#define    DSM_API_SETGET_STEREO_CROSSOVER_MODE        129  //32-bit data, Q0,                  valid floating-point range = {0, 1, 2}

#define    DSM_API_SETGET_ENABLE_UPDATE_FC_Q           130  //32-bit data, Q0,                  valid floating-point range = {0, 1}

#ifdef WINTEL_AIRFORCE
#define    DSM_API_GET_V_VALIDATION                	   131  //32-bits data, Q0
#else
#define    DSM_API_SETGET_RECEIVER_PHYSICAL_LAYOUT     131  //32-bit data, Q0,                  valid floating-point range = {LANDSCAPE_1_RCV_RIGHT, LANDSCAPE_2_RCV_LEFT}
#endif

#define    DSM_API_SETGET_MAX_V_SAMPLE                 132  //obsolete, the same information can be got in command DSM_API_GET_ADAPTIVE_STATISTICS.
#define    DSM_API_SETGET_MAX_I_SAMPLE                 133  //obsolete, the same information can be got in command DSM_API_GET_ADAPTIVE_STATISTICS.
#define    DSM_API_SETGET_MAX_INPUT_SAMPLE             134  //obsolete, the same information can be got in command DSM_API_GET_ADAPTIVE_STATISTICS.
#define    DSM_API_SETGET_MAX_OUTPUT_SAMPLE            135  //obsolete, the same information can be got in command DSM_API_GET_ADAPTIVE_STATISTICS.

#define    DSM_API_GET_ADAPTIVE_PARAMS                 136  //32-bit data, Q0,                  the address of the data structure Dsm_adapt_params
#define    DSM_API_GET_ADAPTIVE_STATISTICS             137  //32-bit data, Q0,                  the address of the data structure Dsm_adapt_statistics

#define    DSM_API_SETGET_SILENCE_UNMUTED_IN_SPT       138  //32-bit data, Q0,                  valid floating-point range = {0, 1}

#define    DSM_API_SETGET_Q_ADJUSTMENT                 139  //32-bit data, Q29,                 valid floating-point range = [0, 4)

#define    DSM_API_SETGET_XOVER_MIXING_ENABLED         140  //32-bit data, Q0,                  valid floating-point range = {0, 1}
#define    DSM_API_SETGET_XOVER_BOOST_GAIN_PERCENTAGE  141  //32-bit data, Q27,                 valid floating-point range = [0, 16)
#define    DSM_API_SETGET_XOVER_FILTER_CUTOFF_FC       142  //32-bit data, Q9,                  valid floating-point range = [0, 4194304)
#define    DSM_API_SETGET_XOVER_FILTER_Q               143  //32-bit data, Q29,                 valid floating-point range = [0, 4)
#define    DSM_API_SET_XOVER_FILTER_UPDATE             144  //32-bit data, Q0,                  valid floating-point range = {0, 1}

#define    DSM_API_SETGET_XOVER_SPKER_GAIN_PERCENTAGE  145  //32-bit data, Q27,                 valid floating-point range = [0, 16)

#define    DSM_API_SETGET_IV_FORMAT                    146  //32-bit data, Q0 ,                 valid floating-point range = [0, 3)
#define    DSM_API_SETGET_EXCL_LOGGING_THRESH          147   //32-bits data, Q0
#define    DSM_API_SETGET_COILTEMP_LOGGING_THRESH      148   //32-bits data, Q0
#define    DSM_API_GET_LOGGING_DATA                    149   //array of data, 372 bytes
#define    DSM_API_SETGET_CHANNEL_MAPPING              150   //32-bits data, Q0, valid range= 0, 1

#define    DSM_API_SETGET_EQ_BAND_0_FC                 151   //32-bits data, Q9,  in Hz,          valid floating-point range = [0, 4194304)
#define    DSM_API_SETGET_EQ_BAND_0_Q                  152   //32-bits data, Q26,                 valid floating-point range = [0, 32)
#define    DSM_API_SETGET_EQ_BAND_0_ATTENUATION_DB     153   //32-bits data, Q20, in db,          valid floating-point range = [-2048, 2048)
#define    DSM_API_SETGET_EQ_BAND_1_FC                 154   //32-bits data, Q9,  in Hz,          valid floating-point range = [0, 4194304)
#define    DSM_API_SETGET_EQ_BAND_1_Q                  155   //32-bits data, Q26,                 valid floating-point range = [0, 32)
#define    DSM_API_SETGET_EQ_BAND_1_ATTENUATION_DB     156   //32-bits data, Q20, in db,          valid floating-point range = [-2048, 2048)
#define    DSM_API_SETGET_EQ_BAND_2_FC                 157   //32-bits data, Q9,  in Hz,          valid floating-point range = [0, 4194304)
#define    DSM_API_SETGET_EQ_BAND_2_Q                  158   //32-bits data, Q26,                 valid floating-point range = [0, 32)
#define    DSM_API_SETGET_EQ_BAND_2_ATTENUATION_DB     159   //32-bits data, Q20, in db,          valid floating-point range = [-2048, 2048)
#define    DSM_API_SETGET_EQ_BAND_3_FC                 160   //32-bits data, Q9,  in Hz,          valid floating-point range = [0, 4194304)
#define    DSM_API_SETGET_EQ_BAND_3_Q                  161   //32-bits data, Q26,                 valid floating-point range = [0, 32)
#define    DSM_API_SETGET_EQ_BAND_3_ATTENUATION_DB     162   //32-bits data, Q20, in db,          valid floating-point range = [-2048, 2048)
#define    DSM_API_SETGET_EQ_BAND_4_FC                 163   //32-bits data, Q9,  in Hz,          valid floating-point range = [0, 4194304)
#define    DSM_API_SETGET_EQ_BAND_4_Q                  164   //32-bits data, Q26,                 valid floating-point range = [0, 32)
#define    DSM_API_SETGET_EQ_BAND_4_ATTENUATION_DB     165   //32-bits data, Q20, in db,          valid floating-point range = [-2048, 2048)
#define    DSM_API_SETGET_EQ_BAND_5_FC                 166   //32-bits data, Q9,  in Hz,          valid floating-point range = [0, 4194304)
#define    DSM_API_SETGET_EQ_BAND_5_Q                  167   //32-bits data, Q26,                 valid floating-point range = [0, 32)
#define    DSM_API_SETGET_EQ_BAND_5_ATTENUATION_DB     168   //32-bits data, Q20, in db,          valid floating-point range = [-2048, 2048)
#define    DSM_API_SETGET_EQ_BAND_6_FC                 169   //32-bits data, Q9,  in Hz,          valid floating-point range = [0, 4194304)
#define    DSM_API_SETGET_EQ_BAND_6_Q                  170   //32-bits data, Q26,                 valid floating-point range = [0, 32)
#define    DSM_API_SETGET_EQ_BAND_6_ATTENUATION_DB     171   //32-bits data, Q20, in db,          valid floating-point range = [-2048, 2048)
#define    DSM_API_SETGET_EQ_BAND_7_FC                 172   //32-bits data, Q9,  in Hz,          valid floating-point range = [0, 4194304)
#define    DSM_API_SETGET_EQ_BAND_7_Q                  173   //32-bits data, Q26,                 valid floating-point range = [0, 32)
#define    DSM_API_SETGET_EQ_BAND_7_ATTENUATION_DB     174   //32-bits data, Q20, in db,          valid floating-point range = [-2048, 2048)
#define    DSM_API_SETGET_DRC_BAND_0_TRHESHOLD         175   //32-bit data, Q20, DRC threshold in dB, valid floating-point range = (-120.0, 0.0]
#define    DSM_API_SETGET_DRC_BAND_0_RATIO             176   //32-bit data, Q20, DRC compression ratio,valid floating-point range = [1.0, 1000.0]
#define    DSM_API_SETGET_DRC_BAND_0_ATTACK            177   //32-bit data, Q20, DRC attack time in seconds, valid floating-point range = (0.0, 5.0)
#define    DSM_API_SETGET_DRC_BAND_0_RELEASE           178   //32-bit data, Q20, DRC release time in seconds, valid floating-point range = (0.0, 5.0)
#define    DSM_API_SETGET_DRC_BAND_1_TRHESHOLD         179   //32-bit data, Q20, DRC threshold in dB, valid floating-point range = (-120.0, 0.0]
#define    DSM_API_SETGET_DRC_BAND_1_RATIO             180   //32-bit data, Q20, DRC compression ratio,valid floating-point range = [1.0, 1000.0]
#define    DSM_API_SETGET_DRC_BAND_1_ATTACK            181   //32-bit data, Q20, DRC attack time in seconds, valid floating-point range = (0.0, 5.0)
#define    DSM_API_SETGET_DRC_BAND_1_RELEASE           182   //32-bit data, Q20, DRC release time in seconds, valid floating-point range = (0.0, 5.0)
#define    DSM_API_SETGET_DRC_BAND_2_TRHESHOLD         183   //32-bit data, Q20, DRC threshold in dB, valid floating-point range = (-120.0, 0.0]
#define    DSM_API_SETGET_DRC_BAND_2_RATIO             184   //32-bit data, Q20, DRC compression ratio,valid floating-point range = [1.0, 1000.0]
#define    DSM_API_SETGET_DRC_BAND_2_ATTACK            185   //32-bit data, Q20, DRC attack time in seconds, valid floating-point range = (0.0, 5.0)
#define    DSM_API_SETGET_DRC_BAND_2_RELEASE           186   //32-bit data, Q20, DRC release time in seconds, valid floating-point range = (0.0, 5.0)

#define    DSM_API_SETGET_INTEL_MAKEUP_GAIN            187   //32-bit data, Q29, valid floating-point range = [0, 4)
#define    DSM_API_SETGET_SPEAKER_FULLSCALE            188   //32-bit data, Q26, valid floating-point range = [0, 31)
#define    DSM_API_SETGET_CUSTOM_FUNC            	   189   //32-bit data, Q0
#define    DSM_API_GET_ADAPTIVE_POWER            	   190   //32-bit data, Q8, valid floating-point range = [0, 16777216)
#define    DSM_API_GET_KURTOSIS            			   191   //32-bit data, Q10, valid floating-point range = [0, 2097152)
#define    DSM_API_SETGET_KURTOSIS_THRESH			   192   //32-bit data, Q10 , valid floating-point range = [0, 2097152)

#define    DSM_API_SETGET_AUTOCAL_ENABLE               193   //32-bit data, Q0
#define    DSM_API_SETGET_AUTOCAL_COOL_TIME            194   //32-bit data, Q0
#define    DSM_API_SETGET_AUTOCAL_AMBIENT_TEMP         195   //32-bit data, Q19
#define    DSM_API_SETGET_AUTOCAL_RDC_DATA0            196   //32-bit data, Q27
#define    DSM_API_SETGET_AUTOCAL_RDC_DATA1            197   //32-bit data, Q19

#define DSM_API_SETGET_DEBUZZER_ENABLE                 198  //32-bit data, Q0,                  valid floating-point range = {0, 1}
#define DSM_API_SETGET_DEBUZZER_NOTCH_FC               199   //32-bit data, Q9, in Hz,          valid floating-point range = [0, 4000]
#define DSM_API_SETGET_DEBUZZER_NOTCH_Q                200  //32-bits data, Q28,                valid floating-point range = (0, 8).
#define DSM_API_SETGET_DEBUZZER_PORT_FC                201  //32-bit data, Q9,                  valid floating-point range = [20.0, 24000]
#define DSM_API_SETGET_DEBUZZER_PORT_Q                 202  //32-bit data, Q28,                 valid floating-point range = [0, 8)
#define DSM_API_SETGET_DEBUZZER_THRESH_LINEAR          203  //32-bit data, Q28,                 valid floating-point range = [0.01, 1] where 0.01 is (-40dB)
#define DSM_API_SETGET_DEBUZZER_CREST_THRESH           204  //32-bits data, Q20, in db,         valid floating-point range = [-32, 32]
#define DSM_API_SETGET_DEBUZZER_ATTACK_TIME_SEC        205  //32-bits data, Q27, in second,     valid floating-point range = [0, 16)
#define DSM_API_SETGET_DEBUZZER_RELEASE_TIME_SEC       206  //32-bits data, Q27, in second,     valid floating-point range = [0, 16)

#define DSM_API_GET_EXCURSION_MAX					   207  //32-bits data, Q27,                 valid floating-point range = [0, 16)
#define DSM_API_GET_EXCURSION_OVERCNT				   208  //32-bits data, Q27,                 valid floating-point range = [0, 16)
#define DSM_API_GET_COILTEMP_MAX		               209  //32-bits data, Q19,                 valid floating-point range = [0, 4096)
#define DSM_API_GET_COILTEMP_OVERCNT	               210  //32-bits data, Q19,                 valid floating-point range = [0, 4096)

#define DSM_API_SETGET_EQ_BAND_0_TYPE                  211   //32-bits data, Q0,                  valid floating-point range = enum {
                                                          //                                                                  Filter_Type_parametric    = 0,
                                                          //                                                                  Filter_Type_FLAT,
                                                          //                                                                  Filter_Type_MUTE,
                                                          //                                                                  Filter_Type_Low_pass,
                                                          //                                                                  Filter_Type_High_pass,
                                                          //                                                                  Filter_Type_ALLPASS,
                                                          //                                                                  Filter_Type_BANDPASS,
                                                          //                                                                  Filter_Type_Notch,
                                                          //                                                                  Filter_Type_Low_Shelf,
                                                          //                                                                  Filter_Type_High_Shelf
                                                          //                                                                  }
#define DSM_API_SETGET_EQ_BAND_1_TYPE                  212   //32-bits data, Q0,
#define DSM_API_SETGET_EQ_BAND_2_TYPE                  213   //32-bits data, Q0,
#define DSM_API_SETGET_EQ_BAND_3_TYPE                  214   //32-bits data, Q0,
#define DSM_API_SETGET_EQ_BAND_4_TYPE                  215   //32-bits data, Q0,
#define DSM_API_SETGET_EQ_BAND_5_TYPE                  216   //32-bits data, Q0,
#define DSM_API_SETGET_EQ_BAND_6_TYPE                  217   //32-bits data, Q0,
#define DSM_API_SETGET_EQ_BAND_7_TYPE                  218   //32-bits data, Q0,
#define DSM_API_SETGET_EQ_BAND_8_TYPE                  219   //32-bits data, Q0,
#define DSM_API_SETGET_EQ_BAND_9_TYPE                  220   //32-bits data, Q0,
#define DSM_API_SETGET_EQ_BAND_8_FC                    221   //32-bits data, Q9,  in Hz,          valid floating-point range = [0, 4194304)
#define DSM_API_SETGET_EQ_BAND_8_Q                     222   //32-bits data, Q26,                 valid floating-point range = [0, 32)
#define DSM_API_SETGET_EQ_BAND_8_ATTENUATION_DB        223   //32-bits data, Q20, in db,          valid floating-point range = [-2048, 2048)
#define DSM_API_SETGET_EQ_BAND_9_FC                    224   //32-bits data, Q9,  in Hz,          valid floating-point range = [0, 4194304)
#define DSM_API_SETGET_EQ_BAND_9_Q                     225   //32-bits data, Q26,                 valid floating-point range = [0, 32)
#define DSM_API_SETGET_EQ_BAND_9_ATTENUATION_DB        226   //32-bits data, Q20, in db,          valid floating-point range = [-2048, 2048)

#define    DSM_API_SETGET_PARAMS_COUNT                 227

/*******************************************
 *
 *  Data type DSM_API_MESSAGE
 *
 * Description:
 *     The enum data type DSM_API_MESSAGE is used
 * as the data type of DSM APIs return.
 *
 * *****************************************/
typedef enum {
    DSM_API_OK = 0,
    DSM_API_MSG_NULL_MODULE_HANDLER         = 1<<1,
    DSM_API_MSG_NULL_PARAM_POINTER          = 1<<2,
    DSM_API_MSG_NULL_INPUT_BUFFER_POINTER   = 1<<3,
    DSM_API_MSG_NULL_OUTPUT_BUFFER_POINTER  = 1<<4,
    DSM_API_MSG_INVALIDE_CMD_ID             = 1<<5,
    DSM_API_MSG_INVALIDE_PARAM              = 1<<6,
    DSM_API_MSG_INVALIDE_PARAMS_NUM         = 1<<7,
    DSM_API_MSG_INVALIDE_SAMPLING_RATE      = 1<<8,

    DSM_API_MSG_NOT_IMPLEMENTED             = 1<<9,

    DSM_API_MSG_INVALID_MEMORY              = 1<<10,
    DSM_API_MSG_ZERO_I                      = 1<<11,
    DSM_API_MSG_ZERO_V                      = 1<<12,
    DSM_API_MSG_MIN_RDC_BEYOND_THRESHOLD    = 1<<13,
    DSM_API_MSG_MAX_RDC_BEYOND_THRESHOLD    = 1<<14,

    DSM_API_MISMATCHED_SETGET_CMD           = 1<<15,

    DSM_API_MSG_IV_DATA_WARNING             = 1<<16,
    DSM_API_MSG_COIL_TEMPERATURE_WARNING    = 1<<17,
    DSM_API_MSG_EXCURSION_WARNING           = 1<<18,

    DSM_API_MSG_WRONG_COMMAND_TYPE          = 1<<19,
    DSM_API_MSG_COMMAND_OBSOLETE            = 1<<20,

    DSM_API_MSG_INSUFFICIENT_INPUT_DATA     = 1<<21,

    DSM_API_MSG_FF_NOT_START                = 1<<22,
    
    DSM_API_MSG_INVALID
} DSM_API_MESSAGE;

/****************************************************
 *
 * Data type DSM_API_Data_Type
 *
 * Description:
 *     The enum data type DSM_API_Data_Type is used
 * to indicate the data type of DSM parameters.
 *
 * *************************************************/
typedef enum {
    DSM_Int8 = 0,                           //8-bit integer
    DSM_Int16,                              //16-bit integer
    DSM_Int32,                              //32-bit integer
    DSM_Int64,                              //64-bit integer
    DSM_Float,                              //32-bit IEEE float
    DSM_Double,                             //64-bit IEEE double float
    DSM_C_String,                           //C string with the termination '\0'
    DSM_Buffer,                             //a pointer pointed at a data buffer allocated by caller function

    DSM_Invalide //invalide
} DSM_API_Data_Type;

enum DSM_FEATURES {
    DSM_FEATURE_PILOT_ENABLE                = (1<<0x00),
    DSM_FEATURE_EXC_ENABLE                  = (1<<0x01),
    DSM_FEATURE_THERMAL_ENABLE              = (1<<0x02),
    DSM_FEATURE_OUTPUT_GAIN_CONTROL_ENABLE  = (1<<0x03),
    DSM_FEATURE_NUM
};

/*      Interface functions       */

/*******************************************************************************
 *    DSM_API_get_memory_size()
 * 
 * Description:
 *     This function returns the size of data memory which is required by DSM module
 * and must be called before any other DSM API functions.
 *     The DSP framework should be responsible for allocating memory for
 * DSM module.
 * 
 * Input:
 *     iChannels:  the number of channels which are requested to be supported.
 *     ipCircBufferSize:   a 32-bit integer array which contains the sizes of input, output,
 *                I data and V data buffers. These buffers are used for debugging or 
 *                performance tuning.
 *                   The format of this argument is as below:
 *                +--------------------+\
 *                |  input buffer size | \
 *                +--------------------+  \
 *                | output buffer size |   \
 *                +--------------------+   -> channel 1
 *                | I data buffer size |   /
 *                +--------------------+  /
 *                | V data buffer size | /
 *                +--------------------+/
 *                       .
 *                       .
 *                       .
 *                +--------------------+\
 *                |  input buffer size | \
 *                +--------------------+  \
 *                | output buffer size |   \
 *                +--------------------+   -> channel N
 *                | I data buffer size |   /
 *                +--------------------+  /
 *                | V data buffer size | /
 *                +--------------------+/
 *
 * Output:
 *     opMemSizeRequired:
 *          the address of a 32-bit variable which contains the size of memory.
 *
 * Returns:
 *     DSM_API_OK: successful;
 *     Otherwise:  error code.
 *
 *******************************************************************************/
extern DSM_API_MESSAGE DSM_API_get_memory_size(
        short    iChannels,
        int     *ipCircBufferSize,
        int     *opMemSizeRequired
);

/*******************************************************************************
 *    DSM_API_get_memory_size_ext()
 *
 * Description:
 *     This function returns the size of data memory which is required by DSM module
 * and must be called before any other DSM API functions.
 *     The DSP framework should be responsible for allocating memory for
 * DSM module.
 *
 * Input:
 *     iParamSize: the size of the data structure dsm_api_memory_size_ext_t.
 *
 * Output:
 *     N/A
 *
 * I/O:
 *     iopMemParam: the address of data structure dsm_api_memory_size_ext_t
 *     which contains input and output arguments.
 *
 * Returns:
 *     DSM_API_OK: successful;
 *     Otherwise:  error code.
 *
 *******************************************************************************/
DSM_API_MESSAGE DSM_API_get_memory_size_ext(
        dsm_api_memory_size_ext_t   *iopMmemParam,
        int                         iParamSize);

/*******************************************************************************
 *    DSM_API_get_memory_size_ext2()
 *
 * Description:
 *     This function returns the size of data memory which is required by DSM module
 * and must be called before any other DSM API functions.
 *     The DSP framework should be responsible for allocating memory for
 * DSM module.
 *
 * Input:
 *     iParamSize: the size of the data structure dsm_api_memory_size_ext_t.
 *
 * Output:
 *     N/A
 *
 * I/O:
 *     iopMemParam: the address of data structure dsm_api_memory_size_ext2_t
 *     which contains input and output arguments.
 *
 * Returns:
 *     DSM_API_OK: successful;
 *     Otherwise:  error code.
 *
 *******************************************************************************/
DSM_API_MESSAGE DSM_API_get_memory_size_ext2(
        dsm_api_memory_size_ext2_t   *iopMmemParam,
        int                         iParamSize);

/*******************************************************************************
 *    DSM_API_Init()
 * 
 * Description:
 *     This function is used to initialize DSM module and must be called after
 * DSM_API_get_memory_size() and before all other DSM API functions.
 * 
 * Inputs:
 *     ipModuleHandler:
 *                 the address of DSM module which is allocated by framework caller.
 *     iSamplerate: the sampling rate of input PCM data to DSM module.
 *     iChannels:   the number of channels which are requested to be supported.
 *                     The returned memory size and frame sizes depend on the input sampling rate.
 *                 independent of the sampling rate.
 *     ipCircBufferSize: a 32-bit integer array which contains the sizes of input, output,
 *                 I data and V data buffers. These buffers are used for debugging or 
 *                 performance tuning. This parameter should be the same as the one used
 *                 in DSM_API_get_memory_size().
 *
 * Outputs:
 *     opFrame_size:the address of a 32-bit integer which contains the size of frame in samples.
 *
 * Returns:
 *     DSM_API_OK: successful;
 *     Otherwise:  error codes.
 *
 *******************************************************************************/
extern DSM_API_MESSAGE DSM_API_Init(
        void            *ipModuleHandler, 
        unsigned int     iSamplerate,
        short            iChannels,
        int             *ipCircBufferSize,
        int             *opFrame_size
);

/*******************************************************************************
 *    DSM_API_Init_ext()
 *
 * Description:
 *     This function is used to initialize DSM module and must be called after
 * DSM_API_get_memory_size() and before all other DSM API functions.
 *
 * Inputs:
 *     ipModuleHandler:
 *                  the handler of DSM module which is allocated by framework caller.
 *     iParamSize:  the size of the data structure dsm_api_init_ext_t.
 *
 * Outputs:
 *     N/A
 *
 * I/O:
 *     iopParamStruct: the address of a data structure dsm_api_init_ext_t
 *     which contains input and output arguments.
 *
 * Returns:
 *     DSM_API_OK: successful;
 *     Otherwise:  error codes.
 *
 *******************************************************************************/
extern DSM_API_MESSAGE DSM_API_Init_ext(
        void                *ipModuleHandler,
        dsm_api_init_ext_t  *iopParamStruct,
        int                  iParamSize
);

/*******************************************************************************
 *    DSM_API_Init_ext2()
 *
 * Description:
 *     This function is used to initialize DSM module and must be called after
 * DSM_API_get_memory_size() and before all other DSM API functions.
 *
 * Inputs:
 *     ipModuleHandler:
 *                  the handler of DSM module which is allocated by framework caller.
 *     iParamSize:  the size of the data structure dsm_api_init_ext_t.
 *
 * Outputs:
 *     N/A
 *
 * I/O:
 *     iopParamStruct: the address of a data structure dsm_api_init_ext2_t
 *     which contains input and output arguments.
 *
 * Returns:
 *     DSM_API_OK: successful;
 *     Otherwise:  error codes.
 *
 *******************************************************************************/
extern DSM_API_MESSAGE DSM_API_Init_ext2(
        void                *ipModuleHandler,
        dsm_api_init_ext2_t  *iopParamStruct,
        int                  iParamSize
);


/*******************************************************************************
 *    DSM_API_FF_process()
 * 
 * Description:
 *     This function is used to process the input audio PCM DSM data.
 * 
 * Inputs:
 *     ipModuleHandler:
 *                    the handler of DSM module which is allocated by framework caller.
 *     iChannelMask: the low 8-bits indicate which channels should be executed.
 *                  0: default channels setting, mono or stereo
 *                  1: L channel
 *                  2: R channel
 *                  3: L and R channels
 *                 -1: place input L channel onto output R channel
 *                 -2: place input R channel onto output L channel
 *                 -3: switch L and R channel
 *     ipInputBuffer:    the input buffer which contains 16-bit audio PCM input data.
 *                 The multi-channel input PCM data are ordered in the format below:
 *                 +-----------------------+    ...    +-------------------------+
 *                 |    one frame samples  |    ...    |   one frame samples     |
 *                 +-----------------------+    ...    +-------------------------+
 *                      1st channel             ...           N-th channel
 *
 * Outputs:
 *     opOutputBuffer:   the output buffer which contains the 16-bit PCM data processed by DSM.
 *                The multi-channel output PCM data are ordered in the format below:
 *                 +-----------------------+    ...    +-------------------------+
 *                 |    one frame samples  |    ...    |   one frame samples     |
 *                 +-----------------------+    ...    +-------------------------+
 *                      1st channel             ...           N-th channel
 *     opNrSamples:   the address of a variable which contains the number of samples in output buffer.
 *
 * I/O:
 *     iopInputSamples: the number of audio PCM samples to be processed, in 32-bit
 *                   long integer.
 *                       The returned value indicates how many samples of input data are 
 *                   not used in the input buffer. In this case, DSP framework should 
 *                   send back the remaining data in next process.
 *
 * Returns:
 *     DSM_API_OK: successful;
 *     Otherwise:  error codes.
 *
 *******************************************************************************/
extern DSM_API_MESSAGE DSM_API_FF_process(
        void            *ipModuleHandler, 
        int              iChannelMask,
        short           *ipInputBuffer,
        int             *iopInputSamples,
        short           *opOutputBuffer,
        int             *opOutputSamples
);

/*******************************************************************************
 *    DSM_API_FB_process()
 * 
 * Description:
 *     This function is used to process current(I) and voltage(V) feedback data.
 * 
 * Inputs:
 *     ipModuleHandler:
 *                    the handler of DSM module which is allocated by framework caller.
 *     channelMask:   the low 8-bits indicate which channels should be executed.
 *                  0: default channels setting, mono or stereo
 *                  1: L channel
 *                  2: R channel
 *                  3: L and R channels
 *                 -1: place input L channel onto output R channel
 *                 -2: place input R channel onto output L channel
 *                 -3: switch L and R channel
 *     iCurrBuffer:   the input buffer which contains I data. 
 *                 The multi-channel I data are ordered in the format below:
 *                 +-----------------------+    ...    +-------------------------+
 *                 |    one frame samples  |    ...    |   one frame samples     |
 *                 +-----------------------+    ...    +-------------------------+
 *                      1st channel             ...           N-th channel
 *     iVoltBuffer:   the input buffer which contains V data. 
 *                 The multi-channel V data are ordered in the format below:
 *                 +-----------------------+    ...    +-------------------------+
 *                 |    one frame samples  |    ...    |   one frame samples     |
 *                 +-----------------------+    ...    +-------------------------+
 *                      1st channel             ...           N-th channel
 *
 * I/O:
 *     iopNrSamples:  the address of a variable which contains number of I/V data to 
 *                    be processed. 
 *                        The returned value indicates how many samples of I/V data are 
 *                    not used in the I/V buffer. In this case, DSP framework should send 
 *                    back the remaining I and V data in next process.
 *
 * Outputs:
 *     N/A
 *
 * Returns:
 *     DSM_API_OK: successful;
 *     Otherwise:  error codes.
 *
 *******************************************************************************/
extern DSM_API_MESSAGE DSM_API_FB_process(
        void            *ipModuleHandler,
        int              iChannelMask,
        short           *iCurrBuffer, 
        short           *iVoltBuffer,
        int             *iopNrSamples
);
#define DSM_API_process_iv      DSM_API_FB_process  //keep backward compatible

/*******************************************************************************
 *    DSM_API_FB_process_v2()
 *
 * Description:
 *     This function is the v2 version to process current(I) and voltage(V) feedback data.
 *
 * Inputs:
 *     ipModuleHandler:
 *                    the handler of DSM module which is allocated by framework caller.
 *     iLeftBufferOrg:   the input buffer from left channel, depending on ivformat, it can
 *                       hold data in the following three formats:
 *                       (1) 16-bit I data when ivFormat=DSM_IV_FORMAT_DETERLEAVED_16_BIT
 *                 The multi-channel I data are ordered in the format below:
 *                 +-----------------------+    ...    +-------------------------+
 *                 |    one frame samples  |    ...    |   one frame samples     |
 *                 +-----------------------+    ...    +-------------------------+
 *                      1st channel             ...           N-th channel
 *
 *                      (2) interleaved 16-bit IV data for left channel when ivFomat=DSM_IV_FORMAT_INTERLEAVED_16_BIT
 *                          LSB = 0 for Voltage(V), LSB = 1 for current(I)
 *                 +-----------------------+    ...    +-------------------------+
 *                 | I | V | I | V |    ...
 *                 +-----------------------+    ...    +-------------------------+
 *
 *                      (3) 16-bit I data for left channel when ivFormat = DSM_IV_FORMAT_8_BIT_VOLTAGE
 *
 *     iRightBufferOrg:   the input buffer from right channel, depending on ivformat, it can
 *                        hold data in the following three formats:
 *                       (1) 16-bit V data when ivFormat=DSM_IV_FORMAT_DETERLEAVED_16_BIT
 *                 The multi-channel V data are ordered in the format below:
 *                 +-----------------------+    ...    +-------------------------+
 *                 |    one frame samples  |    ...    |   one frame samples     |
 *                 +-----------------------+    ...    +-------------------------+
 *                      1st channel             ...           N-th channel
 *
 *                      (2) interleaved 16-bit IV data for right channel when ivFomat=DSM_IV_FORMAT_INTERLEAVED_16_BIT
 *                          LSB = 0 for Voltage(V), LSB = 1 for current(I)
 *                 +-----------------------+    ...    +-------------------------+
 *                 | I | V | I | V |    ...
 *                 +-----------------------+    ...    +-------------------------+
 *                      (3) 8-bit V data for right channel
 *
 *                The ivFormat is set through DSM_API_Set_Params() with parameter ID = DSM_API_SETGET_IV_FORMAT
 * I/O:
 *     ipNrSamplesPerBuffer:  the address of a variable which contains number of I/V samples in each buffer.
 *                           the left and right buffer must have equal number of samples.
 *                        The returned value indicates how many samples of I/V data are
 *                    not used in the I/V buffer. In this case, DSP framework should send
 *                    back the remaining I and V data in next process.
 *
 * Outputs:
 *     N/A
 *
 * Returns:
 *     DSM_API_OK: successful;
 *     Otherwise:  error codes.
 *
 *******************************************************************************/
DSM_API_MESSAGE DSM_API_FB_process_v2(
        void            *ipModuleHandler,
        short           *iLeftBufferOrg,
        short           *iRightBufferOrg,
        int             *ipNrSamplesPerBuffer
        );

/*******************************************************************************
 *    DSM_API_generate_auxiliary_channels()
 *
 * Description:
 *     This function is used to generate data for auxiliary channels which are
 * usually connected to speakers that don't need DSM processing.
 *
 * Inputs:
 *     ipModuleHandler:
 *                  the handler of DSM module which is allocated by framework caller.
 *     iChannelMask:  the low 8-bits indicate which channels should be executed.
 *                  0: default channels setting, mono or stereo
 *                  1: L channel
 *                  2: R channel
 *                  3: L and R channels
 *                 -1: place input L channel onto output R channel
 *                 -2: place input R channel onto output L channel
 *                 -3: switch L and R channel
 *     iChannelBuffer:   the input buffer which contains input PCM data.
 *                 The multi-channel I data are ordered in the format below:
 *                 +-----------------------+    ...    +-------------------------+
 *                 |    one frame samples  |    ...    |   one frame samples     |
 *                 +-----------------------+    ...    +-------------------------+
 *                      1st channel             ...           N-th channel
 *
 * Outputs:
 *     oChannelBuffer:   the output buffer which contains the PCM data generated based on input data.
 *                The multi-channel output PCM data are ordered in the format below:
 *                 +-----------------------+    ...    +-------------------------+
 *                 |    one frame samples  |    ...    |   one frame samples     |
 *                 +-----------------------+    ...    +-------------------------+
 *                      1st channel             ...           N-th channel
 *
 * I/O:
 *     iopNrSamples:  the address of a variable which contains number of I/V data to
 *                    be processed.
 *                        The returned value indicates how many samples of I/V data are
 *                    not used in the I/V buffer. In this case, DSP framework should send
 *                    back the remaining I and V data in next process.
 *
 * Outputs:
 *     N/A
 *
 * Returns:
 *     DSM_API_OK: successful;
 *     Otherwise:  error codes.
 *
 *******************************************************************************/
extern DSM_API_MESSAGE DSM_API_generate_auxiliary_channels(
        void            *ipModuleHandler,
        int             iChannelMask,
        void            *iChannelBuffer,
        void            *oChannelBuffer,
        int             *iopNrSamples
);

/*******************************************************************************
 *    DSM_API_param_size_inquery()
 * 
 * Description:
 *     This function is used to inquiry the memory size of a parameter.
 * 
 * Inputs:
 *     cmd:      a 32-bit command ID.
 * Outputs:
 *     opBytes:  the memory size of the inquired parameter, in bytes.
 *     oDataType:the data type of the inquired parameter
 * Returns:
 *     DSM_API_OK: successful;
 *     Otherwise:  error codes.
 *
 *******************************************************************************/
extern DSM_API_MESSAGE DSM_API_param_size_inquery(
        int                 cmd, 
        int                 *opBytes,
        DSM_API_Data_Type   *oDataType
);

/*******************************************************************************
 *    DSM_API_get_excursion_log_data()
 *    DSM_API_get_temperature_log_data()
 * 
 * Description:
 *     These two functions are used to get log data which are recorded 
 * when excursion or coil temperature hit their thresholds respectively.
 * 
 * Inputs:
 *     ipModuleHandler:
 *              the handler of DSM module which is allocated by framework caller.
 *
 * Outputs:
 *     oLogDataBuf: the address of the log data in the format of structure dsm_param_log_t. 
 *
 * Returns:
 *     DSM_API_OK: successful;
 *     Otherwise:  error codes.
 *
 *******************************************************************************/
#ifdef ENABLE_DSM_LOG_DATA
extern DSM_API_MESSAGE DSM_API_get_excursion_log_data(
        void            *ipModuleHandler,
        dsm_param_log_t *oLogDataBuf
);

extern DSM_API_MESSAGE DSM_API_get_temperature_log_data(
        void            *ipModuleHandler,
        dsm_param_log_t *oLogDataBuf
);

void DSM_Get_Logging_Data(dsm_logging_data_t *log_data_ptr, void *pState);
#endif

/*******************************************************************************
 *    DSM_API_Set_Params()
 * 
 * Description:
 *     This function is used to set a serial of DSM parameters in one call.
 * 
 * Inputs:
 *     ipModuleHandler:
 *               the handler of DSM module which is allocated by framework caller.
 *     cmdNum:   the number of commands. The total memory size of this 
 *               input argument 
 *                    = (cmdNum * 2) * sizeof(int) bytes.
 *     ipParams: the buffer of input parameters which are stored in the format below.
 *               The parameters should be set separately for each channel.
 *               ipValue memory map:
 *               ---+-----------+-------------------
 *                  |   cmd_1   |  32-bits integer command
 *                  +-----------+
 *                  |  param_1  |  32-bits data
 *               ---+-----------+-------------------
 *                  |    ...    |
 *                  |    ...    |
 *                  |    ...    |
 *               ---+-----------+-------------------
 *                  |   cmd_N   |  32-bits integer command
 *                  +-----------+
 *                  |  param_N  |  N-th 32-bit data
 *               ---+-----------+-------------------
 *            Total: N parameters
 *
 * Outputs:
 *     N/A. 
 *
 * Returns:
 *     DSM_API_OK: successful;
 *     Otherwise:  error codes.
 *
 *******************************************************************************/
extern DSM_API_MESSAGE DSM_API_Set_Params(
        void    *ipModuleHandler,
        int      iCommandNumber,
        void    *ipParamsBuffer
);

/*******************************************************************************
 *    DSM_API_Get_Params()
 * 
 * Description:
 *     This function is used to get a serial of DSM parameters in one call.
 * 
 * Inputs:
 *     ipModuleHandler:
 *              the handler of DSM module which is allocated by framework caller.
 *     cmdNum:   the number of parameters. The total memory size of this 
 *               input argument 
 *                    = cmdNum * (1 + channel_number) ) * sizeof(int) bytes 
 *
 * Outputs:
 *     opParams: the buffer of the output parameters which are stored in the format below.
 *               opParams memory map:
 *             -----+-------------+-----------------------------
 *                  |    cmd_1    |  32-bits integer command Id
 *                  +-------------+
 *                  |  param_1_1  |  32-bits data of channel 1\
 *                  +-------------+                            \
 *                  +    ......   +                             -> parameters of N channels
 *                  +-------------+                            /
 *                  |  param_1_C  |  32-bits data of channel N/
 *             -----+-------------+-----------------------------
 *                  +    ......   +
 *                  +    ......   +
 *                  +    ......   +
 *             -----+-------------+-----------------------------
 *                  +    cmd_M    +  32-bits integer command Id
 *                  +-------------+
 *                  |  param_M_1  |  32-bits data of channel 1\
 *                  +-------------+                            \
 *                  +    ......   +                             -> parameters of N channels
 *                  +-------------+                            /
 *                  |  param_M_C  |  32-bits data of channel N/
 *             -----+-------------+------------------------------
 *            Total: M parameters of N channels
 * 
 * 			Exception: the parameter sizes of the commands
 * 	                       DSM_API_GET_FIRMWARE_BUILD_TIME      = 88, //C string
 *                         DSM_API_GET_FIRMWARE_BUILD_DATE      = 89, //C string
 *                         DSM_API_GET_FIRMWARE_VERSION         = 90, //C string
 *                         DSM_API_GET_CHIPSET_MODEL            = 91, //C string
 *               are DSM_MAX_STRING_PARAM_SIZE (=32) bytes rather than 4 bytes of 
 *               32-bits data because these parameters are in the format C string.
 * 
 * Returns:
 *     DSM_API_OK: successful;
 *     Otherwise:  error codes.
 *
 *******************************************************************************/
extern DSM_API_MESSAGE DSM_API_Get_Params(
        void    *ipModuleHandler,
        int      iCommandNum,
        void    *opParams
);


/*******************************************************************************
 *    DSM_API_Assign_Mutex()
 * 
 * Description:
 *     This function is used to set a mutex to each channel in a multithreading system.
 * 
 * Inputs:
 *     chanId:  a 32-bit channel ID starting from 0 through the maximum number of channels.
 *     iMutex:  the address of a mutex which is created by DSP framework.
 *
 * Outputs:
 *     N/A. 
 *
 * Returns:
 *     DSM_API_OK: successful;
 *     Otherwise:  error codes.
 *
 *******************************************************************************/
extern DSM_API_MESSAGE DSM_API_Assign_Mutex(
		void	*pHandle,
        int     chanId, 
        void    *iMutex
);

/*******************************************************************************
 *    iMutexLockFunc()
 * 
 * Description:
 *     This function is used to assign a mutex lock function in a multi-threading
 * system.
 * 
 * Inputs:
 *     iMutexLockFunc:  the address of the function which is used to lock a mutex.
 *                  This function should have only one input argument which passes
 *                  the address of a mutex.
 *
 * Outputs:
 *     N/A. 
 *
 * Returns:
 *     DSM_API_OK: successful;
 *     Otherwise:  error codes.
 *
 *******************************************************************************/
extern DSM_API_MESSAGE DSM_API_Assign_Mutex_Lock_function(
		void	*pHandle,
        void (*iMutexLockFunc)(void*)
);

/*******************************************************************************
 *    iMutexUnlockFunc()
 * 
 * Description:
 *     This function is used to assign a mutex unlock function in a multi-threading
 * system.
 * 
 * Inputs:
 *     iMutexUnlockFunc:  the address of the function which is used to unlock a mutex.
 *                  This function should have only one input argument which passes 
 *                  the address of a mutex.
 *
 * Outputs:
 *     N/A. 
 *
 * Returns:
 *     DSM_API_OK: successful;
 *     Otherwise:  error codes.
 *
 *******************************************************************************/
DSM_API_MESSAGE DSM_API_Assign_Mutex_Unlock_function(
		void	*pHandle,
        void 	(*iMutexUnlockFunc)(void*)
);


/***********************************************************************
 *   The Example Use of DSM APIs
 * 
 *     There are several steps to use the DSM APIs:
 * -- get the memory size which is required by DSM module
 * -- allocate memory for DSM module
 * -- set configuration parameters, if necessary
 * -- Execute DSM forward and feedback processing
 * -- retrieve some DSM parameters for debugging and monitoring 
 * 
 * ------------------------------
 * 
 * Step_1: allocate data memory for DSM memory
 * 
 *  #define MAX_CHANNELS        2
 * 
 * 	//request the size of memory which is allocated to DSM module
 *  void            *dsm_handler;
 *  int             memSize;
 *  DSM_API_MESSAGE   retCode;
 *  int             bufferSize[MAX_CHANNELS*4];
 * 	
 *  for(i=0; i<MAX_CHANNELS; i++)
 *  {
 *      bufferSize[4*i+0]     = 0;//input circular buffer size
 *      bufferSize[4*i+1]     = 0;//output circular buffer size
 *      bufferSize[4*i+2]     = 0;//I circular buffer size
 *      bufferSize[4*i+3]     = 0;//V circular buffer size
 *  }
 *
 * 	retCode = DSM_API_get_memory_size(MAX_CHANNELS, bufferSize, &memSize);
 *  if( DSM_API_OK == retCode ){
 * 		dsm_handler = (int*)malloc(memSize);
 * 		if( 0==dsm_handler ){
 * 			//error handling
 * 		}
 *  }
 *  else{
 * 		//error handling
 *  }
 * 
 * 	  The dsm_handler will be used by all of DSM API functions to specify the instance 
 * of a DSM module.
 * 
 * ------------------------------
 * 
 * Step_2: Initialize DSM module
 * 
 * 	//Initialize DSM module
 *  DSM_API_MESSAGE	retCode;
 *  int             frame_size;
 *  int             bufferSize[MAX_CHANNELS*4];
 *  int             delayedSamples[MAX_CHANNELS*4];
 *
 *
 *  // !!! the buffer sizes MUST be the same as used for calling DSM_API_get_memory_size()
 *
 *  for(i=0; i<MAX_CHANNELS; i++)
 *  {
 *      bufferSize[4*i+0]     = 0;//input circular buffer size
 *      bufferSize[4*i+1]     = 0;//output circular buffer size
 *      bufferSize[4*i+2]     = 0;//I circular buffer size
 *      bufferSize[4*i+3]     = 0;//V circular buffer size
 *
 *      delayedSamples[4*i+0] = 0;//input buffer delayed samples
 *      delayedSamples[4*i+1] = 0;//output buffer delayed samples
 *      delayedSamples[4*i+2] = 0;//I buffer delayed samples
 *      delayedSamples[4*i+3] = 0;//V buffer delayed samples
 *  }
 *
 *	retCode = DSM_API_Init(dsm_handler,
 *	                       SAMPLING_RATE_SUPPORTED,
 *	                       MAX_CHANNELS,
 *	                       bufferSize,
 *	                       (unsigned int *)&frame_size,
 *	                       delayedSamples
 *	                       );
 *  if( DSM_API_OK != retCode ){
 * 		//error handling
 *  }
 *  if( 0 == frame_size )
 * {
 *      //error handling
 * }
 * 
 * ------------------------------
 * 
 * Step_3: Set a group of DSM parameters in run-time
 * 
 *  #define MAX_CHANNELS            2//1=mono; 2=stereo
 *  #include "dsm_api.h"
 *
 *  DSM_API_MESSAGE	retCode;
 *  int             userParam[2*3];	//assuming to set 3 parameters
 *   
 * 
 * 
 *  //assume to set 3 parameters
 *  userParam[2*0  ] = DSM_SET_CMD_ID(2);    //set coil temperature limit
 *  userParam[2*0+1] = TO_FIX(80.0,19);     //set to 80C degree
 * 
 *  userParam[2*1]   = DSM_SET_CMD_ID(3);    //set excursion limit
 *  userParam[2*1+1] = TO_FIX(0.45,27);     //set to 0.45mm
 * 
 *  userParam[2*2]   = DSM_SET_CMD_ID(6);    //set default Rdc
 *  userParam[2*2+1] = TO_FIX(0.206, 27);   //set to 2.06
 * 
 *  retCode = DSM_API_Set_Params(dsm_handler, 3, userParam,);
 *  if( DSM_API_OK != retCode ){
 *      printf("DSM_API_Set_Params() failed: error code = %i\n", (int)retCode);
 * 		//error handling
 *  }
 * ------------------------------
 * 
 * Step_4: Process audio PCM data and feedback I-V data
 * 
 *  DSM_API_MESSAGE	retCode;
 *  int             remaining_samples, outputSamples;
 *  short           *pcmIn, *pcmOut;
 *  short           *iData, *vData;
 *
 *  remaining_samples  = input_samples;//the samples to be processed
 *  pcmIn              = (short*)???;//assign the 16-bit PCM input buffer
 *  pcmOut             = (short*)???;//assign the 16-bit PCM output buffer
 *
 *  iData              = (short*)???;//assign the 16-bit I buffer
 *  vData              = (short*)???;//assign the 16-bit V buffer
 * 
 *  //process DSM feedback path
 *  retCode = DSM_API_FB_process((void *)dsm_handler, iData, vData, &remaining_samples);
 *  if( DSM_API_OK != retCode ){
 *      //error handling
 *  }
 *  if( 0 != remaining_samples )
 *  {
 *      //store the remaining (input_samples-remaining_samples) samples
 *      //input data for processing in next call
 *  }
 * 
 * 	//process DSM forward path
 *  remaining_samples  = input_samples;//the samples to be processed
 *	retCode = DSM_API_FF_process((void *)dsm_handler, pcmIn, &remaining_samples, pcmOut, &outputSamples);
 *  if( DSM_API_OK != retCode ){
 * 		//error handling
 *  }
 *  if( 0 != remaining_samples ){
 *      //store the remaining (input_samples-remaining_samples) samples
 *      //input data for processing in next call
 *  }
 * 
 * 
 * ------------------------------
 * 
 * Step_5 (optional): Inquiry DSM parameters in run-time
 * 
 *  //parameter size is recommended to be inquired in run-time.
 *  long  			paramBytes, *paramBuf;
 *  DSM_Data_Type	dataType;
 *  DSM_API_MESSAGE	retCode;
 *
 *  //all parameters are in 32-bit containers except for 
 *  //commands 88, 89, 90 and 91 which are in C string format.
 *  retCode = DSM_API_param_size_inquery(cmdId, &paramBytes, &DSM_Data_Type);
 *  if( DSM_API_OK == retCode ){
 *  	if( paramBytes > 0 ){
 *    		paramBuf = malloc(paramBytes);
 *          if( o == paramBuf ){
 * 				//error handling
 *          }
 *  	}
 *  	else{
 * 			//error handling
 *  	}
 * 
 * 		//get/set DSM parameters
 * 		...
 * 
 * 		if( paramBuf )
 * 			free(paramBuf);
 *  }
 *  else{
 * 		//error handling
 *  }
 *
 * ------------------------------
 * 
 * Step_6: Get a group of DSM parameters in run-time
 * 
 *  DSM_API_MESSAGE	retCode;
 *  long    		dsmParam[(1+MAX_CHANNELS)*5];	//assuming to get 5 parameters of 32-bit integer
 *
 *   //get 5 adaptive DSM parameters
 *   *lPtr++ = DSM_API_GET_ADAPTIVE_FC;
 *    lPtr+=MAX_CHANNELS; //floating-point value stored here
 *    *lPtr++ = DSM_API_GET_ADAPTIVE_Q;
 *    lPtr+=MAX_CHANNELS; //floating-point value stored here
 *    *lPtr++ = DSM_API_GET_ADAPTIVE_DC_RES;
 *    lPtr+=MAX_CHANNELS; //floating-point value stored here
 *    *lPtr++ = DSM_API_GET_ADAPTIVE_COILTEMP;
 *    lPtr+=MAX_CHANNELS; //floating-point value stored here
 *    *lPtr++ = DSM_API_GET_EXCURSION;
 *    lPtr+=MAX_CHANNELS; //floating-point value stored here
 *
 *   //issue one command to get 5 run-time DSM parameters
 *    retCode = DSM_API_Get_Params(dsm_handler, 5, (void *)dsmParam);
 *    if (DSM_API_OK != retCode) {
 *        printf("DSM_API_Get_Params() failed: error code = %i\n", (int)retCode);
 *        return retCode;
 *    }
 * 
 * 
 *  (END of example codes)
 * 
 ***********************************************************************/


#ifdef __cplusplus
}
#endif

#endif
