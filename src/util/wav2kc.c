/*
 *  KCemu -- the KC 85/3 and KC 85/4 Emulator
 *  Copyright (C) 1997-1998 Torsten Paul
 *
 *  $Id: wav2kc.c,v 1.5 2000/09/30 19:02:02 tp Exp $
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
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

/*
 *  usage: brec -s 48000 -b 8 -t 900 | ./wav2kc
 */
#include <math.h>
#include <stdio.h>
#include <ctype.h>

#define PLUS (4)
#define MINUS (-4)

#define START_LONG_SYNC (500)
#define START_SHORT_SYNC (120)

#define SHOW_BLOCK
#define SHOW_BYTES
#define SHOW_BITS

extern int _bytes;

static int start_block = 1;
static int no_display = 1;

static long bytes_read = 0;
static int sync_val = 0;
static int last_vals[8];
static int BIT_0, BIT_1, BIT_S;
typedef void (*state_func)(int bit);
static struct
{
  int sync;
  int done;
  int state;
  int bit_nr;
  int byte_nr;
  int block_nr;
  int last_bnr;
  unsigned char byte;
  unsigned char check_sum;
  char buf[128];
} state = {
  START_LONG_SYNC, 0,
};

static void
st_start(int bit)
{
  // printf("*** %s\n", __PRETTY_FUNCTION__);
  state.byte_nr = 0;
  state.check_sum = 0;
  state.state = 2;
  state.byte = 0;
  state.bit_nr = 8;
}

static void
st_wait_S(int bit)
{
  // printf("*** %s\n", __PRETTY_FUNCTION__);
  switch (bit)
    {
    case 1:
      break;
    case 2:
      state.state++;
      state.byte = 0;
      state.bit_nr = 8;
      break;
    default:
      printf("restart sync!\n");
      state.sync = START_SHORT_SYNC;
      state.state--;
    }
}

static void
handle_block(int block_nr)
{
  int a, b;
  char name[9];
  char filename[13];
  static FILE *f = NULL;

  if (block_nr == start_block)
    {
      for (a = b = 0;a < 8;a++)
        if (isprint(state.buf[a]) && !isspace(state.buf[a]))
          name[b++] = tolower(state.buf[a]);
      name[b] = '\0';
      strcpy(filename, name);
      strcat(filename, ".prg");
      f = fopen(filename, "wb");
      fprintf(stderr, "'%s' (%s)\n", name, filename);
    }
#ifdef SHOW_BLOCK
  printf("BLOCK %d [%d]\n", state.block_nr, _bytes);
#endif
  fprintf(stderr, "[%02x] ", state.block_nr);
  //fprintf(stderr, "[%02x / %d] ", state.block_nr, _bytes);
  //if (state.block_nr == 0x27) no_display = 0;
  if (f)
    if (fwrite(state.buf, 128, 1, f) != 1)
      {
        fprintf(stderr, "write error!\n");
        fclose(f);
        f = NULL;
      }
    else
      {
	fflush(f);
        // fprintf(stderr, "* <%d> ", bytes_read);
      }
  if (block_nr == 255)
    {
      fclose(f);
      f = NULL;
      fprintf(stderr, "\n");
    }
}

static void
st_read_byte(int bit)
{
  // printf("*** %s [%d]\n", __PRETTY_FUNCTION__, state.bit_nr);
  state.bit_nr--;
  state.byte /= 2;
  state.byte |= (bit << 7);
  if (state.bit_nr == 0)
    {
#ifdef SHOW_BYTES
      printf(" - %02x [%c] nr = %d, byte = %d\n",
		state.byte & 0xff,
		isprint(state.byte) ? state.byte : '.',
		state.byte_nr,
		_bytes);
#endif
      switch (state.byte_nr)
        {
        case 0:
          state.block_nr = state.byte;
          state.state--;
          break;
        case 129:
          state.check_sum &= 0xff;
          if (state.check_sum != state.byte)
            {
              fprintf(stderr, "*** checksum error [%d/ %d]! ***\n",
			state.check_sum, state.byte);
	      if (state.last_bnr == 0)
	        {
	          //state.done = 1;
		  //return;
		}
            }
          state.state++;
	  if ((state.last_bnr == 0) && (state.block_nr == 255))
	    {
	      state.done = 1;
	      return;
	    }
	  //printf("[%3d/%3d]", state.last_bnr, state.block_nr);
	  if (state.block_nr != 255)
	    if ((state.last_bnr + 1) != state.block_nr)
	      {
	        state.done = 1;
		fprintf(stderr, "[%02x?] ", state.block_nr);
	        return;
	      }
          handle_block(state.block_nr);
	  state.last_bnr++;
          if (state.block_nr == 255)
            state.done = 1;
          break;
        default:
          state.buf[state.byte_nr - 1] = state.byte;
          state.check_sum += state.byte;
          state.state--;
          break;
        }
      state.byte_nr++;
    }
}

static void
st_end_of_block(int bit)
{
  // printf("*** %s\n", __PRETTY_FUNCTION__);
  switch (bit)
    {
    case 2:
      break;
    default:
      state.sync = START_SHORT_SYNC;
      state.state = 0;
      break;
    }
}

static state_func states[] =
{
  st_start,
  st_wait_S,
  st_read_byte,
  st_end_of_block,
};

void
handle_bit(int bit, int counter)
{
  static int old_state = -1;

  // printf("*** %s\n", __PRETTY_FUNCTION__);
  if (old_state != state.state)
    {
      // printf("\n+++ new state: %d +++\n", state.state);
    }
#ifdef SHOW_BITS
  printf("[bit = %d] / counter = %d\n", bit, counter);
#endif
  old_state = state.state;
  (states[state.state])(bit);
}

void
display(int c)
{
  int a;

  if (no_display) return;

  c /= 4;
  printf("\n");
  for (a = 0;a < 64;a++)
    {
      printf(" ");
      if (a == c) printf("*");
    }
}

void
handle_half_wave(int stat, int counter)
{
  int a;
  static int locked = 0;
  static int first = 1;
  static int flip_flop = 0;

  if ((state.sync == 0) && (locked < 2))
    {
      if (counter > BIT_0)
        {
          locked++;
	  if (locked == 2)
	    {
	      //printf("locked with stat %d (counter = %d)\n",
		//     stat, counter);
	      handle_bit(2, counter);
	    }
        }
      return;
    }
  
  if (flip_flop == 0)
    {
      flip_flop = counter;
      return;
    }
  counter = counter + flip_flop;
  flip_flop = 0;

  // printf("%d [%3d] - sync = %d\n", counter, stat, state.sync);
  if (state.sync > 0)
    {
      if ((counter > (sync_val - 5)) && (counter < (sync_val + 5)))
        {
          state.sync--;
          if (state.sync == 0)
            {
              locked = 0;
	      if (first)
	        {
		  first = 0;
                  //BIT_1 = 2 * sync_val;
                  //BIT_0 = BIT_1 - sync_val / 2;
                  //BIT_S = BIT_1 + sync_val / 2;
                  BIT_1 = sync_val;
                  BIT_0 = BIT_1 - sync_val / 4;
                  BIT_S = BIT_1 + sync_val / 4;
                  printf("locked with sync_val = %d [%d/%d/%d]\n",
              		sync_val, BIT_S, BIT_1, BIT_0);
	        }
            }
        }
      else
        {
	  sync_val = 0;
	  for (a = 0;a < 8;a++)
            sync_val += last_vals[a];
	  sync_val /= 4;
          state.sync = START_SHORT_SYNC;
	  //printf("*** restart sync (sync_val = %d)\n", sync_val);
        }
      return;
    }

  if (counter <= BIT_0)
    {
      handle_bit(0, counter);
    }
  else if (counter >= BIT_S)
    {
      handle_bit(2, counter);
    }
  else
    {
      handle_bit(1, counter);
    }
}

void
do_read1(void)
{
  int c;
  int stat;
  int counter;

  stat = MINUS;
  counter = 0;

  while (242)
    {
      c = getchar();
      bytes_read++;
      if (c == EOF) exit(0);
      if (state.done) break;
      display(c);
      c -= 128;
      
      //printf("%5d ", c);
      if (stat == PLUS)
        {
          if (c < MINUS)
            {
              stat = MINUS;
              printf(" - %d\n", counter);
              handle_half_wave(stat, counter);
              counter = 0;
            }
        }
      else if (stat == MINUS)
        {
          if (c > PLUS)
            {
              stat = PLUS;
              printf(" + %d\n", counter);
              handle_half_wave(stat, counter);
              counter = 0;
            }
        }
      //printf("\n");
      counter++;
    }
}

#if 1
#define X(x) x
#else
#define X(x)
#endif
void
do_read2(void)
{
  int a;
  int stat, stat_old;
  int c, old;
  int max, min;
  int wait_for_0;
  unsigned long counter;

  old = 0;
  stat = stat_old = 0;
  counter = 0;
  max = 0;
  min = 255;
  while (242)
    {
      c = getchar();
      bytes_read++;
      if (c == EOF)
        exit(0);
      if (state.done) break;
      display(c);
      
      for (a = 0;a < 7;a++)
	last_vals[a] = last_vals[a + 1];
      last_vals[7] = counter;

      //X(printf("%3d | %3d | old = %5d", c, c - 128, old));
      if ((old <= 128) && (c > 128))
        {
          //X(printf(" +++ %d\n", counter));
          handle_half_wave(PLUS, counter);
          counter = 0;
        }
      if ((old >= 128) && (c < 128))
        {
          //X(printf(" --- %d\n", counter));
          handle_half_wave(MINUS, counter);
          counter = 0;
        }

      //X(printf("\n"));
      old = c;
      counter++;
    }
}

void
new_counter(int counter)
{
  int a;

  for (a = 0;a < 7;a++)
    last_vals[a] = last_vals[a + 1];
  last_vals[7] = counter;
}

void
init(int start)
{
  int a;

  start_block = start;

  for (a = 0;a < 8;a++)
    last_vals[a] = 0;
  state.done = 0;
  state.state = 0;
  state.sync = START_LONG_SYNC;
  state.last_bnr = start_block - 1;
}

#if 0
int
main(int argc, char **argv)
{
  int a;

  setbuf(stdin, NULL);
  setbuf(stdout, NULL);
  
  if (argc == 2)
	  if (strcmp(argv[1], "-0") == 0)
		  start_block = 0;
  
  printf("first block must have number %d\n", start_block);
  
  while (42)
    {
      for (a = 0;a < 8;a++)
        last_vals[a] = 0;
      state.done = 0;
      state.state = 0;
      state.sync = START_LONG_SYNC;
      state.last_bnr = start_block - 1;
      //do_read1();
      do_read2();
    }
}
#endif
