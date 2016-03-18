/*
 * Copyright (C) 2011 Samsung Electronics Co., Ltd. All rights reserved.
 *
 * Contact:
 * Jinhyung Choi <jinhyung2.choi@samsung.com>
 * JiHye Kim <jihye1128.kim@samsung.com>
 * YeongKyoon Lee <yeongkyoon.lee@samsung.com>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */


#include <string.h>
#include <sys/types.h>
#include <stdio.h>
#include <dirent.h>
#include <devman_plugin_intf.h>

#include "devman_define_node_path.h"

#define EXPORT_API  __attribute__((visibility("default")))

#define BUFF_MAX	255
#define MAX_NAME 255

#if 0
#define GENERATE_ACCESSORS_CHAR_RW(_suffix, _item)	\
char *OEM_sys_get_##_suffix(char* str)			\
{						\
	return sys_get_str(_item, str);	\
}						\
						\
int OEM_sys_set_##_suffix(char *str)			\
{						\
	return sys_set_str(_item, value);	\
}

#define GENERATE_ACCESSORS_CHAR_R(_suffix, _item)	\
char *OEM_sys_get_##_suffix(char* str)			\
{						\
	return sys_get_str(_item, str);	\
}

#define GENERATE_ACCESSORS_CHAR_W(_suffix, _item)	\
int OEM_sys_set_##_suffix(char *str)			\
{						\
	return sys_set_str(_item, str);		\
}
#endif

/* TODO: Add APIs has (char *) params */

#define GENERATE_ACCESSORS_INT_RW(_suffix, _item)	\
int OEM_sys_get_##_suffix(int *value)			\
{						\
	return sys_get_int(_item, value);	\
}						\
						\
int OEM_sys_set_##_suffix(int value)	\
{						\
	return sys_set_int(_item, value);	\
}

#define GENERATE_ACCESSORS_INT_R(_suffix, _item)	\
int OEM_sys_get_##_suffix(int *value)			\
{						\
	return sys_get_int(_item, value);	\
}

#define GENERATE_ACCESSORS_INT_W(_suffix, _item)	\
int OEM_sys_set_##_suffix(int value)			\
{						\
	return sys_set_int(_item, value);	\
}

/*
GENERATE_ACCESSORS_INT_R(backlight_max_brightness, BACKLIGHT_MAX_BRIGHTNESS_PATH)
GENERATE_ACCESSORS_INT_RW(backlight_brightness, BACKLIGHT_BRIGHTNESS_PATH)
GENERATE_ACCESSORS_INT_RW(backlight_acl_control, LCD_ACL_CONTROL_PATH)
GENERATE_ACCESSORS_INT_RW(lcd_power, LCD_POWER_PATH)
*/

#define DEVMGR_LOG
#if defined(DEVMGR_LOG)
#define LOG_TAG     "DEVICE_PLUGIN"
#include <dlog/dlog.h>
#define devmgr_log(fmt, args...)	SLOGD(fmt, ##args)
#else
#define devmgr_log(fmt, args...)
#endif

enum display_type
{
	DISP_MAIN = 0,
	DISP_SUB,
	DISP_MAX
};

enum lux_status {
	decrement,
	no_change,
	increment,
};

struct display_info
{
	enum display_type etype; /* FIXME:!! Main LCD or Sub LCD node */
	char bl_name[MAX_NAME]; /* backlight name */
	char lcd_name[MAX_NAME]; /* lcd name */
};

#define MAX_CANDELA_CRITERION	300
#define PWR_SAVING_CANDELA_CRITERION	20

/* FIXME:!! change to global_ctx */
int lcd_index;
struct display_info disp_info[DISP_MAX];

/*
int OEM_sys_get_backlight_brightness_by_lux(unsigned int lux, enum lux_status status)
{
	static int brightness = -1;

	if (status == no_change) {
		if (brightness == -1)
			status = increment;
		else
			return brightness;
	}
	if (status == decrement) {
		switch (lux) {
		case 10000 ... 0xffffffff:
			brightness =  100;
			break;
		case 1000 ... 9999:
			brightness =  80;
			break;
		case 75 ... 999:
			brightness =  60;
			break;
		case 7 ... 74:
			brightness =  40;
			break;
		case 0 ... 6:
			brightness =  1;
			break;
		default:
			return -1;
		}
	} else if (status == increment) {
		switch (lux) {
		case 15001 ... 0xffffffff:
			brightness =  100;
			break;
		case 1501 ... 15000:
			brightness =  80;
			break;
		case 151 ...  1500:
			brightness =  60;
			break;
		case 16 ... 150:
			brightness =  40;
			break;
		case 0 ... 15:
			brightness =  1;
			break;
		default:
			return -1;
		}
	} else
		return -1;

	return brightness;
}
*/
int OEM_sys_get_brightness(unsigned int lux)
{
	const unsigned int Nr_Table[] = {
		0, 5, 5, 6, 6, 7, 8, 8, 9, 9,
		10, 11, 12, 13, 13, 14, 15, 16, 18, 19,
		20, 22, 23, 25, 29, 32, 36, 39, 43, 46,
		50, 54, 57, 61, 64, 68, 71, 75, 79, 82,
		86, 89, 93, 96, 100, 119, 138, 157, 176, 195,
		214, 233, 252, 271, 290, 310, 329, 348, 367, 386,
		405, 424, 443, 462, 481, 500, 571, 643, 714, 786,
		857, 929, 1000, 1100, 1200, 1300, 1400, 1500, 1667, 1833,
		2000, 2250, 2500, 2750, 3000, 3333, 3667, 4000, 4083, 4167,
		4250, 4333, 4417, 4500, 4583, 4667, 4750, 4833, 4917, 5000,
	};
	int brightness;

	for (brightness=0; (lux > Nr_Table[brightness]) && (brightness < 99); brightness++);

	return brightness;
}

int OEM_sys_get_backlight_brightness_by_lux(int lux, int *value)
{
	const unsigned int Max_Table[] = {
		15, 15, 15, 15, 16, 17, 18, 20, 21, 23,
		26, 28, 31, 33, 35, 38, 40, 44, 48, 51,
		55, 60, 65, 70, 81, 92, 103, 114, 125, 136,
		147, 158, 169, 180, 190, 201, 212, 223, 234, 245,
		256, 267, 278, 289, 300, 357, 414, 471, 529, 586,
		643, 700, 757, 814, 871, 929, 986, 1043, 1100, 1157,
		1214, 1271, 1329, 1386, 1443, 1500, 1678, 1855, 2033, 2211,
		2389, 2566, 2744, 2977, 3209, 3442, 3674, 3907, 4274, 4642,
		5009, 5215, 5422, 5628, 5834, 6057, 6279, 6502, 6585, 6669,
		6752, 6835, 6919, 7002, 7085, 7169, 7252, 7335, 7419, 7502,
	};
	const unsigned int Min_Table[] = {
		1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
		1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
		1, 1, 1, 1, 1, 1, 1, 2, 2, 3,
		3, 4, 4, 5, 5, 6, 6, 7, 7, 8,
		8, 9, 9, 10, 10, 12, 14, 16, 17, 19,
		21, 23, 25, 27, 29, 30, 32, 34, 36, 38,
		40, 42, 43, 45, 47, 49, 60, 70, 81, 91,
		102, 112, 123, 140, 157, 174, 191, 208, 240, 271,
		303, 356, 409, 461, 514, 592, 670, 748, 769, 790,
		811, 832, 853, 874, 895, 916, 937, 958, 979, 1000,
	};
	static int brightness = -1;

	if (brightness == -1) {
		brightness = OEM_sys_get_brightness(lux);
		devmgr_log("lux: %d, brightness: %d.\n", lux, brightness+1);
	} else if ((lux > Max_Table[brightness]) || (lux < Min_Table[brightness])) {
		brightness = OEM_sys_get_brightness(lux);
		devmgr_log("lux: %d, brightness: %d.\n", lux, brightness+1);
	}
	*value = brightness+1;
	return 0;
}

static int OEM_sys_display_info(struct display_info *disp_info)
{
	struct dirent *dent;
	struct dirent entry;
	DIR *dirp;
	int i, index;
	const char * bl_path = BACKLIGHT_PATH;
	const char * lcd_path = LCD_PATH;

	/* Backlight */
	index = 0;
	dirp = opendir(bl_path);
	if (dirp) {
		while (!readdir_r(dirp, &entry, &dent)) {
			if (index >= DISP_MAX) {
				devmgr_log("supports %d display node", DISP_MAX);
				break;
			}

			if (!strncmp(".", dent->d_name, sizeof(".")) || !strncmp("..", dent->d_name, sizeof("..")))
				continue;
			else {
				strncpy(disp_info[index].bl_name, dent->d_name, sizeof(disp_info[index].bl_name) - 1);
				index++;
			}
		}
		closedir(dirp);
	}

	for (i = 0; i < index; i++)
		devmgr_log("bl_name[%s]", disp_info[i].bl_name);

	/* LCD */
	index = 0;
	dirp = opendir(lcd_path);
	if (dirp) {
		while (!readdir_r(dirp, &entry, &dent)) {
			if (index >= DISP_MAX) {
				devmgr_log("supports %d display node", DISP_MAX);
				break;
			}

			if (!strncmp(".", dent->d_name, sizeof(".")) || !strncmp("..", dent->d_name, sizeof("..")))
				continue;
			else {
				strncpy(disp_info[index].lcd_name, dent->d_name, sizeof(disp_info[index].lcd_name) - 1);
				index++;
			}
		}
		closedir(dirp);
	}

	for (i = 0; i < index; i++)
		devmgr_log("lcd_name[%s]", disp_info[i].lcd_name);

	lcd_index = index;

	return 0;
}

int OEM_sys_get_display_count(int *value)
{
	int ret = -1;

	/* TODO: We should implement to find out current number of display */
	*value = lcd_index;
	ret = 0;
	/* ********************* */

	devmgr_log("value[%d]", *value);

	return ret;
}

int OEM_sys_get_backlight_max_brightness(int index, int *value)
{
	int ret = -1;
	char path[MAX_NAME+1];

	if (index >= DISP_MAX) {
		devmgr_log("supports %d display node", DISP_MAX);
		return ret;
	}

	snprintf(path, MAX_NAME, BACKLIGHT_MAX_BRIGHTNESS_PATH, disp_info[index].bl_name);
	ret = sys_get_int(path, value);
	devmgr_log("path[%s]value[%d]", path, *value);

	return ret;
}

int OEM_sys_get_backlight_min_brightness(int index, int *value)
{
    int ret = -1;
    char path[MAX_NAME+1];

    if (index >= DISP_MAX) {
        devmgr_log("supports %d display node", DISP_MAX);
        return ret;
    }

    snprintf(path, MAX_NAME, BACKLIGHT_MIN_BRIGHTNESS_PATH, disp_info[index].bl_name);
    ret = sys_get_int(path, value);
    devmgr_log("path[%s]value[%d]", path, *value);

    return ret;
}

int OEM_sys_get_backlight_brightness(int index, int *value, int power_saving)
{
	int ret = -1;
	char path[MAX_NAME+1];
	int max_brightness;
	int pwr_saving_offset;

	if (index >= DISP_MAX) {
		devmgr_log("supports %d display node", DISP_MAX);
		return ret;
	}

	snprintf(path, MAX_NAME, BACKLIGHT_BRIGHTNESS_PATH, disp_info[index].bl_name);
	ret = sys_get_int(path, value);
	devmgr_log("path[%s]value[%d]power_saving[%d]", path, *value, power_saving);

	if (power_saving){
		snprintf(path, MAX_NAME, BACKLIGHT_MAX_BRIGHTNESS_PATH, disp_info[index].bl_name);
		ret = sys_get_int(path, &max_brightness);
		if (ret)
		{
			devmgr_log("Can't read max_brightness node[%s]", path);
			return ret;
		}
		pwr_saving_offset = (PWR_SAVING_CANDELA_CRITERION * max_brightness / MAX_CANDELA_CRITERION) + 0.5;

		if (*value > max_brightness - pwr_saving_offset)
			*value = max_brightness;
		else
			*value = *value + pwr_saving_offset;

		devmgr_log("power_saving result[%d]", *value);
	}

	return ret;
}

static int OEM_sys_set_backlight_brightness(int index, int value, int power_saving)
{
	int ret = -1;
	char path[MAX_NAME+1];
	int max_brightness;
	int pwr_saving_offset;

	if (index >= DISP_MAX) {
		devmgr_log("supports %d display node", DISP_MAX);
		return ret;
	}

	devmgr_log("path[%s]value[%d]power_saving[%d]", path, value, power_saving);

	if (power_saving){
		snprintf(path, MAX_NAME, BACKLIGHT_MAX_BRIGHTNESS_PATH, disp_info[index].bl_name);
		ret = sys_get_int(path, &max_brightness);
		if (ret)
		{
			devmgr_log("Can't read max_brightness node[%s]", path);
			return ret;
		}
		pwr_saving_offset = (PWR_SAVING_CANDELA_CRITERION * max_brightness / MAX_CANDELA_CRITERION) + 0.5;

		if (value < pwr_saving_offset)
			value = 0;
		else
			value = value - pwr_saving_offset;

		devmgr_log("power_saving result[%d]", value);
	}

	snprintf(path, MAX_NAME, BACKLIGHT_BRIGHTNESS_PATH, disp_info[index].bl_name);
	ret = sys_set_int(path, value);

	return ret;
}

int OEM_sys_set_backlight_dimming(int index, int value)
{
    /*
    int ret = -1;
    char path[MAX_NAME+1];

    if (index >= DISP_MAX) {
        devmgr_log("supports %d display node", DISP_MAX);
        return ret;
    }

    snprintf(path, MAX_NAME, BACKLIGHT_DIMMING_PATH, disp_info[index].lcd_name);
    devmgr_log("path[%s]value[%d]", path, value);
    ret = sys_set_int(path, value);
    return ret;
    */

    // TODO : value is only 1
    return OEM_sys_set_backlight_brightness(index, 1, 0/*power_saving*/);

}

int OEM_sys_get_backlight_acl_control(int index, int *value)
{
	int ret = -1;
	char path[MAX_NAME+1];

	if (index >= DISP_MAX) {
		devmgr_log("supports %d display node", DISP_MAX);
		return ret;
	}

	snprintf(path, MAX_NAME, LCD_ACL_CONTROL_PATH, disp_info[index].lcd_name);
	ret = sys_get_int(path, value);
	devmgr_log("path[%s]value[%d]", path, *value);

	return ret;
}

int OEM_sys_set_backlight_acl_control(int index, int value)
{
	int ret = -1;
	char path[MAX_NAME+1];

	if (index >= DISP_MAX) {
		devmgr_log("supports %d display node", DISP_MAX);
		return ret;
	}

	snprintf(path, MAX_NAME, LCD_ACL_CONTROL_PATH, disp_info[index].lcd_name);
	devmgr_log("path[%s]value[%d]", path, value);
	ret = sys_set_int(path, value);

	return ret;
}

int OEM_sys_get_lcd_power(int index, int *value)
{
	int ret = -1;
	char path[MAX_NAME+1];

	if (index >= DISP_MAX) {
		devmgr_log("supports %d display node", DISP_MAX);
		return ret;
	}

	snprintf(path, MAX_NAME, LCD_POWER_PATH, disp_info[index].lcd_name);
	ret = sys_get_int(path, value);
	devmgr_log("path[%s]value[%d]", path, *value);

	return ret;
}

int OEM_sys_set_lcd_power(int index, int value)
{
	int ret = -1;
	char path[MAX_NAME+1];

	if (index >= DISP_MAX) {
		devmgr_log("supports %d display node", DISP_MAX);
		return ret;
	}

	snprintf(path, MAX_NAME, LCD_POWER_PATH, disp_info[index].lcd_name);
	devmgr_log("path[%s]value[%d]", path, value);
	ret = sys_set_int(path, value);

	return ret;
}

/* image_enhance */
int OEM_sys_get_image_enhance_save(void *image_enhance)
{
	int ret = -1;
	return ret;
}

int OEM_sys_set_image_enhance_restore(void *image_enhance)
{
	int ret = -1;
	return ret;
}

int OEM_sys_get_image_enhance_mode(int *value)
{
	int ret = -1;
	return ret;
}

int OEM_sys_set_image_enhance_mode(int value)
{
	int ret = -1;
	return ret;
}

int OEM_sys_get_image_enhance_scenario(int *value)
{
	int ret = -1;
	return ret;
}

int OEM_sys_set_image_enhance_scenario(int value)
{
	int ret = -1;
	return ret;
}

int OEM_sys_get_image_enhance_tone(int *value)
{
	int ret = -1;
	return ret;
}

int OEM_sys_set_image_enhance_tone(int value)
{
	int ret = -1;
	return ret;
}

int OEM_sys_get_image_enhance_outdoor(int *value)
{
	int ret = -1;
	return ret;
}

int OEM_sys_set_image_enhance_outdoor(int value)
{
	int ret = -1;
	return ret;
}

int OEM_sys_get_image_enhance_tune(int *value)
{
	int ret = -1;
	return ret;
}

int OEM_sys_set_image_enhance_tune(int value)
{
	int ret = -1;
	return ret;
}

int OEM_sys_image_enhance_info(int *value)
{
	return 0;
}

int OEM_sys_set_display_frame_rate(int value)
{
/*
	if(value){
		devmgr_log("Display frame rate limited to 40Hz");
		return sys_set_str(DISPLAY_FRAME_RATE_PATH, "40");
	}else{
		devmgr_log("Display frame rate change 40Hz and 60Hz");
		return sys_set_str(DISPLAY_FRAME_RATE_PATH, "60");
	}
*/
	return -1;
}

GENERATE_ACCESSORS_INT_RW(haptic_vibetones_level, HAPTIC_VIBETONES_LEVEL_PATH)
GENERATE_ACCESSORS_INT_R(haptic_vibetones_level_max, HAPTIC_VIBETONES_LEVEL_MAX_PATH)
GENERATE_ACCESSORS_INT_W(haptic_vibetones_enable, HAPTIC_VIBETONES_ENABLE_PATH)
GENERATE_ACCESSORS_INT_W(haptic_vibetones_oneshot, HAPTIC_VIBETONES_ONESHOT_PATH)

GENERATE_ACCESSORS_INT_R(battery_capacity, BATTERY_CAPACITY_PATH)
GENERATE_ACCESSORS_INT_R(battery_charge_full, BATTERY_CHARGE_FULL_PATH)
GENERATE_ACCESSORS_INT_R(battery_charge_now, BATTERY_CHARGE_NOW_PATH)
GENERATE_ACCESSORS_INT_R(battery_present, BATTERY_PRESENT_PATH)

int OEM_sys_get_battery_capacity_raw(int *value)
{
	return 0;
}

static char *health_text[] = {
	"Unknown", "Good", "Overheat", "Dead", "Over voltage",
	"Unspecified failure", "Cold",
};

int OEM_sys_get_battery_health(int *value)
{
	char buf[BUFF_MAX] = {0};
	int ret = 0;
	int i 	= 0;

	ret = sys_get_str(BATTERY_HEALTH_PATH, buf);
	if (ret != 0) {
		return -1;
	}

	for (i = 0; i < BATTERY_HEALTH_MAX; i++) {
		if (strncmp(buf, health_text[i], strlen(health_text[i])) == 0) {
			*value = i;
			return 0;
		}
	}

	return -1;
}

int OEM_sys_get_battery_polling_required(int *value)
{
	*value = 1;

	return 0;
}

int OEM_sys_get_battery_support_insuspend_charging(int *value)
{
	*value = 1;

	return 0;
}

static char uart_node_path[MAX_NAME];
static char usb_node_path[MAX_NAME];

/* find uart/usb node path */
static int OEM_sys_muic_node_path_info()
{
	int err = -1;

	err = sys_check_node(UART_PATH);
	if (!err)
		sys_get_node(UART_PATH, uart_node_path);
	else {
		err = sys_check_node(UART_PATH_TRATS);
		if (err) {
			devmgr_log("uart path node not found");
			return -1;
		}
		sys_get_node(UART_PATH_TRATS, uart_node_path);
	}

	err = sys_check_node(USB_PATH);
	if (!err)
		sys_get_node(USB_PATH, usb_node_path);
	else {
		err = sys_check_node(USB_PATH_TRATS);
		if (err) {
			devmgr_log("usb path node not found");
			return -1;
		}
		sys_get_node(USB_PATH_TRATS, usb_node_path);
	}
	return 0;
}

int OEM_sys_get_uart_path(int *value)
{
	char buf[BUFF_MAX] = {0};
	int ret = 0;

	ret = sys_get_str(uart_node_path, buf);
	if (ret != 0) {
		return -1;
	}

	if (strncmp(buf, "CP", 2) == 0) {
		*value = PATH_CP;
		return 0;
	} else if (strncmp(buf, "AP", 2) == 0) {
		*value = PATH_AP;
		return 0;
	}

	return -1;
}

int OEM_sys_set_uart_path(int value)
{
	switch (value) {
	case PATH_CP:
		return sys_set_str(uart_node_path, "CP");
	case PATH_AP:
		return sys_set_str(uart_node_path, "AP");
	}

	return -1;
}


int OEM_sys_get_usb_path(int *value)
{
	char buf[BUFF_MAX] = {0};
	int ret = 0;

	ret = sys_get_str(usb_node_path, buf);
	if (ret != 0) {
		return -1;
	}

	if (strncmp(buf, "PDA", 3) == 0) {
		*value = PATH_AP;
		return 0;
	} else if (strncmp(buf, "MODEM", 5) == 0) {
		*value = PATH_CP;
		return 0;
	}

	return -1;
}

int OEM_sys_set_usb_path(int value)
{
	switch (value) {
	case PATH_CP:
		return sys_set_str(usb_node_path, "MODEM");
	case PATH_AP:
		return sys_set_str(usb_node_path, "PDA");
	}

	return -1;
}

GENERATE_ACCESSORS_INT_R(jack_charger_online, JACK_CHARGER_ONLINE_PATH)
GENERATE_ACCESSORS_INT_R(jack_earjack_online, JACK_EARJACK_ONLINE_PATH)
GENERATE_ACCESSORS_INT_R(jack_earkey_online, JACK_EARKEY_ONLINE_PATH)
GENERATE_ACCESSORS_INT_R(jack_hdmi_online, JACK_HDMI_ONLINE_PATH)
GENERATE_ACCESSORS_INT_R(jack_usb_online, JACK_USB_ONLINE_PATH)
GENERATE_ACCESSORS_INT_R(jack_cradle_online, JACK_CRADLE_ONLINE_PATH)
GENERATE_ACCESSORS_INT_R(jack_tvout_online, JACK_TVOUT_ONLINE_PATH)

int OEM_sys_get_jack_keyboard_online(int *value)
{
	/* Currently, We don't provide SLP Based platform with keyboard I/F */
	int ret = -1;
	/*return sys_get_int(JACK_KEYBOARD_ONLINE_PATH, value);*/
	return ret;
}

int OEM_sys_get_hdmi_support(int *value)
{
	*value = 1;

	return 0;
}

GENERATE_ACCESSORS_INT_R(leds_torch_max_brightness, LEDS_TORCH_MAX_BRIGHTNESS_PATH)
GENERATE_ACCESSORS_INT_RW(leds_torch_brightness, LEDS_TORCH_BRIGHTNESS_PATH)

int OEM_sys_set_power_state(int value)
{
	switch (value) {
	case POWER_STATE_SUSPEND:
		return sys_set_str(POWER_STATE_PATH, "mem");
	case POWER_STATE_PRE_SUSPEND:
		return sys_set_str(POWER_STATE_PATH, "pre_suspend");
	case POWER_STATE_POST_RESUME:
		return sys_set_str(POWER_STATE_PATH, "post_resume");
	}

	return -1;
}

GENERATE_ACCESSORS_INT_RW(power_wakeup_count, POWER_WAKEUP_COUNT_PATH)

GENERATE_ACCESSORS_INT_W(memnotify_threshold_lv1, MEMNOTIFY_THRESHOLD_LV1_PATH)
GENERATE_ACCESSORS_INT_W(memnotify_threshold_lv2, MEMNOTIFY_THRESHOLD_LV2_PATH)

GENERATE_ACCESSORS_INT_R(cpufreq_cpuinfo_max_freq, CPUFREQ_CPUINFO_MAX_FREQ_PATH)
GENERATE_ACCESSORS_INT_R(cpufreq_cpuinfo_min_freq, CPUFREQ_CPUINFO_MIN_FREQ_PATH)
GENERATE_ACCESSORS_INT_RW(cpufreq_scaling_max_freq, CPUFREQ_SCALING_MAX_FREQ_PATH)
GENERATE_ACCESSORS_INT_RW(cpufreq_scaling_min_freq, CPUFREQ_SCALING_MIN_FREQ_PATH)

#define GENERATE_ACCESSORS_INT_R_NO_CONVERT(_suffix, _item)	\
int OEM_sys_get_##_suffix(int *value)			\
{						\
	return sys_get_int_wo_convert(_item, value);	\
}

#define GENERATE_ACCESSORS_INT_W_NO_CONVERT(_suffix, _item)	\
int OEM_sys_set_##_suffix(int value)			\
{						\
	return sys_set_int_wo_convert(_item, value);	\
}

GENERATE_ACCESSORS_INT_R_NO_CONVERT(memnotify_victim_task, MEMNOTIFY_VICTIM_TASK_PATH)
GENERATE_ACCESSORS_INT_W_NO_CONVERT(process_monitor_mp_pnp, PROCESS_MONITOR_MP_PNP_PATH)
GENERATE_ACCESSORS_INT_W_NO_CONVERT(process_monitor_mp_vip, PROCESS_MONITOR_MP_VIP_PATH)

#define GENERATE_ACCESSORS_GET_NODE_PATH(_suffix, _item)	\
int OEM_sys_get_##_suffix(char *node)			\
{						\
	return sys_get_node(_item, node);	\
}

GENERATE_ACCESSORS_GET_NODE_PATH(touch_event, TOUCH_EVENT_NODE)
GENERATE_ACCESSORS_GET_NODE_PATH(memnotify_node, MEMNOTIFY_NODE)
GENERATE_ACCESSORS_GET_NODE_PATH(process_monitor_node, PROCESS_MONITOR_NODE)

#if 0
int get_uart_path(char *value, int val) {
{
	char *tmp_buff;

	tmp_buff = sys_get_str(UART_PATH);
	if (tmp_buff == NULL)
		return -1;

	if (strncmp(tmp_buff, "AP", 2) == 0) {
		*val = PATH_TO_PDA;
	} else if (strncmp(tmp_buff, "CP", 2) == 0) {
		*val = PATH_TO_MODEM;
	} else {
		free(tmp_buff);
		return -1;
	}
	free(tmp_buff);

	return 0;
}

static int set_uart_path(char *value, int val)
{
	int ret = -1;

	if (val < PATH_TO_MODEM || val > PATH_TO_PDA) {
		return -1;
	}
	ret =
	    sys_set_str(UART_PATH, (val == PATH_TO_MODEM) ? "CP" : "AP");
	if(ret == 0) {
		if (val == PATH_TO_MODEM) {
			system("/usr/bin/save_blenv uartpath CP");
		} else {
			system("/usr/bin/save_blenv uartpath AP");
		}
	}
	return ret;
}

int get_usb_path(char *prop, int *val) {
{
	char *tmp_buff = NULL;
	switch (prop) {
	case JACK_PROP_USB_ONLINE:
	case JACK_PROP_TA_ONLINE:
		return generic_jack_interface_get(NULL, prop, val);

	case JACK_PROP_HDMI_ONLINE:
		tmp_buff =
		    sys_get_str("/sys/class/i2c-adapter/i2c-5/5-0072/status");
		if (tmp_buff && (strstr(tmp_buff, "MHL connected:yes") != 0)) {
			free(tmp_buff);
			*val = 1;
		} else {
			if(tmp_buff)
				free(tmp_buff);
			*val = 0;
		}
		return 0;
	case JACK_PROP_USB_PATH:
		tmp_buff = sys_get_str(USB_PATH);
		if (tmp_buff == NULL)
			return -1;
		if (strncmp(tmp_buff, usb_path_string[PATH_TO_MODEM], 2) == 0)
			*val = PATH_TO_MODEM;
		else
			*val = PATH_TO_PDA;

		free(tmp_buff);
		return 0;
	}

	free(tmp_buff);
	return -1;
}

int set_usb_path(char *prop, int val);
{
	char *cur_path;
	int conn;

	switch (prop) {
	case JACK_PROP_USB_PATH:
		break;
	default:
		return -1;
	}
	cur_path = sys_get_str(USB_PATH);
	if (cur_path == NULL)
		return -1;

	switch (val) {
	case PATH_TO_MODEM:
	case PATH_TO_PDA:
		if (strncmp(cur_path, usb_path_string[val], 2) == 0) {
			free(cur_path);
			return 0;
		}
		free(cur_path);
		if (generic_jack_interface_get
		    (NULL, JACK_USB_ONLINE_PATH, &conn) != 0) {
			/*failed to get the connection status */
			return -1;
		}
		if (sys_set_str(USB_PATH, (char *)usb_path_string[val]) !=
		    0)
			return -1;
		if (val == PATH_TO_MODEM) {
			system("/usr/bin/save_blenv usbpath CP");
		} else {
			system("/usr/bin/save_blenv usbpath AP");
		}
		break;

	default:
		free(cur_path);
		return -1;
	}

	return 0;
}
#endif

static OEM_sys_devman_plugin_interface devman_plugin_interface_emul;

EXPORT_API const OEM_sys_devman_plugin_interface *OEM_sys_get_devman_plugin_interface()
{
	devman_plugin_interface_emul.OEM_sys_get_display_count = &OEM_sys_get_display_count;
	devman_plugin_interface_emul.OEM_sys_get_backlight_min_brightness = &OEM_sys_get_backlight_min_brightness;
	devman_plugin_interface_emul.OEM_sys_get_backlight_max_brightness = &OEM_sys_get_backlight_max_brightness;
	devman_plugin_interface_emul.OEM_sys_get_backlight_brightness = &OEM_sys_get_backlight_brightness;
	devman_plugin_interface_emul.OEM_sys_set_backlight_brightness = &OEM_sys_set_backlight_brightness;
	devman_plugin_interface_emul.OEM_sys_set_backlight_dimming = &OEM_sys_set_backlight_dimming;
	devman_plugin_interface_emul.OEM_sys_get_backlight_acl_control = &OEM_sys_get_backlight_acl_control;
	devman_plugin_interface_emul.OEM_sys_set_backlight_acl_control = &OEM_sys_set_backlight_acl_control;

	devman_plugin_interface_emul.OEM_sys_get_lcd_power = &OEM_sys_get_lcd_power;
	devman_plugin_interface_emul.OEM_sys_set_lcd_power = &OEM_sys_set_lcd_power;

	devman_plugin_interface_emul.OEM_sys_get_image_enhance_mode = &OEM_sys_get_image_enhance_mode;
	devman_plugin_interface_emul.OEM_sys_set_image_enhance_mode = &OEM_sys_set_image_enhance_mode;
	devman_plugin_interface_emul.OEM_sys_get_image_enhance_scenario = &OEM_sys_get_image_enhance_scenario;
	devman_plugin_interface_emul.OEM_sys_set_image_enhance_scenario = &OEM_sys_set_image_enhance_scenario;
	devman_plugin_interface_emul.OEM_sys_get_image_enhance_tone = &OEM_sys_get_image_enhance_tone;
	devman_plugin_interface_emul.OEM_sys_set_image_enhance_tone = &OEM_sys_set_image_enhance_tone;
	devman_plugin_interface_emul.OEM_sys_get_image_enhance_outdoor = &OEM_sys_get_image_enhance_outdoor;
	devman_plugin_interface_emul.OEM_sys_set_image_enhance_outdoor = &OEM_sys_set_image_enhance_outdoor;

	devman_plugin_interface_emul.OEM_sys_get_image_enhance_tune = &OEM_sys_get_image_enhance_tune;
	devman_plugin_interface_emul.OEM_sys_set_image_enhance_tune = &OEM_sys_set_image_enhance_tune;

	devman_plugin_interface_emul.OEM_sys_image_enhance_info = &OEM_sys_image_enhance_info;

	devman_plugin_interface_emul.OEM_sys_set_display_frame_rate = &OEM_sys_set_display_frame_rate;

	devman_plugin_interface_emul.OEM_sys_get_uart_path = &OEM_sys_get_uart_path;
	devman_plugin_interface_emul.OEM_sys_set_uart_path = &OEM_sys_set_uart_path;

	devman_plugin_interface_emul.OEM_sys_get_usb_path = &OEM_sys_get_usb_path;
	devman_plugin_interface_emul.OEM_sys_set_usb_path = &OEM_sys_set_usb_path;

	devman_plugin_interface_emul.OEM_sys_get_haptic_vibetones_level_max = &OEM_sys_get_haptic_vibetones_level_max;
	devman_plugin_interface_emul.OEM_sys_get_haptic_vibetones_level = &OEM_sys_get_haptic_vibetones_level;
	devman_plugin_interface_emul.OEM_sys_set_haptic_vibetones_level = &OEM_sys_set_haptic_vibetones_level;
	devman_plugin_interface_emul.OEM_sys_set_haptic_vibetones_enable = &OEM_sys_set_haptic_vibetones_enable;
	devman_plugin_interface_emul.OEM_sys_set_haptic_vibetones_oneshot = &OEM_sys_set_haptic_vibetones_oneshot;

	devman_plugin_interface_emul.OEM_sys_get_battery_capacity = &OEM_sys_get_battery_capacity;
	devman_plugin_interface_emul.OEM_sys_get_battery_capacity_raw = &OEM_sys_get_battery_capacity_raw;
	devman_plugin_interface_emul.OEM_sys_get_battery_charge_full = &OEM_sys_get_battery_charge_full;
	devman_plugin_interface_emul.OEM_sys_get_battery_charge_now = &OEM_sys_get_battery_charge_now;
	devman_plugin_interface_emul.OEM_sys_get_battery_present = &OEM_sys_get_battery_present;
	devman_plugin_interface_emul.OEM_sys_get_battery_health = &OEM_sys_get_battery_health;
    	devman_plugin_interface_emul.OEM_sys_get_battery_polling_required = &OEM_sys_get_battery_polling_required;
	devman_plugin_interface_emul.OEM_sys_get_battery_support_insuspend_charging = &OEM_sys_get_battery_support_insuspend_charging;

	devman_plugin_interface_emul.OEM_sys_get_jack_charger_online = &OEM_sys_get_jack_charger_online;
	devman_plugin_interface_emul.OEM_sys_get_jack_earjack_online = &OEM_sys_get_jack_earjack_online;
	devman_plugin_interface_emul.OEM_sys_get_jack_earkey_online = &OEM_sys_get_jack_earkey_online;
	devman_plugin_interface_emul.OEM_sys_get_jack_hdmi_online = &OEM_sys_get_jack_hdmi_online;
	devman_plugin_interface_emul.OEM_sys_get_jack_usb_online = &OEM_sys_get_jack_usb_online;
	devman_plugin_interface_emul.OEM_sys_get_jack_cradle_online = &OEM_sys_get_jack_cradle_online;
	devman_plugin_interface_emul.OEM_sys_get_jack_tvout_online = &OEM_sys_get_jack_tvout_online;
	devman_plugin_interface_emul.OEM_sys_get_jack_keyboard_online = &OEM_sys_get_jack_keyboard_online;

	devman_plugin_interface_emul.OEM_sys_get_hdmi_support = &OEM_sys_get_hdmi_support;

	devman_plugin_interface_emul.OEM_sys_get_leds_torch_max_brightness = &OEM_sys_get_leds_torch_max_brightness;
	devman_plugin_interface_emul.OEM_sys_get_leds_torch_brightness = &OEM_sys_get_leds_torch_brightness;
	devman_plugin_interface_emul.OEM_sys_set_leds_torch_brightness = &OEM_sys_set_leds_torch_brightness;

	devman_plugin_interface_emul.OEM_sys_set_power_state = &OEM_sys_set_power_state;

	/* TODO: Should determine enum values of wakeup_count nodes */
	devman_plugin_interface_emul.OEM_sys_get_power_wakeup_count = &OEM_sys_get_power_wakeup_count;
	devman_plugin_interface_emul.OEM_sys_set_power_wakeup_count = &OEM_sys_set_power_wakeup_count;

	devman_plugin_interface_emul.OEM_sys_get_memnotify_node = &OEM_sys_get_memnotify_node;
	devman_plugin_interface_emul.OEM_sys_get_memnotify_victim_task = &OEM_sys_get_memnotify_victim_task;
	devman_plugin_interface_emul.OEM_sys_set_memnotify_threshold_lv1 = &OEM_sys_set_memnotify_threshold_lv1;
	devman_plugin_interface_emul.OEM_sys_set_memnotify_threshold_lv2 = &OEM_sys_set_memnotify_threshold_lv2;

	devman_plugin_interface_emul.OEM_sys_get_process_monitor_node = &OEM_sys_get_process_monitor_node;
	devman_plugin_interface_emul.OEM_sys_set_process_monitor_mp_pnp = &OEM_sys_set_process_monitor_mp_pnp;
	devman_plugin_interface_emul.OEM_sys_set_process_monitor_mp_vip = &OEM_sys_set_process_monitor_mp_vip;

	devman_plugin_interface_emul.OEM_sys_get_cpufreq_cpuinfo_max_freq = &OEM_sys_get_cpufreq_cpuinfo_max_freq;
	devman_plugin_interface_emul.OEM_sys_get_cpufreq_cpuinfo_min_freq = &OEM_sys_get_cpufreq_cpuinfo_min_freq;
	devman_plugin_interface_emul.OEM_sys_get_cpufreq_scaling_max_freq = &OEM_sys_get_cpufreq_scaling_max_freq;
	devman_plugin_interface_emul.OEM_sys_set_cpufreq_scaling_max_freq = &OEM_sys_set_cpufreq_scaling_max_freq;
	devman_plugin_interface_emul.OEM_sys_get_cpufreq_scaling_min_freq = &OEM_sys_get_cpufreq_scaling_min_freq;
	devman_plugin_interface_emul.OEM_sys_set_cpufreq_scaling_min_freq = &OEM_sys_set_cpufreq_scaling_min_freq;

	devman_plugin_interface_emul.OEM_sys_get_backlight_brightness_by_lux = &OEM_sys_get_backlight_brightness_by_lux;
	OEM_sys_display_info(disp_info);
	OEM_sys_muic_node_path_info();

	return &devman_plugin_interface_emul;
}
