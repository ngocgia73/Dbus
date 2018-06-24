#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "unistd.h"
#include "pthread.h"
#include "dbus/dbus.h"
#include "errno.h"
#include "com_giann_dbus.h"


#ifndef DEF_STACK_SIZE
#define DEF_STACK_SIZE 512
#endif

typedef struct DBusPending_t
    {
    void (*cb)(DBusMessage *, void*);
    void * data;
    } DBusPending_t;

static void pending_cb(DBusPendingCall *pending, void *user_data)
    {
    DBusMessage* msg;
    DBusPending_t *dbuspending = NULL;

    if(pending == NULL)
        return;

    if(user_data == NULL)
        {
        //dbus_pending_call_unref(pending);
        return;
        }
    dbuspending = (DBusPending_t*)(user_data);
    // get the reply message
    msg = dbus_pending_call_steal_reply(pending);
    if (NULL == msg)
        {
        fprintf(stderr,"Reply Null\n");
        dbus_pending_call_unref(pending);
        return;
        }
    // free the pending message handle
    dbus_pending_call_unref(pending);
    if(dbuspending->cb)
        dbuspending->cb(msg, dbuspending->data);
    }

static void pending_free(void * user_data)
    {
    if(user_data)
        free(user_data);
    }

/*
- get a bus
- request bus name if conn_name is valid (not null)
*/
int com_giann_dbus_open(DBusServiceCtx_t *ctx,
                            char* conn_name)
    {
    DBusError error;
    DBusConnection * conn;
    int ret = 0;

    dbus_error_init(&error);
#ifndef DBUS_CONN_PRIVATE
    //conn = dbus_bus_get(DBUS_BUS_SYSTEM, &error);
    //conn = dbus_bus_get(DBUS_BUS_STARTER, &error);
    conn = dbus_bus_get(COM_GIANN_DBUS_TYPE, &error);
#else
    conn = dbus_bus_get_private(COM_GIANN_DBUS_TYPE, &error);
#endif
    if (conn == NULL)
        {
        fprintf(stderr,"Error when connecting to the bus: %s\n",
                    error.message);
        return -1;
        }
    fprintf(stdout,"%s\n",dbus_bus_get_unique_name(conn));
    //dbus_connection_set_exit_on_disconnect(ctx->conn, FALSE);
    ctx->quit = 0;
    ctx->conn = conn;
    //FIXME copy
    ctx->conn_name = conn_name;
    ctx->pthread = pthread_self();
    if(conn_name != 0)
        {
        // request our name on the bus and check for errors
        ret = dbus_bus_request_name(ctx->conn, ctx->conn_name,
                                    DBUS_NAME_FLAG_REPLACE_EXISTING , &error);
        if (dbus_error_is_set(&error))
            {
            fprintf(stderr,"Name Error (%s)\n", error.message);
            dbus_error_free(&error);
            ret = -4;
            goto __error;
            }
        else if (DBUS_REQUEST_NAME_REPLY_PRIMARY_OWNER != ret)
            {
            fprintf(stderr,"Not Primary Owner (%d)\n", ret);
            ret = -5;
            goto __error;
            }
        else
            ret = 0;
        }
    else
        {
        ret = 0;
        }
    return ret;
__error:
    dbus_connection_unref(ctx->conn);
    return ret;
    }


int com_giann_dbus_close(DBusServiceCtx_t *ctx)
{
	ctx->quit = 1;
	pthread_join(ctx->pthread, NULL);
	if(ctx->conn_name)
		dbus_bus_release_name(ctx->conn, ctx->conn_name, NULL);
#ifndef DBUS_CONN_PRIVATE
	dbus_connection_unref(ctx->conn);
#else
	dbus_connection_close(ctx->conn);
#endif
	return 0;

}

/*
 * add a match
 * add a cb by add filter
 */
int com_giann_dbus_register(DBusServiceCtx_t *ctx, char *match, DBusHandleMessageFunction cb, void *priv)
{
    if(ctx->conn == NULL || cb == NULL)
        return -1;
    if (!dbus_connection_add_filter(ctx->conn, cb, priv, NULL))
        {
        fprintf(stdout,"Failed to add signal handler callback\n");
        return -1;
        }
    if(match)
        dbus_bus_add_match(ctx->conn, match, NULL);
    return 0;

}

/*
 *  remove the filter
 *  remove the match
 */
int com_giann_dbus_deregister(DBusServiceCtx_t *ctx, char *match, DBusHandleMessageFunction cb)
{
    if(ctx->conn == NULL || cb == NULL)
        return -1;
    dbus_connection_remove_filter(ctx->conn, cb, NULL);
    if(match)
        dbus_bus_remove_match(ctx->conn, match, NULL);
    return 0;
}

/*
 * cv_vtable
 * process for own interfaces (signals, methods)
 */

int com_giann_dbus_add_obj(DBusServiceCtx_t *ctx, char *obj_path, DBusObjectPathVTable *vtable, void *priv)
{
    if(ctx->conn == NULL || obj_path == NULL)
        return -1;
    //! Callback for object paths
    if ( !dbus_connection_register_object_path (ctx->conn, obj_path,
            vtable, priv))
        {
        fprintf(stdout,"No memory\n");
        }
        {
        void *d;
        if (!dbus_connection_get_object_path_data (ctx->conn, obj_path, &d))
	fprintf(stdout,"No memory\n");
        }
    return 0;
}

int com_giann_dbus_remove_obj(DBusServiceCtx_t *ctx, char *obj_path)
{
    if(ctx->conn == NULL || obj_path == NULL)
        return -1;
    //! Callback for object paths
    if ( !dbus_connection_unregister_object_path (ctx->conn, obj_path))
        {
        fprintf(stdout,"No memory\n");
        return -2;
        }
    return 0;
}

static void * dbus_mainloop(DBusServiceCtx_t * ctx)
    {
    int ret = 0;
    if(ctx == NULL)
        return NULL;
    while(ctx->quit == 0)
        {
        ret = dbus_connection_read_write_dispatch(ctx->conn, 1000);
        if(ret == 0)
            {
            fprintf(stdout,"Disconnected\n");
            break;
            }
        }
    fprintf(stdout,"Exit: pid: (%d)\n",getpid());
    return NULL;
    }

/*
 *
 *  run loop for callback
 */
int com_giann_dbus_run(DBusServiceCtx_t *ctx, int detach)
{
    int ret = 0;
    int i32DefaultAttr = 0;
    pthread_attr_t  tattr;

#if (LOW_MEMORY_MODEL)
    if(pthread_attr_init(&tattr) != 0)
    	i32DefaultAttr = 1;
    if(i32DefaultAttr == 0)
        if(pthread_attr_setstacksize(&tattr, 96 * 1024) != 0)
    	     i32DefaultAttr = 1;
#else
    if(pthread_attr_init(&tattr) != 0)
        i32DefaultAttr = 1;
    if(i32DefaultAttr == 0)
        if(pthread_attr_setstacksize(&tattr, DEF_STACK_SIZE * 1024) != 0)
             i32DefaultAttr = 1;
#endif
    if(detach)
        {
        if(i32DefaultAttr == 0)
            {
            ret = pthread_create(&ctx->pthread, &tattr,
                                 (void*(*)(void*))dbus_mainloop, ctx);
            }
        else
            {
            ret = pthread_create(&ctx->pthread, NULL,
                                 (void*(*)(void*))dbus_mainloop, ctx);
            }

        if(ret != 0)
            {
            fprintf(stdout,"Couldn't create:%d(%s)", errno, strerror(errno));
            ret = -6;
            }
        }
    else
        {
        dbus_mainloop(ctx);
        }
    return ret;
    }


int com_giann_dbus_send_broadcast(DBusMessage **msg_in)
{
    DBusMessage *msg;
    DBusError error;
    DBusConnection* conn;
    dbus_uint32_t serial = 0;
    int ret = 0;
    msg = *msg_in;

    if(*msg_in == NULL)
        {
        return -1;
        }
    dbus_error_init(&error);
    // connect to the system bus and check for errors
    conn = dbus_bus_get(COM_GIANN_DBUS_TYPE, &error);
    if (dbus_error_is_set(&error))
        {
        fprintf(stderr,"Connection Error (%s)\n", error.message);
        dbus_error_free(&error);
        }
    if (NULL == conn)
        {
        fprintf(stderr,"conn=NULL\n");
        dbus_message_unref(msg);
        *msg_in = NULL;
        return -2;
        }
    if(!dbus_connection_send(conn, msg, &serial))
        {
        fprintf(stderr,"Send error\n");
        ret = -1;
        }
    else
        {
        ret = 0;
        }
    dbus_connection_flush(conn);
    dbus_message_unref(msg);
    *msg_in = NULL;
    return ret;

}

static int com_giann_dbus_send_method_sync_with_time_out(DBusMessage **msg_in, int timeout)
{
    DBusPendingCall* pending;
    DBusMessage *msg;
    DBusError error;
    DBusConnection* conn;
    msg = *msg_in;

    if(*msg_in == NULL)
        {
        return -1;
        }
#if 0
    conn = ctx->conn;
#else
    dbus_error_init(&error);
    // connect to the system bus and check for errors
    conn = dbus_bus_get(COM_GIANN_DBUS_TYPE, &error);
    if (dbus_error_is_set(&error))
        {
        fprintf(stderr,"Connection Error (%s)\n", error.message);
        dbus_error_free(&error);
        }
    if (NULL == conn)
        {
        fprintf(stderr,"conn=NULL\n");
        dbus_message_unref(msg);
        *msg_in = NULL;
        return -2;
        }
#endif
    if (!dbus_connection_send_with_reply (conn,msg, &pending, timeout))   // -1 is default timeout
        {
        fprintf(stderr,"Out Of Memory!\n");
        dbus_message_unref(msg);
        *msg_in = NULL;
        return -3;
        }

    if (NULL == pending)
        {
        fprintf(stderr,"Pending Call Null\n");
        dbus_message_unref(msg);
        *msg_in = NULL;
        return -4;
        }
    dbus_pending_call_set_notify(pending, pending_cb, NULL, pending_free);
    dbus_connection_flush(conn);

    // free message
    dbus_message_unref(msg);
    // block until we recieve a reply
    dbus_pending_call_block(pending);
    // get the reply message
    msg = dbus_pending_call_steal_reply(pending);
    if (NULL == msg)
        {
        fprintf(stderr,"Reply Null\n");
        *msg_in = NULL;
        return -5;
        }
    if( dbus_message_get_type(msg) == DBUS_MESSAGE_TYPE_ERROR)
        if( dbus_set_error_from_message(&error, msg))
            {
            fprintf(stderr,"message error: %s\n", error.name);
            dbus_error_free(&error);
            dbus_message_unref(msg);
            dbus_pending_call_unref(pending);
            *msg_in = NULL;
            return -6;
            }
    *msg_in = msg;
    // free the pending message handle
    dbus_pending_call_unref(pending);
    return 0;

}

int com_giann_dbus_send_method_sync(DBusMessage **msg_in)
{
	return com_giann_dbus_send_method_sync_with_time_out(msg_in, -1);
}
int com_giann_dbus_send_method_sync_timeout(DBusMessage **msg_in, int timeout)
{
	return com_giann_dbus_send_method_sync_with_time_out(msg_in, timeout);
}

int com_giann_dbus_send_method_async(DBusServiceCtx_t *ctx, DBusMessage *msg, void (*cb)(DBusMessage *, void *), void *user_data)
{
    DBusPendingCall* pending;
    DBusPending_t * dbuspending = NULL;
    DBusConnection* conn;

    if(msg == NULL)
        return -1;
    if(ctx == NULL)
        {
        dbus_message_unref(msg);
        return -2;
        }
    else
        conn = ctx->conn;

    dbuspending = (DBusPending_t*)malloc(sizeof(DBusPending_t));
    if(dbuspending == NULL)
        {
        dbus_message_unref(msg);
        return -2;
        }
    dbuspending->cb = cb;
    dbuspending->data = user_data;
    if (!dbus_connection_send_with_reply (conn, msg, &pending, -1))   // -1 is default timeout
        {
        fprintf(stdout,"Out Of Memory!\n");
        dbus_message_unref(msg);
        return -4;
        }

    if (NULL == pending)
        {
        fprintf(stdout,"Pending Call Null\n");
        dbus_message_unref(msg);
        return -5;
        }
    dbus_pending_call_set_notify(pending, pending_cb, dbuspending, pending_free);
    dbus_connection_flush(conn);
    // free message
    dbus_message_unref(msg);
    return 0;
}
