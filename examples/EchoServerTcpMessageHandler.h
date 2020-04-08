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
#include "SocketAddrIn.h"

using namespace std;

class EchoServerTcpMessageHandler : public MessageHandler
{
public:
  EchoServerTcpMessageHandler() {}
  virtual ~EchoServerTcpMessageHandler() {}

  virtual void handleConnect(int clientKey, const SocketAddrIn &clientAddr, socklen_t clientAddrLen);
  virtual void handleMessage(int clientKey,
                             char *requestMsg,
                             int requestMsgLength,
                             const SocketAddrIn &clientAddr,
                             MessageHandler::SocketMessage *responseMsg);
  virtual void handleTimeout();
};

#endif	/* ECHO_TCP_SERVERHANDLER_H */

