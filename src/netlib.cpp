#include <netlib.hpp>

SSocket::SSocket() : SSocket(NOSSL) {}
SSocket::SSocket(int fd) : SSocket(fd, NOSSL) {}

SSocket::SSocket(enum syncSocketType t)
{
	this->type = t;
	this->listening = false;

	this->sslConfig = NULL;//getDefaultSSocketSSLconfig(t, 0);
	this->ss = NULL;

	//Unknown ipaddr
	this->ipAddr = "::1";
	this->port  = 0;
}

SSocket::SSocket(int fd, enum syncSocketType t) : SSocket(t)
{
	//upgrade
	this->ss = tcp_upgrade2syncSocket(fd, this->type, this->sslConfig);
}

SSocket::~SSocket()
{
	if (this->ss) {
		tcp_sclose(this->ss);
	}

	if (this->listening) {
		close(listeningFd);
	}

	if (this->sslConfig) {
		SSL_CTX_free(this->sslConfig);
	}
}

void SSocket::connect(string ip, uint16_t port)
{
	if (this->ss) {
		throw runtime_error("Alredy connected");
	}

	if (this->listening) {
		throw runtime_error("The socket is listening");
	}

	int fd = tcp_connect_to(ip.c_str(), port);

	if (fd < 0) {
		throw runtime_error("Error connecting");
	}

	SyncSocket *tmpss = tcp_upgrade2syncSocket(fd, this->type, this->sslConfig);

	if (!tmpss) {
		close(fd);
		throw runtime_error("Error upgrading socket");
		this->ss = tmpss;
	}

	this->ipAddr = ip;
	this->port = port;
}

void SSocket::listen(uint16_t port)
{
	if (this->ss) {
		throw runtime_error("Alredy connected");
	}

	if (this->listening) {
		throw runtime_error("Alredy listening");
	}

	int fd = tcp_listen_on_port(port);

	if (fd < 0) {
		throw runtime_error("Error listening");
	}

	this->listening = true;
	this->listeningFd = fd;
	this->ipAddr = "::";
	this->port = port;
}

SSocket *SSocket::accept()
{
	return this->accept(NULL);
}

SSocket *SSocket::accept(struct timeval *timeout)
{
	if (this->ss) {
		throw runtime_error("Alredy connected");
	}

	if (!this->listening) {
		throw runtime_error("Not listening");
	}

	struct sockaddr_storage st;

	int  fd = tcp_acceptext(this->listeningFd, timeout, &st);

	if (fd < 0) {
		return NULL;
	}

	SyncSocket *tmpss = tcp_upgrade2syncSocket(fd, this->type, this->sslConfig);
	SSocket *ret = new SSocket(this->type);
	ret->ss = tmpss;
	ret->sslConfig = sslConfig;

	//Get ip data
	char clienthost   [NI_MAXHOST];
	char clientservice[NI_MAXSERV];
	getnameinfo((struct sockaddr *)&st, sizeof(st),
				clienthost, sizeof(clienthost),
				clientservice, sizeof(clientservice),
				NI_NUMERICHOST);

	ret->ipAddr = string(clienthost);
	ret->port   = atoi(clientservice);

#ifdef NL_DEBUG
	cerr << "[NETLIB][DEBUG] Connection from " << ret->ipAddr << ":" << ret->port << endl;
#endif

	return ret;
}

uint32_t SSocket::startTLS(enum syncSocketType newtype)
{
	if (newtype == NOSSL) {
		return 3;
	}

	if (this->type != NOSSL) {
		return 2;
	}

	uint32_t ret = syncSocketStartSSL(this->ss, newtype, this->sslConfig);

	if (!ret) {
		this->type = newtype;
	}

	return ret;
}

/* Configuration */
void SSocket::getSocketTimeout(struct timeval *timeout)
{
	if (!this->ss) {
		throw runtime_error("Not connected");
	}

	if (getsockopt(this->ss->sockfd, SOL_SOCKET, SO_RCVTIMEO, (char *)timeout,
				   NULL) < 0) {
		throw runtime_error("Cant get socket timeout");
	}
}

void SSocket::setSocketTimeout(struct timeval *timeout)
{
	if (!this->ss) {
		throw runtime_error("Not connected");
	}

	if (setsockopt(this->ss->sockfd, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout,
				   sizeof(timeout)) < 0) {
		throw runtime_error("Cant set socket timeout");
	}

	if (setsockopt(this->ss->sockfd, SOL_SOCKET, SO_SNDTIMEO, (char *)&timeout,
				   sizeof(timeout)) < 0) {
		throw runtime_error("Cant set socket timeout");
	}
}

/* SSL CONFIGs */
SSocket *SSocket::setCA(string path)
{
	if (!sslConfig) {
		this->sslConfig = getDefaultSSocketSSLconfig(this->type, 0);
	}

	SSL_CTX_load_verify_locations(this->sslConfig, path.c_str(), NULL);

	return this;
}

SSocket *SSocket::setCert(string path)
{
	if (!sslConfig) {
		this->sslConfig = getDefaultSSocketSSLconfig(this->type, 0);
	}

	SSL_CTX_use_certificate_file(this->sslConfig, path.c_str(), SSL_FILETYPE_PEM);

	return this;
}

SSocket *SSocket::setPrvKey(string path)
{
	if (!sslConfig) {
		this->sslConfig = getDefaultSSocketSSLconfig(this->type, 0);
	}

	SSL_CTX_use_PrivateKey_file(this->sslConfig, path.c_str(), SSL_FILETYPE_PEM);

	return this;
}

SSocket *SSocket::setVerify(bool verify)
{
	if (!sslConfig) {
		this->sslConfig = getDefaultSSocketSSLconfig(this->type, 0);
	}

	if (verify) {
		SSL_CTX_set_verify(sslConfig, SSL_VERIFY_PEER | SSL_VERIFY_FAIL_IF_NO_PEER_CERT, NULL);
		SSL_CTX_set_verify_depth(sslConfig, 3);

	} else {
		SSL_CTX_set_verify(sslConfig, SSL_VERIFY_NONE, NULL);
		SSL_CTX_set_verify_depth(sslConfig, 3);
	}

	return this;
}