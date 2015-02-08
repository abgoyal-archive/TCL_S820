

/**************************************************************************************************/
/* 																				Date : 04/2006 */
/* 						   PRESENTATION 													   */
/* 			 Copyright (c) 2012 JRD Communications, Inc.									   */
/**************************************************************************************************/
/* 																							   */
/*    This material is company confidential, cannot be reproduced in any						   */
/*    form without the written permission of JRD Communications, Inc.							   */
/* 																							   */
/*================================================================================================*/
/*   Author : Yuan Xiang																	   */
/*   Role :	Idle Screen 																	   */
/*   Reference documents : TN Idle Screen														   */
/*================================================================================================*/
/* Comments :																					   */
/* 	file	: 										   */
/* 	Labels	:																				   */
/*================================================================================================*/
/* Modifications	(month/day/year)															   */
/*================================================================================================*/
/* date	| author	   |FeatureID					|modification							   */
/*=========|==============|============================|==========================================*/
/*01/29/13 |Yuan Xiang    |PR400439-Yuanxiang-001 | No “Maker”and “Model”in image details screen*/
/*=========|==============|============================|==========================================*/



#include <stdlib.h>
#include <stdio.h>
#include "camera_custom_if.h"

//PR395992-yuanxiang-begin
#define LOG_TAG "custm_SetExif"
#include <utils/Log.h>
//PR395992-yuanxiang-end

namespace NSCamCustom
{
#include "cfg_tuning.h"
#include "cfg_facebeauty_tuning.h"
#include "flicker_tuning.h"
//
#include "cfg_setting_imgsensor.h"
#include "cfg_tuning_imgsensor.h"
#define EN_CUSTOM_EXIF_INFO
//PR400439-yuanxiang-begin
//MINT32 custom_SetExif(void **ppCustomExifTag)
//{
//#ifdef EN_CUSTOM_EXIF_INFO
//#define CUSTOM_EXIF_STRING_MAKE  "custom make"
//#define CUSTOM_EXIF_STRING_MODEL "custom model"
//#define CUSTOM_EXIF_STRING_SOFTWARE "custom software"
//static customExifInfo_t exifTag = {CUSTOM_EXIF_STRING_MAKE,CUSTOM_EXIF_STRING_MODEL,CUSTOM_EXIF_STRING_SOFTWARE};
MINT32 custom_SetExif(void **ppCustomExifTag)
{
#ifdef EN_CUSTOM_EXIF_INFO
    char model[32];
    char manufacturer[32];
    property_get("ro.product.display.model", model, "default");
    ALOGI("custom_SetExif model= %s",model);
    property_get("ro.product.manufacturer", manufacturer, "default");
    ALOGI("custom_SetExif manufacturer= %s",manufacturer);
    //#define CUSTOM_EXIF_STRING_MAKE  "custom make"
    //#define CUSTOM_EXIF_STRING_MODEL "custom model"
    static customExifInfo_t exifTag = { 0 };
    for (int i = 0; i < 32; i++) {
        if (model[i] != '\0' && i < strlen(model)) {
            exifTag.strModel[i] = (unsigned char) model[i];
        } else {
            exifTag.strModel[i] = '\0';
        }
        if (manufacturer[i] != '\0' && i < strlen(manufacturer)) {
            exifTag.strMake[i] = (unsigned char) manufacturer[i];
        } else {
            exifTag.strMake[i] = '\0';
        }
    }
//PR400439-yuanxiang-end
    if (0 != ppCustomExifTag) {
        *ppCustomExifTag = (void*)&exifTag;
    }
    return 0;
#else
    return -1;
#endif
}

MUINT32
getLCMPhysicalOrientation()
{
    return ::atoi(MTK_LCM_PHYSICAL_ROTATION); 
}

#define FLASHLIGHT_CALI_LED_GAIN_PRV_TO_CAP_10X 50
MUINT32 custom_GetFlashlightGain10X(void)
{
    // x10 , 1 mean 0.1x gain
    //10 means no difference. use torch mode for preflash and cpaflash
    //> 10 means capture flashlight is lighter than preflash light. < 10 is opposite condition.

    return (MUINT32)FLASHLIGHT_CALI_LED_GAIN_PRV_TO_CAP_10X;
}

MUINT32 custom_BurstFlashlightGain10X(void)
{
	return FLASHLIGHT_CALI_LED_GAIN_PRV_TO_CAP_10X;
}

};  //NSCamCustom

