/*
 * File:   exampleMain.cpp
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
#include <EchoServerTcpMessageHandler.h>
#include <CmdLineParser.h>

using namespace std;

const string ARG_IP_ADDRESS  = "-a";
const string ARG_LISTEN_PORT = "-p";
const string ARG_VERBOSE     = "-v";

// Global SocketServer pointer to be used in the application signal handler
SocketHandler *SOCKET_SERVER = NULL;

struct ConfigInput
{
  string    ipAddress;
  uint16_t  listenPort;
  bool      isVerbose;
  ConfigInput() : ipAddress("127.0.0.1"), listenPort(3868), isVerbose(false) {}
};

void loadCmdLine(CmdLineParser &clp)
{
  clp.setMainHelpText("A simple Echo Server");

  // All options are optional
     // IP Address
  clp.addCmdLineOption(new CmdLineOptionStr(ARG_IP_ADDRESS,
                                            string("IP Address to listen on"),
                                            string("127.0.0.1")));

     // TCP port
  clp.addCmdLineOption(new CmdLineOptionInt(ARG_LISTEN_PORT,
                                            string("TCP port to listen on"),
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
  config.listenPort =  ((CmdLineOptionInt*)  clp.getCmdLineOption(ARG_LISTEN_PORT))->getValue();
  config.isVerbose  =  ((CmdLineOptionFlag*) clp.getCmdLineOption(ARG_VERBOSE))->getValue();

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
    dumpServerStatistics(SOCKET_SERVER->getStatistics());

    return;
  }

  cout << "Exiting application" << endl;
  if(SOCKET_SERVER)
  {
    SOCKET_SERVER->stop();
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
  SocketHandler *server = new TcpSocketHandlerImpl(input.listenPort, input.ipAddress);
  MessageHandler *msgHandler = new EchoServerTcpMessageHandler();
  msgHandler->setDebug(input.isVerbose);
  server->setMessageHandler(msgHandler);
  server->setHandlerMode(SocketHandler::MODE_SERVER);
  server->setDebug(input.isVerbose);

  if(!server->initialize())
  {
    cerr << "Error Initializing the SocketServer, exiting";
    return 1;
  }
  SOCKET_SERVER = server;

  //
  // Run the Echo Server
  // It will be stopped by a SIGINT (CTRL-C) signal
  //
  server->run(); // This call blocks
  cout << "Server stopped" << endl;

  delete server;
  delete msgHandler;

  return 0;
}

