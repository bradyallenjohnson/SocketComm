
#include <iostream>
#include <list>

#include <string.h>
#include <stdint.h> // for NULL, uint32_t, etc
#include <sys/select.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h> // for inet_pton()
#include <errno.h>
#include <unistd.h>

#include "SocketHandler.h"
#include "MessageHandler.h"
#include "SocketAddrIn.h"

using namespace std;

const int SocketHandler::MAX_MESSAGE_LENGTH = 2048;
const string SocketHandler::LOCAL_HOST_STR = "127.0.0.1";
const string SocketHandler::LOCAL_HOST_IPV6_STR = "::1";

SocketHandler::SocketHandler() :
  msgHandler_(NULL),
  port_(-1),
  maxFd_(0),
  ipAddress_(LOCAL_HOST_STR),
  socketFd_(-1),
  isDebug_(false),
  timeout_sec_(0),
  timeout_msec_(0),
  timeout_usec_(0),
  remoteAddrLen_(0),
  tval_(NULL),
  runHandler_(true),
  isInitialized_(false),
  mode_(MODE_UNDEFINED)
{
}

SocketHandler::SocketHandler(int port,
                             const string &ipAddr,      /* default LOCAL_HOST_STR */
                             uint32_t timeout_millis,   /* default 0*/
                             SOCKET_HANDLER_MODE mode,  /* default MODE_UNDEFINED*/
                             MessageHandler *handler) : /* default NULL */
  msgHandler_(handler),
  port_(port),
  socketFd_(-1),
  maxFd_(0),
  ipAddress_(ipAddr),
  isDebug_(false),
  timeout_sec_(0),
  timeout_msec_(timeout_millis),
  timeout_usec_(0),
  remoteAddrLen_(0),
  tval_(NULL),
  runHandler_(true),
  isInitialized_(false),
  mode_(mode)
{
}

SocketHandler::~SocketHandler()
{
  if(socketFd_ >= 0)
  {
    close(socketFd_);
  }
  if(tval_ != NULL)
  {
    delete tval_;
  }
}

SocketHandler::SocketHandlerStatistics::SocketHandlerStatistics() :
      numConnects(0),
      numDisconnects(0),
      numReads(0),
      numWrites(0),
      numSocketExceptions(0),
      numTimeouts(0)
{
}

SocketHandler::SocketHandlerStatistics::SocketHandlerStatistics(const SocketHandlerStatistics &rhs)
{
  numConnects          =  rhs.numConnects;
  numDisconnects       =  rhs.numDisconnects;
  numReads             =  rhs.numReads;
  numWrites            =  rhs.numWrites;
  numSocketExceptions  =  rhs.numSocketExceptions;
  numTimeouts          =  rhs.numTimeouts;
}

void SocketHandler::SocketHandlerStatistics::clear()
{
  numConnects = 0;
  numDisconnects = 0;
  numReads = 0;
  numWrites = 0;
  numSocketExceptions = 0;
  numTimeouts = 0;
}

bool SocketHandler::initialize()
{
  if(!msgHandler_)
  {
    cerr << "ERROR: SocketHandler::initialize() trying to initialize SocketHandler without a MessageHandler."
         << endl;

    return false;
  }

  if(mode_ == MODE_UNDEFINED)
  {
    cerr << "ERROR: SocketHandler::initialize() trying to initialize SocketHandler without having specified the mode."
             << endl;

        return false;
  }

  // Check if the ipAddress is IPV6
  if (ipAddress_.find(":") != string::npos || ipAddress_.find("ip6") != string::npos)
  {
      socketAddress_.isIpv6 = true;
      remoteAddress_.isIpv6 = true;
  }

  // configure the IP address and TCP/UDP port
  if (socketAddress_.isIpv6)
  {
    memset(&socketAddress_.socketAddrIn.sockAddrIpv6, 0, sizeof(struct sockaddr_in6));
    socketAddress_.socketAddrIn.sockAddrIpv6.sin6_family = AF_INET6; // We're always going to use IP
    socketAddress_.socketAddrIn.sockAddrIpv6.sin6_port = htons(port_);
  }
  else
  {
    memset(&socketAddress_.socketAddrIn.sockAddrIpv4, 0, sizeof(struct sockaddr_in));
    socketAddress_.socketAddrIn.sockAddrIpv4.sin_family = AF_INET; // We're always going to use IP
    socketAddress_.socketAddrIn.sockAddrIpv4.sin_port = htons(port_);
  }

  if(ipAddress_ == LOCAL_HOST_STR || ipAddress_ == LOCAL_HOST_IPV6_STR)
  {
    // Notice: this wont work for TCP clients with TCP-MD5, a specific IP should be used
    if (socketAddress_.isIpv6)
    {
      socketAddress_.socketAddrIn.sockAddrIpv6.sin6_addr = in6addr_any;
    }
    else
    {
      socketAddress_.socketAddrIn.sockAddrIpv4.sin_addr.s_addr = INADDR_ANY;
    }
  }
  else
  {
    int retval(inet_pton((socketAddress_.isIpv6 ? AF_INET6 : AF_INET),
                         ipAddress_.c_str(),
                         (socketAddress_.isIpv6 ?
                           (void *) &socketAddress_.socketAddrIn.sockAddrIpv6.sin6_addr :
                           (void *) &socketAddress_.socketAddrIn.sockAddrIpv4.sin_addr)));
    if(retval != 1)
    {
      if(retval == 0)
      {
        cerr << "Error converting IP address string: " << ipAddress_.c_str() << endl;
      }
      else
      {
        int theError(errno);
        cerr << "Error converting IP address: " << ipAddress_.c_str()
             << ", errno: " << theError
             << ", " << strerror(theError)
             << endl;
      }
      return false;
    }
  }

  // The remote address will be used by the derived classes
  remoteAddrLen_ = socketAddress_.isIpv6 ? sizeof(struct sockaddr_in6) : sizeof(struct sockaddr_in);
  memset(&remoteAddress_, 0, remoteAddrLen_);

  // Configure the timeout if specified
  if(timeout_msec_ > 0)
  {
    tval_ = new struct timeval();
    if(timeout_msec_ > 999)
    {
      tval_->tv_sec  = timeout_msec_/1000;
      tval_->tv_usec = (timeout_msec_ - (tval_->tv_sec*1000))*1000;
    }
    else
    {
      tval_->tv_sec = 0;
      tval_->tv_usec = timeout_msec_*1000;
    }
    timeout_sec_  = tval_->tv_sec;
    timeout_usec_ = tval_->tv_usec;
  }

  // Initialize the socket FD sets
  FD_ZERO(&socketFdReadSet_);
  FD_ZERO(&socketFdWriteSet_);
  FD_ZERO(&socketFdErrorSet_);
  FD_ZERO(&allSocketFdReadSet_);
  FD_ZERO(&allSocketFdWriteSet_);
  FD_ZERO(&allSocketFdErrorSet_);

  // Template method design pattern
  // Perform common operations, then let concrete subclasses do their part
  bool retval = initializeSpecific();
  if(retval)
  {
    isInitialized_ = true;
  }

  // The listen socket should always be readable
  // The allSocketFdWriteSet will be managed in inititializeSpecific()
  FD_SET(socketFd_, &allSocketFdReadSet_);

  remoteSockFdList_.push_back(socketFd_);
  maxFd_ = socketFd_;

  return retval;
}

// protected
// virtual
int SocketHandler::getMaxFd()
{
  return maxFd_ + 1;
}

// protected
// virtual
int SocketHandler::getNumFd()
{
  return remoteSockFdList_.size();
}

// protected
// virtual
void SocketHandler::closeSocket(int sockFd)
{
  if(isDebug())
  {
    cout << "Socket closure for sockFd: " << sockFd << ", closing it" << endl;
  }

  close(sockFd);

  FD_CLR(sockFd, &allSocketFdWriteSet_);
  FD_CLR(sockFd, &allSocketFdReadSet_);

  // std::remove() was giving strange results
  // remove(remoteSockFdList_.begin(), remoteSockFdList_.end(), sockFd);

  remoteSockFdList_.remove(sockFd);

  if(sockFd == socketFd_)
  {
    socketFd_ = -1;
  }
}

// NOTICE: This method call may be blocking
// virtual
void SocketHandler::run(int numMessagesToRead /*default 0*/)
{
  if(!isInitialized_)
  {
    cerr << "ERROR: SocketHandler::run() trying to run without called initialize() first" << endl;
    return;
  }

  int numMessagesRead(0);
  int nready;
  char msgBuffer[MAX_MESSAGE_LENGTH];
  list<int> socketsToClose;

  while((numMessagesToRead == 0 || numMessagesRead < numMessagesToRead) && runHandler_)
  {
    // We could be more extensible by having a list of handlers and iterate it

    // Set the Fd's to be read and written to, structure copy
    socketFdReadSet_ = allSocketFdReadSet_;
    socketFdWriteSet_ = allSocketFdWriteSet_;

    // This call blocks
    if((nready = select(getMaxFd(), &socketFdReadSet_, &socketFdWriteSet_, &socketFdErrorSet_, tval_)) < 0)
    {
      // If the application was interrupted via signal handlers (EINTR), dont treat it as an error
      int theError(errno);
      if(theError != EINTR)
      {
        cerr << "Error in select, errno: " << theError
             << ", " << strerror(theError)
             << endl;
        // TODO should we exit? set runHandler_ = false if so
      }

      continue;
    }

    if(nready == 0 && tval_ != NULL)
    {
      msgHandler_->handleTimeout();
      // tval_ is reset to the number of seconds not slept
      tval_->tv_sec  = timeout_sec_;
      tval_->tv_usec = timeout_usec_;
      ++numMessagesRead;
      ++stats_.numTimeouts;

      continue;
    }

    // Iterate the socket FDs to see which needs to be read or written
    RemoteSockFdListType::const_iterator iter;
    for(iter = remoteSockFdList_.begin(); iter != remoteSockFdList_.end(); ++iter)
    {
      int sockFd = *iter;

      //
      // Socket is ready to read
      //
      if(FD_ISSET(sockFd, &socketFdReadSet_))
      {
        int numBytesRead(readSpecific(sockFd, msgBuffer));
        ++numMessagesRead;

        if(isDebug())
        {
          cout << "\nReceived a message[" << numMessagesRead
               << "] of size: " << numBytesRead
               << ", on sockFd: " << sockFd
               << endl;
        }

        // A zero byte read is typically a socket closure on the other end
        if(numBytesRead == 0)
        {
          // Will be closed outside of this List iteration loop
          ++stats_.numDisconnects;
          socketsToClose.push_back(sockFd);
          msgHandler_->handleDisconnect(sockFd);
        }
        else if(numBytesRead > 0)
        {
          ++stats_.numReads;
          // TODO remoteAddress_ is set in UdpSocketHandler, but not in TcpSocketHandler
          msgHandler_->handleMessage(sockFd, msgBuffer, numBytesRead, remoteAddress_);

          // Set the socket for writing if the handler has a response available
          if(msgHandler_->hasOutgoingMessage(sockFd))
          {
            FD_SET(sockFd, &allSocketFdWriteSet_);
          }
          else if(mode_ == SocketHandler::MODE_CLIENT)
          {
            // If in client mode and the MessageHandler does
            // not have anything to send, then lets quit
            runHandler_ = false;
            socketsToClose.push_back(sockFd);
          }
        }
      } // if socketFdReadSet

      //
      // Socket is ready to write
      //
      if(FD_ISSET(sockFd, &socketFdWriteSet_))
      {
        ++stats_.numWrites;
        // Turn off writing now that we're sending a response.
        // The sockets should always be set for reading.
        FD_CLR(sockFd, &allSocketFdWriteSet_);

        MessageHandler::SocketMessage *message(msgHandler_->getMessage(sockFd));
        if(!message)
        {
          continue;
        }

        int numBytesWritten(writeSpecific(sockFd,
                                          message->message,
                                          message->messageLength));

        if(isDebug())
        {
          cout << "Replied with a message of size: " << numBytesWritten
               << ", on sockFd: " << sockFd
               << endl;
        }
      } // if socketFdWriteSet

      //
      // Socket has an error
      //
      if(FD_ISSET(sockFd, &socketFdErrorSet_))
      {
        // TODO should we pass this to the handler too?
          ++stats_.numSocketExceptions;

        cerr << "Exception on socket: " << sockFd << endl;
        // socketToClose = sockFd;
      }

    } // for loop

    // Causes problems if we close the sockets while in the list iteration loop
    while(!socketsToClose.empty())
    {
      closeSocket(socketsToClose.front());
      socketsToClose.pop_front();
    }

  } // while loop
}

