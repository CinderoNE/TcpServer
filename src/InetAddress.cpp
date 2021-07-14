#include "InetAddress.h"

#include<cstring>  //bzero
#include<arpa/inet.h>
#include<iostream>
#include<sstream>
static constexpr in_addr_t kInaddrAny = INADDR_ANY;

InetAddress::InetAddress()
{
	bzero(&addr_, sizeof addr_);
}

InetAddress::InetAddress(uint16_t port)
{
	bzero(&addr_, sizeof addr_);
	addr_.sin_family = AF_INET;
	addr_.sin_addr.s_addr = htonl(kInaddrAny);
	addr_.sin_port = htons(port);
	
}

InetAddress::InetAddress(const std::string& ip, uint16_t port)
{
	bzero(&addr_, sizeof addr_);
	addr_.sin_family = AF_INET;
	addr_.sin_port = htons(port);

	int ret = inet_pton(AF_INET, ip.c_str(), &addr_);
	if (ret < 0) {
		std::cout << "inet_pton error" << std::endl;
	}
}

std::string InetAddress::ToString() const
{
	char ip[INET_ADDRSTRLEN] = "INVALID";
	inet_ntop(AF_INET, &addr_.sin_addr, ip, sizeof ip);
	uint16_t port = ntohs(addr_.sin_port);
	std::stringstream ss;
	ss << ip << ":" << port;
	
	return ss.str();
}

