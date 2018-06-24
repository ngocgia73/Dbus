#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "dbus/dbus.h"
#include "pthread.h"

#include "handle_send_dbus.h"
#include "com_giann_nop.h"
/*
 * PURPOSE : Send message to other process
 * INPUT   : [action] - value you want to send out
 * OUTPUT  : None
 * RETURN  : 0 to success
 *           < 0 to fail
 *           > 0 return value
 * DESCRIPT: NONE
 */
int send_dbus_method(int action)
{
    DBusMessage* msg = NULL;
    dbus_int32_t ret, retval = 0;

    // create a new method call and check for errors
    msg = dbus_message_new_method_call(
              COM_GIANN_NAME_OF_PROCESS_CONN, // target for the method call
              COM_GIANN_NAME_OF_PROCESS_OBJ, // object to call on
              COM_GIANN_NAME_OF_PROCESS_IF, // interface to call on
              COM_GIANN_NAME_OF_PROCESS_M_NAME_OF_METHOD1); // method name
    if (NULL == msg)
        {
        fprintf(stderr,"Message Null\n");
        return -2;
        }
    dbus_message_set_auto_start (msg, TRUE);

#if 0
    if(dbus_message_set_destination (msg,
            COM_GIANN_NAME_OF_PROCESS_CONN) == FALSE)
        {
        fprintf(stderr,"Not enough memory\n");
        ret = -3;
        goto __error;
        }
#endif
#if 1
    // append arguments
    if (!dbus_message_append_args(msg,
                                  DBUS_TYPE_INT32, &action,
                                  DBUS_TYPE_INVALID))
        {
        fprintf(stderr,"Ran out of memory while constructing args\n");
        ret = -3;
        goto __error;
        }
#endif
    ret = com_giann_dbus_send_method_sync(&msg);
    // read the parameters
    if (ret != 0)
        {
        fprintf(stderr,"DBUS send ret %d\n", ret);
        ret = -4;
        goto __error;
        }

    // read the parameters
    if( !dbus_message_get_args(msg, NULL,
                               DBUS_TYPE_INT32, &retval,
                               DBUS_TYPE_INVALID))
        {
        ret = -5;
        fprintf(stderr,"Not found");
        }
    else
        {
        ret = retval;
        }
__error:
    // free reply
    if(msg)
        dbus_message_unref(msg);
    return ret;
}

/*
 * Send signal to other process
 */
int send_dbus_signal(int status)
{
    DBusMessage* msg;

    int ret = 0;

    msg = dbus_message_new_signal(
              COM_GIANN_NAME_OF_PROCESS_OBJ,
              COM_GIANN_NAME_OF_PROCESS_IF, // interface name of the signal
              COM_GIANN_NAME_OF_PROCESS_S_NAME_OF_SIGNAL1); // name of the signal

    if (NULL == msg)
        {
        fprintf(stderr,"Message Null\n");
        return -2;
        }

    if (!dbus_message_append_args(msg,
                                  DBUS_TYPE_INT32, &status,
                                  DBUS_TYPE_INVALID))
        {
        fprintf(stderr,"Out Of Memory!\n");
        ret = -4;
        goto __error;
        }

    ret = com_giann_dbus_send_broadcast(&msg);
    if (ret != 0)
        ret = -4;
__error:
    // free reply
    if(msg)
        dbus_message_unref(msg);
    return ret;

}


