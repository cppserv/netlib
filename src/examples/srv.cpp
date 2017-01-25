#include "../include/netlib.hpp"

int main()
{
	int lfd, afd;

	lfd = tcp_listen_on_port(5000);

	auto sslSock = std::unique_ptr<SSocket>(new SSocket(events[i].data.fd));

	return 0;
}