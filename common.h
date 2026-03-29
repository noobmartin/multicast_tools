/*
This file defines common functions used by the sender and receiver programs.

Copyright (C) 2026  Alexander Rajula

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, see
<https://www.gnu.org/licenses/>.
*/

#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <ifaddrs.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>

#define DEFAULT_MTU 1500
#define MAX_PORT 65535
#define MAX_IPV4_CHARS 15
#define SSCANF_ARG "%15s %u"

bool get_socket(int* sock);

bool conv_addr_to_native(const char* const addr, struct in_addr* const native);

bool get_mcast_addr_port(struct in_addr* const addr, uint16_t* port);

bool populate_mcast_sockaddr_in(struct sockaddr_in* local);

bool bind_socket(int sock, struct sockaddr_in* local);

size_t get_interface_mtu(const int socket, const char* const local_address);

bool get_local_interface_address_and_mtu(int socket, struct in_addr* addr, size_t* mtu);

bool join_group(int sock, struct ip_mreqn* mcast);

