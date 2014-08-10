CC=g++
DEFS=-DHAVE_NETINET_IN_H
OBJ=workercontroller_constants.o workercontroller_types.o workerHandler.o workerHandler-server.o
INCLUDE=-I/usr/local/include/thrift/ -I/usr/local/boost/include/boost-1_33_1/ -I../gen-cpp
TARGET=server
CFLAGS=`pkg-config --cflags glib-2.0 gtk+-2.0 libnotify`
LDFLAGS=`pkg-config --libs glib-2.0 gtk+-2.0 libnotify`
LIB=-L/usr/local/lib/ -lthrift -lthriftnb -lboost_thread -lthrift_c_glib -levent
server: workercontroller_constants.cpp workercontroller_types.cpp workerHandler.cpp workerHandler-server.cpp
	$(CC) $(DEFS) -o $(TARGET) $(INCLUDE) $^ $(CFLAGS) $(LDFLAGS) $(LIB)
clean:
	rm -f *.o $(TARGET)
