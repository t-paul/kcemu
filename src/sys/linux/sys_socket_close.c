#include <unistd.h>
#include <sys/socket.h>

#include "kc/config.h"
#include "sys/sysdep.h"

void
sys_socket_close(int socket)
{
  shutdown(socket, SHUT_RDWR);
  close(socket);
}
