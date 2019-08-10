
/**
 * @brief
 *  N1 对接 Infotmic 设备相关 API 接口定义。
 * @details
 *
 */

#include "n1_device.h"

#ifndef NK_N1_DEVICE_INFOTMIC_H_
#define NK_N1_DEVICE_INFOTMIC_H_
NK_CPP_EXTERN_BEGIN


#define NK_N1_INFOTMIC_ISP_VER (1)

typedef struct Nk_N1InfotmicISP {

	NK_Size version;

	union {
		struct {

			//; //Auto; //White; //Balance; //parameters; //(Planckian)
			NK_Float AWB_MAX_RATIO_DISTANCE; //0.2
			NK_Float AWB_USE_AWS_CONFIG; //0
			NK_Float WBT_TEMPORAL_STRETCH; //300
			NK_Float WBT_USE_FLASH_FILTERING; //0
			NK_Float WBT_USE_SMOOTHING; //0
			NK_Float WBT_WEIGHT_BASE; //2
			NK_Float WB_TARGET_TEMPERATURE; //6500

			//; //Light; //Based; //Controls; //parameters
			NK_Float LBC_CONFIGURATIONS; //0
			NK_Float LBC_UPDATE_SPEED; //0.1

			//; //Tone; //Mapper; //Control; //parameters
			NK_Float TNMC_ADAPTIVE; //0
			NK_Float TNMC_HIST_CLIP_MAX; //0.25
			NK_Float TNMC_HIST_CLIP_MIN; //0.035
			NK_Float TNMC_LOCAL; //0
			NK_Float TNMC_LOCALSTRENGTH; //0
			NK_Float TNMC_SMOOTHING; //0.4
			NK_Float TNMC_TEMPERING; //0.2
			NK_Float TNMC_UPDATE_SPEED; //0.5

			NK_Float TNMC_ENABLE_EQUALIZATION; //1
			NK_Float TNMC_ENABLE_GAMMA; //1
			NK_Float TNMC_EQUAL_BRIGHTSUPRESSRATIO; //0.01
			NK_Float TNMC_EQUAL_DARKSUPRESSRATIO; //0.05
			NK_Float TNMC_EQUAL_FACTOR; //0.05
			NK_Float TNMC_EQUAL_MAXBRIGHTSUPRESS; //0.15
			//NK_Float TNMC_EQUAL_MAXBRIGHTSUPRESS; //0.10
			NK_Float TNMC_EQUAL_MAXDARKSUPRESS; //0.15
			NK_Float TNMC_GAMMA_ACM; //1
			NK_Float TNMC_GAMMA_FCM; //1
			NK_Float TNMC_MAPCURVE_POWER_VALUE; //2
			NK_Float TNMC_MAPCURVE_UPDATE_DAMP; //0.3
			NK_Float TNMC_SMOOTH_HISTOGRAM_METHOD; //1
			NK_Float TNMC_BEZIER_CRV_MODE; //0
			NK_Float TNMC_BEZIER_CTRL_PNT; //0.95

			//; //Auto; //White; //Balance; //Statistics; //parameters; //(AWS; //block)
			NK_Float AWS_BB_DIST; //0.2
			NK_Float AWS_B_CLIP_THRESH; //128
			NK_Float AWS_B_DARK_THRESH; //8
			NK_Float AWS_CURVES_NUM; //5
			NK_Float AWS_CURVE_BOUNDARIES[5]; //0.25; //-0.223049; //-0.532458; //-0.996777; //-0.996777
			NK_Float AWS_CURVE_OFFSETS[5]; //8.13947e-018; //0.0475226; //0.117633; //-0.2; //-0.2
			NK_Float AWS_CURVE_X_COEFFS[5]; //0.809942; //0.696579; //0.622664; //0; //0
			NK_Float AWS_CURVE_Y_COEFFS[5]; //0.586511; //0.71748; //0.782489; //1; //1
			NK_Float AWS_DEBUG_MODE; //0
			NK_Float AWS_ENABLED; //0
			NK_Float AWS_G_CLIP_THRESH; //128
			NK_Float AWS_G_DARK_THRESH; //8
			NK_Float AWS_LOG2_B_QEFF; //0.328147
			NK_Float AWS_LOG2_R_QEFF; //0.812375
			NK_Float AWS_R_CLIP_THRESH; //128
			NK_Float AWS_R_DARK_THRESH; //8
			NK_Float AWS_TILE_SIZE[2]; //16; //16
			NK_Float AWS_TILE_START_COORDS[2]; //0; //0

			//; //Black; //Level; //Correction; //parameters
			NK_Float BLC_SENSOR_BLACK[4]; //18; //18; //18; //18
			NK_Float BLC_SYS_BLACK; //64

			//; //Colour; //Correction; //Matrix; //parameters
			NK_Float CCM_MATRIX[9]; //1.17176; //-0.137259; //-0.0345006; //-0.390281; //1.74745; //-0.357169; //-0.0882326; //-0.531796; //1.62003
			NK_Float CCM_OFFSETS[3]; //-140.592; //-147.781; //-147.032

			//; //Display; //Gamut; //Mapper; //parameters
			NK_Float DGM_CLIP_MAX; //1.5
			NK_Float DGM_CLIP_MIN; //-0.5
			NK_Float DGM_COEFF[6]; //0; //0; //0; //0; //0; //0
			NK_Float DGM_SLOPE[3]; //1; //1; //1
			NK_Float DGM_SRC_NORM; //1

			//; //Defective; //Pixels; //parameters
			NK_Float DPF_DETECT_ENABLE; //0
			NK_Float DPF_READ_MAP_ENABLE; //0
			NK_Float DPF_READ_MAP_FILE;
			NK_Float DPF_THRESHOLD; //0
			NK_Float DPF_WEIGHT; //16
			NK_Float DPF_WRITE_MAP_ENABLE; //0

			//; //Display; //pipeline; //Scaler; //parameters
			NK_Float DSC_ADJUST_CUTOFF_FREQ; //0
			NK_Float DSC_PITCH; //0; //0
			NK_Float DSC_RECT[4]; //0; //0; //1280; //960
			NK_Float DSC_RECT_TYPE; //outsize

			//; //Defective; //Pixels; //parameters
			NK_Float ENS_ENABLE; //0
			NK_Float ENS_REGION_NUMLINES; //16
			NK_Float ENS_SUBS_FACTOR; //1

			//; //Encoder; //pipeline; //Scaler; //parameters
			NK_Float ESC_ADJUST_CUTOFF_FREQ; //0
			NK_Float ESC_CHROMA_MODE; //inter
			NK_Float ESC_PITCH[2]; //0; //0
			NK_Float ESC_RECT[4]; //0; //0; //1280; //960
			NK_Float ESC_RECT_TYPE; //outsize

			//; //global; //and; //local; //histograms
			//NK_Float HIS_GLOBAL_ENABLE; //; //; //; //; //; //; //; //; //; //; //; //1
			//NK_Float HIS_GRID_START_COORDS; //; //; //; //; //; //; //; //0; //0
			//NK_Float HIS_GRID_TILE_DIMENSIONS; //; //; //; //; //50; //50
			//NK_Float HIS_INPUT_OFFSET; //; //; //; //; //; //; //; //; //; //; //; //; //256
			//NK_Float HIS_INPUT_SCALE; //; //; //; //; //; //; //; //; //; //; //; //; //; //32767
			//NK_Float HIS_REGIONAL_ENABLE; //; //; //; //; //; //; //; //; //; //1

			//; //Histogram; //Statistics; //parameters
			NK_Float HIS_GLOBAL_ENABLE; //1
			NK_Float HIS_REGIONAL_ENABLE; //1
			NK_Float HIS_INPUT_OFFSET; //256
			NK_Float HIS_INPUT_SCALE; //32767
			//for; //juan; //fish; //eye; //-; //godspeed
			//NK_Float HIS_GRID_START_COORDS; //129; //0
			//NK_Float HIS_GRID_TILE_DIMENSIONS; //146; //137
			//for; //juan; //fish; //eye; //-; //gary
			//NK_Float HIS_GRID_START_COORDS; //190; //80
			//NK_Float HIS_GRID_TILE_DIMENSIONS; //114; //114
			//for; //all; //screen
			//NK_Float HIS_GRID_START_COORDS; //0; //0
			//NK_Float HIS_GRID_TILE_DIMENSIONS; //182; //137
			//for; //juan; //fish; //eye; //2; //-; //gary
			NK_Float HIS_GRID_START_COORDS; //220; //100
			NK_Float HIS_GRID_TILE_DIMENSIONS; //136; //120

			//; //Imager; //Interface
			//NK_Float IIF_BAYER_FORMAT; //; //; //; //; //; //; //; //; //; //; //; //; //GRBG
			//NK_Float IIF_CAP_RECT_BR; //; //; //; //; //; //; //; //; //; //; //; //; //; //1279; //479; ////1278; //719
			//NK_Float IIF_CAP_RECT_TL; //; //; //; //; //; //; //; //; //; //; //; //; //; //640; //0
			//NK_Float IIF_DECIMATION; //; //; //; //; //; //; //; //; //; //; //; //; //; //; //1; //1

			//; //Wrong; //LCA; //setting
			//; //Lateral; //Chromatic; //Aberration; //parameters
			//NK_Float LCA_BLUEPOLY_X; //0.93; //-0.57; //-0.32
			//NK_Float LCA_BLUEPOLY_Y; //-0.3; //1.63; //-2.15
			//NK_Float LCA_BLUE_CENTER; //480; //272
			//NK_Float LCA_DEC; //0; //0
			//NK_Float LCA_REDPOLY_X; //0.11; //1.35; //-1.51
			//NK_Float LCA_REDPOLY_Y; //-0.41; //2.52; //-4.13
			//NK_Float LCA_RED_CENTER; //480; //272
			//NK_Float LCA_SHIFT; //0; //0

			//; //Main; //Gamut; //Mapper; //parameters
			NK_Float MGM_CLIP_MAX; //1.5
			NK_Float MGM_CLIP_MIN; //-0.5
			NK_Float MGM_COEFF[6]; //0; //0; //0; //0; //0; //0
			NK_Float MGM_SLOPE[3]; //1; //1; //1
			NK_Float MGM_SRC_NORM; //1

			//; //Image; //Enhancer; //parameters
			//NK_Float MIE_BLACK_LEVEL; //0.0625
			//NK_Float MIE_CASPECT_0; //0
			//NK_Float MIE_CASPECT_1; //0
			//NK_Float MIE_CASPECT_2; //0
			//NK_Float MIE_CCENTER_0; //0; //0
			//NK_Float MIE_CCENTER_1; //0; //0
			//NK_Float MIE_CCENTER_2; //0; //0
			//NK_Float MIE_CEXTENT_0; //0.5; //0.5; //0.5; //0.5
			//NK_Float MIE_CEXTENT_1; //0.5; //0.5; //0.5; //0.5
			//NK_Float MIE_CEXTENT_2; //0.5; //0.5; //0.5; //0.5
			//NK_Float MIE_CROTATION_0; //0
			//NK_Float MIE_CROTATION_1; //0
			//NK_Float MIE_CROTATION_2; //0
			//NK_Float MIE_ENABLED_0; //0
			//NK_Float MIE_ENABLED_1; //0
			//NK_Float MIE_ENABLED_2; //0
			//NK_Float MIE_MEMORY_COLOURS; //3
			//NK_Float MIE_OUT_BRIGHTNESS_0; //0
			//NK_Float MIE_OUT_BRIGHTNESS_1; //0
			//NK_Float MIE_OUT_BRIGHTNESS_2; //0
			//NK_Float MIE_OUT_CONTRAST_0; //1
			//NK_Float MIE_OUT_CONTRAST_1; //1
			//NK_Float MIE_OUT_CONTRAST_2; //1
			//NK_Float MIE_OUT_HUE_0; //0
			//NK_Float MIE_OUT_HUE_1; //0
			//NK_Float MIE_OUT_HUE_2; //0
			//NK_Float MIE_OUT_SATURATION_0; //1
			//NK_Float MIE_OUT_SATURATION_1; //1
			//NK_Float MIE_OUT_SATURATION_2; //1
			//NK_Float MIE_YGAINS_0; //0; //0; //0; //0
			//NK_Float MIE_YGAINS_1; //0; //0; //0; //0
			//NK_Float MIE_YGAINS_2; //0; //0; //0; //0
			//NK_Float MIE_YMAX_0; //1
			//NK_Float MIE_YMAX_1; //1
			//NK_Float MIE_YMAX_2; //1
			//NK_Float MIE_YMIN_0; //0
			//NK_Float MIE_YMIN_1; //0
			//NK_Float MIE_YMIN_2; //0

			//; //Output; //formats; //parameters
			NK_Float OUT_DE; //NONE
			NK_Float OUT_DE_HDF; //NONE
			NK_Float OUT_DE_POINT; //1
			NK_Float OUT_DE_RAW2D; //NONE
			NK_Float OUT_DISP; //NONE
			NK_Float OUT_DI_HDF; //NONE
			NK_Float OUT_ENC; //NV12

			//; //RGB; //to; //YUV; //parameters
			//NK_Float R2Y_BRIGHTNESS; //-0.02
			NK_Float R2Y_BRIGHTNESS; //0
			//NK_Float R2Y_CONTRAST; //1.05
			NK_Float R2Y_CONTRAST; //1.0
			NK_Float R2Y_HUE; //0
			NK_Float R2Y_MATRIX; //BT709
			NK_Float R2Y_OFFSETU; //0
			NK_Float R2Y_OFFSETV; //0
			NK_Float R2Y_RANGE_MUL; //1; //1; //1
			//NK_Float R2Y_SATURATION; //1.1
			//NK_Float R2Y_SATURATION; //1.2
			NK_Float R2Y_SATURATION; //1.15

			//; //Tone; //Mapper; //parameters
			NK_Float TNM_BYPASS; //0
			NK_Float TNM_COLOUR_CONFIDENCE; //1.0
			NK_Float TNM_COLOUR_SATURATION; //1.0
			NK_Float TNM_CURVE[63];
			//0.015625 0.03125 0.046875 0.0625 0.078125 0.09375 0.109375 0.125
			//0.140625 0.15625 0.171875 0.1875 0.203125 0.21875 0.234375 0.25
			//0.265625 0.28125 0.296875 0.3125 0.328125 0.34375 0.359375 0.375
			//0.390625 0.40625 0.421875 0.4375 0.453125 0.46875 0.484375 0.5
			//0.515625 0.53125 0.546875 0.5625 0.578125 0.59375 0.609375 0.625
			//0.640625 0.65625 0.671875 0.6875 0.703125 0.71875 0.734375 0.75
			//0.765625 0.78125 0.796875 0.8125 0.828125 0.84375 0.859375 0.875
			//0.890625 0.90625 0.921875 0.9375 0.953125 0.96875 0.984375
			NK_Float TNM_FLAT_FACTOR; //0.01
			NK_Float TNM_FLAT_MIN; //0.34
			NK_Float TNM_IN_Y[2]; //-64; //64
			//NK_Float TNM_OUT_Y; //16; //235
			NK_Float TNM_OUT_Y; //5; //245
			NK_Float TNM_WEIGHT_LINE; //0.075
			NK_Float TNM_WEIGHT_LOCAL; //0.2

			//; //White; //Balance; //Correction; //parameters
			NK_Float LSH_WBCLIP[4]; //1; //1; //1; //1
			NK_Float LSH_WBGAIN[4]; //1.76356; //1.00906; //1; //1.24545
			NK_Float LSH_CTRL_BITS_DIFF; //10
			NK_Float LSH_CTRL_BITS_DIFF_0; //0
			NK_Float LSH_CTRL_CORRECTIONS; //0
			//NK_Float LSH_CTRL_FILE_0; //; ///root/.ispddk/sc1135_direct_5000k_t32.lsh
			//NK_Float LSH_CTRL_SCALE_WB_0; //1.14
			//NK_Float LSH_CTRL_TEMPERATURE_0; //5000
			NK_Float LSH_MATRIX_ENABLE; //0

			//; //YUV; //to; //RGB; //parameters
			NK_Float Y2R_BRIGHTNESS; //0
			NK_Float Y2R_CONTRAST; //1
			NK_Float Y2R_HUE; //0
			NK_Float Y2R_MATRIX; //BT709
			NK_Float Y2R_OFFSETU; //-0.5
			NK_Float Y2R_OFFSETV; //-0.5
			NK_Float Y2R_RANGE_MUL[3]; //1; //1; //1
			NK_Float Y2R_SATURATION; //1

			NK_Float WB_CCM_0[9]; //1.17176; //-0.137259; //-0.0345006; //-0.390281; //1.74745; //-0.357169; //-0.0882326; //-0.531796; //1.62003
			NK_Float WB_CCM_1[9]; //1.14811; //-0.0798704; //-0.0682383; //-0.419256; //1.65839; //-0.239132; //-0.129013; //-0.556937; //1.68595
			NK_Float WB_CCM_2[9]; //1.37743; //-0.310579; //-0.0668551; //-0.509431; //1.98; //-0.536797; //-0.496437; //1; //0.357246
			NK_Float WB_CCM_3[9]; //0.874387; //0.397165; //-0.271552; //-0.706664; //1.98; //-0.322977; //-0.364745; //-0.809817; //1.98
			NK_Float WB_CORRECTIONS; //4
			NK_Float WB_GAINS_0[4]; //1.85534; //1.00957; //1; //1.18564
			NK_Float WB_GAINS_1[4]; //1.65318; //1.01042; //1; //1.37643
			NK_Float WB_GAINS_2[4]; //1.26019; //1.01385; //1; //1.72035
			NK_Float WB_GAINS_3[4]; //1.05871; //1.01464; //1; //2.03118
			NK_Float WB_OFFSETS_0[3]; //-140.592; //-147.781; //-147.032
			NK_Float WB_OFFSETS_1[3]; //-143.153; //-147.846; //-145.097
			NK_Float WB_OFFSETS_2[3]; //-137.698; //-124.498; //-103.368
			NK_Float WB_OFFSETS_3[3]; //-156.342; //-139.087; //-92.2819
			NK_Float WB_TEMPERATURE_0; //6500
			NK_Float WB_TEMPERATURE_1; //5000
			NK_Float WB_TEMPERATURE_2; //4000
			NK_Float WB_TEMPERATURE_3; //2800


			//; //Ungrouped; //parameters
			NK_Float AE_BRACKET_SIZE; //0.05
			NK_Float AE_ENABLE; //1
			NK_Float AE_FLICKER_AUTODETECT; //0
			NK_Float AE_FLICKER_FREQ; //50
			NK_Float AE_FLICKER_REJECTION; //1
			NK_Float AE_MAX_EXPOSURE; //60000
			//NK_Float AE_MAX_GAIN; //30
			NK_Float AE_MAX_GAIN; //20
			//NK_Float AE_TARGET_BRIGHTNESS; //-0.50
			NK_Float AE_TARGET_BRIGHTNESS; //0
			NK_Float AE_TARGET_GAIN; //1.79769e+308
			NK_Float AE_UPDATE_SPEED; //0.3
			NK_Float AF_DISTANCE; //0
			NK_Float AF_ENABLE; //0
			NK_Float AWB_MODE; //1

			//NK_Float DNS_COMBINE_ENABLE; //1
			NK_Float DNS_COMBINE_ENABLE; //0
			NK_Float DNS_GREYSC_PIXTHRESH_MULT; //0.25
			//NK_Float DNS_STRENGTH; //1.28
			NK_Float DNS_STRENGTH; //0
			NK_Float DNS_ISO_GAIN; //2
			NK_Float DNS_SENSOR_BITDEPTH; //10
			NK_Float DNS_WELL_DEPTH; //25000
			NK_Float DNS_READ_NOISE; //1.25

			NK_Float MIE_ENABLE; //0; //0; //0
			NK_Float MIE_VIB_ON; //0
			NK_Float MIE_VIB_SAT_CURVE; //0; //0.0322581; //0.0645161; //0.0967742; //0.129032; //0.16129; //0.193548; //0.225806; //0.258065; //0.290323; //0.322581; //0.354839; //0.387097; //0.419355; //0.451613; //0.483871; //0.516129; //0.548387; //0.580645; //0.612903; //0.645161; //0.677419; //0.709677; //0.741935; //0.774194; //0.806452; //0.83871; //0.870968; //0.903226; //0.935484; //0.967742; //1
			NK_Float MIE_VIB_SCPOINT_IN; //0.1
			NK_Float MIE_VIB_SCPOINT_IN2; //0.9
			NK_Float MIE_VIB_SCPOINT_OUT; //0.1
			NK_Float MIE_VIB_SCPOINT_OUT2; //0.9
			NK_Float SENSOR_SET_EXPOSURE; //70000
			NK_Float SENSOR_SET_GAIN; //2

			//; //Sharpening; //parameters
			NK_Float SHA_RADIUS; //5
			NK_Float SHA_STRENGTH; //0.8
			NK_Float SHA_THRESH; //0
			NK_Float SHA_DETAIL; //0.15
			NK_Float SHA_EDGE_SCALE; //0.10
			NK_Float SHA_EDGE_OFFSET; //-0.25
			//NK_Float SHA_DENOISE_BYPASS; //0
			//NK_Float SHA_DN_TAU_MULTIPLIER; //4.0
			//NK_Float SHA_DN_SIGMA_MULTIPLIER; //8.0
			NK_Float SHA_DENOISE_BYPASS; //1
			NK_Float SHA_DN_TAU_MULTIPLIER; //1
			NK_Float SHA_DN_SIGMA_MULTIPLIER; //1

			NK_Float TNM_ENABLE; //0



			//; //Color; //Mode; //Change; //Control; //parameters
			/////////////////////////////////////////Flat; //Color; //Mode
			NK_Float CMC_R2Y_BRIGHTNESS_FCM; //0.0
			NK_Float CMC_R2Y_CONTRAST_FCM; //1.0
			NK_Float CMC_R2Y_RANGEMULT_FCM[3]; //1; //1; //1
			NK_Float CMC_TNM_COLOUR_SATURATION_FCM; //0.0
			NK_Float CMC_AE_TARGET_BRIGHTNESS_FCM; //-0.60
			//NK_Float CMC_SEMSOR_MAX_GAIN_FCM; //30.0
			NK_Float CMC_SEMSOR_MAX_GAIN_FCM; //20.0

			//CMC; //FCM; //LEVEL; //0
			NK_Float CMC_DN_TARGET_IDX_FCM; //11
			NK_Float CMC_SHA_RADIUS_FCM; //5
			NK_Float CMC_SHA_STRENGTH_FCM; //0.8
			NK_Float CMC_SHA_THRESH_FCM; //0
			NK_Float CMC_SHA_DETAIL_FCM; //0.15
			NK_Float CMC_SHA_EDGE_SCALE_FCM; //0.10
			NK_Float CMC_SHA_EDGE_OFFSET_FCM; //-0.25
			NK_Float CMC_SHA_DN_BYPASS_DENOISE_FCM; //1
			NK_Float CMC_SHA_DN_TAU_MULTIPLIER_FCM; //4.0
			NK_Float CMC_SHA_DN_SIGMA_MULTIPLIER_FCM; //8.0
			NK_Float CMC_TNM_FLAT_FACTOR_FCM; //0.01
			NK_Float CMC_TNM_WEIGHT_LINE_FCM; //0.075
			NK_Float CMC_TNMC_EQUAL_BRIGHTSUPRESSRATIO_FCM; //0.002
			//CMC_TNMC_EQUAL_DARKSUPRESSRATIO_FCM; //0.06
			NK_Float CMC_TNMC_EQUAL_DARKSUPRESSRATIO_FCM; //0.03
			NK_Float CMC_TNMC_BEZIER_CRV_MODE_FCM; //1
			NK_Float CMC_TNMC_BEZIER_CTRL_PNT_FCM; //1.1

			//CMC; //FCM; //LEVEL; //1
			NK_Float CMC_DN_TARGET_IDX_FCM_LV1; //11
			NK_Float CMC_SHA_RADIUS_FCM_LV1; //5
			NK_Float CMC_SHA_STRENGTH_FCM_LV1; //0.8
			NK_Float CMC_SHA_THRESH_FCM_LV1; //0
			NK_Float CMC_SHA_DETAIL_FCM_LV1; //0.15
			NK_Float CMC_SHA_EDGE_SCALE_FCM_LV1; //0.10
			NK_Float CMC_SHA_EDGE_OFFSET_FCM_LV1; //-0.25
			NK_Float CMC_SHA_DN_BYPASS_DENOISE_FCM_LV1; //1
			NK_Float CMC_SHA_DN_TAU_MULTIPLIER_FCM_LV1; //1.0
			NK_Float CMC_SHA_DN_SIGMA_MULTIPLIER_FCM_LV1; //16
			NK_Float CMC_TNM_FLAT_FACTOR_FCM_LV1; //0.01
			NK_Float CMC_TNM_WEIGHT_LINE_FCM_LV1; //0.075
			NK_Float CMC_TNMC_EQUAL_BRIGHTSUPRESSRATIO_FCM_LV1; //0.002
			//NK_Float CMC_TNMC_EQUAL_DARKSUPRESSRATIO_FCM_LV1; //0.06
			NK_Float CMC_TNMC_EQUAL_DARKSUPRESSRATIO_FCM_LV1; //0.03
			NK_Float CMC_TNMC_BEZIER_CRV_MODE_FCM_LV1; //1
			NK_Float CMC_TNMC_BEZIER_CTRL_PNT_FCM_LV1; //1.1

			//CMC; //FCM; //LEVEL; //2
			NK_Float CMC_DN_TARGET_IDX_FCM_LV2; //11
			NK_Float CMC_SHA_RADIUS_FCM_LV2; //5
			NK_Float CMC_SHA_STRENGTH_FCM_LV2; //0.8
			NK_Float CMC_SHA_THRESH_FCM_LV2; //0
			NK_Float CMC_SHA_DETAIL_FCM_LV2; //0.15
			NK_Float CMC_SHA_EDGE_SCALE_FCM_LV2; //0.10
			NK_Float CMC_SHA_EDGE_OFFSET_FCM_LV2; //-0.25
			NK_Float CMC_SHA_DN_BYPASS_DENOISE_FCM_LV2; //1
			NK_Float CMC_SHA_DN_TAU_MULTIPLIER_FCM_LV2; //1.0
			NK_Float CMC_SHA_DN_SIGMA_MULTIPLIER_FCM_LV2; //16
			NK_Float CMC_TNM_FLAT_FACTOR_FCM_LV2; //0.0
			NK_Float CMC_TNM_WEIGHT_LINE_FCM_LV2; //0.075
			NK_Float CMC_TNMC_EQUAL_BRIGHTSUPRESSRATIO_FCM_LV2; //0.002
			//NK_Float CMC_TNMC_EQUAL_DARKSUPRESSRATIO_FCM_LV2; //0.06
			NK_Float CMC_TNMC_EQUAL_DARKSUPRESSRATIO_FCM_LV2; //0.03
			NK_Float CMC_TNMC_BEZIER_CRV_MODE_FCM_LV2; //1
			NK_Float CMC_TNMC_BEZIER_CTRL_PNT_FCM_LV2; //1.1


			///////////////////////////////////////////Advanced; //Color; //Mode
			//; //Sensor; //Gain; //level; //control.
			NK_Float CMC_SENSOR_GAIN_LEVEL_CTRL; //1
			NK_Float CMC_SENSOR_GAIN_LV1; //8.0
			NK_Float CMC_SENSOR_GAIN_LV2; //15.0

			//; //Measure; //temperature; //level; //control.
			NK_Float CMC_TEMPERATURE_LEVEL_CTRL; //1
			NK_Float CMC_TEMPERATURE_LV1; //5000.0
			NK_Float CMC_TEMPERATURE_LV2; //4000.0

			//CMC_SEMSOR_MAX_GAIN_ACM; //30.0
			NK_Float CMC_SEMSOR_MAX_GAIN_ACM; //20.0

			//CMC; //ACM; //LEVEL; //0
			NK_Float CMC_DN_TARGET_IDX_ACM; //11

			//CMC; //ACM; //LEVEL; //1
			NK_Float CMC_DN_TARGET_IDX_ACM_LV1; //11
			//NK_Float CMC_SHA_RADIUS_ACM_LV1; //2.0
			NK_Float CMC_SHA_RADIUS_ACM_LV1; //1.2
			//NK_Float CMC_SHA_STRENGTH_ACM_LV1; //0.7
			NK_Float CMC_SHA_STRENGTH_ACM_LV1; //0.3
			NK_Float CMC_SHA_THRESH_ACM_LV1; //0
			//NK_Float CMC_SHA_DETAIL_ACM_LV1; //0.15
			NK_Float CMC_SHA_DETAIL_ACM_LV1; //0.0
			NK_Float CMC_SHA_EDGE_SCALE_ACM_LV1; //0.08
			NK_Float CMC_SHA_EDGE_OFFSET_ACM_LV1; //-0.25
			NK_Float CMC_SHA_DN_BYPASS_DENOISE_ACM_LV1; //1
			NK_Float CMC_SHA_DN_TAU_MULTIPLIER_ACM_LV1; //2.0
			NK_Float CMC_SHA_DN_SIGMA_MULTIPLIER_ACM_LV1; //8.0
			//NK_Float CMC_TNM_FLAT_FACTOR_ACM_LV1; //0.01
			//NK_Float CMC_TNM_WEIGHT_LINE_ACM_LV1; //0.075
			NK_Float CMC_TNM_FLAT_FACTOR_ACM_LV1; //0.0
			NK_Float CMC_TNM_WEIGHT_LINE_ACM_LV1; //0.0
			NK_Float CMC_TNMC_EQUAL_BRIGHTSUPRESSRATIO_ACM_LV1; //0.002
			//NK_Float CMC_TNMC_EQUAL_DARKSUPRESSRATIO_ACM_LV1; //0.06
			NK_Float CMC_TNMC_EQUAL_DARKSUPRESSRATIO_ACM_LV1; //0.03
			NK_Float CMC_TNMC_BEZIER_CRV_MODE_ACM_LV1; //0
			NK_Float CMC_TNMC_BEZIER_CTRL_PNT_ACM_LV1; //0.95

			//CMC; //ACM; //LEVEL; //2
			NK_Float CMC_DN_TARGET_IDX_ACM_LV2; //11
			NK_Float CMC_SHA_RADIUS_ACM_LV2; //1.2
			//NK_Float CMC_SHA_STRENGTH_ACM_LV2; //0.5
			NK_Float CMC_SHA_STRENGTH_ACM_LV2; //0
			NK_Float CMC_SHA_THRESH_ACM_LV2; //0
			//NK_Float CMC_SHA_DETAIL_ACM_LV2; //0.15
			NK_Float CMC_SHA_EDGE_SCALE_ACM_LV2; //0.01
			NK_Float CMC_SHA_EDGE_OFFSET_ACM_LV2; //-0.25
			NK_Float CMC_SHA_DN_BYPASS_DENOISE_ACM_LV2; //1
			NK_Float CMC_SHA_DN_TAU_MULTIPLIER_ACM_LV2; //1.0
			NK_Float CMC_SHA_DN_SIGMA_MULTIPLIER_ACM_LV2; //16
			NK_Float CMC_TNM_FLAT_FACTOR_ACM_LV2; //0.0
			//NK_Float CMC_TNM_WEIGHT_LINE_ACM_LV2; //0.075
			NK_Float CMC_TNM_WEIGHT_LINE_ACM_LV2; //0.0
			NK_Float CMC_TNMC_EQUAL_BRIGHTSUPRESSRATIO_ACM_LV2; //0.002
			//NK_Float CMC_TNMC_EQUAL_DARKSUPRESSRATIO_ACM_LV2; //0.06
			NK_Float CMC_TNMC_EQUAL_DARKSUPRESSRATIO_ACM_LV2; //0.03
			NK_Float CMC_TNMC_BEZIER_CRV_MODE_ACM_LV2; //0
			NK_Float CMC_TNMC_BEZIER_CTRL_PNT_ACM_LV2; //0.95


			//; //Control; //AE; //parameters
			///////////////////////////////////////
			NK_Float AE_TARGET_MOVE_METHOD; //0
			NK_Float AE_TARGET_MAX; //-0.10
			NK_Float AE_TARGET_MIN; //-0.20
			NK_Float AE_TARGET_GAIN_NRML_MODE_MAX_LMT; //4.0
			NK_Float AE_TARGRT_MOVE_STEP; //0.002
			NK_Float AE_TARGET_MIN_LMT; //-0.1
			//NK_Float AE_TARGET_UP_OVER_THRESHOLD; //0.05
			NK_Float AE_TARGET_UP_OVER_THRESHOLD; //0.02
			NK_Float AE_TARGET_UP_UNDER_THRESHOLD; //0.45
			//NK_Float AE_TARGET_DN_OVER_THRESHOLD; //0.1
			NK_Float AE_TARGET_DN_OVER_THRESHOLD; //0.05
			NK_Float AE_TARGET_DN_UNDER_THRESHOLD; //0.9
			NK_Float AE_TARGET_EXPOSURE_METHOD; //1
			NK_Float AE_TARGET_LOWLUX_GAIN_ENTER; //5.0
			NK_Float AE_TARGET_LOWLUX_GAIN_EXIT; //3.0
			NK_Float AE_TARGET_LOWLUX_EXPOSURE_ENTER; //59500
			//NK_Float AE_TARGET_LOWLUX_FPS; //10.0
			NK_Float AE_TARGET_LOWLUX_FPS; //12.5
			NK_Float AE_TARGET_NORMAL_FPS; //15.0
			NK_Float AE_BRIGHTNESS_METERING_METHOD; //0
			NK_Float AE_REGION_DUCE; //0.65

		} V1;

	};

} NK_N1InfotmicISP;



/**
 * @brief
 *  N1 对接 Infotmic 设备相关事件定义。
 */
typedef struct Nk_N1DeviceEventInfotmic {

	/**
	 * @brief
	 *  获取 ISP 参数事件。
	 *
	 */
	NK_Void
	(*onGetISP)(NK_PVoid ctx, NK_N1InfotmicISP *ISP);

	/**
	 * @brief
	 *  设置 ISP 参数事件。
	 *
	 */
	NK_Void
	(*onSetISP)(NK_PVoid ctx, NK_N1InfotmicISP *ISP);


} NK_N1DeviceEventInfotmic;

/**
 * @brief
 *  配置 N1 设备生产相关事件。
 * @details
 *
 * @param Event [in]
 *  用户事件定义。
 *
 * @return
 *  配置成功返回 0，否则返回 -1。
 */
NK_API NK_Int
NK_N1Device_Infotmic(NK_N1DeviceEventInfotmic *Event);



NK_CPP_EXTERN_END
#endif /* NK_N1_DEVICE_INFOTMIC_H_ */
