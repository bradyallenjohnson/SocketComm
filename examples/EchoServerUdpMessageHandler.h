/*
 * File:   EchoUdpServerHandler.h
 * Author: ebrjohn
 *
 * Created on July 19, 2012, 4:16 PM
 */

#ifndef ECHO_UDP_SERVERHANDLER_H
#define	ECHO_UDP_SERVERHANDLER_H

#include "MessageHandler.h"

class EchoServerUdpMessageHandler : public MessageHandler
{
public:
  EchoServerUdpMessageHandler();
  virtual ~EchoServerUdpMessageHandler();

  virtual void handleMessage(int clientKey,
                             char *requestMsg,
                             int requestMsgLength,
                             struct sockaddr_in *clientAddr,
                             MessageHandler::SocketMessage *responseMsg);
  virtual void handleTimeout();
};

#endif	/* ECHO_UDP_SERVERHANDLER_H */

