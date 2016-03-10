#pragma once

#include "features.h"
#include "../Handler.h"
#include "lib.h"

namespace FactUtilEmbedded
{
  class Console;
  class MenuHandler;
  class ConsoleMenuHandler;
  class NestedMenuHandler;

  class IMenuBase
  {
  public:
    struct Parameters
    {
      char** parameters;
      uint8_t count;

  #ifdef CONSOLE_BEHAVIOR_PROPAGATE
      Console* console;
  #endif

      Parameters(char** parameters, int count, Console* console) :
        parameters(parameters), count(count)
  #ifdef CONSOLE_BEHAVIOR_PROPAGATE
        ,console(console)
  #endif
        {}

      Parameters inc()
      {
  #ifdef CONSOLE_BEHAVIOR_PROPAGATE
        Parameters p(parameters + 1, count - 1, console);
  #else
        Parameters p(parameters + 1, count - 1, NULL);
  #endif

        return p;
      }
    };
  };

  class IMenu : public IMenuBase
  {
    friend ConsoleMenuHandler;
    friend MenuHandler;
    friend NestedMenuHandler;

  protected:
    virtual void showPrompt() = 0;
    virtual void handleCommand(Parameters p) = 0;
    // return value of true means input was processed and needs no further processing.
    // note that process does not mean command executed, but only that the one character
    // was handled in a specific way (i.e. tab completion)
#ifdef CONSOLE_FEATURE_ENHANCED_CHARPROCESSOR
    virtual
#endif
    bool processInput(Console* console, char c) { return false; }

    static void showKeyValuePair(const __FlashStringHelper* key, const __FlashStringHelper* value, uint8_t keyPadding);
  };

  class MenuEnumerator;

  class MenuBase
  {
    friend MenuEnumerator;

  protected:
    // name & description are always PROGMEM residents
    const __FlashStringHelper* name;
    const __FlashStringHelper* description;

    MenuBase() {}

    void setDesc(const __FlashStringHelper* name, const __FlashStringHelper* description)
    {
      this->name = name;
      this->description = description;
    }

  public:
    MenuBase(const __FlashStringHelper* name, const __FlashStringHelper* description) :
      name(name), description(description)
    {
      //this->name = (const char*) name;
      //this->description = (const char*) description;
    }

    MenuBase(const char* name, const char* description) :
      name((const __FlashStringHelper*) name),
      description((const __FlashStringHelper*) description)
    {

    }

    const __FlashStringHelper* getName() { return name; }
    const __FlashStringHelper* getDescription() { return description; }
    /*
    bool canHandle(Console::Parameters p)
    {
      return strcmp_P(p.parameters[0], (const char*) name) == 0;
    }*/
  };



  class MenuCommand :
    public MenuBase,
    public IMenu,
    public IHandler2<IMenu::Parameters, IMenu*>,
    public SinglyLinkedNode
  {
  protected:
    virtual void showPrompt() override;

    MenuCommand() {}

  public:
    MenuCommand(const __FlashStringHelper* name, const __FlashStringHelper* description) :
      MenuBase(name, description) {}

    MenuCommand(const char* name, const char* description) :
      MenuBase(name, description) {}

    virtual IMenu* canHandle(IMenu::Parameters input) override;
  };

  // TODO: Temporary - eventually "MenuHandler" will be named menu,
  // this is a placeholder as we transition over old items that should
  // derive direct from MenuCommand
  class Menu : public MenuCommand
  {
  protected:
    Menu() {}
  public:
    Menu(const __FlashStringHelper* name, const __FlashStringHelper* description) :
      MenuCommand(name, description) {}

    Menu(const char* name, const char* description) :
      MenuCommand(name, description) {}
  };

  typedef void (*menuHandler)(IMenu::Parameters);


  // We allow MenuGeneric some deferred initialization because Global initialization
  // doesn't work well for PSTR and anonymous function pointers
  class MenuGeneric : public MenuCommand
  {
    menuHandler handler;

  protected:
    virtual void handleCommand(Parameters p) override;

  public:
    MenuGeneric() {}

    MenuGeneric(menuHandler handler) :
      MenuCommand(name, description) { this->handler = handler; }

    void setHandler(menuHandler handler) { this->handler = handler; }
  };
}
