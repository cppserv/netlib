#include <netlib.hpp>

SSocket::SSocket() : SSocket(NOSSL) {}

SSocket::SSocket(enum syncSocketType t)
{
	this->type=t;
	this->listening=false;

	this->sslConfig=NULL;
	this->ss=NULL;
}

SSocket::~SSocket()
{
	if(this->ss)
		tcp_sclose(this->ss);

	if(this->listening)
		close(listeningFd);

	if(this->sslConfig)
		SSL_CTX_free(this->sslConfig);
}

void SSocket::connect(string ip, uint16_t port)
{
	if(this->ss)
		throw runtime_error("Alredy connected");

	if(this->listening)
		throw runtime_error("The socket is listening");

	int fd = tcp_connect_to(ip.c_str(), port);
	if(fd<0)
		throw runtime_error("Error connecting");

	SyncSocket * tmpss = tcp_upgrade2syncSocket(fd, this->type, this->sslConfig);
	if(!tmpss)
	{
		close(fd);
		throw runtime_error("Error upgrading socket");
		this->ss=tmpss;
	}	
}


void SSocket::listen(uint16_t port)
{
	if(this->ss)
		throw runtime_error("Alredy connected");

	if(this->listening)
		throw runtime_error("Alredy listening");

	int fd = tcp_listen_on_port(port);
	if(fd<0)
		throw runtime_error("Error listening");

	this->listening=true;
	this->listeningFd=fd;
}

SSocket * SSocket::accept() {
	return this->accept(NULL);
}

SSocket * SSocket::accept(struct timeval *timeout)
{
	if(this->ss)
		throw runtime_error("Alredy connected");

	if(!this->listening)
		throw runtime_error("Not listening");

	int  fd =tcp_accept(this->listeningFd, timeout);
	if(fd<0)
		return NULL;

	SyncSocket * tmpss = tcp_upgrade2syncSocket(fd, this->type, this->sslConfig);
	SSocket * ret = new SSocket(this->type);
	ret->ss= tmpss;
	ret->sslConfig= sslConfig;

	return ret;
}