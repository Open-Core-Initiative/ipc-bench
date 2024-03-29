#include <arpa/inet.h>
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

void cleanup_tcp(int descriptor, void *buffer)
{
	close(descriptor);
	free(buffer);
}

void cleanup(int segment_id, char *shared_memory)
{
	shmdt(shared_memory);
	shmctl(segment_id, IPC_RMID, NULL);
}

void shm_wait(atomic_char *guard)
{
	while (atomic_load(guard) != 's')
		;
}

void shm_notify(atomic_char *guard)
{
	atomic_store(guard, 'c');
}

void communicate(int descriptor,
				 char *shared_memory,
				 struct Arguments *args)
{
	struct Benchmarks bench;
	char buffer[1024] = {0};
	int message;
	atomic_char *guard = (atomic_char *)shared_memory;

	void *shm_buffer = malloc(args->size);

	// Wait for signal from client
	shm_wait(guard);
	setup_benchmarks(&bench);

	struct tcp_conn conn;
	TCPConnection(descriptor, "192.0.3.2", "192.0.2.2", 80, &conn);

	for (message = 0; message < args->count; ++message)
	{
		bench.single_start = now();

		memset(shared_memory + 1, '*', args->size);

		conn.state = TCP_LISTEN;

		shm_notify(guard);
		shm_wait(guard);

		if (message != 0)
		{
			conn.src_port = conn.src_port + 1;
		}

		read(descriptor, buffer, sizeof(buffer));
		conn.state = TCP_SYN_RECEIVED;
		conn.seq = 0;
		conn.ack = 1;

		uint8_t syn_ack_flag = 0;
		syn_ack_flag |= TCP_SYN | TCP_ACK;

		send_tcp_packet(&conn, syn_ack_flag);
		conn.state = TCP_ESTABLISHED;

		shm_notify(guard);
		shm_wait(guard);

		read(descriptor, buffer, sizeof(buffer));
		conn.seq = 1;
		conn.ack = 1 + args->size;

		send_tcp_packet(&conn, TCP_ACK);

		memcpy(shared_memory + 1, buffer, args->size);

		shm_notify(guard);
		shm_wait(guard);

		read(descriptor, buffer, sizeof(buffer));
		conn.seq = 1;
		conn.ack = 1 + 1 + args->size;

		send_tcp_packet(&conn, TCP_ACK);

		uint8_t fin_ack_flag = 0;
		fin_ack_flag |= TCP_FIN | TCP_ACK;
		send_tcp_packet(&conn, fin_ack_flag);
		conn.state = TCP_CLOSED;

		shm_notify(guard);
		shm_wait(guard);

		benchmark(&bench);
	}

	evaluate(&bench, args);
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
		throw("Error allocating segment");
	}

	shared_memory = (char *)shmat(segment_id, NULL, 0);

	if (shared_memory == (char *)-1)
	{
		throw("Error attaching segment");
	}

	int tun = openTun("tun1");

	communicate(tun, shared_memory, &args);
	cleanup(segment_id, shared_memory);

	return EXIT_SUCCESS;
}