/*
 *  KCemu -- the KC 85/3 and KC 85/4 Emulator
 *  Copyright (C) 1997-2001 Torsten Paul
 *
 *  $Id: save_WAV.c,v 1.1 2002/10/31 00:48:52 torsten_paul Exp $
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

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "fileio/loadP.h"

#define RATE    (44100)

#define BIT_0	(2400)
#define BIT_1	(1200)
#define BIT_S	(600)

typedef struct wav_header
{
  unsigned long  MainChunkID;                      // it will be 'RIFF'
  unsigned long  Length;
  unsigned long  ChunkTypeID;                      // it will be 'WAVE'
  unsigned long  SubChunkID;                       // it will be 'fmt '
  unsigned long  SubChunkLength;
  unsigned short AudioFormat;
  unsigned short NumChannels;
  unsigned long  SampleFreq;
  unsigned long  BytesPerSec;
  unsigned short BytesPerSample;
  unsigned short BitsPerSample;
  unsigned long  DataChunkID;                      // it will be 'data'
  unsigned long  DataLength;
} wav_header_t;

static int _value = -1;
static int _start_block = 1;

struct tape_buf
{
  int blockno;
  unsigned char *buf;
  struct tape_buf *next;
};

static void
write_bit(FILE *out)
{
  unsigned char c;
  
  c = 0x80 - (110 * _value);
  fwrite(&c, 1, 1, out);
}

static long
write_bits(FILE *out, int freq, int count, int *sync)
{
  int a, b;
  double fact;
  long flen = 0;
  
  switch (freq)
    {
    case BIT_0: fact = 120; break;
    case BIT_1: fact = 120; break;
    case BIT_S: fact = 120; break;
    default:    fact = 120; break;
    }
  
  for (a = 0;a < count;a++)
    {
      for (b = 0;b < 2;b++)
	{
	  while (*sync < RATE)
	    {
	      *sync += freq * 2;
	      write_bit(out);
	      flen++;
	    }
	  _value = -_value;
	  *sync -= RATE;
	}
    }
  
  return flen;
}

static long
write_byte(FILE *out, unsigned char b, int *sync)
{
  int a;
  long flen = 0;
  
  for (a = 0;a < 8;a++)
    {
      if (b & (1 << a))
	flen += write_bits(out, BIT_1, 1, sync); // 1 bit
      else
	flen += write_bits(out, BIT_0, 1, sync); // 0 bit
    }

  flen += write_bits(out, BIT_S, 1, sync); // sync bit
  
  return flen;
}

static long
write_block(FILE *out, int block, unsigned char *buf)
{
  int a;
  int sync = -1;
  long flen = 0;
  unsigned char crc = 0;
  
  //printf("[%02x] ", block); fflush(stdout);

  if (block == 1)
    flen += write_bits(out, BIT_1, 8000, &sync);
  else
    flen += write_bits(out, BIT_1, 200, &sync);

  flen += write_bits(out, BIT_S, 1, &sync); // sync bit
  flen += write_byte(out, block, &sync); // block number

  for (a = 0;a < 128;a++)
    {
      crc += buf[a];
      flen += write_byte(out, buf[a], &sync); // data
    }

  flen += write_byte(out, crc, &sync); // checksum
  
  return flen;
}

static long
write_file(FILE *out, const unsigned char *data, int size)
{
  long flen;
  int a, len;
  struct tape_buf *first, *buffer, *tmp;
  
  flen = 0;
  first = 0;
  buffer = 0;
  
  a = 0;
  len = size;
  while (len > 0)
    {
      tmp = (struct tape_buf *)malloc(sizeof(struct tape_buf));
      tmp->buf = (unsigned char *)malloc(129);
      tmp->next = 0;

      memset(tmp->buf, 0, 129);
      if (len > 129)
	memcpy(tmp->buf, data + a, 129);
      else
	memcpy(tmp->buf, data + a, len);

      a += 129;
      len -= 129;

      tmp->blockno = tmp->buf[0];

      if (buffer != 0)
	buffer->next = tmp;
      if (first == 0)
	first = tmp;
      buffer = tmp;
    }
  buffer->blockno = 0xff;
  
  for (buffer = first;buffer != 0;buffer = buffer->next)
    if (buffer->blockno >= _start_block)
      flen += write_block(out, buffer->blockno, buffer->buf + 1);
  
  //printf("\n");
  
  return flen;
}

int
fileio_save_wav(const char *filename, const unsigned char *data, int size)
{
  long flen;
  FILE *out;
  wav_header_t header;

  switch (fileio_get_kctype())
    {
    case FILEIO_KC85_1:
      _start_block = 0;
      break;
    case FILEIO_KC85_3:
      _start_block = 1;
      break;
    }
	
  header.MainChunkID    = 'R' | 'I' << 8 | 'F' << 16 | 'F' << 24;
  header.ChunkTypeID    = 'W' | 'A' << 8 | 'V' << 16 | 'E' << 24;
  header.SubChunkID     = 'f' | 'm' << 8 | 't' << 16 | ' ' << 24;
  header.SubChunkLength = 16;
  header.AudioFormat    = 1;
  header.NumChannels    = 1;
  header.SampleFreq     = RATE;
  header.BytesPerSec    = 5512;
  header.BytesPerSample = 1;
  header.BitsPerSample  = 8;
  header.DataChunkID    = 'd' | 'a' << 8 | 't' << 16 | 'a' << 24;
  header.DataLength = 0;
  header.Length     = header.DataLength + 44 - 8;
	
  out = fopen(filename, "wb");
  if (out == NULL)
    return -1;

  if (fwrite(&header, 1, sizeof(header), out) != sizeof(header))
    return -1;
	
  flen = write_file(out, data, size);

  header.DataLength = flen; 
  header.Length     = header.DataLength + 44 - 8;
  fseek(out, 0, SEEK_SET);
  fwrite(&header, 1, sizeof(header), out);
  fclose(out);

  return 0;
}

int
fileio_save_wav_prop(const char *filename, fileio_prop_t *prop)
{
  return fileio_save_wav(filename, prop->data, prop->size);
}
