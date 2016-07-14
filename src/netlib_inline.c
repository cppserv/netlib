#include <netlib.h>

#ifdef __cplusplus
extern "C" {
#endif

	inline void flush_send_async(AsyncSocket *sock)
	{
		sock->to_access[sock->current_send_buf] = 1;
		sock->current_send_buf = (sock->current_send_buf + 1) % 2;
		sock->write_pos[sock->current_send_buf] = 0;
	}

	inline void flush_send_sync(AsyncSocket *sock)
	{
		//pthread_spin_lock(&(sock->lock));
		sock->to_access[sock->current_send_buf] = 1;

		sock->current_send_buf = (sock->current_send_buf + 1) % 2;

		// Wait until the buffer has been sent
		while (sock->to_access[sock->current_send_buf] && !sock->closed) {
			pthread_spin_unlock(&(sock->lock));
			struct timespec ts = {.tv_sec = 0, .tv_nsec = 100};
			nanosleep(&ts, 0);
			pthread_spin_lock(&(sock->lock));
		}

		sock->write_pos[sock->current_send_buf] = 0;
		//pthread_spin_unlock(&(sock->lock));
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
		uint8_t *msgptr = (uint8_t *)message;

		pthread_spin_lock(&(sock->lock));

		while (unlikely(sock->buf_len - sock->write_pos[sock->current_send_buf] < len)) {
			memcpy(sock->buff[sock->current_send_buf] + sock->write_pos[sock->current_send_buf], msgptr, sock->buf_len - sock->write_pos[sock->current_send_buf]);
			msgptr += sock->buf_len - sock->write_pos[sock->current_send_buf];
			len -= sock->buf_len - sock->write_pos[sock->current_send_buf];

			sock->write_pos[sock->current_send_buf] = sock->buf_len;

			//pthread_spin_unlock(&(sock->lock));
			flush_send_sync(sock);
			//pthread_spin_lock(&(sock->lock));
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
				struct timespec ts = {.tv_sec = 0, .tv_nsec = 100};
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

			memcpy(
				(uint8_t *)message + position_in_message,
				sock->buff[sock->current_recv_buf] + sock->read_pos[sock->current_recv_buf],
				to_read);
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

	/** tcp_async_numbuf
	 * @return the number of buffers available to read in the socket
	 */
	inline uint_fast8_t tcp_async_numbuf(AsyncSocket *sock)
	{
		return sock->to_access[0] + sock->to_access[1];
	}

	/** tcp_async_peakInt
	 * looks at the beggining of the socket (without read) and returns its content
	 * @return the first 4 bytes at sock
	 */
	inline uint32_t tcp_async_peakInt(AsyncSocket *sock)
	{
		return *(uint32_t *)(sock->buff[sock->current_recv_buf] + sock->read_pos[sock->current_recv_buf]);
	}

	/** tcp_async_availableBytes
	 * Returns the bytes availables at the socket
	 * @return the bytes availables at the socket
	 */
	inline uint32_t tcp_async_availableBytes(AsyncSocket *sock)
	{
		if (!sock->to_access[sock->current_recv_buf]) {
			return 0;
		}

		size_t available_in_currbuf = sock->write_pos[sock->current_recv_buf] - sock->read_pos[sock->current_recv_buf];
		size_t available_in_otherbuf;

		if (sock->to_access[sock->current_recv_buf]) {
			available_in_otherbuf = sock->write_pos[(sock->current_recv_buf + 1) % 2];

		} else {
			available_in_otherbuf = 0;
		}

		return available_in_currbuf + available_in_otherbuf;
	}

#ifdef __cplusplus
}
#endif