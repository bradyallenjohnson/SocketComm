
#include <iostream>
#include <map>

#include <string.h> // strerror()
#include <errno.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/tcp.h>

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
  socketFd_ = socket((socketAddress_.isIpv6 ? AF_INET6 : AF_INET), SOCK_STREAM, IPPROTO_TCP);
  if(socketFd_ < 0)
  {
    int theError(errno); // stdio may reset the errno
    cerr << "Error initializing TCP socket, errno: " << theError
         << ", " << strerror(theError)
         << endl;
    return false;
  }

  // RFC 2385 TCP MD5 Authentication
  if(!tcpMd5AuthStr_.empty())
  {
    struct tcp_md5sig sig;
    memset(&sig, 0, sizeof(sig));
    memcpy(&sig.tcpm_addr,
           (socketAddress_.isIpv6 ? (struct sockaddr *) &socketAddress_.socketAddrIn.sockAddrIpv6 :
                                    (struct sockaddr *) &socketAddress_.socketAddrIn.sockAddrIpv4),
           (socketAddress_.isIpv6 ? sizeof(struct sockaddr_in6) : sizeof(struct sockaddr_in)));

    if(mode_ == SocketHandler::MODE_SERVER)
    {
      sig.tcpm_flags = TCP_MD5SIG_FLAG_PREFIX;
      sig.tcpm_prefixlen = 0; // Match any address.
    }
    sig.tcpm_keylen = tcpMd5AuthStr_.length();
    memcpy(sig.tcpm_key, tcpMd5AuthStr_.c_str(), sig.tcpm_keylen);
    if (setsockopt(socketFd_, IPPROTO_TCP, TCP_MD5SIG_EXT, &sig, sizeof(sig)) == -1) {
      cerr << "Failed to setsockopt(): " << strerror(errno) << endl;
      return false;
    }
  }

  // Client / Server specific actions
  if(mode_ == SocketHandler::MODE_SERVER)
  {
      return initializeSpecificServer();
  }
  else if(mode_ == SocketHandler::MODE_CLIENT)
  {
      return initializeSpecificClient();
  }

  cerr << "Error: Unknown SocketHandler mode: " << mode_
       << endl;

  return false;
}

bool TcpSocketHandlerImpl::initializeSpecificClient()
{
  if(connect(socketFd_,
      (socketAddress_.isIpv6 ? (struct sockaddr *) &socketAddress_.socketAddrIn.sockAddrIpv6 :
                               (struct sockaddr *) &socketAddress_.socketAddrIn.sockAddrIpv4),
      (socketAddress_.isIpv6 ? sizeof(struct sockaddr_in6) : sizeof(struct sockaddr_in))) < 0)
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

  return true;
}

bool TcpSocketHandlerImpl::initializeSpecificServer()
{
  if(bind(socketFd_,
      (socketAddress_.isIpv6 ? (struct sockaddr *) &socketAddress_.socketAddrIn.sockAddrIpv6 :
                               (struct sockaddr *) &socketAddress_.socketAddrIn.sockAddrIpv4),
      (socketAddress_.isIpv6 ? sizeof(struct sockaddr_in6) : sizeof(struct sockaddr_in))) < 0)
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

  return true;
}

// Will only be called when mode_ == MODE_SERVER
void TcpSocketHandlerImpl::acceptConnection()
{
  int clientFd(accept(socketFd_,
        (remoteAddress_.isIpv6 ? (sockaddr*) &remoteAddress_.socketAddrIn.sockAddrIpv6 :
                                 (sockaddr*) &remoteAddress_.socketAddrIn.sockAddrIpv4),
        (socklen_t*) &remoteAddrLen_));
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

