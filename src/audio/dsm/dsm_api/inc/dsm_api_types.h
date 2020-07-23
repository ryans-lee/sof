#ifndef DSM_API_TYPES
#define DSM_API_TYPES

#include <time.h>
#include "dsm_api.h"
//#include <sof/audio/component.h>

#define MAX_DSM_PARAMS                          200
#define A_COEFF_LEN                             3
#define B_COEFF_LEN                             3

#define AVG_200MS_SIZE                          40
#define LOG_2S_SIZE	                            10

#define MOD2(x)                                 ((x+1)&0xFFFFFFFE)
#define MOD4(x)                                 ((x+3)&0xFFFFFFFC)
#define MOD8(x)                                 ((x+7)&0xFFFFFFF8)

#define DSM_MAX_MEM_BLOCKS_PER_CHANNEL          9

typedef enum{
    Filter_Type_parametric    = 0,
    Filter_Type_FLAT,
    Filter_Type_MUTE,
    Filter_Type_Low_pass,
    Filter_Type_High_pass,
    Filter_Type_ALLPASS,
    Filter_Type_BANDPASS,
    Filter_Type_Notch,
    Filter_Type_Low_Shelf,
    Filter_Type_High_Shelf
}dsmFilter_Type;


typedef struct{
    int     mem_size_bytes[DSM_MAX_MEM_BLOCKS_PER_CHANNEL];     //the size in bytes
}dsm_memBlock_size_per_chann_t;

typedef struct{
    void    *mem_addr_128bits[DSM_MAX_MEM_BLOCKS_PER_CHANNEL];  //128-bit alignment REQUIRED!!!
}dsm_memBlock_addr_per_chann_t;

//! The structure that keeps biquad filter coefficients
//! biquad filter coefficients structure
typedef struct _biqcoeff_t {
    //holds den coeff
    int                     a_coeff[MOD2(A_COEFF_LEN)]; // Q4.28
    //holds num coeff
    int                     b_coeff[MOD2(B_COEFF_LEN)]; // Q4.28
} biqcoeff_t;

typedef struct _dsm_speaker_params
{
    //holds den coeff
    biqcoeff_t spk_xc_extra_coeffQ28;
    float RTH1_f; //(51.5)
    float TCTH1_f; //(2)
    float RTH2_f; //(119)
    float TCTH2_f; //(138)
    float FIXED_ADMIT_A1_f; //TO_FIX(-1.86828671, FIX_ADMIT_QFORMAT)
    float FIXED_ADMIT_A2_f; //TO_FIX(0.87732655,  FIX_ADMIT_QFORMAT)
    float FIXED_ADMIT_B0_f; //TO_FIX(0.146768839, FIX_ADMIT_QFORMAT)
    float FIXED_ADMIT_B1_f; //TO_FIX(-0.287487241,FIX_ADMIT_QFORMAT)
    float FIXED_ADMIT_B2_f; //TO_FIX(0.142050629, FIX_ADMIT_QFORMAT)
} dsm_speaker_params_f;

typedef struct _dsm_param_log
{
   unsigned char coilTempProbPersistentQ0[LOG_2S_SIZE];
    unsigned short excurProbPersistentQ0[LOG_2S_SIZE];
    unsigned short rdcProbPersistentQ11[LOG_2S_SIZE];
    unsigned short freqProbPersistentQ4[LOG_2S_SIZE];
} dsm_param_log_t;

//This data structure contains current 5 adaptive parameters.
typedef struct{
    int     FcQ9;          //=DSM_API_GET_ADAPTIVE_FC                     = 16,   //32-bits data, Q9, in Hz,           valid floating-point range = [0, 4194304)
    int     QcQ29;         //=DSM_API_GET_ADAPTIVE_Q                      = 17,   //32-bits data, Q29,                 valid floating-point range = [0, 4)
    int     RdcQ27;        //=DSM_API_GET_ADAPTIVE_DC_RES                 = 18,   //32-bits data, Q27,                 valid floating-point range = [0, 16)
    int     tempQ19;       //=DSM_API_GET_ADAPTIVE_COILTEMP               = 19,   //32-bits data, Q19,                 valid floating-point range = [0, 4096)
    int     excurQ27;      //=DSM_API_GET_EXCURSION                       = 20,   //32-bits data, Q27,                 valid floating-point range = [0, 16)
    int     RdcQ19;                                                               //32-bits data, Q19,                 valid floating-point range = [0, 4096)
} Dsm_adp_params;


//This data structure holds statistics data of 5 adaptive parameters.
//After reading, the statistics of the 5 adaptive parameters will reset.
typedef struct{
    int         maxFcQ9;		//Q9, in Hz,           valid floating-point range = [0, 4194304)
    int         minFcQ9;        //Q9, in Hz,           valid floating-point range = [0, 4194304)

    int         maxQcQ29;       //Q29,                 valid floating-point range = [0, 4)
    int         minQcQ29;       //Q29,                 valid floating-point range = [0, 4)

    int         maxRdcQ27;      //Q27,                 valid floating-point range = [0, 16)
    int         minRdcQ27;      //Q27,                 valid floating-point range = [0, 16)

    int         maxTempQ19;     //Q19,
    int         minTempQ19;		//Q19
    float       avgTemp;       //floating-point,

    int         maxExcurQ27;    //Q27,                 valid floating-point range = [0, 16)
    int         minExcurQ27;    //Q27,                 valid floating-point range = [0, 16)

    int         maxAbsInputQ0;  //Q0
    int         maxAbsOutputQ0; //Q0

    int         maxAbsIDataQ0;  //Q0
    int         maxAbsVDataQ0;  //Q0

    int         spker_short_circuit_warning;
    int         spker_open_circuit_warning;
    int         calculated_load_resistanceQ19;

    float       elapsed_frames;
    int         exc_limiter_overshoots;
} Dsm_adp_statistics;

#if 1 //def ENABLE_DSM_LOG_DATA
typedef struct _dsm_user_log
{
    dsm_param_log_t afterExcurProbPersistent;
    //	unsigned int 	freqAfterExcurProbPersistent[LOG_2S_SIZE];
    //	unsigned char 	coilTempAfterExcurProbPersistent[LOG_2S_SIZE];
    //	unsigned char 	excurAfterExcurProbPersistent[LOG_2S_SIZE];
    //	unsigned short 	rdcAfterExcurProbPersistent[LOG_2S_SIZE];

    dsm_param_log_t afterTemperatureProbPersistent;
    //	unsigned int 	freqAfterTempProbPersistent[LOG_2S_SIZE];
    //	unsigned char 	coilTempAfterTempProbPersistent[LOG_2S_SIZE];
    //	unsigned char 	excurAfterTempProbPersistent[LOG_2S_SIZE];
    //	unsigned short 	rdcAfterTempProbPersistent[LOG_2S_SIZE];

    unsigned int tempLogStatus;
    unsigned int excurLogStatus;
    unsigned int RdcLogStatus;
    unsigned int FcLogStatus;

    unsigned int minTemp;
    unsigned int minExcur;
    unsigned int minRdc;
    unsigned int minFc;

    unsigned int maxTemp;
    unsigned int maxExcur;
    unsigned int maxRdc;
    unsigned int maxFc;

} dsm_user_log_t;
#endif

typedef struct _dsm_api_memory_size_ext
{
    int     iSamplingRate;                  //the sampling rate of input PCM samples, in Hz
                                            //supported sampling rates={48KHZ, 44.1KHz, 16KHZ}
    int     iChannels;                      //the number of maximum channels supported
    int     *ipCircBufferSizeBytes;         //the sizes of, if used, circular buffers, in samples
    int     oMemSizeRequestedBytes;         //the memory size required by DSM module, in bytes
    int     numEQFilters;                   //the number of EQ filters, up to 32
}dsm_api_memory_size_ext_t;

typedef struct _dsm_api_memory_size_ext2
{
    int     iSamplingRate;                  //the sampling rate of input PCM samples, in Hz
                                            //supported sampling rates={48KHZ, 44.1KHz, 16KHZ}
    int     iChannels;                      //the number of maximum channels supported
    int     *ipCircBufferSizeBytes;         //the sizes of, if used, circular buffers, in samples
    int     oMemSizeRequestedBytes;         //the memory size required by DSM module, in bytes
    int     numEQFilters;                   //the number of EQ filters, up to 32

    dsm_memBlock_size_per_chann_t   *opMemBlockSize;  //the address of a data structure dsm_memBlock_size_per_chann_t
}dsm_api_memory_size_ext2_t;

typedef struct _dsm_api_init_ext
{
    int     iSamplingRate;                  //the sampling rate of input PCM samples, in Hz
                                            //supported sampling rates={48KHZ, 44.1KHz, 16KHZ}
    int     iChannels;                      //the number of maximum channels supported
    int     oFF_FrameSizeSamples;           //the frame size of DSM feed-forward processing, in samples
    int     oFB_FrameSizeSamples;           //the frame size of DSM backward processing, in samples
    int     *ipCircBufferSizeBytes;         //the sizes of, if used, circular buffers, in samples
    int     *ipDelayedSamples;              //the delayed samples if circular buffers are used
    int     iSampleBitWidth;                //the number of bits of one input PCM sample, ={32, 16}
} dsm_api_init_ext_t;

typedef struct _dsm_api_init_ext2
{
    int     iSamplingRate;                  //the sampling rate of input PCM samples, in Hz
                                            //supported sampling rates={48KHZ, 44.1KHz, 16KHZ}
    int     iChannels;                      //the number of maximum channels supported
    int     oFF_FrameSizeSamples;           //the frame size of DSM feed-forward processing, in samples
    int     oFB_FrameSizeSamples;           //the frame size of DSM backward processing, in samples
    int     *ipCircBufferSizeBytes;         //the sizes of, if used, circular buffers, in samples
    int     *ipDelayedSamples;              //the delayed samples if circular buffers are used
    int     iSampleBitWidth;                //the number of bits of one input PCM sample, ={32, 16}

    dsm_memBlock_size_per_chann_t       *pMemBlockSize;
    dsm_memBlock_addr_per_chann_t       *pMemBlockAddr;//the address of the data structure dsm_memBlock_addr_per_chann_t

} dsm_api_init_ext2_t;

typedef union {
    struct{
        unsigned int     eq_coeff_update_enabled:1;      //=1: the commands 32 through 35 will take effect instead of commands 43 through 62
    }flag;
    int     data;
}dsm_control_flag_t;

typedef struct DSM_API_command_map{
    char    *paramName;
    int     paramID;
    int     paramQvalue;
} DSM_API_command_map_t;

typedef struct{
    int     cmdID;
    double  value;
}Dsm_cmd;

typedef struct{
    int     cmdID;
    int     fixed_point_value;
}Dsm_cmd_fixed;

typedef struct{
    Dsm_cmd         userCmd[MAX_DSM_PARAMS];
    Dsm_cmd_fixed   userCmdFixed[MAX_DSM_PARAMS];
    int             cmdNum;
}Dsm_user_cmd;

typedef struct DSM_API_message_map{
    char    *msgName;
    int     msgID;
} DSM_API_message_map_t;

typedef union _dsm_api_configParam
{
    int  paramArray[MAX_DSM_PARAMS];

    struct{
        int   GET_MAXIMUM_CMD_ID;//                  = 0,    //32-bits data, Q0
        int   SETGET_ENABLE;//                       = 1,    //32-bits data, Q0.                  valid floating-point range = {SMART_PA_DISABLE_MODE, SMART_PA_ENABLE_MODE, SMART_PA_BYPASS_ADD_MODE}
        int   SETGET_COILTEMP_THRESHOLD;//           = 2,    //32-bits data, Q19, in C degrees,   valid floating-point range = [0, 4096)
        int   SETGET_XCL_THRESHOLD;//                = 3,    //32-bits data, Q27, in millimeters, valid floating-point range = [0, 16)
        int   SETGET_LIMITERS_RELTIME;//             = 4,    //32-bits data, Q30, in seconds,     valid floating-point range = [0, 2)
        int   SETGET_MAKEUP_GAIN;//                  = 5,    //32-bits data, Q29, in %,           valid floating-point range = [0, 4)
        int   SETGET_RDC_AT_ROOMTEMP;//              = 6,    //32-bits data, Q27,                 valid floating-point range = [0, 16)
        int   SETGET_COPPER_CONSTANT;//              = 7,    //32-bits data, Q30,                 valid floating-point range = [0, 2)
        int   SETGET_COLDTEMP;//                     = 8,    //32-bits data, Q19,                 valid floating-point range = [0, 4096)
        int   SETGET_PITONE_GAIN;//                  = 9,    //32-bits data, Q31, in %,           valid floating-point range = [0, 1)
        int   SETGET_LEAD_RESISTANCE;//              = 10,   //32-bits data, Q27,                 valid floating-point range = [0, 16)
        int   SETGET_HPCUTOFF_FREQ;//                = 11,   //32-bits data, Q9, in Hz,           valid floating-point range = [0, 4194304)
        int   SETGET_LFX_GAIN;//                     = 12,   //32-bits data, Q30,                 valid floating-point range = [0, 2)

        //for impedance model updating
        int   SETGET_REF_FC;//                       = 13,   //32-bits data, Q9, in Hz,           valid floating-point range = [0, 4194304)
        int   SETGET_REF_Q;//                        = 14,   //32-bits data, Q29,                 valid floating-point range = [0, 4)
        int   INIT_F_Q_FILTERS;//                    = 15,   //32-bits data, Q0,                  valid floating-point range = {0, 1}

        //The following messages are read only
        int   GET_ADAPTIVE_FC;//                     = 16,   //32-bits data, Q9, in Hz,           valid floating-point range = [0, 4194304)
        int   GET_ADAPTIVE_Q;//                      = 17,   //32-bits data, Q29,                 valid floating-point range = [0, 4)
        int   GET_ADAPTIVE_DC_RES;//                 = 18,   //32-bits data, Q27,                 valid floating-point range = [0, 16)
        int   GET_ADAPTIVE_COILTEMP;//               = 19,   //32-bits data, Q19,                 valid floating-point range = [0, 4096)
        int   GET_EXCURSION;//                       = 20,   //32-bits data, Q27,                 valid floating-point range = [0, 16)

        //The following 4 commands are used the buffers used for the following commands must be allocated
        //by the caller function. The maxim buffer size is 4K bytes.
        int   GET_PCM_INPUT_DATA;//                  = 21,   //the buffer will be filled with 16-bit input PCM data to DSM module.
        int   GET_IV_DATA;//                         = 22,   //the buffer will be filled with I and V data.
        int   GET_PCM_AND_IV_DATA;//                 = 23,   //the buffer will be filled with PCM, I and V data. All data are in 16-bit integer.
        int   GET_PCM_OUTPUT_DATA;//                 = 24,   //the buffer will be filled with 16-bit output PCM data of DSM module.

        int   SETGET_VLIMIT_RELEASE_TIME;//          = 25,   //This command is obsolete.
        int   SETGET_VLIMIT;//                       = 26,   //32-bits data, Q27, in percentage.  valid floating-point range = [0, 16)
        int   SETGET_NEW_CONTROLS;//                 = 27,   //32-bits data, Q0,                  valid floating-point range = {dsm_control_flag_t}

        int   SETGET_PILOT_ENABLE;//                 = 28,   //32-bits data, Q0,                  valid floating-point range = {0, 1}
        int   SETGET_CLIP_ENABLE;//                  = 29,   //32-bits data, Q0,                  valid floating-point range = {0, 1}
        int   SETGET_EXC_ENABLE;//                   = 30,   //32-bits data, Q0,                  valid floating-point range = {0, 1}
        int   SETGET_THERMAL_ENABLE;//               = 31,   //32-bits data, Q0,                  valid floating-point range = {0, 1}

        int   SETGET_EQ_BAND_FC;//                   = 32,   //32-bits data, Q9, in Hz,           valid floating-point range = [0, 4194304)
        int   SETGET_EQ_BAND_Q;//                    = 33,   //32-bits data, Q29,                 valid floating-point range = [0, 4)
        int   SETGET_EQ_BAND_ATTENUATION_DB;//       = 34,   //32-bits data, Q20, in db,          valid floating-point range = [0, 2048)
        int   DSM_API_SETGET_TARGET_EQ_BAND_ID;//    = 35,   //32-bits data, Q0,                  valid floating-point range = {0, 1}
        int   SETGET_EQ_BAND_ENABLE;//               = 36,   //32-bits data, Q0. Bit field definitions: bit0:band1,bit1:band2,bit2:band3,bit3:band4, valid floating-point range = {0,...,15}
        int   GET_EQ_BAND_COEFFICIENTS;//            = 37,   //32-bits data, Q0.                  The array address of structure biqcoeff_t, the number of structures in the array is 8 by default.
                                                             //                                   Set value to 0 to disable the retrieval of coefficients of EQ filters.

        // ZIMP - impedance filter coefficients, for power from voltage
        int   GET_ZIMP_A1;//                         = 38,   //32-bits data, Q28,                 valid floating-point range = [0, 8)
        int   GET_ZIMP_A2;//                         = 39,   //32-bits data, Q28,                 valid floating-point range = [0, 8)
        int   GET_ZIMP_B0;//                         = 40,   //32-bits data, Q28,                 valid floating-point range = [0, 8)
        int   GET_ZIMP_B1;//                         = 41,   //32-bits data, Q28,                 valid floating-point range = [0, 8)
        int   GET_ZIMP_B2;//                         = 42,   //32-bits data, Q28,                 valid floating-point range = [0, 8)

        // EQ Biquad coefficients
        int   SETGET_EQ1_A1;//                       = 43,   //32-bits data, Q28,                 valid floating-point range = [0, 8)
        int   SETGET_EQ1_A2;//                       = 44,   //32-bits data, Q28,                 valid floating-point range = [0, 8)
        int   SETGET_EQ1_B0;//                       = 45,   //32-bits data, Q28,                 valid floating-point range = [0, 8)
        int   SETGET_EQ1_B1;//                       = 46,   //32-bits data, Q28,                 valid floating-point range = [0, 8)
        int   SETGET_EQ1_B2;//                       = 47,   //32-bits data, Q28,                 valid floating-point range = [0, 8)

        int   SETGET_FC_GUARD_BAND;//                = 48,   //32-bit data, Q9,  Fc guard band in Hz. Valid range = [0, 4194304)
        int   SETGET_FC_NARROW_RANGE;//              = 49,   //32-bit data, Q9,  Fc narrow range in Hz. Valid range = [0, 4194304)
        int   SETGET_FC_WIDE_RANGE;//                = 50,   //32-bit data, Q9,  Fc wide range in Hz. Valid range = [0, 4194304)

        int   SETGET_PPR_XOVER_FREQ_Hz;//            = 51,   //32-bit data, Q9, the cross-over frequency of PPR, in Hz. valid floating-point range = [0, 4194304)
        int   SETGET_EQ2_ENABLE_PPR;//               = 52,   //32-bit data, Q0, enable/disable PPR, valid floating-point range = {0,1}

        int   SETGET_EQ3_A1;//                       = 53,   //32-bits data, Q28,                 valid floating-point range = [0, 8)
        int   SETGET_EQ3_A2;//                       = 54,   //32-bits data, Q28,                 valid floating-point range = [0, 8)

        int   DSM_API_SETGET_MBDRC_ENABLE;//         = 55,   //32-bit data, Q0,  enable/disable a sub-band of DRC.
                                                             //Bit field definitions: bit0:DRC band1, bit1: DRC subband2, bit3:DRC subband3, valid floating-point range = {0, 1, 2, 3, ..., 7}
        int   DSM_API_SETGET_DRC_TRHESHOLD;//        = 56,   //32-bit data, Q20, DRC threshold in dB, valid floating-point range = {-120, 0]
        int   DSM_API_SETGET_DRC_RATIO;//            = 57,   //32-bit data, Q20, DRC compression ratio,valid floating-point range = [1.0, 1000.0]
        int   DSM_API_SETGET_DRC_ATTACK;//           = 58,   //32-bit data, Q20, DRC attack time in seconds, valid floating-point range = {0, 1}
        int   DSM_API_SETGET_DRC_RELEASE;//          = 59,   //32-bit data, Q20, DRC release time in seconds, valid floating-point range = {0, 1}
        int   DSM_API_SETGET_MBDRC_CUTOFF_F1;//      = 60,   //32-bit data, Q0, DRC low-middle subband cutoff frequency in Hz. valid floating-point range = {200, 1000}
        int   DSM_API_SETGET_MBDRC_CUTOFF_F2;//      = 61,   //32-bit data, Q0, DRC middle-high subband cutoff frequency in Hz. valid floating-point range ={1000, 4000}
        int   DSM_API_SETGET_MBDRC_PARAM_UPDATE;//   = 62,   //32-bit data, Q0, multi-band DRC channel ID. It is a control command to updates DRC parameters (Th, ratio, aTime,rTime,f1,f2).
                                                             //                 Bit field definitions: bit0 = band1; bit1 = band2; bit2 = band3, valid floating-point range = {1, 2, 4}

        int   SETGET_TRAJECTORY;//                   = 63,   //32-bits data, Q0,                  valid floating-point range = {0, 1}
        int   GET_GUARD_BEEN_MEAN_SCALE;//           = 64,   //32-bits data, Q30,                 valid floating-point range = [0, 2.0)

        int   SET_UPDATE_DELAY;//                    = 65,   //32-bits data, Q0,                  valid floating-point range = {0, 1}
        int   SETGET_DELAY;//                        = 66,   //32-bits data, Q0,                  valid floating-point range = [0, 0x7FFFFFFF]
        int   GET_MAXIMUM_DELAY;//                   = 67,   //32-bits data, Q0,                  valid floating-point range = [0, 0x7FFFFFFF]

        int   SETGET_ENABLE_FAST_FC;//               = 68,   //32-bits data, Q0,                  valid floating-point range = {0, 1}

        int   SETGET_ORM_QUALIFY_THRESH;//           = 69,   //32-bits data, Q0,  reuse obsolete MSG_ID "SETGET_MEASURE_DELAY" to facilitate ORM debugging

        int   SETGET_ENABLE_LOGGING;//               = 70,   //32-bits data, Q0,                  valid floating-point range = {0, 1}

        int   SETGET_SPEAKER_PARAM_LFX_A1;//         = 71,   //32-bits data, Q28,                 valid floating-point range = [0, 8)
        int   SETGET_SPEAKER_PARAM_LFX_A2;//         = 72,   //32-bits data, Q28,                 valid floating-point range = [0, 8)
        int   SETGET_SPEAKER_PARAM_LFX_B0;//         = 73,   //32-bits data, Q28,                 valid floating-point range = [0, 8)
        int   SETGET_SPEAKER_PARAM_LFX_B1;//         = 74,   //32-bits data, Q28,                 valid floating-point range = [0, 8)
        int   SETGET_SPEAKER_PARAM_LFX_B2;//         = 75,   //32-bits data, Q28,                 valid floating-point range = [0, 8)

        int   SETGET_SPEAKER_PARAM_TCTH1;//          = 76,   //32-bits data, Q20,                 valid floating-point range = [0, 2048)
        int   SETGET_SPEAKER_PARAM_TCTH2;//          = 77,   //32-bits data, Q20,                 valid floating-point range = [0, 2048)
        int   SETGET_SPEAKER_PARAM_RTH1;//           = 78,   //32-bits data, Q24,                 valid floating-point range = [0, 128)
        int   SETGET_SPEAKER_PARAM_RTH2;//           = 79,   //32-bits data, Q23,                 valid floating-point range = [0, 256)

        int   SETGET_SPEAKER_PARAM_ADMIT_A1;//       = 80,   //32-bits data, Q28,                 valid floating-point range = [0, 8)
        int   SETGET_SPEAKER_PARAM_ADMIT_A2;//       = 81,   //32-bits data, Q28,                 valid floating-point range = [0, 8)
        int   SETGET_SPEAKER_PARAM_ADMIT_B0;//       = 82,   //32-bits data, Q28,                 valid floating-point range = [0, 8)
        int   SETGET_SPEAKER_PARAM_ADMIT_B1;//       = 83,   //32-bits data, Q28,                 valid floating-point range = [0, 8)
        int   SETGET_SPEAKER_PARAM_ADMIT_B2;//       = 84,   //32-bits data, Q28,                 valid floating-point range = [0, 8)

        int   SETGET_SPEAKER_PARAM_UPDATE;//         = 85,   //unsigned 32-bit, Q0,               valid floating-point range = {0, 1}

        int   SETGET_FC_RESET_COUNT;//               = 86,   //unsigned 32-bit, Q0,               valid floating-point range = {0, 1}

        int   SETGET_GENERATE_SINE_WAVE;//           = 87,   //32-bit, Q0,                        valid floating-point range = {0, 1}

        // special APIs
        int   GET_FIRMWARE_BUILD_TIME;//             = 88,   //C string,                          valid maximum length of C string = DSM_TIME_MAX_SIZE
        int   GET_FIRMWARE_BUILD_DATE;//             = 89,   //C string,                          valid maximum length of C string = DSM_DATE_MAX_SIZE
        int   GET_FIRMWARE_VERSION;//                = 90,   //C string,                          valid maximum length of C string = DSM_VERSION_MAX_SIZE
        int   GET_CHIPSET_MODEL;//                   = 91,   //C string,                          valid maximum length of C string = DSM_CHIPSET_MAX_SIZE
        int   GET_ENDIAN;//                          = 92,   //32-bits data, Q0,                  valid value = {0xdeadbeef}

        int   SETGET_SINE_WAVE_GAIN;//               = 93,   //32-bits data, Q15,                 valid floating-point range = [0, 65536)

        int   SETGET_VIRTUAL_V_ENABLE;//             = 94,   //32-bits data, Q0,                  valid floating-point range = {0, 1}
        int   SETGET_FORCED_VIRTUAL_V;//             = 95,   //32-bits data, Q0,                  valid floating-point range = {0, 1}

        int   SETGET_I_SHIFT_BITS;//                 = 96,   //32-bits data, Q0,                  valid floating-point range = [0, 15]

        int   SETGET_EXC_FUNC_GAIN_ADJUSTED;//       = 97,   //32-bits data, Q0,                  valid floating-point range = {0, 1}

        int   SETGET_FADE_IN_TIME_MS;//              = 98,   //32-bits data, Q0,                  valid floating-point range = [0, 0x7FFFFFFF]
        int   SETGET_FADE_OUT_TIME_MS;//             = 99,   //32-bits data, Q0,                  valid floating-point range = [0, 0x7FFFFFFF]

        //This command takes effect only after
        //FADE_OUT_TIME_MS
        int   SETGET_FADE_OUT_MUTE_TIME_MS;//        = 100,  //32-bits data, Q0,                  valid floating-point range = [0, 0x7FFFFFFF]

        int   SETGET_FADE_IN_ENABLE;//               = 101,  //32-bits data, Q0,                  valid floating-point range = {0, 1}
        int   SETGET_FADE_OUT_ENABLE;//              = 102,  //32-bits data, Q0,                  valid floating-point range = {0, 1}

        int   SETGET_ENABLE_MULTICHAN_LINKING;//     = 103,  //32-bits data, Q0,                  valid floating-point range = {0, 1}

        int   SETGET_ENABLE_SMART_PT;//              = 104,  //32-bits data, Q0,                  valid floating-point range = {0, 1}
        int   SETGET_PILOTTONE_SILENCE_THRESHOLD;//  = 105,  //32-bits data, Q0,                  valid floating-point range = [0, 0x7FFFFFFF]
        int   SETGET_SILENCE_PILOTTONE_GAIN_Q31;//   = 106,  //32-bits data, Q31,                 valid floating-point range = [0, 1.0)
        int   SETGET_SILENCE_FRAMES;//               = 107,  //32-bits data, Q0,                  valid floating-point range = [-1, 0x7FFFFFFF], -1 indicates infinite.
        int   SETGET_PILOTTONE_TRANSITION_FRAMES;//  = 108,  //32-bits data, Q0,                  valid floating-point range = [-1, 0x7FFFFFFF], -1 indicates infinite.

        int   SETGET_LARGE_PILOTTONE_THRESHOLD;//    = 109,  //32-bits data, Q0,                 valid floating-point range = [0, 0x7FFFFFFF]
        int   SETGET_LARGE_PILOTTONE_GAIN_Q31;//     = 110,  //32-bits data, Q31,                valid floating-point range = [0, 1.0)

        int   SETGET_ENABLE_LINKWITZ_EQ;//           = 111,  //32-bits data, Q0,                 valid floating-point range = {0, 1}

        int   SETGET_CHANNEL_MASK;//                 = 112,  //32-bits data, Q0,                 valid floating-point range = [0, 0x00FF]
        int   SETGET_ENABLE_FF_FB_MODULES;//         = 113,  //32-bits data, Q0: bit0=FF module, bit1=FB module,valid floating-point range = {0, ..., 3}

        int   SETGET_ENABLE_CROSSOVER;//             = 114,  //32-bits data, Q0,                 valid floating-point range = {0, 1}

        int   SETGET_AUX_EQ_BAND_FC;//               = 115,  //32-bits data, Q9, in Hz,          valid floating-point range = [0, 4194304)
        int   SETGET_AUX_EQ_BAND_Q;//                = 116,  //32-bits data, Q29,                valid floating-point range = [0, 4)
        int   SETGET_AUX_EQ_BAND_ATTENUATION_DB;//   = 117,  //32-bits data, Q20, in db,         valid floating-point range = [0, 2048)
        int   SET_EQ_AUX_BAND_COEFF_UPDATE;//        = 118,  //32-bits data, Q0,                 valid floating-point range = {0, 1}
        int   SETGET_AUX_EQ_BAND_ENABLE;//           = 119,  //32-bits data, Q0. Bit field definitions: bit0:band1,bit1:band2,bit2:band3,bit3:band4, valid floating-point range = {0, ..., 15}

        int   SETGET_ENABLE_AUX_CROSSOVER;//         = 120,  //32-bits data, Q0,                 valid floating-point range = {0, 1}

        int   SETGET_ENABLE_AUX1_DELAYED_SAMPLES;//  = 121,  //32-bits data, Q0,                 valid floating-point range = [0, 0x7FFFFFFF]
        int   SETGET_ENABLE_AUX2_DELAYED_SAMPLES;//  = 122,  //32-bits data, Q0,                 valid floating-point range = [0, 0x7FFFFFFF]
        int   SETGET_ENABLE_AUX3_DELAYED_SAMPLES;//  = 123,  //32-bits data, Q0,                 valid floating-point range = [0, 0x7FFFFFFF]
        int   SETGET_ENABLE_AUX4_DELAYED_SAMPLES;//  = 124,  //32-bits data, Q0,                 valid floating-point range = [0, 0x7FFFFFFF]

        int   SETGET_SPEECH_GUARD_BINS;//            = 125,  //32-bits data, Q0,                 valid floating-point range = [0, 0x7FFFFFFF]
        int   SETGET_MEAN_SPEECH_THRESHOLD;//        = 126,  //32-bits data, Q30,                valid floating-point range = [0, 2)

        int   SETGET_HPCUTOFF_FREQ_AUX;//            = 127,  //32-bits data, Q9,                 valid floating-point range = [0, 4194304)
        int   SETGET_AUX_HP_FILTER_ENABLE;//         = 128,  //32-bits data, Q0,                 valid floating-point range = {0, 1}

        int   SETGET_STEREO_CROSSOVER_MODE;//        = 129,  //32-bit data, Q0,                  valid floating-point range = {0, 1}

        int   SETGET_ENABLE_UPDATE_FC_Q;//           = 130,  //32-bit data, Q0,                  valid floating-point range = {0, 1}

        int   SETGET_RECEIVER_PHYSICAL_LAYOUT;//     = 131,  //32-bit data, Q0,                  valid floating-point range = {LANDSCAPE_1_RCV_RIGHT, LANDSCAPE_2_RCV_LEFT}

        int   SETGET_MAX_V_SAMPLE;//                 = 132,  //32-bit data, Q0,                  valid floating-point range = [0, 32767], setting any number will reset the value to zero.
        int   SETGET_MAX_I_SAMPLE;//                 = 133,  //32-bit data, Q0,                  valid floating-point range = [0, 32767], setting any number will reset the value to zero.
        int   SETGET_MAX_INPUT_SAMPLE;//             = 134,  //32-bit data, Q0,                  valid floating-point range = [0, 32767], setting any number will reset the value to zero.
        int   SETGET_MAX_OUTPUT_SAMPLE;//            = 135,  //32-bit data, Q0,                  valid floating-point range = [0, 32767], setting any number will reset the value to zero.

        int   GET_ADAPTIVE_PARAMS;//                 = 136,  //32-bit data, Q0,                  the address of the data structure Dsm_adapt_params
        int   GET_ADAPTIVE_STATISTICS;//             = 137,  //32-bit data, Q0,                  the address of the data structure Dsm_adapt_statistics

        int   SETGET_SILENCE_UNMUTED_IN_SPT;//       = 138,  //32-bit data, Q0,                  valid floating-point range = {0, 1}
        int   SETGET_Q_ADJUSTMENT;//                 = 139,  //32-bit data, Q29,                 valid floating-point range = [0, 4)
        int   SETGET_XOVER_MIXING_ENABLED;//         = 140,  //32-bit data, Q0,                  valid floating-point range = {0, 1}
        int   DSM_API_SETGET_XOVER_BOOST_GAIN_PERCENTAGE;//  = 141,  //32-bit data, Q27,         valid floating-point range = [0, 16)

        int   DSM_API_SETGET_XOVER_FILTER_CUTOFF_FC;//= 142,  //32-bit data, Q9,                 valid floating-point range = [0, 4194304)
        int   DSM_API_SETGET_XOVER_FILTER_Q;//        = 143,  //32-bit data, Q29,                valid floating-point range = [0, 4)
        int   DSM_API_SET_XOVER_FILTER_UPDATE; //     = 144,  //32-bit data, Q0,                 valid floating-point range = {0, 1}

        int   DSM_API_SETGET_XOVER_SPKER_GAIN_PERCENTAGE;//= 145,  //32-bit data, Q27,           valid floating-point range = [0, 16)
    };
} dsm_api_configParam_t;

#endif	//DSM_API_TYPES
