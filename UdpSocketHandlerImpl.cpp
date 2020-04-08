#include <iostream>

#include <sys/socket.h>
#include <string.h> // strerror()
#include <errno.h>

#include "SocketHandler.h"
#include "UdpSocketHandlerImpl.h"

using namespace std;

UdpSocketHandlerImpl::UdpSocketHandlerImpl()
{
}

UdpSocketHandlerImpl::UdpSocketHandlerImpl(int port,
                                           const string &ipAddr,                     /* default LOCAL_HOST_STR */
                                           int timeout_millis,                       /* default 0*/
                                           SocketHandler::SOCKET_HANDLER_MODE mode,  /* default MODE_UNDEFINED */
                                           MessageHandler *handler) :                /* default NULL */
    SocketHandler(port, ipAddr, timeout_millis, mode, handler)
{
}

// virtual
bool UdpSocketHandlerImpl::initializeSpecific()
{
  socketFd_ = socket((socketAddress_.isIpv6 ? AF_INET6 : AF_INET), SOCK_DGRAM, 0);
  if(socketFd_ < 0)
  {
    int theError(errno);
    cerr << "Error initializing UDP socket, errno: " << theError
         << ", " << strerror(theError)
         << std::endl;
    return false;
  }

  if(mode_ == SocketHandler::MODE_SERVER)
  {
    if(bind(socketFd_,
        (socketAddress_.isIpv6 ? (struct sockaddr *) &socketAddress_.socketAddrIn.sockAddrIpv6 :
                                 (struct sockaddr *) &socketAddress_.socketAddrIn.sockAddrIpv4),
        (socketAddress_.isIpv6 ? sizeof(struct sockaddr_in6) : sizeof(struct sockaddr_in))) < 0)
    {
      int theError(errno);
      std::cerr << "Error binding socket, errno: " << theError
          << ", " << strerror(theError)
          << std::endl;
      return false;
    }

    if(isDebug())
    {
      std::cout << "UDP server successfully bound to port: " << port_;
    }
  }
  else if(mode_ == MODE_CLIENT)
  {
    // Only set it writable if in client mode
    FD_SET(socketFd_, &allSocketFdWriteSet_);
  }


  return true;
}

// virtual
int UdpSocketHandlerImpl::readSpecific(int sockFd, char *buffer)
{
  // TODO need to check if more bytes are avail to be read
  int numBytesRead(recvfrom(sockFd,
                            buffer,
                            MAX_MESSAGE_LENGTH,
                            0, // flags
                            (remoteAddress_.isIpv6 ?
                                 (struct sockaddr *) &remoteAddress_.socketAddrIn.sockAddrIpv6 :
                                 (struct sockaddr *) &remoteAddress_.socketAddrIn.sockAddrIpv4),
                            &remoteAddrLen_));

  return numBytesRead;
}

// virtual
int UdpSocketHandlerImpl::writeSpecific(int sockFd, char *buffer, int bufLength)
{
  int numBytesWritten(sendto(sockFd,
                             buffer,
                             bufLength,
                             0, // flags
                             (remoteAddress_.isIpv6 ?
                                 (struct sockaddr *) &remoteAddress_.socketAddrIn.sockAddrIpv6 :
                                 (struct sockaddr *) &remoteAddress_.socketAddrIn.sockAddrIpv4),
                             remoteAddrLen_));

  return numBytesWritten;
}

