#include <stdio.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <termios.h>

struct termios _tio_old;
struct termios _tio_new;

int
open_device(const char *filename)
{
  int fd;
  
  fd = open(filename, O_RDWR | O_NOCTTY);
  if (fd == 0) return 0;

  tcgetattr(fd, &_tio_old);
  bzero(&_tio_new, sizeof(_tio_new));
  
  /*
   *  BAUDRATE: Set bps rate. You could also use cfsetispeed and cfsetospeed.
   *  CRTSCTS : output hardware flow control (only used if the cable has
   *            all necessary lines. See sect. 7 of Serial-HOWTO)
   *  CS8     : 8n1 (8bit,no parity,1 stopbit)
   *  CLOCAL  : local connection, no modem contol
   *  CREAD   : enable receiving characters
   */
  _tio_new.c_cflag = B1200 | CRTSCTS | CS8 | CLOCAL | CREAD;

  /*
   *  IGNPAR  : ignore bytes with parity errors
   *  otherwise make device raw (no other input processing)
   */
  _tio_new.c_iflag = IGNPAR;

  /*
   *  Raw output.
   */
  _tio_new.c_oflag = 0;
  
  /*
   *  ICANON  : enable canonical input
   *  disable all echo functionality, and don't send signals to calling program
   */
  _tio_new.c_lflag = ICANON;
  _tio_new.c_lflag = 0;

  tcflush(fd, TCIFLUSH);
  tcsetattr(fd, TCSANOW, &_tio_new);

  return fd;
}

int
main(int argc, char **argv)
{
  FILE *f;
  int a, b, fd;
  unsigned char c;
  struct stat statbuf;
  int size, load, start, do_start;
  
  if ((argc < 3) || (argc > 4))
    {
      fprintf(stderr, "usage: %s file load_addr [start_addr]\n",
              argv[0]);
      exit(1);
    }
  
  fd = open_device("/dev/ttyS1");
  if (fd == 0) exit(1);

#if 0
  write(fd, "\r", 1); sleep(1);
  write(fd, "M", 1); sleep(1);
  write(fd, "\r", 1); sleep(1);
#endif
  
  setbuf(stdout, 0);
  
  if (stat(argv[1], &statbuf) != 0)
    {
      fprintf(stderr, "can't stat file `%s'\n", argv[1]);
      exit(1);
    }
  size = statbuf.st_size;
  
  do_start = 0;
  printf("argc = %d\n", argc);
  load = strtol(argv[2], NULL, 0);
  printf("load  = %04x\n", load);
  if (argc == 4)
    {
      do_start = 1;
      start = strtol(argv[3], NULL, 0);
      printf("start = %04x\n", start);
    }
  
  printf("filesize = %d\n", size);
  
  printf("<ESC>"); write(fd, "\x1b", 1); sleep(1);
  printf(" T"); write(fd, "T", 1); tcdrain(fd);
  c = load & 0xff;
  printf(" %02x", c); write(fd, &c, 1);
  c = (load >> 8) & 0xff;
  printf(" %02x", c); write(fd, &c, 1);
  c = size & 0xff;
  printf(" %02x", c); write(fd, &c, 1);
  c = (size >> 8) & 0xff;
  printf(" %02x\n", c); write(fd, &c, 1);
  
  f = fopen(argv[1], "rb");
  if (f == NULL)
    {
      fprintf(stderr, "can't open `%s'\n", argv[1]);
      exit(1);
    }
  
  for (a = 0;a < size;a++)
    {
      if ((a & 31) == 0) printf("\n");
      c = getc(f);
      printf("%c", isprint(c) ? c : '.');
      write(fd, &c, 1);
      //tcdrain(fd);
    }
  
  if ((a & 31) != 0) printf("\n");
  printf("\n");
  fclose(f);
  close(fd);
  
  fd = open_device("/dev/ttyS1");
  if (fd == 0) exit(1);
  
  if (do_start)
    {
      sleep(1);
      printf("<ESC>"); write(fd, "\x1b", 1);
      sleep(1);
      printf("U"); write(fd, "U", 1);
      c = start & 0xff;
      printf(" %02x", c); write(fd, &c, 1);
      c = (start >> 8) & 0xff;
      printf(" %02x\n", c); write(fd, &c, 1);
    }
//tcdrain(fd);
  close(fd);
  
  return 0;
}
