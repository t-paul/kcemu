#include <sys/socket.h>

#include "kc/config.h"
#include "sys/sysdep.h"

int
sys_socket_send(int socket, unsigned char *buf, int bufsize)
{
  return send(socket, buf, bufsize, 0);
}
