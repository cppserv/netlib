#ifndef __NETLIB_HPP__
#define __NETLIB_HPP__

#include <iostream>
#include <vector>
#include <cstdlib>
#include <string>
#include <stdexcept>
#include <memory>

extern "C" {
#include "netlib.h"
}
//#include "../src/netlib_inline.c" // FOR ASYNCSOCKETS

using namespace std;

class SSocket //Sync socket
{
 private:
	SyncSocket *ss;
	SSL_CTX *sslConfig;
	enum syncSocketType type;

	bool listening;
	int listeningFd;

	//connection data
	string ipAddr;
	uint16_t port;

 protected:

 public:
	SSocket(); //NO SSL
	SSocket(int fd); //NO SSL with an alredy-opened socket
	SSocket(enum syncSocketType);
	SSocket(int fd, enum syncSocketType); //an fd should be provided. Default SSL conf would be used
	~SSocket();

	//Version
	static string getVersion(){
		return netlib_VERSION;
	}

	/* SSL_config : Every return, is this object
	 * This config functions should be called before call connect or accept.
	 */
	SSocket *setCA(string path);
	SSocket *setCert(string path);
	SSocket *setPrvKey(string path);
	SSocket *setVerify(bool verify);

	/* StartTLS and setupTLS: Are a couple of functions to upgrade a socket into SSock with SSL.
	 * startTLS: You should only call those functions when the ssock
	 * has been created from an alredy opened FD.
	 * configuration should be set previusly or default configuration would be used.
	 * The typical call should be:
	 * setupTLS->setCA->setCert->setPrvKey->setVerify->startTLS
	 */
	SSocket *setupTLS(enum syncSocketType newtype);
	void startTLS();

	void connect(string ip, uint16_t port);
	void listen(uint16_t port);

	//Can return NULL or exception
	SSocket *accept();
	SSocket *accept(struct timeval *timeout);

	//Configuration
	void getSocketTimeout(struct timeval *timeout);
	void setSocketTimeout(struct timeval *timeout);

	//recv and send functions have NO error check.
	inline int send(const void *message, size_t len) {
		return tcp_message_ssend(this->ss, message, len);
	}

	inline ssize_t recv(void *message, size_t len) { //simple recv
		return tcp_message_srecv(this->ss, message, len, 1);
	}

	inline ssize_t recv(void *message, size_t len, uint8_t sync) { //advanced recv
		return tcp_message_srecv(this->ss, message, len, sync);
	}

	//getters/setters
	inline int getFd() {
		return this->ss->sockfd;
	}

	inline string getRemoteIPAddr() { //TODO
		return this->ipAddr;
	}

	inline int getRemotePort() { //TODO
		return this->port;
	}

	inline string getLocalIPAddr() { //TODO
		return this->ipAddr;
	}

	inline uint16_t getLocalPort() { //TODO
		return this->port;
	}

	inline bool operator==(const int &rhs) {
		return this->getFd() == rhs;
	}

	/** setKeepAlive
	 * Sets the keepalive params
	 * @param socket The shocket itself
	 * @param cnt The maximum number of keepalive probes TCP should send before dropping the connection.
	 * @param idl The time (in seconds) the connection needs to remain idle before TCP starts sending keepalive probes.
	 * @param intlv The time (in seconds) between individual keepalive probes.
	 * @return 0 if OK, something else if error.
	 */
	inline int setKeepAlive(int  cnt, int  idl, int  intvl) {
		return tcp_setKeepAlive(ss->sockfd, cnt, idl, intvl);
	}

	/** getKeepAlive
	 * Gets the keepalive params
	 * @param socket The shocket itself
	 * @param cnt The maximum number of keepalive probes TCP should send before dropping the connection.
	 * @param idl The time (in seconds) the connection needs to remain idle before TCP starts sending keepalive probes.
	 * @param intlv The time (in seconds) between individual keepalive probes.
	 * @return 0 if OK, something else if error (like keepAlive is disabled).
	 */
	inline int getKeepAlive(int *cnt, int *idl, int *intvl) {
		return tcp_getKeepAlive(ss->sockfd, cnt, idl, intvl);
	}


};

inline bool operator==(const unique_ptr<SSocket> &lhs, const int &rhs)
{
	return lhs->getFd() == rhs;
}

#endif
