#ifndef __COM_GIANN_DBUS_H__
#define __COM_GIANN_DBUS_H__
/*#define DBUS_CONN_PRIVATE*/
#define COM_GIANN_DBUS_TYPE 	DBUS_BUS_SYSTEM
#include "dbus_service.h"
int com_giann_dbus_open(DBusServiceCtx_t *ctx, char *conn_name);
int com_giann_dbus_close(DBusServiceCtx_t *ctx);

int com_giann_dbus_register(DBusServiceCtx_t *ctx, char *match, DBusHandleMessageFunction cb, void *priv);
int com_giann_dbus_deregister(DBusServiceCtx_t *ctx, char *match, DBusHandleMessageFunction cb);

int com_giann_dbus_add_obj(DBusServiceCtx_t *ctx, char *obj_path, DBusObjectPathVTable *vtable, void *priv);
int com_giann_dbus_remove_obj(DBusServiceCtx_t *ctx, char *obj_path);

int com_giann_dbus_run(DBusServiceCtx_t *ctx, int detach);
int com_giann_dbus_send_broadcast(DBusMessage **msg_in);

int com_giann_dbus_send_method_sync(DBusMessage **msg);
int com_giann_dbus_send_method_sync_timeout(DBusMessage **msg_in, int timeout);
int com_giann_dbus_send_method_async(DBusServiceCtx_t *ctx, DBusMessage *msg, void (*cb)(DBusMessage *, void *), void *user_data);

#endif // __COM_GIANN_DBUS_H__
