
#ifndef TCP_SOCKET_HANDLER_IMPL_H
#define TCP_SOCKET_HANDLER_IMPL_H

#include <string>

#include "SocketHandler.h"

using namespace std;

//
// Concrete TCP implementation of SocketHandler
//
class TcpSocketHandlerImpl : public SocketHandler
{
  public:
    TcpSocketHandlerImpl();
    TcpSocketHandlerImpl(int port,
                         const string &ipAddr = LOCAL_HOST_STR,
                         int timeout_millis = 0,
                         SocketHandler::SOCKET_HANDLER_MODE mode = SocketHandler::MODE_UNDEFINED,
                         MessageHandler *handler = NULL);

    inline void setTcpMd5AuthStr(const string &authStr) { tcpMd5AuthStr_ = authStr; }
    inline string getTcpMd5AuthStr() { return tcpMd5AuthStr_; }

  protected:
    string tcpMd5AuthStr_; /* RFC 2385 */
    virtual bool initializeSpecific();

    virtual int readSpecific(int sockFd, char *buffer);
    virtual int writeSpecific(int sockFd, char *buffer, int bufLength);

  private:
    void acceptConnection();
    bool initializeSpecificClient();
    bool initializeSpecificServer();

};

#endif
