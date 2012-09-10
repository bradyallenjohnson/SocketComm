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

#include <SocketServer.h>
#include <UdpSocketServerImpl.h>
#include <EchoUdpServerHandler.h>
#include <CmdLineParser.h>

using namespace std;

const string ARG_LISTEN_PORT = "-p";
const string ARG_VERBOSE     = "-v";

// Global SocketServer pointer to be used in the application signal handler
SocketServer *SOCKET_SERVER = NULL;

struct ConfigInput
{
  uint16_t  listenPort;
  bool      isVerbose;
  ConfigInput() : listenPort(3868), isVerbose(false) {}
};

void loadCmdLine(CmdLineParser &clp)
{
  clp.setMainHelpText("A simple UDP Echo Server");

  // All options are optional
     // Number of Threads
  clp.addCmdLineOption(new CmdLineOptionInt(ARG_LISTEN_PORT,
                                            string("UDP port to listen on"),
                                            12345));
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

  config.listenPort = ((CmdLineOptionInt*)  clp.getCmdLineOption(ARG_LISTEN_PORT))->getValue();
  config.isVerbose =  ((CmdLineOptionFlag*) clp.getCmdLineOption(ARG_VERBOSE))->getValue();

  return true;
}

void dumpServerStatistics(SocketServer::SocketServerStatistics stats)
{
  cout << "\n*** ServerStatistics:"
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
  SOCKET_SERVER->stop();
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
  // Create and initialize the SocketServer and its handler
  //
  SocketServer *server(NULL);
  ServerHandler *handler(NULL);
  try
  {
    server = new UdpSocketHandlerImpl(input.listenPort);
    handler = new EchoUdpServerHandler();
    server->setHandler(handler);
    server->setDebug(input.isVerbose);
    server->initialize();
    SOCKET_SERVER = server; // Global SocketServer pointer to be used in signal handler
  }
  catch(const exception &e)
  {
    cerr << "Error Initializing the SocketServer: " << e.what();
    return 1;
  }

  //
  // Run the Echo Server
  // It will be stopped by a SIGINT (CTRL-C) signal
  //
  try
  {
    // This call blocks
    server->run();
    cout << "Server stopped" << endl;
  }
  catch(const exception &e)
  {
    cerr << "Error Running the Server: " << e.what();
    return 1;
  }

  delete server;
  delete handler;

  return 0;
}

