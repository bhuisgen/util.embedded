#include "catch.hpp"

//#include "Arduino.h"

#include <iostream>

#include <Console.h>
#include <fact/Menu.h>

using namespace util;
using namespace std;

void dummyHandler(IMenu::Parameters p) {}

MenuGeneric m(dummyHandler);

class TestMenuHandler : public Menu
{
public:
  void begin();
};

TestMenuHandler mh;
ConsoleMenu console(&mh);

void TestMenuHandler::begin()
{
  // NOTE: this causes a compiler error when it's inline
  add(m, F("status"), F("dbg"));
}

#define LED_PIN 0


class Test
{
public:
  const __FlashStringHelper* name;

  void doCompare();
  virtual void doCompare2(IMenu::Parameters p);
};


class Test2 :
  public Test,
  public IHandler2<IMenu::Parameters, Test*>
{
public:
  virtual Test* canHandle(IMenu::Parameters) override;
};


Test* Test2::canHandle(IMenu::Parameters p)
{
  if(strcmp_P(p.parameters[0], (PGM_P) name) == 0)
  {
#ifdef DEBUG2
    cout << F("Exit this");
    delay(500);
#endif
    return this;
  }
  else
  {
#ifdef DEBUG2
    cout << F("Exit NULL");
    delay(500);
#endif
    return NULL;
  }
}


void Test::doCompare()
{
  int result = strcmp_P("abc", (PGM_P) name);
  cout << result;
}

void Test::doCompare2(IMenu::Parameters p)
{
  int result = strcmp_P(p.parameters[0], (PGM_P) name);
  cout << result;
}

Test test;
Test2 test2;



SCENARIO( "Virtual/strcmp_P glitch check", "[glitch1]" )
{
  const char* parameters[1];
  parameters[0] = "abc";

  IMenu::Parameters p(parameters,1,NULL);

  GIVEN("Synthetic test")
  {
    test2.name = F("Hi");
    test2.canHandle(p);
  }

  GIVEN("Actual IMenu class")
  {
    mh.begin();
    m.canHandle(p);
  }
}
