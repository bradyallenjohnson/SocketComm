/*
 * File:   EchoUdpServerHandler.cpp
 * Author: ebrjohn
 *
 * Created on July 11, 2012, 4:16 PM
 */

#include <iostream>
#include <string.h> // memcpy

#include "EchoServerUdpMessageHandler.h"

using namespace std;

EchoServerUdpMessageHandler::EchoServerUdpMessageHandler()
{
}

EchoServerUdpMessageHandler::~EchoServerUdpMessageHandler()
{
}

// virtual
// The responseMsg is passed down the chain so each member can see or modify it
void EchoServerUdpMessageHandler::handleMessage(int clientKey,
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
void EchoServerUdpMessageHandler::handleTimeout()
{
  cout << "EchoServerUdpMessageHandler::handleTimeout() received a timeout" << endl;
}
