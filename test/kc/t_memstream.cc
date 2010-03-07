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

#include <iostream>

#include <cppunit/TestCase.h>
#include <cppunit/TestSuite.h>
#include <cppunit/TestResult.h>
#include <cppunit/TestCaller.h>
#include <cppunit/TextTestRunner.h>

#include "kc/memstream.h"

using namespace std;

class MemStreamTest : public CppUnit::TestCase { 
private:
  iostream *stream;
  memstream *mstream;
  
public: 
  void setUp();
  void tearDown();

  void testRead(void);
  void testReadBinary(void);
  void testWrite(void);
  void testGCount(void);
  void testPeek(void);
  void testTellG(void);
};

void
MemStreamTest::setUp(void)
{
  mstream = new memstream((unsigned char *)"test", 4);
  stream = mstream;
}

void
MemStreamTest::tearDown(void)
{
  delete stream;
}

void
MemStreamTest::testRead(void)
{
  int c;

  cout << "MemStreamTest::testRead()" << endl;

  CPPUNIT_ASSERT(mstream->size() == 4);

  c = stream->get();
  CPPUNIT_ASSERT(c == 't');
  c = stream->get();
  CPPUNIT_ASSERT(c == 'e');
  c = stream->get();
  CPPUNIT_ASSERT(c == 's');
  c = stream->get();
  CPPUNIT_ASSERT(c == 't');
  c = stream->get();
  CPPUNIT_ASSERT(c == EOF);

  CPPUNIT_ASSERT(mstream->size() == 4);

  CPPUNIT_ASSERT(strcmp(mstream->str(), "test") == 0);
}

void
MemStreamTest::testReadBinary(void)
{
  int a, c;
  unsigned char buf[256];

  cout << "MemStreamTest::testReadBinary()" << endl;

  for (a = 0;a < 256;a++)
    buf[a] = a;

  memstream *s = new memstream(buf, 256);

  for (a = 0;a < 256;a++)
    CPPUNIT_ASSERT(s->get() == a);
  
  CPPUNIT_ASSERT(s->get() == EOF);

  delete s;
}

void
MemStreamTest::testWrite(void)
{
  int c;

  cout << "MemStreamTest::testWrite()" << endl;

  CPPUNIT_ASSERT(mstream->size() == 4);

  *stream << (unsigned char)0 << (unsigned char)1 << "23" << endl;

  CPPUNIT_ASSERT(mstream->size() == 9);

  c = stream->get();
  CPPUNIT_ASSERT(c == 't');
  c = stream->get();
  CPPUNIT_ASSERT(c == 'e');
  c = stream->get();
  CPPUNIT_ASSERT(c == 's');
  c = stream->get();
  CPPUNIT_ASSERT(c == 't');
  c = stream->get();
  CPPUNIT_ASSERT(c == '\0');
  c = stream->get();
  CPPUNIT_ASSERT(c == '\1');
  c = stream->get();
  CPPUNIT_ASSERT(c == '2');
  c = stream->get();
  CPPUNIT_ASSERT(c == '3');
  c = stream->get();
  CPPUNIT_ASSERT(c == '\n');
  c = stream->get();
  CPPUNIT_ASSERT(c == EOF);

  CPPUNIT_ASSERT(mstream->size() == 9);

  CPPUNIT_ASSERT(memcmp(mstream->str(), "test\000\00123\n", 9) == 0);
}

void
MemStreamTest::testGCount(void)
{
  cout << "MemStreamTest::testGCount()" << endl;
  cout << stream->gcount() << " ";
  cout << stream->get()    << " ";
  cout << stream->gcount() << " ";
  cout << stream->get()    << " ";
  cout << stream->gcount() << " ";
  cout << stream->get()    << " ";
  cout << stream->gcount() << " ";
  cout << stream->get()    << " ";
  cout << stream->gcount() << " ";
  cout << stream->get()    << " ";
  cout << stream->gcount() << " ";
  cout << endl;
}

void
MemStreamTest::testPeek(void)
{
  cout << "MemStreamTest::testPeek()" << endl;
  CPPUNIT_ASSERT(stream->peek() == 't');
  CPPUNIT_ASSERT(stream->peek() == 't');
  CPPUNIT_ASSERT(stream->get() == 't');
  CPPUNIT_ASSERT(stream->peek() == 'e');
  CPPUNIT_ASSERT(stream->peek() == 'e');
  CPPUNIT_ASSERT(stream->get() == 'e');
  CPPUNIT_ASSERT(stream->peek() == 's');
  CPPUNIT_ASSERT(stream->peek() == 's');
  CPPUNIT_ASSERT(stream->get() == 's');
  CPPUNIT_ASSERT(stream->peek() == 't');
  CPPUNIT_ASSERT(stream->peek() == 't');
  CPPUNIT_ASSERT(stream->get() == 't');
  CPPUNIT_ASSERT(stream->peek() == EOF);
  CPPUNIT_ASSERT(stream->peek() == EOF);
  CPPUNIT_ASSERT(stream->get() == EOF);
  CPPUNIT_ASSERT(stream->peek() == EOF);
  CPPUNIT_ASSERT(stream->peek() == EOF);
}

void
MemStreamTest::testTellG(void)
{
  cout << "MemStreamTest::testTellG()" << endl;

  CPPUNIT_ASSERT(stream->tellg() == 0);
  CPPUNIT_ASSERT(stream->get() == 't');
  CPPUNIT_ASSERT(stream->tellg() == 1);
  CPPUNIT_ASSERT(stream->get() == 'e');
  CPPUNIT_ASSERT(stream->tellg() == 2);
  CPPUNIT_ASSERT(stream->get() == 's');
  CPPUNIT_ASSERT(stream->tellg() == 3);
  CPPUNIT_ASSERT(stream->get() == 't');
  CPPUNIT_ASSERT(stream->tellg() == 4);
  CPPUNIT_ASSERT(stream->get() == EOF);
  CPPUNIT_ASSERT(stream->tellg() == 4);
}

int
main(void)
{
  CppUnit::TestResult result;
  CppUnit::TextTestRunner runner;

  CppUnit::TestSuite *suite = new CppUnit::TestSuite();
  suite->addTest(new CppUnit::TestCaller<MemStreamTest>("testRead",       &MemStreamTest::testRead));
  suite->addTest(new CppUnit::TestCaller<MemStreamTest>("testReadBinary", &MemStreamTest::testReadBinary));
  suite->addTest(new CppUnit::TestCaller<MemStreamTest>("testWrite",  	  &MemStreamTest::testWrite));
  suite->addTest(new CppUnit::TestCaller<MemStreamTest>("testGCount", 	  &MemStreamTest::testGCount));
  suite->addTest(new CppUnit::TestCaller<MemStreamTest>("testPeek",   	  &MemStreamTest::testPeek));
  suite->addTest(new CppUnit::TestCaller<MemStreamTest>("testTellG",  	  &MemStreamTest::testTellG));
  runner.addTest(suite);
  runner.run();

  return 0;
}
