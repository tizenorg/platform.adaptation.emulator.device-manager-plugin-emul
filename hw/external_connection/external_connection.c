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
#include <stdbool.h>
#include <string.h>
#include <errno.h>
#include <linux/limits.h>
#include <dirent.h>

#include <hw/external_connection.h>
#include "../shared.h"
#include "../dbus.h"

#define EXTCON_BUS      "org.tizen.system.deviced"
#define EXTCON_OBJECT   "/Org/Tizen/System/DeviceD/ExtCon"
#define EXTCON_IFACE    EXTCON_BUS".ExtCon"
#define EXTCON_SIGNAL   "device_changed"

#define EXTCON_EARJACK  "earjack"
#define EXTCON_USB      "usb"

#define EXTCON_EARJACK_PATH "/sys/devices/platform/jack/earjack_online"
#define EXTCON_USB_PATH     "/sys/devices/platform/jack/usb_online"

struct extcon_device {
	char *name;
	char *type;
	char *path;
} extcon_devices[] = {
	{ EXTCON_USB,     EXTERNAL_CONNECTION_USB,       EXTCON_USB_PATH,     },
	{ EXTCON_EARJACK, EXTERNAL_CONNECTION_HEADPHONE, EXTCON_EARJACK_PATH, },
};

static int id; /* signal handler id */

static struct signal_data {
	ConnectionUpdated updated_cb;
	void *data;
} sdata = { 0, };

static void signal_delivered(GDBusConnection *conn,
		const gchar *sender,
		const gchar *object,
		const gchar *iface,
		const gchar *signal,
		GVariant *parameters,
		gpointer user_data)
{
	struct connection_info info;
	char *sig, *name, *state;
	char st[32];
	int num, i;
	size_t len;
	bool found;

	if (strncmp(signal, EXTCON_SIGNAL, sizeof(EXTCON_SIGNAL)))
		return;

	g_variant_get(parameters, "(siss)", &sig, &num, &name, &state);

	_I("extcon uevent is delivered: (%s) state (%s)", name, state);

	found = false;
	len = strlen(name) + 1;
	for (i = 0 ; i < ARRAY_SIZE(extcon_devices) ; i++) {
		if (strncmp(name, extcon_devices[i].name, len))
			continue;
		found = true;
		break;
	}
	if (!found)
		goto out;

	info.name = extcon_devices[i].type;
	snprintf(st, sizeof(st), "%s", state);
	info.state = st;
	info.flags = 0;

	if (sdata.updated_cb)
		sdata.updated_cb(&info, sdata.data);
	else
			_E("callback is NULL");

out:
	free(sig);
	free(name);
	free(state);
}

static int external_connection_register_changed_event(
		ConnectionUpdated updated_cb, void *data)
{
	int ret;

	if (sdata.updated_cb) {
		_E("update callback is already registered");
		return -EEXIST;
	}

	ret = register_dbus_signal(EXTCON_OBJECT,
			EXTCON_IFACE, EXTCON_SIGNAL,
			signal_delivered, &sdata, &id);
	if (ret < 0) {
		_E("Failed to register signal");
		return -ENOMEM;
	}

	sdata.updated_cb = updated_cb;
	sdata.data = data;

	return 0;
}

static void external_connection_unregister_changed_event(
		ConnectionUpdated updated_cb)
{
	unregister_dbus_signal(&id);
	sdata.updated_cb = NULL;
	sdata.data = NULL;
}

static int external_connection_get_current_state(
		ConnectionUpdated updated_cb, void *data)
{
	int ret, i, val;
	struct connection_info info;
	char buf[8];

	if (!updated_cb)
		return -EINVAL;

	for (i = 0 ; i < ARRAY_SIZE(extcon_devices) ; i++) {
		ret = sys_get_int(extcon_devices[i].path, &val);
		if (ret < 0) {
			_E("Failed to get value of (%s, ret:%d)",
					extcon_devices[i].path, ret);
			continue;
		}

		info.name = extcon_devices[i].type;
		snprintf(buf, sizeof(buf), "%d", val);
		info.state = buf;

		updated_cb(&info, data);
	}

	return 0;
}

static int external_connection_open(struct hw_info *info,
		const char *id, struct hw_common **common)
{
	struct external_connection_device *external_connection_dev;

	if (!info || !common)
		return -EINVAL;

	external_connection_dev = calloc(1, sizeof(struct external_connection_device));
	if (!external_connection_dev)
		return -ENOMEM;

	external_connection_dev->common.info = info;
	external_connection_dev->register_changed_event
		= external_connection_register_changed_event;
	external_connection_dev->unregister_changed_event
		= external_connection_unregister_changed_event;
	external_connection_dev->get_current_state
		= external_connection_get_current_state;

	*common = (struct hw_common *)external_connection_dev;
	return 0;
}

static int external_connection_close(struct hw_common *common)
{
	if (!common)
		return -EINVAL;

	free(common);
	return 0;
}

HARDWARE_MODULE_STRUCTURE = {
	.magic = HARDWARE_INFO_TAG,
	.hal_version = HARDWARE_INFO_VERSION,
	.device_version = EXTERNAL_CONNECTION_HARDWARE_DEVICE_VERSION,
	.id = EXTERNAL_CONNECTION_HARDWARE_DEVICE_ID,
	.name = "external_connection",
	.open = external_connection_open,
	.close = external_connection_close,
};
