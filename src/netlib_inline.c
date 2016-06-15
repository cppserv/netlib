#include <netlib.h>


inline void flush_send(AsyncSocket *sock)
{
	pthread_spin_lock(&(sock->lock));
	sock->to_access[sock->current_send_buf] = 1;

	sock->current_send_buf = (sock->current_send_buf + 1) % 2;

	// Wait until the buffer has been sent
	while (sock->to_access[sock->current_send_buf] && !sock->closed) {
		pthread_spin_unlock(&(sock->lock));
		struct timespec ts;
		ts.tv_sec = 0;
		ts.tv_nsec = 100;
		nanosleep(&ts, 0);
		pthread_spin_lock(&(sock->lock));
	}

	sock->write_pos[sock->current_send_buf] = 0;
	pthread_spin_unlock(&(sock->lock));
}
inline int can_be_read(AsyncSocket *s)
{
	int can_read = 0;
	pthread_spin_lock(&(s->lock));

	if (s->to_access[s->current_recv_buf]) {
		if (!s->closed) {
			can_read = 1;

		} else {
			can_read = s->read_pos[s->current_recv_buf] != s->write_pos[s->current_recv_buf];
		}
	}

	pthread_spin_unlock(&(s->lock));

	return can_read;
}

/* Name tcp_message_send_async
	* Sends a full message to a socket
	* Return 0 if OK, something else if error.
	*/
inline int tcp_message_send_async(AsyncSocket *sock, const void *message, size_t len)
{
	void *msgptr = (void *)message;

	pthread_spin_lock(&(sock->lock));

	while (unlikely(sock->buf_len - sock->write_pos[sock->current_send_buf] < len)) {
		memcpy(sock->buff[sock->current_send_buf] + sock->write_pos[sock->current_send_buf], msgptr, sock->buf_len - sock->write_pos[sock->current_send_buf]);
		msgptr += sock->buf_len - sock->write_pos[sock->current_send_buf];
		len -= sock->buf_len - sock->write_pos[sock->current_send_buf];

		sock->write_pos[sock->current_send_buf] = sock->buf_len;

		pthread_spin_unlock(&(sock->lock));
		flush_send(sock);
		pthread_spin_lock(&(sock->lock));
	}

	memcpy(sock->buff[sock->current_send_buf] + sock->write_pos[sock->current_send_buf], msgptr, len);
	sock->write_pos[sock->current_send_buf] += len;

	pthread_spin_unlock(&(sock->lock));
	return 0;
}

/* Name tcp_message_recv_async
	* Receives a full message from a socket
	* Return 0 if OK, something else if error.
	*/
inline int tcp_message_recv_async(AsyncSocket *sock, void *message, size_t len)
{
	size_t position_in_message = 0;

	while (position_in_message < len) {
		while (!sock->can_read) {
			struct timespec ts;
			ts.tv_sec = 0;
			ts.tv_nsec = 100;
			nanosleep(&ts, 0);

			pthread_spin_lock(&(sock->lock));

			if (sock->to_access[sock->current_recv_buf]) {
				sock->can_read = 1;

			} else if (sock->closed) {
				pthread_spin_unlock(&(sock->lock));
				return -1;
			}


			pthread_spin_unlock(&(sock->lock));
		}

		size_t needed_size = len - position_in_message;
		size_t available_in_socket = sock->write_pos[sock->current_recv_buf] - sock->read_pos[sock->current_recv_buf];
		size_t to_read;

		if (needed_size < available_in_socket) {
			to_read = needed_size;

		} else {
			to_read = available_in_socket;
		}

		memcpy(message + position_in_message, sock->buff[sock->current_recv_buf] + sock->read_pos[sock->current_recv_buf], to_read);
		position_in_message += to_read;
		sock->read_pos[sock->current_recv_buf] += to_read;


		if (sock->read_pos[sock->current_recv_buf] == sock->write_pos[sock->current_recv_buf]) {
			pthread_spin_lock(&(sock->lock));
			sock->to_access[sock->current_recv_buf] = 0;

			sock->current_recv_buf = (sock->current_recv_buf + 1) % 2;
			sock->can_read = 0;
			sock->read_pos[sock->current_recv_buf] = 0;

			pthread_spin_unlock(&(sock->lock));
		}
	}

	return 0;
}