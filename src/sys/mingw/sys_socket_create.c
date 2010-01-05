#include <winsock2.h>

#include "kc/config.h"
#include "sys/sysdep.h"

int
sys_socket_create(int stream, int nonblocking)
{
  int type = stream ? SOCK_STREAM : SOCK_DGRAM;

  int s = socket(AF_INET, type, 0);
  if (s < 0)
    return -1;

  if (!nonblocking)
    return s;

  unsigned long val = 1;
  if (ioctlsocket(s, FIONBIO, &val) == 0)
    return s;

  close(s);
  return -1;
}
