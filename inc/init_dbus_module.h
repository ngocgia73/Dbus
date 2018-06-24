#ifndef __INIT_DBUS_MODULE_H__
#define __INIT_DBUS_MODULE_H__
#include "dbus_service.h"

int dbus_service_start(DBusServiceCtx_t *ctx, int flag);
int dbus_service_stop(DBusServiceCtx_t *ctx);


#endif // __INIT_DBUS_MODULE_H__
