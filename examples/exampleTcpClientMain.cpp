/*
 * File:   exampleTcpClientMain.cpp
 * Author: Brady Johnson
 *
 * Created on July 11, 2012, 4:03 PM
 */

#include <cstdlib>
#include <iostream>

#include <string.h> // memset
#include <signal.h>

#include <SocketHandler.h>
#include <MessageHandler.h>
#include <TcpSocketHandlerImpl.h>
#include <FileReaderMessageHandler.h>
#include <CmdLineParser.h>

using namespace std;

const string ARG_IP_ADDRESS  = "-a";
const string ARG_LISTEN_PORT = "-p";
const string ARG_VERBOSE     = "-v";

// Global SocketClient pointer to be used in the application signal handler
SocketHandler *SOCKET_CLIENT = NULL;

struct ConfigInput
{
  string    ipAddress;
  uint16_t  remotePort;
  bool      isVerbose;
  ConfigInput() : ipAddress("127.0.0.1"), remotePort(3868), isVerbose(false) {}
};

void loadCmdLine(CmdLineParser &clp)
{
  clp.setMainHelpText("A simple Echo Server");

  // All options are optional
    // IP Address
  clp.addCmdLineOption(new CmdLineOptionStr(ARG_IP_ADDRESS,
                                            string("IP Address to connect to"),
                                            string("127.0.0.1")));

    // Number of Threads
  clp.addCmdLineOption(new CmdLineOptionInt(ARG_LISTEN_PORT,
                                            string("TCP port to connect to"),
                                            3868));
    // Verbosity
  clp.addCmdLineOption(new CmdLineOptionFlag(ARG_VERBOSE,
                                             string("Turn on verbosity"),
                                             false));
}

bool parseCommandLine(int argc, char **argv, CmdLineParser &clp, ConfigInput &config)
{
  if(!clp.parseCmdLine(argc, argv))
  {
    clp.printUsage();
    return false;
  }

  config.ipAddress  =  ((CmdLineOptionStr*)  clp.getCmdLineOption(ARG_IP_ADDRESS))->getValue();
  config.remotePort = ((CmdLineOptionInt*)  clp.getCmdLineOption(ARG_LISTEN_PORT))->getValue();
  config.isVerbose =  ((CmdLineOptionFlag*) clp.getCmdLineOption(ARG_VERBOSE))->getValue();

  return true;
}

void dumpServerStatistics(SocketHandler::SocketHandlerStatistics stats)
{
  cout << "\n*** ServerStatistics:"
       << "\n\t Connects:    " << stats.numConnects
       << "\n\t Disconnects: " << stats.numDisconnects
       << "\n\t Reads:       " << stats.numReads
       << "\n\t Writes:      " << stats.numWrites
       << "\n\t Timeouts:    " << stats.numTimeouts
       << "\n\t Exceptions:  " << stats.numSocketExceptions
       << endl;
}

void signalCallbackHandler(int signum)
{
  if(signum == SIGUSR1)
  {
    dumpServerStatistics(SOCKET_CLIENT->getStatistics());

    return;
  }

  cout << "Exiting application" << endl;
  if(SOCKET_CLIENT)
  {
    SOCKET_CLIENT->stop();
  }
}

void setupSignalHandling()
{
  struct sigaction action;
  memset(&action, 0, sizeof(struct sigaction));
  action.sa_handler = signalCallbackHandler;

  // restart system calls
  action.sa_flags = SA_RESTART;

  // Signals to block while handling the signal
  sigaddset(&action.sa_mask, SIGINT);
  sigaddset(&action.sa_mask, SIGUSR1);

  // Signals to handle
  sigaction(SIGINT,  &action, NULL);
  sigaction(SIGUSR1, &action, NULL);
}

int main(int argc, char** argv)
{
  CmdLineParser clp;
  loadCmdLine(clp);

  ConfigInput input;
  if(!parseCommandLine(argc, argv, clp, input))
  {
    return 1;
  }

  setupSignalHandling();

  //
  // Create and Initialize the SocketHandler and MessageHandler
  //
  SocketHandler *client = new TcpSocketHandlerImpl(input.remotePort, input.ipAddress);
  MessageHandler *msgHandler = new FileReaderMessageHandler("exampleFileData.txt");
  msgHandler->setDebug(input.isVerbose);
  client->setMessageHandler(msgHandler);
  client->setHandlerMode(SocketHandler::MODE_CLIENT);
  client->setDebug(input.isVerbose);

  if(!client->initialize())
  {
    cerr << "Error Initializing the SocketServer, exiting" << endl;
    return 1;
  }
  SOCKET_CLIENT = client; // Global SocketHandler pointer to only be used in signal handler


  //
  // Run the Echo Server
  // It will be stopped by a SIGINT (CTRL-C) signal
  //

  // This call blocks
  // The socket client will prompt the Message Handler for when its ready to send messages
  client->run();
  cout << "Client stopped" << endl;

  delete client;
  delete msgHandler;

  return 0;
}

