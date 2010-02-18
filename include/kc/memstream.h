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

#ifndef __kc_memstream_h
#define __kc_memstream_h

#include <stdio.h>
#include <string.h>

#include "kc/config.h"

using namespace std;

class membuf : public std::streambuf
{
 private:
  int _idx;
  int _size;
  int _bufsize;
  unsigned char *_data;

 protected:
  virtual int overflow(int c = EOF) {
    unsigned char *tmp;

    //std::cout << "overflow()" << std::endl;

    if (_size >= _bufsize)
      {
	_bufsize *= 2;
	tmp = new unsigned char[_bufsize];
	memcpy(tmp, _data, _size);
	delete _data;
	_data = tmp;
      }

    _data[_size++] = c;

    return c;
  }

  /*
   *  read character and advance input pointer
   */
  virtual int uflow(void) {
    int c;

    if (_idx >= _size)
      c = EOF;
    else
      c = _data[_idx++];

    //std::cout << "uflow(): " << c << std::endl;
    return c;
  }

  /*
   *  read character but don't advance input pointer
   */
  virtual int underflow(void) {

    //std::cout << "underflow()" << std::endl;

    if (_idx >= _size)
      return EOF;

    return _data[_idx];
  }

  /*
   *  put character back onto the input stream
   */
  virtual int pbackfail(int c = EOF) {

    //std::cout << "pbackfail()" << std::endl;

    if (c != EOF)
      {
        _data[_idx] = c;
        if (_idx > 0)
          _idx--;
      }
    return c;
  }

 public:
  membuf(void) {
    _idx = 0;
    _size = 0;
    _bufsize = 1024;
    _data = new unsigned char[_bufsize];
  }

  membuf(unsigned char *data, int size) {
    _idx = 0;
    _size = size;
    _bufsize = size;
    _data = new unsigned char[size];
    memcpy(_data, data, size);
  }

  virtual int tellg(void) {
    return _idx;
  }

  virtual int peek() {
    return underflow();
  }

  virtual int size(void) {
    return _size;
  }

  virtual const char * data(void) {
    return (const char *)_data;
  }
};

class memstream : public std::iostream
{
 private:
  membuf _buf;

 public:
  memstream(void) : std::iostream(&_buf) {
  }

  memstream(unsigned char *data, int size) : std::iostream(&_buf), _buf(data, size) {
  }

  /*
   *  not correctly overwritten, will not be called via a
   *  reference of type istream
   */
  virtual int tellg(void) {
    return _buf.tellg();
  }

  /*
   *  not correctly overwritten, will not be called via a
   *  reference of type istream
   */
  virtual int peek() {
    //std::cout << "peek()" << std::endl;
    return _buf.peek();
  }
  
  virtual int size(void) {
    return _buf.size();
  }

  virtual const char * str(void) {
    return _buf.data();
  }
};

#endif /* __kc_memstream_h */
