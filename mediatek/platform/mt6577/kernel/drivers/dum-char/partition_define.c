#include <linux/module.h>
#include "partition_define.h"
struct excel_info PartInfo[PART_NUM]={
			{"preloader",262144,0x0, EMMC, 0,BOOT_1},
			{"dsp_bl",16515072,0x40000, EMMC, 0,BOOT_1},
			{"mbr",16384,0x1000000, EMMC, 0,USER},
			{"ebr1",32768,0x1004000, EMMC, 1,USER},
			{"pmt",4194304,0x100c000, EMMC, 0,USER},
			{"nvram",5242880,0x140c000, EMMC, 0,USER},
			{"seccfg",131072,0x190c000, EMMC, 0,USER},
			{"uboot",393216,0x192c000, EMMC, 0,USER},
			{"bootimg",6291456,0x198c000, EMMC, 0,USER},
			{"recovery",6291456,0x1f8c000, EMMC, 0,USER},
			{"sec_ro",6291456,0x258c000, EMMC, 2,USER},
			{"misc",393216,0x2b8c000, EMMC, 0,USER},
			{"logo",3145728,0x2bec000, EMMC, 0,USER},
			{"expdb",2097152,0x2eec000, EMMC, 0,USER},
			{"pro_info",3145728,0x30ec000, EMMC, 0,USER},
			{"custpack",433061888,0x33ec000, EMMC, 3,USER},
			{"mobile_info",8388608,0x1d0ec000, EMMC, 4,USER},
			{"android",433061888,0x1d8ec000, EMMC, 5,USER},
			{"cache",328204288,0x375ec000, EMMC, 6,USER},
			{"usrdata",2544893952,0x4aeec000, EMMC, 7,USER},
			{"otp",45088768,0xFFFF0200, EMMC, 0,USER},
			{"bmtpool",22020096,0xFFFF00a8, EMMC, 0,USER},
 };
EXPORT_SYMBOL(PartInfo);

#ifdef  MTK_EMMC_SUPPORT
struct MBR_EBR_struct MBR_EBR_px[MBR_COUNT]={
	{"mbr", {1, 2, 3, 4, }},
	{"ebr1", {5, 6, 7, }},
};

EXPORT_SYMBOL(MBR_EBR_px);
#endif

