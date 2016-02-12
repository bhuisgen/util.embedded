#include "Console.h"
#include "Menu.h"
//#include "Service.h"

using namespace FactUtilEmbedded;


#ifdef MEMORY_OPT_CODE
#define ERROR_UNRECOGNIZED_COMMAND "Unrecognized: "
#define INFO_AVAILABLE_COMMAND "Available: "
#else
#define ERROR_UNRECOGNIZED_COMMAND "Unrecognized command: "
#define INFO_AVAILABLE_COMMAND "Following commands are available: "
#endif


#ifndef AVR
// samd is supposed to have a strncmp_P mapping to regular strncmp,
// but doesn't for some reason (probably platformio outdated) so do it here
#define strncmp_P(s1, s2, n) strncmp((s1), (s2), (n))
#endif


Menu* MenuHandler::canHandle(Parameters p)
{
  return MenuEnumerator::canHandle(p);
}

void MenuHandler::handleCommand(Parameters p)
{
  Menu* menu = canHandle(p);
  if(menu != NULL)
    menu->handleCommand(p.inc());
  else if(strcmp_P(*p.parameters, PSTR("help")) == 0)
  {
    showHelp(p.inc());
  }
  else
  {
    cout.println();
    cout << F(ERROR_UNRECOGNIZED_COMMAND) << *p.parameters;
    cout.println();
  }
}


void MenuHandler::showHelp(Parameters p)
{
  if(p.count == 0)
  {
    SinglyLinkedNode* node = getHeadMenu();

    cout.println();

#ifndef MEMORY_OPT_CODE
    if(node == NULL)
    {
      cout << F("No commands registered");
      return;
    }
#endif

    cout << F(INFO_AVAILABLE_COMMAND);

    cout.println();
    cout.println();

    for(; node != NULL; node = node->getNext())
    {
      Menu* menu = (Menu*) node;

      cout << F("  ");
      showKeyValuePair(menu->getName(), menu->getDescription(), 16);
      cout.println();
    }
  }
}



#if defined(CONSOLE_FEATURE_AUTOCOMPLETE) && defined(CONSOLE_FEATURE_ENHANCED_CHARPROCESSOR)
bool MenuHandler::processInput(Console* console, char received)
{
  // look for tab character
  if(received == 9)
  {
    char* inputLine = console->getInputLine();
    uint8_t inputPos = console->getInputPos();

    SinglyLinkedNode* node = getHeadMenu();
    for(; node != NULL; node = node->getNext())
    {
      Menu* menu = (Menu*) node;
      const char* commandName = (const char*) menu->getName();
      if(strncmp_P(inputLine, commandName, inputPos) == 0)
      {
        // TODO: make an appendToInputLine_P
        char temp[32];

        strcpy_P(temp, commandName + inputPos);

        cout << temp;

        console->appendToInputLine(temp);
      }
    }
    return true;
  }
  else
    return false;
}
#endif


void Menu::showPrompt()
{
  cout << name;
}

IMenu* Menu::canHandle(IMenu::Parameters p)
{
  if(strcmp_P(p.parameters[0], (const char*) name) == 0)
    return this;
  else
    return NULL;
}

void MenuGeneric::handleCommand(IMenu::Parameters p)
{
  handler(p);
}

void IMenu::showKeyValuePair(const __FlashStringHelper* key, const __FlashStringHelper* value, uint8_t keyPadding)
{
  // FIX: some Print classes don't seem to return proper bytes-written
  size_t nameLength = strlen_P((const char*) key);
  //size_t nameLength = cout.print(menu->getName());
  cout.print(key);
  if(keyPadding > nameLength)
    keyPadding -= nameLength;

  while(keyPadding-- > 0) cout.print(' ');
  cout.print(value);
}

void NestedMenuHandler::handleCommand(Parameters p)
{
  if(strcmp_P(*p.parameters, PSTR("cd")) == 0)
  {
    IMenu* handleAble = canHandle(p.inc());

    if(handleAble)
    {
      selected = handleAble;
    }
  }
}

void NestedMenuHandler::showPrompt()
{
  // TODO: We can turn getName into showLocalPrompt if we want to go even more OOP
  cout << getName();
  if(getSelected())
  {
    cout << ' ';
    getSelected()->showPrompt();
  }
  else
  {
    cout << '>';
  }
}
