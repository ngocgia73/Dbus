#ifndef __DBUS_SERVICE_H__
#define __DBUS_SERVICE_H__
#include "pthread.h"
#include "dbus/dbus.h"
typedef struct DBusService
{
	DBusConnection* conn;
	char* conn_name;
	pthread_t pthread;
	int quit;
	int flag;
} DBusServiceCtx_t;


#endif // __DBUS_SERVICE_H__
