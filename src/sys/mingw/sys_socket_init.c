#include <winsock2.h>

#include "kc/config.h"
#include "sys/sysdep.h"

int
sys_socket_init(void)
{
  WORD wVersionRequested = MAKEWORD(2, 2);
  WSADATA wsaData;

  int err = WSAStartup(wVersionRequested, &wsaData);
  if (err != 0)
    {
      return 1;
    }

  if (LOBYTE(wsaData.wVersion) != 2 || HIBYTE(wsaData.wVersion) != 2)
    {
      WSACleanup();
      return 1;
    }

  return 0;
}
