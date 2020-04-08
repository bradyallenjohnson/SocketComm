
#include "MessageHandler.h"
#include "SocketHandler.h"

MessageHandler::MessageHandler() :
  chainedHandler_(NULL),
  hasOutgoingMessage_(true),
  isDebug_(false)
{
}

MessageHandler::~MessageHandler()
{
  ResponseMessageMapType::iterator iter;
  for(iter = responseMessageMap_.begin(); iter != responseMessageMap_.end(); )
  {
    delete iter->second;
    // erase() makes the iter invalid, use the returned iter
    iter = responseMessageMap_.erase(iter);
  }
}

// virtual
MessageHandler::SocketMessage *MessageHandler::getMessage(int clientKey)
{
  MessageHandler::SocketMessage *msgPtr(NULL);

  ResponseMessageMapType::iterator iter = responseMessageMap_.find(clientKey);
  if(iter != responseMessageMap_.end())
  {
    msgPtr = iter->second;
  }

  return msgPtr;
}
