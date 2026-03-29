/*
This file implements common functions used by the sender and receiver programs.

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

#include "common.h"

#include <stdio.h>
#include <sys/socket.h>

bool get_socket(int* sock) {
  bool success = true;
  *sock = socket(AF_INET, SOCK_DGRAM, 0);
  if (*sock == -1) {
    perror("Failed to create socket");
    success = false;
  }
  return success;
}

bool conv_addr_to_native(const char* const addr, struct in_addr* const native){
  bool success = true;
  int ret = inet_pton(AF_INET, addr, native);
  if (ret != 1) {
    if (ret == 0) {
      printf("Failed to convert IPv4 address to native format.\n");
    } else {
      perror("Invalid address family");
    }
    success = false;
  }
  return success;
}

bool get_mcast_addr_port(struct in_addr* const addr, uint16_t* port) {
  bool   success = true;
  char*  user_string = NULL;
  char   ipv4_address[MAX_IPV4_CHARS];
  size_t user_string_bytes;
  memset(ipv4_address, 0, MAX_IPV4_CHARS);

  printf("Enter multicast address and port (IPv4addr port): ");

  ssize_t user_chars = getline(&user_string, &user_string_bytes, stdin);
  if (user_chars == -1) {
    perror("Failed to read user input");
    success = false;
  } else {
    unsigned uport;
    int matches = sscanf(user_string, SSCANF_ARG, ipv4_address, &uport);
    if (matches != 2) {
      printf("Failed to find a valid IPv4 address and port number in your input.\n");
      success = false;
    } else {
      if (uport > MAX_PORT) {
	printf("Too large port number - maximum is %u.\n", MAX_PORT);
        success = false;
      } else {
        *port = htons(uport);
        if (!conv_addr_to_native(ipv4_address, addr)) {
	  success = false;
        }
      }
    }
  }

  if (user_string != NULL) {
    free(user_string);
  }

  return success;
}

bool populate_mcast_sockaddr_in(struct sockaddr_in* local) {
  bool success = true;
  local->sin_family = AF_INET;
  if (!get_mcast_addr_port((struct in_addr*)&(local->sin_addr.s_addr), &(local->sin_port))) {
    printf("Exiting.\n");
    success = false;
  }
  return success;
}

bool bind_socket(int sock, struct sockaddr_in* local) {
  bool success = true;
  int res = bind(sock, (struct sockaddr*)local, sizeof(*local));
  if (res == -1) {
    perror("Failed to bind socket");
    success = false;
  }
  return success;
}

size_t get_interface_mtu(const int socket, const char* const local_address) {
  size_t mtu = 0;
  struct ifreq ifr;
  memset(&ifr, 0, sizeof(ifr));

  struct ifaddrs *addrs, *iap;
  struct sockaddr_in *sa;
  char buf[32];

  if (getifaddrs(&addrs) == -1) {
    perror("Failed to retrieve interface addresses");
    if (addrs != NULL) {
      freeifaddrs(addrs);
    }
    return mtu;
  }

  for (iap = addrs; iap != NULL; iap = iap->ifa_next) {
    if (iap->ifa_addr && (iap->ifa_flags & IFF_UP) && iap->ifa_addr->sa_family == AF_INET) {
      sa = (struct sockaddr_in *)(iap->ifa_addr);
      if (inet_ntop(iap->ifa_addr->sa_family, (void *)&(sa->sin_addr), buf, sizeof(buf)) == NULL) {
	perror("Failed to convert interface address to textual representation");
      }
      if (!strcmp(local_address, buf)) {
        printf("Found interface name %s matching local address %s\n", iap->ifa_name, local_address);
	memcpy(ifr.ifr_name, iap->ifa_name, IFNAMSIZ);
	break;
      }
    }
  }
  freeifaddrs(addrs);

  if (ioctl(socket, SIOCGIFMTU, &ifr) == -1) {
    perror("Failed to retrieve MTU for interface");
  } else {
    printf("Found an MTU of %u for interface %s\n", ifr.ifr_mtu, ifr.ifr_name);
    mtu = ifr.ifr_mtu;
  }

  return mtu;
}

bool get_local_interface_address_and_mtu(int socket, struct in_addr* addr, size_t* mtu) {
  bool   success = true;
  char*  user_string = NULL;
  char   ipv4_address[15];
  size_t user_string_bytes;

  printf("Enter the IPv4 address of the local interface used to receive the traffic: ");
  ssize_t user_chars = getline(&user_string, &user_string_bytes, stdin);
  if (user_chars == -1) {
    perror("Failed to read user input");
    success = false;
  } else {
    int matches = sscanf(user_string, "%s", ipv4_address);
    if (matches != 1) {
      printf("Failed to find a valid IPv4 address in your input.\n");
      success = false;
    } else {
      if (!conv_addr_to_native(ipv4_address, addr)) {
	success = false;
      } else {
        *mtu = get_interface_mtu(socket, ipv4_address);
      }
    }
  }

  if (user_string != NULL) {
    free(user_string);
  }

  return success;
}


bool join_group(int sock, struct ip_mreqn* mcast) {
  bool success = true;
  int res = setsockopt(sock, IPPROTO_IP, IP_ADD_MEMBERSHIP, mcast, sizeof(*mcast));
  if (res == -1) {
    perror("Failed in call to join membership");
    success = false;
  }
  return success;
}
