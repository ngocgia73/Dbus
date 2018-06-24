#include "stdio.h"
#include "stdlib.h"
#include "unistd.h"
#include "string.h"
#include "dbus/dbus.h"

#include "handle_reply_dbus.h"


/*
 * Reply to specific process
 */
void reply_dbus_method_to_process(DBusMessage* msg, DBusConnection* conn, void * data)
{
    DBusMessage* reply;
    dbus_uint32_t retval = 0;
    //Check Dbus type of streaming status, no require any args from source
#if 1
    // read the parameters
    if( !dbus_message_get_args(msg, NULL,
                               DBUS_TYPE_INT32, &retval,
                               DBUS_TYPE_INVALID))
        {
        fprintf(stdout,"Not found -> Args\n");
        return;
        }
#endif

    // create a reply from the message
    reply = dbus_message_new_method_return(msg);
    if (reply == NULL)
        {
        fprintf(stdout,"Can't allocate\n");
        goto __error;
        }
    // append arguments
    //  return with recevied value  
    if (!dbus_message_append_args(reply,
                                  DBUS_TYPE_INT32, &retval,
                                  DBUS_TYPE_INVALID))
        {
        fprintf(stdout,"Ran out of memory while constructing args\n");
        goto __error;
        }

    // send the reply && flush the connection
    if (!dbus_connection_send(conn, reply, &retval))
        {
        fprintf(stdout,"Out Of Memory!\n");
        goto __error;
        }
    dbus_connection_flush(conn);

__error:
    // free the reply
    dbus_message_unref(reply);
}

