#include "../include/netlib.hpp"

int main()
{
	auto ssl_socket = std::unique_ptr<SSocket>(new SSocket(CLISSL));
	ssl_socket->setCA("certs/ca.pem")->setVerify(false);
	ssl_socket->connect("::1", 5000);

	return 0;
}