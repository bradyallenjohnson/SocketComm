
#ifndef SOCKET_HANDLER_H
#define SOCKET_HANDLER_H

#include <list>
#include <string>

#include <sys/types.h>
#include <netinet/in.h>
#include <time.h>

#include "MessageHandler.h"

using namespace std;

//
// Abstract base class SocketHandler
// Concrete implementations are UdpSocketHandler and TcpSocketHandler
//
class SocketHandler
{
  public:
    class SocketHandlerStatistics
    {
    public:
      uint64_t numConnects;
      uint64_t numDisconnects;
      uint64_t numReads;
      uint64_t numWrites;
      uint64_t numSocketExceptions;
      uint64_t numTimeouts;
      SocketHandlerStatistics();
      SocketHandlerStatistics(const SocketHandlerStatistics &rhs);
      void clear();
    };

    const static int MAX_MESSAGE_LENGTH;
    const static string LOCAL_HOST_STR;

    enum SOCKET_HANDLER_MODE
    {
      MODE_SERVER = 0,
      MODE_CLIENT,
      MODE_UNDEFINED
    };

    SocketHandler();
    SocketHandler(int port,
                  const string &ipAddr = LOCAL_HOST_STR,
                  uint32_t timeout_millis = 0,
                  SOCKET_HANDLER_MODE mode = MODE_UNDEFINED,
                  MessageHandler *handler = NULL);
    virtual ~SocketHandler();


    // Various setters/getters
    // These setters should be called before calling initialize()
    inline void setPort(int port) {port_ = port;}
    inline int getPort() const {return port_;}

    inline void setIpAddress(const string &ipAddr) {ipAddress_ = ipAddr;}
    inline string getIpAddress() const {return ipAddress_;}

    inline void setTimeoutMillis(uint32_t millis) {timeout_msec_ = millis;}
    inline uint32_t getTimeoutMillis() const {return timeout_msec_;}

    inline void setMessageHandler(MessageHandler *msgHandler) {msgHandler_ = msgHandler;}
    inline MessageHandler *getMessageHandler() {return msgHandler_;}

    inline void setHandlerMode(SOCKET_HANDLER_MODE mode) {mode_ = mode;}
    inline SOCKET_HANDLER_MODE getHandlerMode() const {return mode_;}

    inline void setDebug(bool dbg) {isDebug_ = dbg;}
    inline bool isDebug() {return isDebug_;}

    inline SocketHandler::SocketHandlerStatistics getStatistics() const {return stats_;}

    bool initialize();

    // NOTICE: This method call may be BLOCKING
    // If numMessagesToRead is set, this call will execute until that number of messages is read
    // If numMessagesToRead is 0, then the call will block and run continuously until stop() is called
    virtual void run(int numMessagesToRead=0);
    virtual inline void stop() { runHandler_ = false; }

  protected:
    // Template method design pattern
    virtual bool initializeSpecific() = 0;
    virtual int writeSpecific(int sockFd, char *buffer, int bufLength) = 0;
    virtual int readSpecific(int sockFd, char *buffer) = 0;

    virtual int getMaxFd();
    virtual int getNumFd();
    virtual void closeSocket(int sockFd);

    // Internal attributes
    MessageHandler *msgHandler_;
    int port_;
    int socketFd_;
    int maxFd_;
    string ipAddress_;
    typedef list<int> RemoteSockFdListType;
    RemoteSockFdListType remoteSockFdList_; // used by TCP

    fd_set socketFdReadSet_;
    fd_set socketFdWriteSet_;
    fd_set socketFdErrorSet_;
    fd_set allSocketFdReadSet_;
    fd_set allSocketFdWriteSet_;
    fd_set allSocketFdErrorSet_;

    struct sockaddr_in socketAddress_;
    struct sockaddr_in remoteAddress_;
    socklen_t remoteAddrLen_;
    SocketHandler::SocketHandlerStatistics stats_;

    bool isDebug_;
    uint32_t timeout_sec_;
    uint32_t timeout_msec_;
    uint32_t timeout_usec_;
    struct timeval *tval_;
    bool runHandler_;
    bool isInitialized_;
    SOCKET_HANDLER_MODE mode_;
};

#endif
