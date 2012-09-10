/*
 * File:   EchoTcpServerHandler.cpp
 * Author: ebrjohn
 *
 * Created on July 11, 2012, 4:16 PM
 */

#include <iostream>
#include <string.h> // memcpy

#include "SocketHandler.h"
#include "EchoServerTcpMessageHandler.h"

using namespace std;

// virtual
void EchoServerTcpMessageHandler::handleConnect(int clientKey,
                                                const struct sockaddr_in &clientAddr,
                                                socklen_t clientAddrLen)
{
  // We could also store the clientAddr associated with the clientKey

  MessageHandler::SocketMessage *responseMsg(getMessage(clientKey));
  if(!responseMsg)
  {
    responseMessageMap_[clientKey] =
            new MessageHandler::SocketMessage(new char[SocketHandler::MAX_MESSAGE_LENGTH], 0);
  }
}

// The responseMsg is passed down the chain so each member can see or modify it
// virtual
void EchoServerTcpMessageHandler::handleMessage(int clientKey,
                                          char *requestMsg,
                                          int requestMsgLength,
                                          struct sockaddr_in *clientAddr,
                                          MessageHandler::SocketMessage *responseMsg) /* default NULL */
{
  if(!responseMsg)
  {
    // The messages will be reused
    responseMsg = getMessage(clientKey);
    if(!responseMsg)
    {
      cerr << "Error, clientKey: " << clientKey << " does not exist!" << endl;
      return;
    }
  }
  responseMsg->messageLength = requestMsgLength;

  if(isDebug_)
  {
    cout << "EchoServerTcpMessageHandler::handleMessage() received: " << requestMsg << endl;
  }

  // have to copy the input message since it may be overwritten before replying

  memcpy(responseMsg->message, requestMsg, requestMsgLength);

  // Send it down the chain, if there is one
  if(chainedHandler_)
  {
    chainedHandler_->handleMessage(clientKey,
                                   requestMsg,
                                   requestMsgLength,
                                   clientAddr,
                                   responseMsg);
  }
}

// virtual
void EchoServerTcpMessageHandler::handleTimeout()
{
  cout << "EchoTcpServerHandler::handleTimeout() received a timeout" << endl;
}
