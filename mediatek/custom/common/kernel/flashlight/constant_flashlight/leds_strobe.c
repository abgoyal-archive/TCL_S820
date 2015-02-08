#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/types.h>
#include <linux/wait.h>
#include <linux/slab.h>
#include <linux/fs.h>
#include <linux/sched.h>
#include <linux/poll.h>
#include <linux/device.h>
#include <linux/interrupt.h>
#include <linux/delay.h>
#include <linux/platform_device.h>
#include <linux/cdev.h>
#include <linux/errno.h>
#include <linux/time.h>
#include <linux/timer.h>
#include "kd_flashlight.h"
#include <asm/io.h>
#include <asm/uaccess.h>
#include "kd_camera_hw.h"

#if defined (CONFIG_ARCH_MT6577)
#include <mach/mt_pwm.h>
#include <mach/pmic_mt6329_hw_bank1.h>
#include <mach/pmic_mt6329_sw_bank1.h>
#include <mach/pmic_mt6329_hw.h>
#include <mach/pmic_mt6329_sw.h>
#include <mach/upmu_common_sw.h>
#include <mach/upmu_hw.h>
#endif

extern void mt_power_on (U32 pwm_no);
extern void mt_power_off (U32 pwm_no);
extern S32 mt_set_pwm_enable ( U32 pwm_no ) ;
extern S32 mt_set_pwm_disable ( U32 pwm_no ) ;
extern int s4AS3647_read_byte(u8 addr, u8 *data);
extern int s4AS3647_write_byte(u8 addr, u8 data);


/*******************************************************************************
* structure & enumeration
*******************************************************************************/
struct strobe_data{
    spinlock_t lock;
    wait_queue_head_t read_wait;
    struct semaphore sem;
};
/******************************************************************************
 * local variables
******************************************************************************/
static struct strobe_data strobe_private;

static u32 strobe_Res = 0;
static u32 strobe_Timeus = 0;
static int g_strobe_On = FALSE;
static u32 strobe_width = 100; //0 is disable
static eFlashlightState strobe_eState = FLASHLIGHTDRV_STATE_PREVIEW;
static int video_mode;

#define REGISTER_CURRENT_SET		0x01
#define LED_CURRENT				    0x7F

#define REGISTER_TXMASK 			0x03
#define EXT_TORCH_ON				0x10<<0
#define COIL_PEAK					0x01<<2
#define FLASH_TXMASK_CURRENT	0x06<<4

#define REGISTER_LOW_VOLTAGE 	0x04
#define VIN_LOW_V_RUN 			0x04<<0
#define VIN_LOW_V 				0x07<<3
#define VIN_LOW_V_SHUTDOWN 		0x00<<6
#define CONST_V_MODE 			0x00<<7


ssize_t gpio_FL_Init(void) 
{
    return 0;
}
ssize_t gpio_FL_Uninit(void) 
{
    return 0;
}
ssize_t gpio_FL_Enable(void) 
{
	int gpio_mode;
	static struct pwm_easy_config pwm_setting;
	//u8 as3647id7 = 0;
	//s4AS3647_read_byte(0x00, &as3647id);
	//printk("xxxxxxxxxxxxxxx read chipID:  register 0x00  = %d\n xxxxxxxxxxxxxx",as3647id);
	if(video_mode == 0) {
		printk("######################## high current mode 800mA #####################\n");
		s4AS3647_write_byte(0x07, 0xc0);
		s4AS3647_write_byte(REGISTER_CURRENT_SET, LED_CURRENT);
		s4AS3647_write_byte(REGISTER_TXMASK, EXT_TORCH_ON|COIL_PEAK|FLASH_TXMASK_CURRENT);
		s4AS3647_write_byte(REGISTER_LOW_VOLTAGE, VIN_LOW_V_RUN|VIN_LOW_V|VIN_LOW_V_SHUTDOWN|CONST_V_MODE);
		s4AS3647_write_byte(0x05, 0xf0);
		s4AS3647_write_byte(0x06, 0x0b);

		//msleep(10);
		mt_set_gpio_mode(66, GPIO_MODE_00);
		mt_set_gpio_dir(66, GPIO_DIR_OUT);
		mt_set_gpio_out(66, GPIO_OUT_ONE);
	}
	else {
		printk("######################## low current mode 100mA #####################\n");
		s4AS3647_write_byte(0x01, 0x10);
		s4AS3647_write_byte(0x03, 0x62);
		s4AS3647_write_byte(0x04, 0x00);
		s4AS3647_write_byte(0x05, 0x23);
		s4AS3647_write_byte(0x06, 0x00);
		s4AS3647_write_byte(0x07, 0x00);
		//msleep(10);
		mt_set_gpio_mode(85, GPIO_MODE_00);
		mt_set_gpio_dir(85, GPIO_DIR_OUT);
		mt_set_gpio_out(85, GPIO_OUT_ONE);
	}
	
	return 0;
}
    ssize_t gpio_FL_Disable(void) {
	mt_set_gpio_mode(85, GPIO_MODE_00);
	mt_set_gpio_dir(85, GPIO_DIR_OUT);
	mt_set_gpio_out(85, GPIO_OUT_ZERO);
	
	mt_set_gpio_mode(66, GPIO_MODE_00);
	mt_set_gpio_dir(66, GPIO_DIR_OUT);
	mt_set_gpio_out(66, GPIO_OUT_ZERO);

   	return 0;
    }

	ssize_t gpio_TORCH_Disable(void) {
	mt_set_gpio_mode(85, GPIO_MODE_00);
	mt_set_gpio_dir(85, GPIO_DIR_OUT);
	mt_set_gpio_out(85, GPIO_OUT_ZERO);

	mt_set_gpio_mode(66, GPIO_MODE_00);
	mt_set_gpio_dir(66, GPIO_DIR_OUT);
	mt_set_gpio_out(66, GPIO_OUT_ZERO);

	hwPowerDown(MT65XX_POWER_LDO_VCAM_IO, "flashlight");
   	return 0;
    }
	
    ssize_t gpio_FL_dim_duty(kal_uint8 duty) {
        return 0;
    }
	ssize_t gpio_TORCH_Enable(void)
	{
		static struct pwm_easy_config pwm_setting;
		
		u8 as3647id =0;
		mdelay(300);
		mt_set_gpio_mode(228, GPIO_MODE_00);
		mt_set_gpio_dir(228, GPIO_DIR_OUT);
		mt_set_gpio_out(228, GPIO_OUT_ZERO);

		if(TRUE != hwPowerOn(MT65XX_POWER_LDO_VCAM_IO, VOL_1800,"flashlight"))
			{
            		printk("[CAMERA SENSOR] Fail to enable analog power\n");
          		return -EIO;
      		  }   

		msleep(1);
		s4AS3647_write_byte(0x01, 0x30);
		s4AS3647_write_byte(0x03, 0x62);
		s4AS3647_write_byte(0x04, 0x00);
		s4AS3647_write_byte(0x05, 0x23);
		s4AS3647_write_byte(0x06, 0x00);
		s4AS3647_write_byte(0x07, 0x00);
		msleep(10);
		mt_set_gpio_mode(85, GPIO_MODE_00);
		mt_set_gpio_dir(85, GPIO_DIR_OUT);
		mt_set_gpio_out(85, GPIO_OUT_ONE);
		switch(strobe_width)
				{
				case 30:
					s4AS3647_write_byte(0x01, 0x10);
					break;
				case 60:
					s4AS3647_write_byte(0x01, 0x20);
					break;
				case 99:
					s4AS3647_write_byte(0x01, 0x2c);
					break;
				default :
    					break;
				}

		return 0;
	}

//abstract interface
#define FL_Init gpio_FL_Init
#define FL_Uninit gpio_FL_Uninit
#define FL_Enable gpio_FL_Enable
#define FL_Disable gpio_FL_Disable
#define FL_dim_duty gpio_FL_dim_duty
#define TORCH_select gpio_TORCH_Enable
#define TORCH_Disable gpio_TORCH_Disable
/*****************************************************************************
User interface
*****************************************************************************/
static int constant_flashlight_ioctl(MUINT32 cmd, MUINT32 arg)
{
    int i4RetValue = 0;
    int iFlashType = (int)FLASHLIGHT_NONE;

    switch(cmd)
    {
    	case FLASHLIGHTIOC_T_ENABLE :
		printk("xxxxxxxxxx enable arg = %d, width = %d xxxxxxxx\n", arg, strobe_width);
            if (arg && strobe_width) {
                //enable strobe watchDog timer.
                if(FL_Enable()){
                    printk("FL_Enable fail!\n");
                    goto strobe_ioctl_error;
                }
                g_strobe_On = TRUE;
           }
            else {
               if(FL_Disable()) {
                    printk("FL_Disable fail!\n");
                    goto strobe_ioctl_error;
                }
                g_strobe_On = FALSE;
            }
    		break;
        case FLASHLIGHTIOC_T_LEVEL:
					strobe_width = 100;
				if(1 == (int)arg || 12 == (int)arg || 20 == (int)arg)
					video_mode = 1;
				else
					video_mode = 0;
				printk("FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF level:%d FFFFFFFFFF\n",(int)arg);		

				//PK_DBG("level:%d \n",(int)arg);
				if (arg > 0)
				{
					if(FL_dim_duty((kal_int8)arg-1)) {
						//PK_ERR("FL_dim_duty fail!\n");
						//i4RetValue = -EINVAL;
						goto strobe_ioctl_error;
					}
				}
            break;
        case FLASHLIGHTIOC_T_FLASHTIME:
            strobe_Timeus = (u32)arg;
            printk("strobe_Timeus:%d \n",(int)strobe_Timeus);
            break;
        case FLASHLIGHTIOC_T_STATE:
            strobe_eState = (eFlashlightState)arg;
            break;
        case FLASHLIGHTIOC_G_FLASHTYPE:
            iFlashType = FLASHLIGHT_LED_CONSTANT;
            if(copy_to_user((void __user *) arg , (void*)&iFlashType , _IOC_SIZE(cmd)))
            {
                printk("[strobe_ioctl] ioctl copy to user failed\n");
                return -EFAULT;
            }
            break;
         case FLASHLIGHTIOC_X_SET_FLASHLEVEL:
				strobe_width = arg;
                printk("level:%d \n",(int)arg);
	     break;
	case FLASHLIGHTIOC_ENABLE_STATUS:
//		printk("**********g_strobe_on = %d \n", g_strobe_On);
		copy_to_user((void __user *) arg , (void*)&g_strobe_On , sizeof(int));
		break;
	case FLASHLIGHT_TORCH_SELECT:
		printk("@@@@@@FLASHLIGHT_TORCH_SELECT@@@@@@\n");
		if (arg && strobe_width){
			TORCH_select();
			g_strobe_On = TRUE;
		}
		else {
			if(TORCH_Disable()) {
                   	 printk("FL_Disable fail!\n");
                    	goto strobe_ioctl_error;
                }
                g_strobe_On = FALSE;
            }
		
		break;
    	default :
			printk("No such command \n");
    		i4RetValue = -EPERM;
    		break;
    }

    return i4RetValue;

strobe_ioctl_error:
    printk("Error or ON state!\n");
    return -EPERM;
}

static int constant_flashlight_open(void *pArg)
{
    int i4RetValue = 0;
    if (0 == strobe_Res) {
        FL_Init();
        
		//enable HW here if necessary
        if(FL_dim_duty(0)) {
            //0(weak)~31(strong)
            printk("FL_dim_duty fail!\n");
            i4RetValue = -EINVAL;
        }
    }

    spin_lock_irq(&strobe_private.lock);

    if(strobe_Res)
    {
        printk("busy!\n");
        i4RetValue = -EBUSY;
    }
    else
    {
        strobe_Res += 1;
    }

    //LED On Status
    //g_strobe_On = FALSE;

    spin_unlock_irq(&strobe_private.lock);

    return i4RetValue;
}

static int constant_flashlight_release(void *pArg)
{
    if (strobe_Res)
    {
        spin_lock_irq(&strobe_private.lock);

        strobe_Res = 0;
        strobe_Timeus = 0;

        //LED On Status
        //g_strobe_On = FALSE;

        spin_unlock_irq(&strobe_private.lock);

    	//un-init. HW here if necessary
        if(FL_dim_duty(0)) {
            printk("FL_dim_duty fail!\n");
        }

    	FL_Uninit();
    }

    return 0;
}

FLASHLIGHT_FUNCTION_STRUCT	constantFlashlightFunc=
{
	constant_flashlight_open,
	constant_flashlight_release,
	constant_flashlight_ioctl,
};

MUINT32 constantFlashlightInit(PFLASHLIGHT_FUNCTION_STRUCT *pfFunc) { 
    if (pfFunc!=NULL) {
        *pfFunc=&constantFlashlightFunc;
    }
    return 0;
}

ssize_t strobe_VDIrq(void)
{
	return 0;
}

EXPORT_SYMBOL(strobe_VDIrq);
