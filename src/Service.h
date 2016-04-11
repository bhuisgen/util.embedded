#pragma once

#include <Arduino.h>
#include "EventManager.h"
#include "fact/lib.h"

namespace FactUtilEmbedded
{

#define SERVICE_ERROR ((const __FlashStringHelper*) LightweightService::genericError)

#define SERVICE_STATUS_STARTING   "Starting"
#define SERVICE_STATUS_STARTED    "Started"
#ifdef MEMORY_OPT_CODE
#define SERVICE_STATUS_UNSTARTED  "Unstarted"
#else
#define SERVICE_STATUS_UNSTARTED  "Waiting to start"
#endif
#define SERVICE_STATUS_ERROR      "Error"
#ifdef MEMORY_OPT_CODE
#define SERVICE_STATUS_WAITING    "Waiting"
#else
#define SERVICE_STATUS_WAITING    "Waiting for other service"
#endif

typedef void (*initBasic)(void);
typedef const __FlashStringHelper* (*initErrorStatus)(void);
typedef const bool (*initErrorStatus2)(const __FlashStringHelper** status);
typedef const __FlashStringHelper* (*initFullStatus)(const __FlashStringHelper** status);

class Service;

typedef bool (*startService1)(Service& service);
typedef bool (*startService2);


// aka lightweight service
// TODO: consider begin/end paradigm instead of start/stop , since Arduino world prefers that
// TODO: refactor state machine and make Service into one --
class LightweightService
{
protected:
  enum State : uint8_t
  {
    Unstarted = 0,
    Starting = 1,
    Started = 2,
    Error = 3,
    Waiting = 4 // waiting on not yet started dependency
  };

  void setState(State state) { this->state = state; }
  State getState() const { return state; }

  // true = all dependencies satisfied, false = dependency did't initiailze
  bool awaitDependency(LightweightService* dependsOn);

private:
  State state;
  const __FlashStringHelper* statusMessage = (const __FlashStringHelper*) emptyString;

protected:
  void setStatusMessage(const __FlashStringHelper* statusMessage)
  {
    this->statusMessage = statusMessage;
  }

public:
  bool start(initErrorStatus initFunc);
  bool start(initErrorStatus initFunc, LightweightService* dependsOn);
  bool start(initErrorStatus2 initFunc);
  bool start(initErrorStatus2 initFunc, LightweightService* dependsOn);
  bool start(initFullStatus initFunc);
  bool start(initFullStatus initFunc, LightweightService* dependsOn);
  void start(initBasic initFunc)
  {
    setState(Starting);
    initFunc();
    setState(Started);
  }
  void start(initBasic initFunc, LightweightService* dependsOn)
  {
    if(awaitDependency(dependsOn))
      start(initFunc);
  }

  const __FlashStringHelper* getStateString();
  const __FlashStringHelper* getStatusMessage()
  {
    return statusMessage != NULL ? statusMessage : getStateString();
  }

  static const char genericError[] PROGMEM;
  static const char emptyString[] PROGMEM;

  bool isInitialized() const { return getState() == Started; }
};

class MenuService;

class Service : public LightweightService, public Named
{
  friend MenuService;

  LOCAL_EVENT(Service);

protected:
  void setState(State state)
  {
    LightweightService::setState(state);
#ifdef SERVICE_FEATURE_EVENTS
    statusUpdated.invoke(this);
#endif
  }

#ifdef SERVICE_FEATURE_RETAINED_STARTFUNC
  startService1 startFunc;
#endif

#ifdef SERVICE_FEATURE_RETAINED_DEPENDENCY
  LightweightService* dependsOn;
#endif

public:
  Service() {}
  Service(const char* name) : Named((const __FlashStringHelper*)name) {}
#ifdef SERVICE_FEATURE_RETAINED_STARTFUNC
  Service(const char* name, startService1 startFunc) : Named((const __FlashStringHelper*)name)
  {
    this->startFunc = startFunc;
  }
#endif

#ifdef SERVICE_FEATURE_EVENTS
  // fired when state or status message changes
  Event statusUpdated;
#endif

  // TODO: phase out explicit naming in favor of constructor-named,
  // this way unstarted services are still named
  //void start(const __FlashStringHelper* name, startService2);

  void start(startService1 startFunc)
  {
#ifdef SERVICE_FEATURE_RETAINED_STARTFUNC
    this->startFunc = startFunc;
#endif
    restart(startFunc);
  }
  void start(startService1, LightweightService* dependsOn);

  void restart(startService1);
#ifdef SERVICE_FEATURE_RETAINED_STARTFUNC
  void restart();
  bool start();
#endif

  //void restart(startService1) { }
  void addDependency(LightweightService* dependsOn)
  {

  }

  // TODO: iron out fact that these shouldn't be public (use private / protected)
  // base class
  void setStatusMessage(const __FlashStringHelper* statusMessage)
  {
    LightweightService::setStatusMessage(statusMessage);
#ifdef SERVICE_FEATURE_EVENTS
    statusUpdated.invoke(this);
#endif
  }
};

class IService
{
  PSTR_Property statusMessage;

protected:
  virtual void start() = 0;
  virtual void stop() {};
  virtual void pause() {};
};

class RestartableService : public Service
{

};

inline Print& operator <<(Print& p, LightweightService& arg)
{
  p.print(arg.getStateString());
  if(arg.getStatusMessage() != NULL)
  {
    p.print(F(": "));
    p.print(arg.getStatusMessage());
  }
  return p;
}

namespace layer5
{
  class Service : IService, Named
  {
  public:
    void doStart();
    void doStop();
    void doRestart();
  };
}

}
