
#ifndef UDP_SOCKET_HANDLER_IMPL_H
#define UDP_SOCKET_HANDLER_IMPL_H

#include "SocketHandler.h"

//
// Concrete UDP implementation of SocketHandler
//
class UdpSocketHandlerImpl : public SocketHandler
{
  public:
    UdpSocketHandlerImpl();
    UdpSocketHandlerImpl(int port,
                         const string &ipAddr = LOCAL_HOST_STR,
                         int timeout_millis = 0,
                         SocketHandler::SOCKET_HANDLER_MODE mode = SocketHandler::MODE_UNDEFINED,
                         MessageHandler *handler = NULL);

  protected:
    virtual bool initializeSpecific();
    virtual int readSpecific(int sockFd, char *buffer);
    virtual int writeSpecific(int sockFd, char *buffer, int bufLength);

};

#endif
