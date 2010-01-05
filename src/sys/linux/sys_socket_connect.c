#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "kc/config.h"
#include "sys/sysdep.h"

int
sys_socket_connect(int socket, unsigned char ip0, unsigned char ip1, unsigned char ip2, unsigned char ip3, unsigned short port)
{
  int err;
  char buf[4096];
  struct sockaddr_in _send_addr;
  
  snprintf(buf, sizeof(buf), "%d.%d.%d.%d", ip0, ip1, ip2, ip3);

  memset(&_send_addr, 0, sizeof (_send_addr));
  _send_addr.sin_family = AF_INET;
  _send_addr.sin_port = htons(port);
  inet_pton(AF_INET, buf, &_send_addr.sin_addr);

  err = connect(socket, (struct sockaddr *)&_send_addr, sizeof(_send_addr));
  if (err >= 0)
    return 0;

  switch (errno)
    {
      case EINTR:
        return SYS_SOCKET_ERR_INTR;
      case EINPROGRESS:
	return SYS_SOCKET_ERR_INPROGRESS;
      case EALREADY:
	return SYS_SOCKET_ERR_ALREADY;
      case EISCONN:
	return SYS_SOCKET_ERR_ISCONN;
      default:
        return errno;
    }
}
