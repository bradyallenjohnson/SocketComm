/*
 * File:   FileReaderMessageHandler.cpp
 * Author: ebrjohn
 *
 * Created on July 11, 2012, 4:16 PM
 */

#include <iostream>
#include <fstream>
#include <list>
#include <string>
#include <string.h> // memcpy()

#include "SocketHandler.h"
#include "FileReaderMessageHandler.h"

using namespace std;

//
// This is a protocol-independent, client-oriented implementation of a MessageHandler.
// It will open a file, store each line, and then provide each line to the SocketHandler
// to write it to the socket.
//
FileReaderMessageHandler::FileReaderMessageHandler(const string &filePath)
{
  ifstream file(filePath.c_str());
  char buf[256];

  while(!file.eof())
  {
    file.getline(buf, 256); // each line should be less than 256 chars
    string tmpStr(buf);

    if(!tmpStr.empty())
    {
      //cout << "FileReaderMessageHandler loading Message: " << buf << endl;
      messageList_.push_back(tmpStr);
    }
  }
  file.close();

  messageListIterator_ = messageList_.begin();
  hasOutgoingMessage_ = true;

  // We're only going to  need one SocketMessage
  // will be deleted in MessageHandler dtor
  responseMessageMap_[0] =
            new MessageHandler::SocketMessage(new char[SocketHandler::MAX_MESSAGE_LENGTH], 0);
}

FileReaderMessageHandler::~FileReaderMessageHandler()
{
}

// virtual
MessageHandler::SocketMessage *FileReaderMessageHandler::getMessage(int remoteKey)
{
  if(!hasOutgoingMessage_)
  {
    return NULL;
  }

  MessageHandler::SocketMessage *responseMsg(NULL);

  ResponseMessageMapType::iterator iter = responseMessageMap_.find(0);
  if(iter != responseMessageMap_.end())
  {
    responseMsg = iter->second;
  }
  else
  {
    hasOutgoingMessage_ = false;
    return NULL;
  }

  responseMsg->messageLength = messageListIterator_->length();
  memcpy(responseMsg->message, (char*) messageListIterator_->c_str(), responseMsg->messageLength);
  ++messageListIterator_;

  if(isDebug_)
  {
    cout << "FileReaderMessageHandler::getMessage() Message: " << responseMsg->message
         << ", length: " << responseMsg->messageLength
         << endl;
  }

  if(messageListIterator_ == messageList_.end())
  {
    hasOutgoingMessage_ = false;
  }

  return responseMsg;
}

// The responseMsg is passed down the chain so each member can see or modify it
// virtual
void FileReaderMessageHandler::handleMessage(int clientKey,
                                             char *requestMsg,
                                             int requestMsgLength,
                                             struct sockaddr_in *clientAddr,
                                             MessageHandler::SocketMessage *responseMsg)
{
  if(isDebug_)
  {
    cout << "FileReaderMessageHandler::handleMessage() Message: " << requestMsg << endl;
  }
}

// virtual
void FileReaderMessageHandler::handleTimeout()
{
  if(isDebug_)
  {
    cout << "FileReaderMessageHandler::handleTimeout() received a timeout" << endl;
  }
}
