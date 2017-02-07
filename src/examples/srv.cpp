#include <netlib.hpp>

int main()
{
	int lfd;

	lfd = tcp_listen_on_port(5000);

	auto sslSock = std::unique_ptr<SSocket>(new SSocket(lfd));

	cout << "Starting srv version: " << sslSock->getVersion() << endl;

	return 0;
}