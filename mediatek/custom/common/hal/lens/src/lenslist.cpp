

#include <utils/Log.h>
#include <utils/Errors.h>
#include <fcntl.h>
#include <math.h>

#include "MediaHal.h"

//#include "lens_custom_cfg.h"
//#include "msdk_lens_exp.h"
#include "camera_custom_lens.h"
//#include "lens.h"
//nclude "image_sensor.h"
#include "kd_imgsensor.h"

#if defined(DUMMY_LENS)
extern PFUNC_GETLENSDEFAULT pDummy_getDefaultData;
#endif

#if defined(SENSORDRIVE)
extern PFUNC_GETLENSDEFAULT pSensorDrive_getDefaultData;
#endif

#if defined(FM50AF)
extern PFUNC_GETLENSDEFAULT pFM50AF_getDefaultData;
#endif

#if defined(DW9714)
extern PFUNC_GETLENSDEFAULT pDW_getDefaultData;
#endif

#if defined(MT9P017AF)
extern PFUNC_GETLENSDEFAULT pMT9P017AF_getDefaultData;
#endif

#if defined(OV8850AF)
extern PFUNC_GETLENSDEFAULT pOV8850AF_getDefaultData;
#endif



MSDK_LENS_INIT_FUNCTION_STRUCT LensList[MAX_NUM_OF_SUPPORT_LENS] =
{

#if defined(DUMMY_LENS)
	{DUMMY_SENSOR_ID, DUMMY_LENS_ID, "Dummy", pDummy_getDefaultData},
#endif

#if defined(SENSORDRIVE)
	{DUMMY_SENSOR_ID, SENSOR_DRIVE_LENS_ID, "kd_camera_hw", pSensorDrive_getDefaultData},	

    //  for backup lens, need assign correct SensorID
    //{OV5642_SENSOR_ID, SENSOR_DRIVE_LENS_ID, "kd_camera_hw", pSensorDrive_getDefaultData},
#endif

#if defined(FM50AF)
	{DUMMY_SENSOR_ID, FM50AF_LENS_ID, "FM50AF", pFM50AF_getDefaultData},
#endif

#if defined(DW9714)
	{OV8850DW9714_SENSOR_ID, DW9714_LENS_ID, "DW9714", pDW_getDefaultData},
#endif

    //  for new added lens, need assign correct SensorID
#if defined(MT9P017AF)
	{MT9P017MIPI_SENSOR_ID, MT9P017AF_LENS_ID, "MT9P017AF", pMT9P017AF_getDefaultData},
#endif

#if defined(OV8850AF)
	{OV8850_SENSOR_ID, OV8850AF_LENS_ID, "OV8850AF", pOV8850AF_getDefaultData},
#endif
};

UINT32 GetLensInitFuncList(PMSDK_LENS_INIT_FUNCTION_STRUCT pLensList)
{
    memcpy(pLensList, &LensList[0], sizeof(MSDK_LENS_INIT_FUNCTION_STRUCT)* MAX_NUM_OF_SUPPORT_LENS);
    return MHAL_NO_ERROR;
} // GetLensInitFuncList()






