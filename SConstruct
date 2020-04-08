env = Environment()

sourceFiles = [
    'MessageHandler.cpp',
    'SocketHandler.cpp',
    'TcpSocketHandlerImpl.cpp',
    'UdpSocketHandlerImpl.cpp',
]

env.Append(CPPPATH = '#.', CPPFLAGS = ['-O2', '-g'])
libTarget = env.Library(target = 'socketComm', source = sourceFiles)
env.Default(libTarget)
env.Alias('library', libTarget)

tcpServerSource = [
    'examples/EchoServerTcpMessageHandler.cpp',
    'examples/exampleTcpServerMain.cpp',
]

tcpClientSource = [
    'examples/FileReaderMessageHandler.cpp',
    'examples/exampleTcpClientMain.cpp',
]

udpServerSource = [
    'examples/EchoServerUdpMessageHandler.cpp',
    'examples/exampleUdpServerMain.cpp',
]

udpClientSource = [
    'examples/FileReaderMessageHandler.cpp',
    'examples/exampleUdpClientMain.cpp',
]

exampleEnv = env.Clone()
clpPath = '../CommandLineParser'
exampleEnv.Append(CPPPATH = [clpPath, '#', '#examples'],
                  CPPFLAGS = ['-g'],
                  LIBPATH = [clpPath, '#'],
                  LIBS = ['CmdLineParser', 'socketComm'])
exampleTcpClient = exampleEnv.Program(target = 'examples/exampleTcpClientMain', source = tcpClientSource)
exampleTcpServer = exampleEnv.Program(target = 'examples/exampleTcpServerMain', source = tcpServerSource)
#exampleUdpClient = exampleEnv.Program(target = 'examples/exampleUdpClientMain', source = udpClientSource)
#exampleUdpServer = exampleEnv.Program(target = 'examples/exampleUdpServerMain', source = udpServerSource)
exampleEnv.Alias('examples', [exampleTcpClient, exampleTcpServer])
