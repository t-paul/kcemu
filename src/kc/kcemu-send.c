#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/uio.h>
#include <sys/un.h>
#include <fcntl.h>
#include <curses.h>

int
main(void)
{
  int s, ret;
  char buf[1024];
  int len;
  struct sockaddr_un unix_addr;

  s = socket(AF_UNIX, SOCK_STREAM, 0);
  perror("socket: ");
  printf("socket returned %d\n", s);

  bzero((char *)&unix_addr, sizeof(unix_addr));
  unix_addr.sun_family = AF_UNIX;
  strcpy(unix_addr.sun_path, "/tmp/KCemu-in");
  len = strlen(unix_addr.sun_path) + sizeof(unix_addr.sun_family);

  ret = connect(s, (struct sockaddr *)&unix_addr, len);
  perror("connect: ");
  printf("connect returned %d\n", ret);

  initscr();
  cbreak();
  /* noecho(); */
  nonl();
  keypad(stdscr, TRUE);

  while (242)
    {
      buf[0] = getch();
      buf[1] = '\0';
      ret = send(s, buf, strlen(buf), 0);
    }
  return 0;
}
