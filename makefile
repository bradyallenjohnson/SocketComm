include ../makefile.include

LIBRARY_NAME=socketServer
LIBRARY=$(LIBRARY_PREFIX)$(LIBRARY_NAME)$(LIBRARY_SUFFIX)
OBJS=ServerHandler.o SocketServer.o UdpSocketServerImpl.o

#
# Targets
#

all: $(OBJS) $(LIBRARY)

$(LIBRARY): $(OBJS)
	$(AR) r $(LIB_DIR)/$(LIBRARY) $(OBJS)
	$(COPY_CMD) *.h $(INCLUDE_DIR)/socketServer

ServerHandler.o: ServerHandler.cpp ServerHandler.h
	$(CC) $(CCFLAGS) $(INCLUDE_PATH) -c ServerHandler.cpp -o ServerHandler$(OBJ_EXTENSION)

SocketServer.o: SocketServer.cpp SocketServer.h
	$(CC) $(CCFLAGS) $(INCLUDE_PATH) -c SocketServer.cpp -o SocketServer$(OBJ_EXTENSION)

UdpSocketServerImpl.o: UdpSocketServerImpl.cpp UdpSocketServerImpl.h SocketServer.h
	$(CC) $(CCFLAGS) $(INCLUDE_PATH) -c UdpSocketServerImpl.cpp -o UdpSocketServerImpl$(OBJ_EXTENSION)

clean:
	$(DELETE_CMD) $(OBJS) $(LIB_DIR)/$(LIBRARY) $(INCLUDE_DIR)/socketServer/*.h
