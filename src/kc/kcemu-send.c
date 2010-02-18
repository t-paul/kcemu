/*
 *  KCemu -- The emulator for the KC85 homecomputer series and much more.
 *  Copyright (C) 1997-2010 Torsten Paul
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

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
