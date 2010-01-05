#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

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

  if (fcntl(s, F_SETFL, O_NONBLOCK) == 0)
    return s;

  close(s);
  return -1;
}
