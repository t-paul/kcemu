#include <cppunit/TestCase.h>
#include <cppunit/TestSuite.h>
#include <cppunit/TestResult.h>
#include <cppunit/TestCaller.h>
#include <cppunit/TextTestRunner.h>

#include "kc/system.h"

#include "kc/ctc.h"

class TestCTC : public CTC {
public:
  bool irq_0() {}
  bool irq_1() {}
  bool irq_2() {}
  bool irq_3() {}
  long int counter_value_0() { return 0; }
  long int counter_value_1() { return 0; }
  long int counter_value_2() { return 0; }
  long int counter_value_3() { return 0; }

  byte_t in(short unsigned int) { return 0; }
  void out(short unsigned int, unsigned char) {}

  long long int get_counter() { return 0; }
  byte_t trigger_irq(unsigned char) { return 1; }
  void add_callback(long long unsigned int, Callback *, void *) {}
};

class CTCTest : public CppUnit::TestCase { 
private:
  TestCTC *ctc;

public: 
  void setUp();
  void tearDown();

  void testNew(void);
};

void
CTCTest::setUp(void)
{
  ctc = new TestCTC();
}

void
CTCTest::tearDown(void)
{
  delete ctc;
}

void
CTCTest::testNew(void)
{
  cout << "CTCTest::testNew()" << endl;
}

int
main(void)
{
  CppUnit::TestResult result;
  CppUnit::TextTestRunner runner;

  CppUnit::TestSuite *suite = new CppUnit::TestSuite();
  suite->addTest(new CppUnit::TestCaller<CTCTest>("testNew", &CTCTest::testNew));
  runner.addTest(suite);
  runner.run();

  return 0;
}
