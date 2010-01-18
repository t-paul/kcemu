#include <winsock2.h>

#include "kc/config.h"
#include "sys/sysdep.h"

void
sys_socket_close(int socket)
{
  closesocket(socket);
}
