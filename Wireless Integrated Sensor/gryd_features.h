#ifndef GRYD_FEATURES_H_
#define GRYD_FEATURES_H_

/*
 * features enabled based on the project
 * PIR and ALS FEATURE
 * DALI FETAURE
 * SNSR_F1024KB_v_5_2(schematic version 5.2)
 *
 * by default : TH FEATURE is always enabled
 */

/******************************************************************************/
#define PIR_FEATURE
#define ALS_FEATURE
#define TH_FEATURE
/******************************************************************************/
#undef GSLINK_FEATURE
#undef ANALOG_FEATURE
#define DALI_FEATURE
#undef TRIAC_FEATURE
/******************************************************************************/
#undef SNSR_F1024KB
#define SNSR_F1024KB_v_5_2
#undef SNSR_F512KB_v_5_2

#define DALI_SPACE			//added a triac control feature to switch off the DALI driver when the lights are off.

#endif /* GRYD_FEATURES_H_ */
