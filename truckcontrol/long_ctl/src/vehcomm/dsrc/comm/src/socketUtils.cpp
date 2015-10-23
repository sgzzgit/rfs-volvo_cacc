#include <iostream>
#include <sstream>
#include <string>
#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <algorithm>			// std::count

#include "socketUtils.h"

/// set up a socket for reception on a local port
int socketUtils::server(const socketAddress_t& myAddress,const bool isNonBlocking)
{
	int socketfd;
	int backlog = 5;
	if (myAddress.sockeyProtocol == sockeprotocol_enum_t::UNKNOWN)
	{
		perror("Unknown protocol");
		return (-1);
	}
	// construct the socket address	
	struct addrinfo hints;
	memset(&hints,0,sizeof hints);
//	hints.ai_family = AF_UNSPEC;						
	hints.ai_family = AF_INET;								// AF_INET or AF_INET6 
	hints.ai_protocol = 0;
//	hints.ai_flags = AI_PASSIVE|AI_ADDRCONFIG;	// allow wildcard address, must be IPv4 or IPv6	
	hints.ai_flags = AI_PASSIVE;									// allow wildcard address
	hints.ai_socktype = SOCK_DGRAM;
	if (myAddress.sockeyProtocol == sockeprotocol_enum_t::TCP)
		hints.ai_socktype = SOCK_STREAM;
	const char* node = 0;
	const char* service = 0;
	if (!myAddress.name.empty())
	{
		node = myAddress.name.c_str();
	}
	if (!myAddress.port.empty())
	{
		service = myAddress.port.c_str();
	}
	struct addrinfo* res;
	if (getaddrinfo(node,service,&hints,&res) != 0)
	{
		perror("getaddrinfo");
		return (-1);
	}
	// create and bind the socket: loop through res until we found a working one
	struct addrinfo* p;
	int reuseaddr = 1;
	for(p = res; p != NULL; p = p->ai_next) 
	{
		// create the socket
		if ((socketfd = socket(p->ai_family,p->ai_socktype,p->ai_protocol)) < 0) 
		{
			perror("socket");
			continue;
		}
		// allow socket descriptor to be reusable
		if (setsockopt(socketfd,SOL_SOCKET,SO_REUSEADDR,&reuseaddr,sizeof(reuseaddr)) < 0)
		{
			perror("setsockopt SO_REUSEADDR");
			close(socketfd);
			continue;
		}	
		// bind the socket
		if (bind(socketfd,p->ai_addr,p->ai_addrlen) < 0) 
		{
			perror("bind");
			close(socketfd);
			continue;
		}
		break;
	}
	if (p == NULL)
	{
		perror("Failed to create/bind socket");
		return (-1);
	}	
	freeaddrinfo(res); 
	// set the socket to be non-blocking
	if (isNonBlocking)
	{
		int fileflags = fcntl(socketfd,F_GETFL,0);
		if (fileflags < 0)
		{
			perror("fcntl F_GETFL");
			close(socketfd);
			return (-1);
		}	
		if (fcntl(socketfd,F_SETFL,fileflags|O_NONBLOCK) < 0)
		{
			perror("fcntl O_NONBLOCK");
			close(socketfd);
			return (-1);
		}
	}
	// set TCP socket to listen for connection
	if (myAddress.sockeyProtocol == sockeprotocol_enum_t::TCP)
	{
		if (listen(socketfd,backlog) < 0)
		{
			perror("listen");
			close(socketfd);
			return (-1);
		}
	}
	return socketfd;
}

/// set up a socket for sending
int socketUtils::client(const socketAddress_t& theirAddress,const bool isNonBlocking)
{
	int socketfd;
	if (theirAddress.sockeyProtocol == sockeprotocol_enum_t::UNKNOWN)
	{
		perror("Unknown protocol");
		return (-1);
	}
	// construct the socket address	
	struct addrinfo hints;
	memset(&hints,0,sizeof hints);
//	hints.ai_family = AF_UNSPEC;			// AF_INET or AF_INET6 
	hints.ai_family = AF_INET;			
	hints.ai_protocol = 0;
//	hints.ai_flags = AI_ADDRCONFIG;		// must be IPv4 or IPv6	
	hints.ai_socktype = SOCK_DGRAM;
	if (theirAddress.sockeyProtocol == sockeprotocol_enum_t::TCP)
		hints.ai_socktype = SOCK_STREAM;
	const char* node = 0;
	const char* service = 0;
	if (!theirAddress.name.empty())
	{
		node = theirAddress.name.c_str();
	}
	if (!theirAddress.port.empty())
	{
		service = theirAddress.port.c_str();
	}
	struct addrinfo* res;
	if (getaddrinfo(node,service,&hints,&res) != 0)
	{
		perror("getaddrinfo");
		return (-1);
	}
	// create and connect the socket: loop through res until we found a working one
	struct addrinfo* p;
	fd_set wfds;
	int optval;
	socklen_t optlen;
	struct timeval timeout;
	for(p = res; p != NULL; p = p->ai_next) 
	{
		// create the socket
		if ((socketfd = socket(p->ai_family,p->ai_socktype,p->ai_protocol)) < 0) 
		{
			perror("socket");
			continue;
		}
		// set the socket to be non-blocking
		if (isNonBlocking)
		{
			int fileflags = fcntl(socketfd,F_GETFL,0);
			if (fileflags < 0)
			{
				perror("fcntl F_GETFL");
				close(socketfd);
				continue;
			}	
			if (fcntl(socketfd,F_SETFL,fileflags|O_NONBLOCK) < 0)
			{
				perror("fcntl O_NONBLOCK");
				close(socketfd);
				continue;
			}
		}
		// connect the socket		
		if (connect(socketfd,p->ai_addr,p->ai_addrlen) < 0) 
		{
			if (errno != EINPROGRESS)
			{
				perror("connect");
				close(socketfd);
				continue;
			}
			// check writability
			FD_ZERO(&wfds);
			FD_SET(socketfd,&wfds);
			timeout.tv_sec = 5;			// select() may update timeout
			timeout.tv_usec  = 0;				
			if ( select(socketfd+1,NULL,&wfds,NULL,&timeout) <= 0 || !FD_ISSET(socketfd,&wfds) )
			{
				perror("select");
				close(socketfd);
				continue;
			}
			// check whether connect() completed successfully
			optval = 0;
			optlen = (socklen_t)sizeof(optval);
			if (getsockopt(socketfd,SOL_SOCKET,SO_ERROR,&optval,&optlen) < 0 || optval > 0)
			{
				perror("getsockopt SO_ERROR");
				close(socketfd);
				continue;
			}
			break;
		}	
		break;
	}
	if (p == NULL)
	{
		perror("Failed to create/connect socket");
		return (-1);
	}	
	freeaddrinfo(res); 
	return socketfd;	
}

bool socketUtils::sendall(const int sfd, const char* buf, const size_t len)
{
	size_t total = 0; // how many bytes we've sent
	size_t bytesleft = len;
	ssize_t bytes_sent = 0;
	while (total < len)
	{
		bytes_sent = send(sfd,buf+total,bytesleft,0);
		if (bytes_sent < 0)
			break;
		total += (size_t)bytes_sent;
		bytesleft -= (size_t)bytes_sent;
	}
	return (bytes_sent == -1 ? false:true);
}

bool socketUtils::setSocketAddress(const char* str,socketAddress_t& socketAddr)
{
	// str in the formate of UDP/TCP:xx.xx.xx.xx:port
	std::string in_str(str);	
	if (std::count(in_str.begin(),in_str.end(),':') != 2)
		return false;	
	std::istringstream iss(in_str);
	std::string s;
	std::getline(iss, s, ':'); // sockeyProtocol
	if (s.empty())
		return false;
	socketAddr.sockeyProtocol = sockeprotocol_enum_t::UNKNOWN;
	if (s == "UDP")
		socketAddr.sockeyProtocol = sockeprotocol_enum_t::UDP;
	else if (s == "TCP")
		socketAddr.sockeyProtocol = sockeprotocol_enum_t::TCP;
	if (socketAddr.sockeyProtocol == sockeprotocol_enum_t::UNKNOWN)
		return false;
	std::getline(iss, s, ':'); // name
	if (s.empty())
		return false;
	socketAddr.name = s;
	std::getline(iss, s, ':'); // port
	if (s.empty())
		return false;
	socketAddr.port = s;
	return true;
}
