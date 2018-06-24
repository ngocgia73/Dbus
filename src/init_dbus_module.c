#include "stdio.h"
#include "string.h"
#include "stdlib.h"
#include "unistd.h"
#include "dbus/dbus.h"

#include "dbus_service.h"
#include "com_giann_dbus.h"
#include "com_giann_nop.h"
#include "init_dbus_module.h"

static DBusHandlerResult filter_func(
    DBusConnection *conn, DBusMessage *msg, void *usrdata)
    {
    int i32Ret = DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
    int state = 0;
    if (dbus_message_is_signal(msg,
                                    COM_GIANN_NAME_OF_PROCESS_IF,
                                    COM_GIANN_NAME_OF_PROCESS_S_NAME_OF_SIGNAL1))
        {
        if( !dbus_message_get_args(msg, NULL,
                                   DBUS_TYPE_INT32, &state,
                                   DBUS_TYPE_INVALID))
            {
            fprintf(stderr,"Not found status, ignore \n");
            }
        else
            {
            fprintf(stdout,"DBUS - IF:%s Signal:%s - Value(%d)\n",
                        COM_GIANN_NAME_OF_PROCESS_IF,
                        COM_GIANN_NAME_OF_PROCESS_S_NAME_OF_SIGNAL1, state);
		// do something
            }

            i32Ret = DBUS_HANDLER_RESULT_HANDLED;
        }
    else
    if (dbus_message_is_signal(msg,
                                COM_GIANN_NAME_OF_PROCESS_IF,
                                COM_GIANN_NAME_OF_PROCESS_S_NAME_OF_SIGNAL2))
    {
	    if( !dbus_message_get_args(msg, NULL,
	                               DBUS_TYPE_INT32, &state,
	                               DBUS_TYPE_INVALID))
	        {
	        fprintf(stderr,"Not found status, ignore \n");
	        }
	    else
	        {
	        fprintf(stdout,"DBUS - IF:%s Signal2:%s - Value(%d)\n",
	                    COM_GIANN_NAME_OF_PROCESS_IF,
	                    COM_GIANN_NAME_OF_PROCESS_S_NAME_OF_SIGNAL2, state);
	    	// do something
	        }

            i32Ret = DBUS_HANDLER_RESULT_HANDLED;
    }

    	return i32Ret;
    }


/*
 * Handle messages for a_obj object
 */
static DBusHandlerResult nop_obj_message_func (DBusConnection *conn,
        DBusMessage *msg, void *usrdata)
    {
    int i32Ret = DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
    fprintf(stdout,"if: %s, member: %s, path: %s\n",
           dbus_message_get_interface(msg),
           dbus_message_get_member(msg),
           dbus_message_get_path(msg));

    if (dbus_message_is_method_call(msg,
                                    COM_GIANN_NAME_OF_PROCESS_IF,
                                    COM_GIANN_NAME_OF_PROCESS_M_NAME_OF_METHOD1))
        {
		// do somthing
		reply_dbus_method_to_process(msg, conn, usrdata);
		i32Ret = DBUS_HANDLER_RESULT_HANDLED;
        }
    else
        {
        fprintf(stderr,"Not found un-definition message method\n");
        }

    return i32Ret;
    }

// nof => name of process
static DBusObjectPathVTable nop_obj_vtable =
{
	NULL,
	nop_obj_message_func,
	NULL
};


int dbus_service_start(DBusServiceCtx_t *ctx, int flag)
{
	int ret = 0;
	if(ctx == NULL)
	    return -1;
	ret = com_giann_dbus_open(ctx, COM_GIANN_NAME_OF_PROCESS_CONN);
	if(ret != 0)
	    {
	    fprintf(stderr,"init error: %d\n",ret);
	    return 1;
	    }
	com_giann_dbus_register(ctx, "type='signal'", filter_func, ctx);
	
	
	// Add own object
	com_giann_dbus_add_obj(ctx, COM_GIANN_NAME_OF_PROCESS_OBJ,
	                           &nop_obj_vtable, ctx);
	if(flag == 0)
	    {
	    ctx->flag = 0;
	    ret = com_giann_dbus_run(ctx, 1);
	    if (ret != 0)
	        {
	        fprintf(stderr,"start fail %d\n", ret);
	        return 2;
	        }
	    }
	else
	    {
	    ctx->flag = 1;
	    com_giann_dbus_run(ctx, 0);
	    com_giann_dbus_close(ctx);
	    }
	return 0;
}

int dbus_service_stop(DBusServiceCtx_t *ctx)
{
	com_giann_dbus_close(ctx);
	return 0;
}
