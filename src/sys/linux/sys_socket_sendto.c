#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#include "kc/config.h"
#include "sys/sysdep.h"

int
sys_socket_sendto(int socket, unsigned char *buf, int bufsize, unsigned char ip0, unsigned char ip1, unsigned char ip2, unsigned char ip3, unsigned short port)
{
  char ip[256];
  snprintf(ip, sizeof(ip), "%d.%d.%d.%d", ip0, ip1, ip2, ip3);

  struct sockaddr_in addr;
  memset(&addr, 0, sizeof (addr));
  addr.sin_family = AF_INET;
  addr.sin_port = htons(port);
  inet_pton(AF_INET, ip, &addr.sin_addr);

  return sendto(socket, buf, bufsize, 0, (struct sockaddr*)&addr, sizeof(addr));
}
