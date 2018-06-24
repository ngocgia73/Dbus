#include "stdio.h"
#include "string.h"
#include "stdlib.h"
#include "pthread.h"
#include "unistd.h"

#include "init_dbus_module.h"
#include "handle_send_dbus.h"
#include "dbus_service.h"


pthread_t dbus_thread;
static DBusServiceCtx_t ctx;
void *dbus_thread_result;

static void *dbus_thread_handle(void *argv)
{
	int i = 10;
	int ret = -1;
	while(i--)
	{
		// send signal msg every 2s
		usleep(2000*1000); 
		ret = send_dbus_signal(i);
		if (ret != 0)
		{
			fprintf(stderr,"can't send dbus signal : %d\n",i);
		}
//		ret = send_dbus_method(i);	
//		fprintf(stdout," return value from method dbus : %d\n",ret);

	}	
}
int main(int argc, char **argv)
{
	int ret;
	// start dbus service
	dbus_service_start(&ctx, 0);
	if(pthread_create(&dbus_thread, 0, dbus_thread_handle, NULL) != 0)
	{
		fprintf(stderr,"can not start dbus_thread\n");
		ret = -1;
	}
	fprintf(stdout,"waiting for thread to finish ...\n");
	ret = pthread_join(dbus_thread, &dbus_thread_result);
	if (ret != 0)
	{
		fprintf(stdout,"thread wait error\n");
		return -1;
	}
	// close dbus service
	ret = dbus_service_stop(&ctx);
	if(ret != 0)
	{
		fprintf(stderr,"can not close dbus service\n");
		ret = -1;
	}
	return 0;
}
