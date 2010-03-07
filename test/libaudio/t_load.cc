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
#include <unistd.h>

#include <cppunit/TestCase.h>
#include <cppunit/TestSuite.h>
#include <cppunit/TestResult.h>
#include <cppunit/TestCaller.h>
#include <cppunit/TextTestRunner.h>

#include "libaudio/libaudio.h"

using namespace std;

class LibAudioTest : public CppUnit::TestCase { 
private:

public: 
  void setUp();
  void tearDown();

  void testOpenNull(void);
  void testOpenNotExisting(void);
  void testOpenEmpty(void);
  void testOpenShort(void);
  void testOpenRandom(void);
};

void
LibAudioTest::setUp(void)
{
  libaudio_init(LIBAUDIO_TYPE_ALL & ~LIBAUDIO_TYPE_MP3);
}

void
LibAudioTest::tearDown(void)
{
}

void
LibAudioTest::testOpenNull(void)
{
  cout << "LibAudioTest::testOpenNull()" << endl;
  libaudio_prop_t *prop = libaudio_open(NULL);
  CPPUNIT_ASSERT(prop == NULL);
}

void
LibAudioTest::testOpenNotExisting(void)
{
  cout << "LibAudioTest::testOpenNotExisting()" << endl;
  libaudio_prop_t *prop = libaudio_open("data/notexisting.wav");
  CPPUNIT_ASSERT(prop == NULL);
}

void
LibAudioTest::testOpenEmpty(void)
{
  cout << "LibAudioTest::testOpenEmpty()" << endl;
  libaudio_prop_t *prop = libaudio_open("data/empty.wav");
  CPPUNIT_ASSERT(prop == NULL);
}

void
LibAudioTest::testOpenShort(void)
{
  FILE *f;
  int a, b, ret;
  unsigned char c;
  libaudio_prop_t *prop;
  const char *filename = "data/short.wav";

  cout << "LibAudioTest::testOpenShort()" << endl;
  for (a = 1;a < 1024;a++)
    {
      f = fopen(filename, "wb");
      CPPUNIT_ASSERT(f != NULL);
      for (b = 0;b < a;b++)
	{
	  c = b & 0xff;
	  ret = fwrite(&c, 1, 1, f);
	  CPPUNIT_ASSERT(ret == 1);
	}
      fclose(f);
      cout << ".." << a << endl;
      prop = libaudio_open(filename);
      CPPUNIT_ASSERT(prop != NULL);
      ret = unlink(filename);
      CPPUNIT_ASSERT(ret == 0);
    }
}

void
LibAudioTest::testOpenRandom(void)
{
  cout << "LibAudioTest::testOpenRandom()" << endl;
  libaudio_prop_t *prop = libaudio_open("data/random.wav");
  CPPUNIT_ASSERT(prop == NULL);
}

int EF_DISABLE_BANNER = 1;
int EF_PROTECT_FREE = 0;
int EF_FREE_WIPES = 1;
int EF_ALLOW_MALLOC_0 = 1;

int
main(void)
{
  CppUnit::TestResult result;
  CppUnit::TextTestRunner runner;

  CppUnit::TestSuite *suite = new CppUnit::TestSuite();
  suite->addTest(new CppUnit::TestCaller<LibAudioTest>("testOpenNull", &LibAudioTest::testOpenNull));
  suite->addTest(new CppUnit::TestCaller<LibAudioTest>("testOpenNotExisting", &LibAudioTest::testOpenNotExisting));
  suite->addTest(new CppUnit::TestCaller<LibAudioTest>("testOpenEmpty", &LibAudioTest::testOpenEmpty));
  suite->addTest(new CppUnit::TestCaller<LibAudioTest>("testOpenShort", &LibAudioTest::testOpenShort));
  suite->addTest(new CppUnit::TestCaller<LibAudioTest>("testOpenRandom", &LibAudioTest::testOpenRandom));
  runner.addTest(suite);
  runner.run();

  return 0;
}
