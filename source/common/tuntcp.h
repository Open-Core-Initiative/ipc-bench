#ifndef TUNTCP_H
#define TUNTCP_H

#include <stdint.h>
#include <stdlib.h>

#define PROTO_ICMP 1
#define PROTO_TCP 6

#define TCP_FIN 1
#define TCP_SYN 2
#define TCP_RST 4
#define TCP_PSH 8
#define TCP_ACK 16
#define TCP_URG 32
#define TCP_ECE 64
#define TCP_CWR 128

#define iphlen(ip) ((ip)->version_ihl >> 4 | 5)
#define ipdlen(ip) ((ip)->len - iphlen(ip))
#define tcphlen(tcp) (tcp->rsvd_offset << 2)
#define buf2ip(buf) ((struct ipv4 *)buf)
#define buf2tcp(buf, ip) ((struct tcp *)(buf + iphlen(ip) * 4))

enum tcp_state {
	TCP_LISTEN,
	TCP_SYN_SENT,
	TCP_SYN_RECEIVED,
	TCP_ESTABLISHED,
	TCP_FIN_WAIT_1,
	TCP_FIN_WAIT_2,
	TCP_CLOSE_WAIT,
	TCP_CLOSING,
	TCP_LAST_ACK,
	TCP_TIME_WAIT,
	TCP_CLOSED
};

struct ipv4 {
	uint8_t version_ihl;
	uint8_t tos;
	uint16_t len;
	uint16_t id;
	uint16_t frag_offset;
	uint8_t ttl;
	uint8_t proto;
	uint16_t checksum;
	uint32_t src;
	uint32_t dst;
};

struct icmpecho {
	uint8_t type;
	uint8_t code;
	uint16_t checksum;
	uint16_t id;
	uint16_t seq;
};

struct tcp {
	uint16_t sport;
	uint16_t dport;
	uint32_t seq;
	uint32_t ack;
	uint8_t rsvd_offset;
	uint8_t flags;
	uint16_t win;
	uint16_t checksum;
	uint16_t urp;
};

struct pseudoheader {
	uint32_t src;
	uint32_t dst;
	uint8_t zero;
	uint8_t proto;
	uint16_t tcp_len;
};

struct tcp_conn {
	int tun;
	enum tcp_state state;
	char *src_addr;
	uint16_t src_port;
	char *dst_addr;
	uint16_t dst_port;
	uint32_t seq;
	uint32_t ack;
};

void IPV4(size_t len_contents,
					uint8_t protocol,
					char *saddr,
					char *daddr,
					struct ipv4 *ip);
void ICMPEcho(uint16_t seq, struct icmpecho *echo);
void TCP(uint16_t sport,
				 uint16_t dport,
				 uint32_t seq,
				 uint32_t ack,
				 uint8_t flags,
				 struct tcp *tcp);
void TCPConnection(
		int tun, char *saddr, char *daddr, uint16_t port, struct tcp_conn *conn);
void send_tcp_packet(struct tcp_conn *conn, uint8_t flags);
void send_tcp_packet_data(struct tcp_conn *conn, uint8_t flags, int data_size);

uint16_t checksum(void *data, size_t count);
uint16_t tcp_checksum(struct ipv4 *ip, struct tcp *tcp);
uint16_t tcp_checksum_data(struct ipv4 *ip,
													 struct tcp *tcp,
													 char *data,
													 int data_size);
void print_bytes(void *bytes, size_t len);
void to_bytes(void *data, char *dst, size_t len);
int openTun(char *dev);

#endif// TUNTCP_H