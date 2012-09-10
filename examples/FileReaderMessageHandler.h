/*
 * File:   FileReaderMessageHandler.h
 * Author: ebrjohn
 *
 * Created on July 11, 2012, 4:16 PM
 */

#ifndef FILE_READER_MESSAGE_HANDLER_H
#define	FILE_READER_MESSAGE_HANDLER_H

#include <string>
#include <list>

#include "MessageHandler.h"

using namespace std;

class FileReaderMessageHandler : public MessageHandler
{
public:
  FileReaderMessageHandler(const string &filePath);
  virtual ~FileReaderMessageHandler();

  virtual void handleMessage(int remoteKey,
                             char *requestMsg,
                             int requestMsgLength,
                             struct sockaddr_in *clientAddr,
                             MessageHandler::SocketMessage *responseMsg);
  virtual void handleTimeout();
  virtual MessageHandler::SocketMessage *getMessage(int remoteKey);

private:
  typedef list<string> FileMessageListType;
  FileMessageListType messageList_;
  FileMessageListType::iterator messageListIterator_;

};

#endif	/* ECHO_TCP_SERVERHANDLER_H */

