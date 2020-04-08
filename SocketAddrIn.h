
#ifndef SOCKET_ADDR_IN_H
#define SOCKET_ADDR_IN_H

#include <netinet/in.h>
#include <string.h>

//
// Simple class to encapsulate a union for ipv4 and ipv6 addresses.
// This helps avoid duplicating methods for v4 and v6
//
class SocketAddrIn
{
public:
    union socketAddrIn
    {
        struct sockaddr_in  sockAddrIpv4;
        struct sockaddr_in6 sockAddrIpv6;
    } socketAddrIn;
    bool isIpv6;

    // Default to Ipv4 INADDR_ANY
    SocketAddrIn() : isIpv6(false) {
        memset(&socketAddrIn.sockAddrIpv4, 0, sizeof(struct sockaddr_in));
        socketAddrIn.sockAddrIpv4.sin_family = AF_INET;
        socketAddrIn.sockAddrIpv4.sin_addr.s_addr = INADDR_ANY;
    }

    SocketAddrIn(const SocketAddrIn &rhs) {
        isIpv6 = rhs.isIpv6;
        if (isIpv6) {
            memcpy(&socketAddrIn.sockAddrIpv6, &rhs.socketAddrIn.sockAddrIpv6, sizeof(struct sockaddr_in6));
        } else {
            memcpy(&socketAddrIn.sockAddrIpv4, &rhs.socketAddrIn.sockAddrIpv4, sizeof(struct sockaddr_in));
        }
    }

    SocketAddrIn(struct sockaddr_in *sockAddr) : isIpv6(false) {
        memcpy(&socketAddrIn.sockAddrIpv4, sockAddr, sizeof(struct sockaddr_in));
    }

    SocketAddrIn(struct sockaddr_in6 *sockAddr) : isIpv6(true) {
        memcpy(&socketAddrIn.sockAddrIpv6, sockAddr, sizeof(struct sockaddr_in6));
    }
};

#endif
