/*
This program opens a socket and binds it to an IPv4 address present on the 
computer. Subsequently, it joins a multicast group on that interface.

IPv4 addresses and port numbers are provided interactively by the user,
but the user may also feed the program with pre-defined values by writing
the following to a text file and feeding it to the sender:
Example file config.txt:
<IPv4 mcast address> <port number>
<local IPv4 address>

Example execution of the program:
cat config.txt | ./sender

The program proceeds by attempting to transmit traffic to the multicast
group over that interface.

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

#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>

#include "common.h"

void send_data(int sock, struct sockaddr_in* remote) {
  char buf[] = "Hello";
  do{
    int res = sendto(sock, buf, 6, 0, (struct sockaddr*)remote, sizeof(*remote));
    if (res == -1) {
      perror("Failed in call to send: ");
    }
    sleep(1);
  }while(1);
}

int main() {
  int sock;
  if (!get_socket(&sock)) {return 0;}

  struct sockaddr_in mcast, local;
  if (!populate_mcast_sockaddr_in(&mcast)) {return 0;}
  
  /* Populate the structure so that we can join the multicast group on the socket via a local interface. */
  struct ip_mreqn mcast_join;
  size_t mtu;
  mcast_join.imr_multiaddr.s_addr = mcast.sin_addr.s_addr;
  mcast_join.imr_ifindex = 0;
  if (!get_local_interface_address_and_mtu(sock, &mcast_join.imr_address, &mtu)) {
    printf("Exiting.\n");
    return 0;
  }
  local.sin_addr   = mcast_join.imr_multiaddr;
  local.sin_family = AF_INET;
  local.sin_port   = htons(ntohs(mcast.sin_port) + 1);

  if (!bind_socket(sock, &local)) {return 0;}

  if (!join_group(sock, &mcast_join)) {return 0;}

  send_data(sock, &mcast);
}
