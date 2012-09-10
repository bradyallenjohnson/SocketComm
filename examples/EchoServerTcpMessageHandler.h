/*
 * File:   EchoTcpServerHandler.h
 * Author: ebrjohn
 *
 * Created on July 11, 2012, 4:16 PM
 */

#ifndef ECHO_TCP_SERVERHANDLER_H
#define	ECHO_TCP_SERVERHANDLER_H

#include <map>

#include "MessageHandler.h"

using namespace std;

class EchoServerTcpMessageHandler : public MessageHandler
{
public:
  EchoServerTcpMessageHandler() {}
  virtual ~EchoServerTcpMessageHandler() {}

  virtual void handleConnect(int clientKey, const struct sockaddr_in &clientAddr, socklen_t clientAddrLen);
  virtual void handleMessage(int clientKey,
                             char *requestMsg,
                             int requestMsgLength,
                             struct sockaddr_in *clientAddr,
                             MessageHandler::SocketMessage *responseMsg);
  virtual void handleTimeout();
};

#endif	/* ECHO_TCP_SERVERHANDLER_H */

