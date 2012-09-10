
#include <iostream>
#include <map>

#include <string.h> // strerror()
#include <errno.h>
#include <sys/socket.h>

#include "SocketHandler.h"
#include "TcpSocketHandlerImpl.h"

using namespace std;

TcpSocketHandlerImpl::TcpSocketHandlerImpl()
{
}

TcpSocketHandlerImpl::TcpSocketHandlerImpl(int port,
                                           const string &ipAddr,                    /* default LOCAL_HOST_STR */
                                           int timeout_millis,                      /* default 0*/
                                           SocketHandler::SOCKET_HANDLER_MODE mode, /* default MODE_UNDEFINED */
                                           MessageHandler *handler) :               /* default NULL */
    SocketHandler(port, ipAddr, timeout_millis, mode, handler)
{
}

// virtual
bool TcpSocketHandlerImpl::initializeSpecific()
{
  socketFd_ = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  if(socketFd_ < 0)
  {
    int theError(errno); // stdio may reset the errno
    cerr << "Error initializing TCP socket, errno: " << theError
         << ", " << strerror(theError)
         << endl;
    return false;
  }

  if(mode_ == SocketHandler::MODE_SERVER)
  {
    if(bind(socketFd_, (struct sockaddr *) &socketAddress_, sizeof(struct sockaddr_in)) < 0)
    {
      int theError(errno);
      cerr << "Error binding socket, errno: " << theError
           << ", " << strerror(theError)
           << endl;
      return false;
    }

    // max socket backlog of 5
    if(listen(socketFd_, 5) < 0)
    {
      int theError(errno);
      cerr << "Error listening on socket, errno: " << theError
           << ", " << strerror(theError)
           << endl;
      return false;
    }

    if(isDebug())
    {
      cout << "TCP server successfully bound to port: " << port_ << endl;
    }
  }
  else if(mode_ == SocketHandler::MODE_CLIENT)
  {
    if(connect(socketFd_, (struct sockaddr *) &socketAddress_, sizeof(struct sockaddr_in)) < 0)
    {
      int theError(errno);
      cerr << "Error Connecting to socket, errno: " << theError
           << ", " << strerror(theError)
           << endl;
      return false;
    }

    FD_SET(socketFd_, &allSocketFdWriteSet_);

    if(isDebug())
    {
      cout << "TCP client successfully connected to server." << endl;
    }
  }

  return true;
}

// Will only be called when mode_ == MODE_SERVER
void TcpSocketHandlerImpl::acceptConnection()
{
  int clientFd(accept(socketFd_, (sockaddr*) &remoteAddress_, (socklen_t*) &remoteAddrLen_));
  remoteSockFdList_.push_back(clientFd);
  ++stats_.numConnects;

  if(isDebug())
  {
    cout << "Accepted a new client connection, socketFd: " << socketFd_
         << ", clientSockFd: " << clientFd
         << endl;
  }

  FD_SET(clientFd, &allSocketFdReadSet_);

  if(clientFd > maxFd_)
  {
    maxFd_ = clientFd;
  }

  msgHandler_->handleConnect(clientFd, remoteAddress_, remoteAddrLen_);
}

// virtual
int TcpSocketHandlerImpl::readSpecific(int sockFd, char *buffer)
{
  int numBytesRead(-1);

  if(sockFd == socketFd_ && mode_ == SocketHandler::MODE_SERVER)
  {
    acceptConnection();
  }
  else
  {
    // TODO need to check if more bytes are avail to be read
    // zero byte reads handled in SocketServer::run()
    numBytesRead = read(sockFd, buffer, MAX_MESSAGE_LENGTH);
  }

  return numBytesRead;
}

// virtual
int TcpSocketHandlerImpl::writeSpecific(int sockFd, char *buffer, int bufLength)
{
  int numBytesWritten(write( sockFd, buffer, bufLength));

  return numBytesWritten;
}

