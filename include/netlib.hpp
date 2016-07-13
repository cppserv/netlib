#ifndef __NETLIB_HPP__
#define __NETLIB_HPP__

#include <netlib.h>
//#include "../src/netlib_inline.c" // FOR ASYNCSOCKETS

#include <iostream>
#include <vector>
#include <cstdlib>
#include <string>
#include <stdexcept>
#include <memory>

using namespace std;

class SSocket //Sync socket
{
 private:
	SyncSocket *ss;
	SSL_CTX *sslConfig;
	enum syncSocketType type;

	bool listening;
	int listeningFd;

 protected:

 public:
	SSocket(); //NO SSL
	SSocket(enum syncSocketType);
	~SSocket();

	void connect(string ip, uint16_t port);
	void listen(uint16_t port);

	//Can return NULL or exception
	SSocket *accept();
	SSocket *accept(struct timeval *timeout);

	//Configuration
	void getSocketTimeout(struct timeval *timeout);
	void setSocketTimeout(struct timeval *timeout);

	//SSL_config : Every return, is this object
	SSocket *setCA(string path);
	SSocket *setCert(string path);
	SSocket *setPrvKey(string path);

	//recv and send functions have NO error check.
	inline int send(const void *message, size_t len) {
		return tcp_message_ssend(this->ss, message, len);
	}

	inline ssize_t recv(void *message, size_t len, uint8_t sync) {
		return tcp_message_srecv(this->ss, message, len, sync);
	}

	//getters/setters
	inline int getFd() {
		return this->ss->sockfd;
	}

	inline bool operator==(const int &rhs) {
		return this->getFd() == rhs;
	}

};

inline bool operator==(const unique_ptr<SSocket> &lhs, const int &rhs)
{
	return lhs->getFd() == rhs;
}

#endif
