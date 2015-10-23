#ifndef _SOCKET_UTILS_H
#define _SOCKET_UTILS_H

#include <sys/time.h>			// struct timeval
#include <sys/types.h>
#include <sys/socket.h>		
#include <sys/select.h>		// select
#include <netdb.h>				// struct addrinfo
#include <errno.h>				
#include <fcntl.h>

#include <stdint.h>		// c++11 <cstdint>

namespace socketUtils
{
	struct sockeprotocol_enum_t
	{
		enum SocketProtocol {UNKNOWN,UDP,TCP};
	};
	
	struct socketAddress_t
	{
		sockeprotocol_enum_t::SocketProtocol sockeyProtocol;
		std::string name;
		std::string port;
		socketAddress_t& operator=(const socketAddress_t& p)
    {
			sockeyProtocol = p.sockeyProtocol;
			name = p.name;
			port = p.port;
			return *this;
    };		
		void reset(void)
		{
			sockeyProtocol = sockeprotocol_enum_t::UNKNOWN;
			name.clear();
			port.clear();
		}		
	};
	
	bool setSocketAddress(const char* str,socketAddress_t& socketAddr);	
	int server(const socketAddress_t& myAddress,const bool isNonBlocking);
	int client(const socketAddress_t& theirAddress,const bool isNonBlocking);
	bool sendall(const int sfd, const char* buf, const size_t len);
}

#endif
