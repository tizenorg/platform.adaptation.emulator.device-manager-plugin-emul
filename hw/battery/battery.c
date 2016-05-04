/*
 * device-node
 *
 * Copyright (c) 2015 Samsung Electronics Co., Ltd.
 *
 * Licensed under the Apache License, Version 2.0 (the License);
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <linux/limits.h>
#include <dirent.h>

#include <hw/battery.h>
#include "../shared.h"
#include "../dbus.h"

#define BATTERY_BUS      "org.tizen.system.deviced"
#define BATTERY_OBJECT   "/Org/Tizen/System/DeviceD/SysNoti"
#define BATTERY_IFACE    BATTERY_BUS".SysNoti"
#define BATTERY_SIGNAL   "power_supply"

#define FILE_BATTERY_CAPACITY "/sys/class/power_supply/battery/capacity"
#define FILE_BATTERY_CHARGER_ONLINE "/sys/devices/platform/jack/charger_online"
#define FILE_BATTERY_CHARGE_FULL "/sys/class/power_supply/battery/charge_full"
#define FILE_BATTERY_CHARGE_NOW "/sys/class/power_supply/battery/charge_now"

static struct signal_data {
	BatteryUpdated updated_cb;
	void *data;
} sdata = { 0, };

static int id; /* signal handler id */

static int get_power_source(int online, char **src)
{
	if (!src)
		return -EINVAL;

	switch (online) {
	case 2:
		*src = POWER_SOURCE_AC;
		break;
	case 4:
		*src = POWER_SOURCE_USB;
		break;
	default:
		*src = POWER_SOURCE_NONE;
		break;
	}

	return 0;
}

static void signal_delivered(GDBusConnection *conn,
		const gchar *sender,
		const gchar *object,
		const gchar *iface,
		const gchar *signal,
		GVariant *parameters,
		gpointer user_data)
{
	struct battery_info info;
	int num, ret;
	char *sig, *capacity, *status, *health, *online, *present;
	char *val;

	if (strncmp(signal, BATTERY_SIGNAL, sizeof(BATTERY_SIGNAL)))
		return;

	_I("POWER_SUPPLY uevent is delivered");

	if (!sdata.updated_cb) {
		_E("POWER_SUPPLY callback is NULL");
		return;
	}

	g_variant_get(parameters, "(sisssss)", &sig, &num,
			&capacity, &status, &health, &online, &present);

	info.name = BATTERY_HARDWARE_DEVICE_ID;
	info.status = status;
	info.health = health;
	info.online = atoi(online);
	info.present = atoi(present);
	info.capacity = atoi(capacity);

	if (!strncmp(status, "Charging", strlen(status))) {
		info.current_now = 1000; /* uA */
		info.current_average = 1000; /* uA */
	} else {
		info.current_now = -1000; /* uA */
		info.current_average = -1000; /* uA */
	}

	ret = get_power_source(info.online, &val);
	if (ret < 0)
		goto out;
	info.power_source = val;

	sdata.updated_cb(&info, sdata.data);

out:
	free(status);
	free(health);
	free(online);
	free(present);
	free(capacity);
}

static int battery_register_changed_event(
		BatteryUpdated updated_cb, void *data)
{
	int ret;

	if (sdata.updated_cb) {
		_E("update callback is already registered");
		return -EEXIST;
	}

	ret = register_dbus_signal(BATTERY_OBJECT,
			BATTERY_IFACE, BATTERY_SIGNAL,
			signal_delivered, &sdata, &id);
	if (ret < 0) {
		_E("Failed to register signal");
		return -ENOMEM;
	}

	sdata.updated_cb = updated_cb;
	sdata.data = data;

	return ret;
}

static void battery_unregister_changed_event(
		BatteryUpdated updated_cb)
{
	unregister_dbus_signal(&id);
	sdata.updated_cb = NULL;
	sdata.data = NULL;
}

static int battery_get_current_state(
		BatteryUpdated updated_cb, void *data)
{
	int ret;
	struct battery_info info;
	char *status;
	char *val;
	char capacity_str[8];
	char charger_str[8];
	char charge_full_str[8];
	char charge_now_str[8];
	int capacity, charger, charge_full, charge_now;

	if (!updated_cb)
		return -EINVAL;

	info.name = BATTERY_HARDWARE_DEVICE_ID;

	ret = sys_get_str(FILE_BATTERY_CAPACITY, capacity_str, sizeof(capacity_str));
	if (ret < 0) {
		_E("Failed to get value of (%s, %d)", FILE_BATTERY_CAPACITY, ret);
		return ret;
	}
	ret = sys_get_str(FILE_BATTERY_CHARGER_ONLINE, charger_str, sizeof(charger_str));
	if (ret < 0) {
		_E("Failed to get value of (%s, %d)", FILE_BATTERY_CHARGER_ONLINE, ret);
		return ret;
	}
	ret = sys_get_str(FILE_BATTERY_CHARGE_FULL, charge_full_str, sizeof(charge_full_str));
	if (ret < 0) {
		_E("Failed to get value of (%s, %d)", FILE_BATTERY_CHARGE_FULL, ret);
		return ret;
	}
	ret = sys_get_str(FILE_BATTERY_CHARGE_NOW, charge_now_str, sizeof(charge_now_str));
	if (ret < 0) {
		_E("Failed to get value of (%s, %d)", FILE_BATTERY_CHARGE_NOW, ret);
		return ret;
	}

	capacity = atoi(capacity_str);
	charger = atoi(charger_str);
	charge_full = atoi(charge_full_str);
	charge_now = atoi(charge_now_str);

	if (charge_full == 1)
		status = "Full";
	else if (charge_now == 1)
		status = "Charging";
	else
		status = "Discharging";
	info.status = status;

	info.health = "Good";
	info.online = charger + 1;
	info.present = 1;
	info.capacity = capacity;

	if (charge_now == 1) {
		info.current_now = 1000; /* uA */
		info.current_average = 1000; /* uA */
	} else {
		info.current_now = -1000; /* uA */
		info.current_average = -1000; /* uA */
	}

	ret = get_power_source(info.online, &val);
	if (ret < 0)
		return ret;
	info.power_source = val;

	updated_cb(&info, data);

	return 0;
}

static int battery_open(struct hw_info *info,
		const char *id, struct hw_common **common)
{
	struct battery_device *battery_dev;

	if (!info || !common)
		return -EINVAL;

	battery_dev = calloc(1, sizeof(struct battery_device));
	if (!battery_dev)
		return -ENOMEM;

	battery_dev->common.info = info;
	battery_dev->register_changed_event
		= battery_register_changed_event;
	battery_dev->unregister_changed_event
		= battery_unregister_changed_event;
	battery_dev->get_current_state
		= battery_get_current_state;

	*common = (struct hw_common *)battery_dev;
	return 0;
}

static int battery_close(struct hw_common *common)
{
	if (!common)
		return -EINVAL;

	free(common);
	return 0;
}

HARDWARE_MODULE_STRUCTURE = {
	.magic = HARDWARE_INFO_TAG,
	.hal_version = HARDWARE_INFO_VERSION,
	.device_version = BATTERY_HARDWARE_DEVICE_VERSION,
	.id = BATTERY_HARDWARE_DEVICE_ID,
	.name = "battery",
	.open = battery_open,
	.close = battery_close,
};
