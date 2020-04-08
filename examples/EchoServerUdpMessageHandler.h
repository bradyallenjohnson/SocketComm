/*
 * File:   EchoUdpServerHandler.h
 * Author: ebrjohn
 *
 * Created on July 19, 2012, 4:16 PM
 */

#ifndef ECHO_UDP_SERVERHANDLER_H
#define	ECHO_UDP_SERVERHANDLER_H

#include "MessageHandler.h"
#include "SocketAddrIn.h"

class EchoServerUdpMessageHandler : public MessageHandler
{
public:
  EchoServerUdpMessageHandler();
  virtual ~EchoServerUdpMessageHandler();

  virtual void handleMessage(int clientKey,
                             char *requestMsg,
                             int requestMsgLength,
                             const SocketAddrIn &clientAddr,
                             MessageHandler::SocketMessage *responseMsg);
  virtual void handleTimeout();
};

#endif	/* ECHO_UDP_SERVERHANDLER_H */

