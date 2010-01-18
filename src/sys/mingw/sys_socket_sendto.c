#include <stdio.h>
#include <string.h>
#include <winsock2.h>

#include "kc/config.h"
#include "sys/sysdep.h"

int
sys_socket_sendto(int socket, unsigned char *buf, int bufsize, unsigned char ip0, unsigned char ip1, unsigned char ip2, unsigned char ip3, unsigned short port)
{
  struct sockaddr_in addr;

  memset(&addr, 0, sizeof (addr));
  addr.sin_family = AF_INET;
  addr.sin_port = htons(port);
  addr.sin_addr.s_addr = ip0 | ip1 << 8 | ip2 << 16 | ip3 << 24;

  return sendto(socket, buf, bufsize, 0, (struct sockaddr*)&addr, sizeof(addr));
}
