
#ifndef MESSAGE_HANDLER_H
#define MESSAGE_HANDLER_H

#include <map>

#include <arpa/inet.h>
#include <sys/types.h>
#include <netinet/in.h>

#include "SocketAddrIn.h"

using namespace std;

//
// Abstract base class MessageHandler
// handles SocketHandler incoming messages
// Can be used by both TCP and UDP
//
class MessageHandler
{
public:
  MessageHandler();
  virtual ~MessageHandler();

  class SocketMessage
  {
  public:
    char *message;
    uint32_t messageLength;
    SocketMessage() : message(0), messageLength(0) {}
    SocketMessage(char *msg, uint32_t len) : message(msg), messageLength(len) {}
    ~SocketMessage() {if(message != 0) { delete [] message; messageLength = 0; } }
  };

  // Set chained MessageHandlers
  // Each MessageHandler will call the next one if it has one
  void setChainedHandler(MessageHandler *chainedHandler) { chainedHandler_ = chainedHandler; }
  MessageHandler *getChainedHandler() { return chainedHandler_; }

  inline bool isDebug() {return isDebug_;}
  inline void setDebug(bool dbg) {isDebug_ = dbg;}

  // Called when a message is read from a remote socket
  // The remoteKey should be used when more there will be more
  // than one remote socket and more than one message can be stored
  // The responseMsg is only set when calling the method internally
  virtual void handleMessage(int remoteKey,
                             char *msg,
                             int msgLength,
                             const SocketAddrIn &remoteAddr,
                             MessageHandler::SocketMessage *responseMsg = 0) = 0;
  virtual void handleTimeout() = 0;

  // Called to accept connections/disconnects, default behavior is provided
  // May never be called from some protocols, thus they arent pure virtual
  virtual void handleConnect(int remoteKey, const SocketAddrIn &remoteAddr, socklen_t remoteAddrLen) {};
  virtual void handleDisconnect(int remoteKey) {};

  // Called to get a message to write to the remote end
  // The key is in case this class is overridden and more than one
  // message is stored, it is the same as passed to handleMessage
  virtual MessageHandler::SocketMessage *getMessage(int remoteKey);
  inline virtual bool hasOutgoingMessage(int remoteKey) {return hasOutgoingMessage_;}

protected:
  MessageHandler *chainedHandler_;
  bool hasOutgoingMessage_;
  bool isDebug_;
  typedef map<int, MessageHandler::SocketMessage*> ResponseMessageMapType;
  ResponseMessageMapType responseMessageMap_;
};

#endif
