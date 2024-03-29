#include <arpa/inet.h>
#include <assert.h>
#include <ctype.h>
#include <fcntl.h>
#include <linux/if.h>
#include <linux/if_tun.h>
#include <netdb.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdatomic.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#include "common/common.h"
#include "common/sockets.h"
#include "common/tuntcp.h"

void cleanup(char *shared_memory)
{
	shmdt(shared_memory);
}

void cleanup_tcp(int descriptor, void *buffer)
{
	close(descriptor);
	free(buffer);
}

void shm_wait(atomic_char *guard)
{
	while (atomic_load(guard) != 'c')
		;
}

void shm_notify(atomic_char *guard)
{
	atomic_store(guard, 's');
}

void communicate(int descriptor,
				 char *shared_memory,
				 struct Arguments *args)
{
	char buffer[1024] = {0};
	void *shm_buffer = malloc(args->size);

	atomic_char *guard = (atomic_char *)shared_memory;
	atomic_init(guard, 's');
	assert(sizeof(atomic_char) == 1);

	struct tcp_conn conn;
	TCPConnection(descriptor, "192.0.2.2", "192.0.3.2", 80, &conn);

	const int setCount = args -> count;

	for (; args->count > 0; --args->count)
	{
		shm_wait(guard);

		memcpy(shm_buffer, shared_memory + 1, args->size);

		if (args->count != setCount)
		{
			conn.seq = 0;
			conn.ack = 0;
			conn.src_port = conn.src_port + 1;
		}

		send_tcp_packet(&conn, TCP_SYN);
		conn.state = TCP_SYN_SENT;

		shm_notify(guard);
		shm_wait(guard);

		read(descriptor, buffer, sizeof(buffer));
		conn.seq = 1;
		conn.ack = 1;

		send_tcp_packet(&conn, TCP_ACK);
		conn.state = TCP_ESTABLISHED;

		send_tcp_packet_data(&conn, TCP_ACK, args->size);

		shm_notify(guard);
		shm_wait(guard);

		memcpy(shm_buffer, shared_memory + 1, args->size);

		read(descriptor, buffer, sizeof(buffer));

		conn.seq = 1 + args->size;
		conn.ack = 1;

		uint8_t fin_ack_flag = 0;
		fin_ack_flag |= TCP_FIN | TCP_ACK;
		send_tcp_packet(&conn, fin_ack_flag);

		shm_notify(guard);
		shm_wait(guard);

		read(descriptor, buffer, sizeof(buffer));
		conn.seq = 1 + 1 + args->size;
		conn.ack = 1 + 1;

		send_tcp_packet(&conn, TCP_ACK);
		conn.state = TCP_CLOSED;

		shm_notify(guard);
	}

	cleanup_tcp(descriptor, shm_buffer);
}

int main(int argc, char *argv[])
{
	int segment_id;
	char *shared_memory;

	key_t segment_key;

	struct Arguments args;
	parse_arguments(&args, argc, argv);

	segment_key = generate_key("shm");
	segment_id = shmget(segment_key, 1 + args.size, IPC_CREAT | 0666);

	if (segment_id < 0)
	{
		throw("Could not get segment");
	}

	shared_memory = (char *)shmat(segment_id, NULL, 0);

	if (shared_memory < (char *)0)
	{
		throw("Could not attach segment");
	}

	int tun = openTun("tun0");

	communicate(tun, shared_memory, &args);

	cleanup(shared_memory);

	return EXIT_SUCCESS;
}