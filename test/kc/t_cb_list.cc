/*
 *  KCemu -- the KC 85/3 and KC 85/4 Emulator
 *  Copyright (C) 1997-2001 Torsten Paul
 *
 *  $Id: t_cb_list.cc,v 1.1 2002/01/02 00:35:06 torsten_paul Exp $
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

#include <cppunit/TestCase.h>
#include <cppunit/TestSuite.h>
#include <cppunit/TestResult.h>
#include <cppunit/TestCaller.h>
#include <cppunit/TextTestRunner.h>

#include "kc/cb.h"
#include "kc/cb_list.h"

static int _cnt = 0;
static void *_data = 0;

class CallbackTest : public Callback
{
  void callback(void *data)
  {
    _data = data;
    _cnt++;
  }
};

class CallbackListTest : public CppUnit::TestCase { 
private:
  CallbackTest *cb;
  CallbackList *cb_list;
  
public: 
  void setUp();
  void tearDown();

  void testEmpty(void);
  void testCallback(void);
  void testClear(void);
  void testOrder1(void);
  void testOrder2(void);
  void testOrder3(void);
};

void
CallbackListTest::setUp(void)
{
  cb = new CallbackTest();
  cb_list = new CallbackList();
}

void
CallbackListTest::tearDown(void)
{
  delete cb_list;
  delete cb;
}

void
CallbackListTest::testEmpty(void)
{
  cout << "CallbackListTest::testEmpty()" << endl;
  cb_list->run_callbacks(8);
  CPPUNIT_ASSERT(_cnt == 0);
  cb_list->run_callbacks(12);
  CPPUNIT_ASSERT(_cnt == 0);
  cb_list->run_callbacks(14);
  CPPUNIT_ASSERT(_cnt == 0);
}

void
CallbackListTest::testCallback(void)
{
  cout << "CallbackListTest::testCallback()" << endl;

  _cnt = 0;
  cb_list->add_callback(10, cb, 0);

  cb_list->run_callbacks(8);
  CPPUNIT_ASSERT(_cnt == 0);
  cb_list->run_callbacks(12);
  CPPUNIT_ASSERT(_cnt == 1);
  cb_list->run_callbacks(14);
  CPPUNIT_ASSERT(_cnt == 1);
}

void
CallbackListTest::testClear(void)
{
  cout << "CallbackListTest::testClear()" << endl;

  _cnt = 0;
  cb_list->add_callback(10, cb, (void *)1);
  cb_list->add_callback(10, cb, (void *)2);
  cb_list->add_callback(10, cb, (void *)3);
  cb_list->clear();

  cb_list->run_callbacks(8);
  CPPUNIT_ASSERT(_cnt == 0);
  cb_list->run_callbacks(12);
  CPPUNIT_ASSERT(_cnt == 0);
  cb_list->run_callbacks(14);
  CPPUNIT_ASSERT(_cnt == 0);
}

void
CallbackListTest::testOrder1(void)
{
  cout << "CallbackListTest::testOrder1()" << endl;

  _cnt = 0;
  _data = 0;
  cb_list->add_callback(10, cb, (void *)1);
  cb_list->add_callback(10, cb, (void *)2);
  cb_list->add_callback(10, cb, (void *)3);

  cb_list->run_callbacks(8);
  CPPUNIT_ASSERT(_cnt == 0);
  CPPUNIT_ASSERT(_data == 0);
  cb_list->run_callbacks(12);
  CPPUNIT_ASSERT(_cnt == 3);
  CPPUNIT_ASSERT(_data == (void *)3);
  cb_list->run_callbacks(14);
  CPPUNIT_ASSERT(_cnt == 3);
  CPPUNIT_ASSERT(_data == (void *)3);
}

void
CallbackListTest::testOrder2(void)
{
  cout << "CallbackListTest::testOrder2()" << endl;

  _cnt = 0;
  _data = 0;
  cb_list->add_callback(10, cb, (void *)1);
  cb_list->add_callback(11, cb, (void *)2);
  cb_list->add_callback(12, cb, (void *)3);

  cb_list->run_callbacks(8);
  CPPUNIT_ASSERT(_cnt == 0);
  CPPUNIT_ASSERT(_data == 0);
  cb_list->run_callbacks(20);
  CPPUNIT_ASSERT(_cnt == 3);
  CPPUNIT_ASSERT(_data == (void *)3);
  cb_list->run_callbacks(30);
  CPPUNIT_ASSERT(_cnt == 3);
  CPPUNIT_ASSERT(_data == (void *)3);
}

void
CallbackListTest::testOrder3(void)
{
  cout << "CallbackListTest::testOrder3()" << endl;

  _cnt = 0;
  _data = 0;
  cb_list->add_callback(12, cb, (void *)1);
  cb_list->add_callback(11, cb, (void *)2);
  cb_list->add_callback(10, cb, (void *)3);

  cb_list->run_callbacks(8);
  CPPUNIT_ASSERT(_cnt == 0);
  CPPUNIT_ASSERT(_data == 0);
  cb_list->run_callbacks(20);
  CPPUNIT_ASSERT(_cnt == 3);
  CPPUNIT_ASSERT(_data == (void *)1);
  cb_list->run_callbacks(30);
  CPPUNIT_ASSERT(_cnt == 3);
  CPPUNIT_ASSERT(_data == (void *)1);
}

int
main(void)
{
  CppUnit::TestResult result;
  CppUnit::TextTestRunner runner;

  CppUnit::TestSuite *suite = new CppUnit::TestSuite();
  suite->addTest(new CppUnit::TestCaller<CallbackListTest>("testEmpty",    &CallbackListTest::testEmpty));
  suite->addTest(new CppUnit::TestCaller<CallbackListTest>("testCallback", &CallbackListTest::testCallback));
  suite->addTest(new CppUnit::TestCaller<CallbackListTest>("testClear",    &CallbackListTest::testClear));
  suite->addTest(new CppUnit::TestCaller<CallbackListTest>("testOrder1",   &CallbackListTest::testOrder1));
  suite->addTest(new CppUnit::TestCaller<CallbackListTest>("testOrder2",   &CallbackListTest::testOrder2));
  suite->addTest(new CppUnit::TestCaller<CallbackListTest>("testOrder3",   &CallbackListTest::testOrder3));
  runner.addTest(suite);
  runner.run();

  return 0;
}
