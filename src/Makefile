OBJS = main.o init_dbus_module.o handle_send_dbus.o com_giann_dbus.o handle_reply_dbus.o 
CFLAG = -Wall -g
CC = gcc
INCLUDE = -I ${PWD}/../inc/ \
	  -I /usr/include/dbus-1.0/ \
   	  -I/usr/lib/x86_64-linux-gnu/dbus-1.0/include \
	  -I/usr/include/glib-2.0 \
	  -I/usr/lib/x86_64-linux-gnu/glib-2.0/include/ 
LIBS = -lm -lpthread -ldbus-1 -ldbus-glib-1
TARGET = demo_dbus

${TARGET}:${OBJS}
	${CC} ${CFLAG} ${INCLUDE} -o $@ ${OBJS} ${LIBS}
clean:
	-rm -f *.o demo_dbus
.c.o:
	${CC} ${CFLAG} ${INCLUDE} -c $<

