/***************************************************
 * File:touch.c
 * Copyright (c)  2008- 2030  oplus Mobile communication Corp.ltd.
 * Description:
 *             tp dev
 * Version:1.0:
 * Date created:2016/09/02
 * TAG: BSP.TP.Init
*/

#include <linux/errno.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/input.h>
#include <linux/serio.h>
#include "oplus_touchscreen/tp_devices.h"
#include "oplus_touchscreen/touchpanel_common.h"
#include <soc/oplus/system/oplus_project.h>
#include <soc/oplus/device_info.h>
#include "touch.h"

#define MAX_LIMIT_DATA_LENGTH         100
#define S3706_FW_NAME_18073 "tp/18073/18073_FW_S3706_SYNAPTICS.img"
#define S3706_BASELINE_TEST_LIMIT_NAME_18073 "tp/18073/18073_Limit_data.img"
#define S3706_FW_NAME_19301 "tp/19301/19301_FW_S3706_SYNAPTICS.img"
#define S3706_BASELINE_TEST_LIMIT_NAME_19301 "tp/19301/19301_Limit_data.img"
#define GT9886_FW_NAME_19551 "tp/19551/FW_GT9886_SAMSUNG.img"
#define GT9886_BASELINE_TEST_LIMIT_NAME_19551 "tp/19551/LIMIT_GT9886_SAMSUNG.img"
#define GT9886_FW_NAME_19357 "tp/19357/FW_GT9886_SAMSUNG.img"
#define GT9886_BASELINE_TEST_LIMIT_NAME_19357 "tp/19357/LIMIT_GT9886_SAMSUNG.img"
#define GT9886_FW_NAME_19537 "tp/19537/FW_GT9886_SAMSUNG.img"
#define GT9886_BASELINE_TEST_LIMIT_NAME_19537 "tp/19537/LIMIT_GT9886_SAMSUNG.img"
#define TD4330_NF_CHIP_NAME "TD4330_NF"

extern char *saved_command_line;
int g_tp_dev_vendor = TP_UNKNOWN;
int g_tp_prj_id = 0;
int ret = 0;

/*if can not compile success, please update vendor/oppo_touchsreen*/
struct tp_dev_name tp_dev_names[] =
{
	{TP_OFILM, "OFILM"},
	{TP_BIEL, "BIEL"},
	{TP_TRULY, "TRULY"},
	{TP_BOE, "BOE"},
	{TP_G2Y, "G2Y"},
	{TP_TPK, "TPK"},
	{TP_JDI, "JDI"},
	{TP_TIANMA, "TIANMA"},
	{TP_SAMSUNG, "SAMSUNG"},
	{TP_DSJM, "DSJM"},
	{TP_BOE_B8, "BOEB8"},
	{TP_INNOLUX, "INNOLUX"},
	{TP_HIMAX_DPT, "DPT"},
	{TP_AUO, "AUO"},
	{TP_DEPUTE, "DEPUTE"},
	{TP_UNKNOWN, "UNKNOWN"},
};

#define GET_TP_DEV_NAME(tp_type) ((tp_dev_names[tp_type].type == (tp_type))?tp_dev_names[tp_type].name:"UNMATCH")


bool __init tp_judge_ic_match(char *tp_ic_name)
{
	int prj_id = 0;
	pr_err("[TP] tp_ic_name = %s \n", tp_ic_name);
#ifdef CONFIG_MACH_MT6765
	prj_id = 21331;
#else
	prj_id = get_project();
#endif
	switch(prj_id)
	{
	case 132769:
		pr_info("[TP] tp judge ic forward for 206A1\n");
		if (strstr(tp_ic_name, "nf_nt36525b") && strstr(saved_command_line, "nt36525b_hlt"))
		{
			return true;
		}
		else if (strstr(tp_ic_name, "hx83102d_nf") && strstr(saved_command_line, "hx83102d"))
		{
			return true;
		}
		else if (strstr(tp_ic_name, "ili9881h") && strstr(saved_command_line, "ilt9881h"))
		{
			return true;
		}
		pr_err("[TP] ERROR! ic is not match driver\n");
		return false;
	case 21331:
	case 21332:
	case 21333:
	case 21334:
	case 21335:
	case 21336:
	case 21337:
	case 21338:
	case 21339:
                if (strstr(tp_ic_name, "td4160") && strstr(saved_command_line, "td4160_inx")) {
                        return true;
                }
                else if (strstr(tp_ic_name, "ilitek,ili7807s") && strstr(saved_command_line, "ili9883c_hlt")) {
                        return true;
                }
                else if (strstr(tp_ic_name, "ilitek,ili7807s") && strstr(saved_command_line, "ili9883c_boe")) {
                        return true;
                }
		return false;
	default:
		break;
	}
	return true;
}

bool  tp_judge_ic_match_commandline(struct panel_info *panel_data)
{
	int prj_id = 0;
	int i = 0;
#ifdef CONFIG_MACH_MT6765
	prj_id = 21331;
#else
	prj_id = get_project();
#endif
	pr_err("[TP] get_project() = %d \n", prj_id);
	pr_err("[TP] boot_command_line = %s \n", saved_command_line);
	for(i = 0; i < panel_data->project_num; i++)
	{
		if(prj_id == panel_data->platform_support_project[i])
		{
			g_tp_prj_id = panel_data->platform_support_project_dir[i];
			if(strstr(saved_command_line, panel_data->platform_support_commandline[i]) || strstr("default_commandline", panel_data->platform_support_commandline[i]) )
			{
				pr_err("[TP] Driver match the project\n");
				return true;
			}
			else
			{
				break;
			}
		}
	}
	pr_err("[TP] Driver does not match the project\n");
	pr_err("Lcd module not found\n");
	return false;
}

int tp_util_get_vendor(struct hw_resource *hw_res, struct panel_info *panel_data)
{
	char *vendor;

	panel_data->test_limit_name = kzalloc(MAX_LIMIT_DATA_LENGTH, GFP_KERNEL);
	if (panel_data->test_limit_name == NULL)
	{
		pr_err("[TP]panel_data.test_limit_name kzalloc error\n");
	}
	panel_data->extra = kzalloc(MAX_LIMIT_DATA_LENGTH, GFP_KERNEL);
	if (panel_data->extra == NULL)
	{
		pr_err("[TP]panel_data.test_limit_name kzalloc error\n");
	}
	//   panel_data->tp_type = g_tp_dev_vendor;
	if (panel_data->tp_type == TP_UNKNOWN)
	{
		pr_err("[TP]%s type is unknown\n", __func__);
		return 0;
	}

	vendor = GET_TP_DEV_NAME(panel_data->tp_type);

	strcpy(panel_data->manufacture_info.manufacture, vendor);

	snprintf(panel_data->fw_name, MAX_FW_NAME_LENGTH,
	         "tp/%d/FW_%s_%s.img",
	         g_tp_prj_id, panel_data->chip_name, vendor);

	if (panel_data->test_limit_name)
	{
		snprintf(panel_data->test_limit_name, MAX_LIMIT_DATA_LENGTH,
		         "tp/%d/LIMIT_%s_%s.img",
		         g_tp_prj_id, panel_data->chip_name, vendor);
	}

	if (strstr(saved_command_line, "oppo19551_samsung_ams644vk01_1080p_dsi_cmd"))
	{
		memcpy(panel_data->manufacture_info.version, "0xbd2860000", 11); //For 19550 19551 19553 19556 19597
	}
	if (strstr(saved_command_line, "oppo19357_samsung_ams644va04_1080p_dsi_cmd"))
	{
		memcpy(panel_data->manufacture_info.version, "0xfa1920000", 11); //For 19357 19358 19359 19354
	}
	if (strstr(saved_command_line, "oppo19537_samsung_ams643xf01_1080p_dsi_cmd"))
	{
		memcpy(panel_data->manufacture_info.version, "0xdd3400000", 11); //For 19536 19537 19538 19539
	}
	if (strstr(saved_command_line, "oppo20291_samsung_ams643xy01_1080p_dsi_vdo"))
	{
		memcpy(panel_data->manufacture_info.version, "0xFA270000", 11); //For 20291 20292 20293 20294 20295
	}
	if (strstr(saved_command_line, "nt36525b_hlt"))
	{
		memcpy(panel_data->manufacture_info.version, "HLT_NT25_", 9);
		panel_data->firmware_headfile.firmware_data = FW_206A1_NF_NT36525B_HLT;
		panel_data->firmware_headfile.firmware_size = sizeof(FW_206A1_NF_NT36525B_HLT);
	}
	if (strstr(saved_command_line, "hx83102d"))
	{
		memcpy(panel_data->manufacture_info.version, "HLT_NT25_", 9);
		panel_data->firmware_headfile.firmware_data = FW_206A1_NF_HX83102D_TRULY;
		panel_data->firmware_headfile.firmware_size = sizeof(FW_206A1_NF_HX83102D_TRULY);
	}
	if (strstr(saved_command_line, "ilt9881h"))
	{
		memcpy(panel_data->manufacture_info.version, "HLT_NT25_", 9);
		panel_data->firmware_headfile.firmware_data = FW_206A1_NF_ILT9881H_LS;
		panel_data->firmware_headfile.firmware_size = sizeof(FW_206A1_NF_ILT9881H_LS);
	}

        if (strstr(saved_command_line, "oplus21331_td4160_inx")) {
                memcpy(panel_data->manufacture_info.version, "AA340IOA1", 9);
                panel_data->firmware_headfile.firmware_data = FW_21331_NF_ZHAOYUN;
                panel_data->firmware_headfile.firmware_size = sizeof(FW_21331_NF_ZHAOYUN);
        	snprintf(panel_data->fw_name, MAX_FW_NAME_LENGTH,
                 	"tp/21331/FW_%s_%s.img",
                 	"TD4160", "INX");

        	if (panel_data->test_limit_name) {
                	snprintf(panel_data->test_limit_name, MAX_LIMIT_DATA_LENGTH,
                         	"tp/21331/LIMIT_%s_%s.img",
                         	"TD4160", "INX");
        	}
        }
        if (strstr(saved_command_line, "oplus21331_ili9883c_hlt")) {
                memcpy(panel_data->manufacture_info.version, "AA340HI01", 9);
                panel_data->firmware_headfile.firmware_data = FW_21331_NF_9883A;
                panel_data->firmware_headfile.firmware_size = sizeof(FW_21331_NF_9883A);
                snprintf(panel_data->fw_name, MAX_FW_NAME_LENGTH,
                        "tp/21331/FW_%s_%s.img",
                        "9883A", "HLT");
                if (panel_data->test_limit_name) {
                        snprintf(panel_data->test_limit_name, MAX_LIMIT_DATA_LENGTH,
                                "tp/21331/LIMIT_%s_%s.img",
                                "9883A", "HLT");
                }
        }
        if (strstr(saved_command_line, "oplus21331_ili9883c_boe")) {
                memcpy(panel_data->manufacture_info.version, "AA340BI01", 9);
                panel_data->firmware_headfile.firmware_data = FW_21331_NF_9883A;
                panel_data->firmware_headfile.firmware_size = sizeof(FW_21331_NF_9883A);
                snprintf(panel_data->fw_name, MAX_FW_NAME_LENGTH,
                        "tp/21331/FW_%s_%s.img",
                        "9883A", "BOE");
                if (panel_data->test_limit_name) {
                        snprintf(panel_data->test_limit_name, MAX_LIMIT_DATA_LENGTH,
                                "tp/21331/LIMIT_%s_%s.img",
                                "9883A", "BOE");
                }
        }

	panel_data->manufacture_info.fw_path = panel_data->fw_name;
	pr_info("[TP]vendor:%s fw:%s limit:%s\n",
	        vendor,
	        panel_data->fw_name,
	        panel_data->test_limit_name == NULL ? "NO Limit" : panel_data->test_limit_name);

	return 0;
}

