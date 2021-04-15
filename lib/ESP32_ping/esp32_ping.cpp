/*
* ESP32 Ping library
*
* All rights reserved.
*
* Permission to use, copy, modify, and distribute this software
* and its documentation for any purpose and without fee is hereby
* granted, provided that the above copyright notice appear in all
* copies and that both that the copyright notice and this
* permission notice and warranty disclaimer appear in supporting
* documentation, and that the name of the author not be used in
* advertising or publicity pertaining to distribution of the
* software without specific, written prior permission.
*
* The author disclaim all warranties with regard to this
* software, including all implied warranties of merchantability
* and fitness.  In no event shall the author be liable for any
* special, indirect or consequential damages or any damages
* whatsoever resulting from loss of use, data or profits, whether
* in an action of contract, negligence or other tortious action,
* arising out of or in connection with the use or performance of
* this software.
*
* --------------------------------------------------------------------------------
*  Ping Library is based on the following source code:
*
* Lua RTOS, ping utility
*
*
* Author: Jaume Oliv� (jolive@iberoxarxa.com / jolive@whitecatboard.org)
*
* --------------------------------------------------------------------------------
*
* Redistribution and use in source and binary forms, with or without modification,
* are permitted provided that the following conditions are met:
*
* 1. Redistributions of source code must retain the above copyright notice,
*    this list of conditions and the following disclaimer.
* 2. Redistributions in binary form must reproduce the above copyright notice,
*    this list of conditions and the following disclaimer in the documentation
*    and/or other materials provided with the distribution.
* 3. The name of the author may not be used to endorse or promote products
*    derived from this software without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED
* WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
* MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT
* SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
* EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT
* OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
* INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
* CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
* IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY
* OF SUCH DAMAGE.
*
* This file is part of the lwIP TCP/IP stack.
*
*/

#ifdef ESP32

#include <Arduino.h>

#include <math.h>
#include <float.h>
#include <signal.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>

#include "esp32_ping.h"

#include "lwip/inet_chksum.h"
#include "lwip/ip.h"
#include "lwip/ip4.h"
#include "lwip/err.h"
#include "lwip/icmp.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include "lwip/netdb.h"
#include "lwip/dns.h"

static uint16_t ping_seq_num;
static uint8_t stopped = 0;

/*
* Statistics
*/
static uint32_t transmitted = 0;
static uint32_t received = 0;
static float min_time = 0;
static float max_time = 0;
static float mean_time = 0;
static float last_mean_time = 0;
static float var_time = 0;

#define PING_ID 0xAFAF

#define PING_DEFAULT_COUNT    10
#define PING_DEFAULT_INTERVAL  1
#define PING_DEFAULT_SIZE     32
#define PING_DEFAULT_TIMEOUT   1

/*
* Helper functions
*
*/
static void ping_prepare_echo(struct icmp_echo_hdr *iecho, uint16_t len) {
	size_t i;
	size_t data_len = len - sizeof(struct icmp_echo_hdr);

	ICMPH_TYPE_SET(iecho, ICMP_ECHO);
	ICMPH_CODE_SET(iecho, 0);
	iecho->chksum = 0;
	iecho->id = PING_ID;
	iecho->seqno = htons(++ping_seq_num);

	/* fill the additional data buffer with some data */
	for (i = 0; i < data_len; i++) {
		((char*)iecho)[sizeof(struct icmp_echo_hdr) + i] = (char)i;
	}

	iecho->chksum = inet_chksum(iecho, len);
}

static err_t ping_send(int s, ip4_addr_t *addr, int size) {
	struct icmp_echo_hdr *iecho;
	struct sockaddr_in to;
	size_t ping_size = sizeof(struct icmp_echo_hdr) + size;
	int err;

	iecho = (struct icmp_echo_hdr *)mem_malloc((mem_size_t)ping_size);
	if (!iecho) {
		return ERR_MEM;
	}

	ping_prepare_echo(iecho, (uint16_t)ping_size);

	to.sin_len = sizeof(to);
	to.sin_family = AF_INET;
	inet_addr_from_ipaddr(&to.sin_addr, addr);

	if ((err = sendto(s, iecho, ping_size, 0, (struct sockaddr*)&to, sizeof(to)))) {
		transmitted++;
	}

	return (err ? ERR_OK : ERR_VAL);
}

static void ping_recv(int s) {
	char buf[64];
	int fromlen, len;
	struct sockaddr_in from;
	struct ip_hdr *iphdr;
	struct icmp_echo_hdr *iecho = NULL;
	char ipa[16];
	struct timeval begin;
	struct timeval end;
	uint64_t micros_begin;
	uint64_t micros_end;
	float elapsed;

	// Register begin time
	gettimeofday(&begin, NULL);

	// Send
	while ((len = recvfrom(s, buf, sizeof(buf), 0, (struct sockaddr*)&from, (socklen_t*)&fromlen)) > 0) {
		if (len >= (int)(sizeof(struct ip_hdr) + sizeof(struct icmp_echo_hdr))) {
			// Register end time
			gettimeofday(&end, NULL);

			/// Get from IP address
			ip4_addr_t fromaddr;
			inet_addr_to_ipaddr(&fromaddr, &from.sin_addr);

			strcpy(ipa, inet_ntoa(fromaddr));

			// Get echo
			iphdr = (struct ip_hdr *)buf;
			iecho = (struct icmp_echo_hdr *)(buf + (IPH_HL(iphdr) * 4));

			// Print ....
			if ((iecho->id == PING_ID) && (iecho->seqno == htons(ping_seq_num))) {
				received++;

				// Get elapsed time in milliseconds
				micros_begin = begin.tv_sec * 1000000;
				micros_begin += begin.tv_usec;

				micros_end = end.tv_sec * 1000000;
				micros_end += end.tv_usec;

				elapsed = (float)(micros_end - micros_begin) / (float)1000.0;

				// Update statistics
				// Mean and variance are computed in an incremental way
				if (elapsed < min_time) {
					min_time = elapsed;
				}

				if (elapsed > max_time) {
					max_time = elapsed;
				}

				last_mean_time = mean_time;
				mean_time = (((received - 1) * mean_time) + elapsed) / received;

				if (received > 1) {
					var_time = var_time + ((elapsed - last_mean_time) * (elapsed - mean_time));
				}

				// Print ...
				log_d("%d bytes from %s: icmp_seq=%d time=%.3f ms\r\n", len, ipa,
					ntohs(iecho->seqno), elapsed
				);

				return;
			}
			else {
				// TODO
			}
		}
	}

	if (len < 0) {
		log_d("Request timeout for icmp_seq %d\r\n", ping_seq_num);
	}
}
/*
static void stop_action(int i) {
	signal(i, SIG_DFL);

	stopped = 1;
}
+/
/*
* Operation functions
*
*/
void ping(const char *name, int count, int interval, int size, int timeout) {
	// Resolve name
	hostent * target = gethostbyname(name);
	IPAddress adr = *target->h_addr_list[0];
	if (target->h_length == 0) {
		// TODO: error not found target?????
		return;
	}
	ping_start(adr, count, interval, size, timeout);
}
bool ping_start(struct ping_option *ping_o) {


	return ping_start(ping_o->ip,ping_o->count,0,0,0);

}
bool ping_start(IPAddress adr, int count=0, int interval=0, int size=0, int timeout=0) {
//	driver_error_t *error;
	struct sockaddr_in address;
	ip4_addr_t ping_target;
	int s;
	// Get default values if argument are not provided
	if (count == 0) {
		count = PING_DEFAULT_COUNT;
	}

	if (interval == 0) {
		interval = PING_DEFAULT_INTERVAL;
	}

	if (size == 0) {
		size = PING_DEFAULT_SIZE;
	}

	if (timeout == 0) {
		timeout = PING_DEFAULT_TIMEOUT;
	}

	// Create socket
	if ((s = socket(AF_INET, SOCK_RAW, IP_PROTO_ICMP)) < 0) {
		// TODO: error
		return false;
	}


	address.sin_addr.s_addr = adr;
	ping_target.addr = address.sin_addr.s_addr;

	// Setup socket
	struct timeval tout;

	// Timeout
	tout.tv_sec = timeout;
	tout.tv_usec = 0;

	if (setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, &tout, sizeof(tout)) < 0) {
		closesocket(s);
		// TODO: error
		return false;
	}

	stopped = 0;
	transmitted = 0;
	received = 0;
	min_time = 1.E+9;// FLT_MAX;
	max_time = 0.0;
	mean_time = 0.0;
	var_time = 0.0;

	// Register signal for stop ping
	//signal(SIGINT, stop_action);

	// Begin ping ...
	char ipa[16];

	strcpy(ipa, inet_ntoa(ping_target));
	log_i("PING %s: %d data bytes\r\n",  ipa, size);

	ping_seq_num = 0;

	while ((ping_seq_num < count) && (!stopped)) {
		if (ping_send(s, &ping_target, size) == ERR_OK) {
			ping_recv(s);
		}
		delay( interval*1000L);
	}

	closesocket(s);

	log_i("%d packets transmitted, %d packets received, %.1f%% packet loss\r\n",
		transmitted,
		received,
		((((float)transmitted - (float)received) / (float)transmitted) * 100.0)
	);

	if (received) {
		ping_resp pingresp;
		log_i("round-trip min/avg/max/stddev = %.3f/%.3f/%.3f/%.3f ms\r\n", min_time, mean_time, max_time, sqrt(var_time / received));
		pingresp.total_count = 10;
		pingresp.timeout_count = 10;
		pingresp.total_bytes = 1;
		pingresp.total_time = mean_time;
		pingresp.ping_err = 0;
		return true;
	//	ping_o->sent_function(ping_o, (uint8*)&pingresp);
	}
	return false;
}

bool ping_regist_recv(struct ping_option *ping_opt, ping_recv_function ping_recv)
{
	if (ping_opt == NULL)
		return false;

	ping_opt->recv_function = ping_recv;
	return true;
}

bool ping_regist_sent(struct ping_option *ping_opt, ping_sent_function ping_sent)
{
	if (ping_opt == NULL)
		return false;

	ping_opt->sent_function = ping_sent;
	return true;
}
#endif
