/*
 * device-node
 *
 * Copyright (c) 2016 Samsung Electronics Co., Ltd.
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

#include "dbus.h"
#include "shared.h"

int register_dbus_signal(const char *object,
		const char *iface, const char *signal,
		GDBusSignalCallback callback, void *data, int *id)
{
	GError *err = NULL;
	GDBusConnection *conn;
	int sid;

#if !GLIB_CHECK_VERSION(2, 35, 0)
	g_type_init();
#endif

	conn = g_bus_get_sync(G_BUS_TYPE_SYSTEM, NULL, &err);
	if (!conn) {
		_E("fail to get dbus connection : %s", err->message);
		g_clear_error(&err);
		return -EPERM;
	}

	/* subscribe signal */
	sid = g_dbus_connection_signal_subscribe(conn,
			NULL, iface, signal, object,
			NULL, G_DBUS_SIGNAL_FLAGS_NONE,
			callback, data, NULL);
	if (sid == 0) {
		_E("fail to connect %s signal", signal);
		return -EPERM;
	}

	if (id)
		*id = sid;

	return 0;
}

void unregister_dbus_signal(int *id)
{
	GError *err = NULL;
	GDBusConnection *conn;

	if (!id || *id <= 0)
		return;

	conn = g_bus_get_sync(G_BUS_TYPE_SYSTEM, NULL, &err);
	if (!conn) {
		_E("fail to get dbus connection : %s", err->message);
		g_clear_error(&err);
		return;
	}

	/* unsubscribe signal */
	g_dbus_connection_signal_unsubscribe(conn, *id);
	*id = 0;
}
