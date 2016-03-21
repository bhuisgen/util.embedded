#pragma once

// TODO: add a 2nd parameter, since the sender/this pointer can be weak by itself
// TODO: make wrappers so that this pointer is passed automatically
#include <Arduino.h>
// FIX: NODUINO is clunky check to ensure we grab our own uint8_t & friends
// however really we should try to grab it from stdint.h
#ifdef NODUINO
#include "fact/lib.h"
#endif
#include "fact/features.h"

// Thank you Mikael Patel for inspiration https://github.com/mikaelpatel/Arduino-Scheduler/blob/master/Scheduler/Queue.h


typedef void (*eventCallback)(void* parameter);

#ifndef HANDLEMANAGER_CAPACITY
#ifdef MEMORY_OPT_DATA
#define HANDLEMANAGER_CAPACITY 10
#else
#define HANDLEMANAGER_CAPACITY 20
#endif
#endif

class HandleBase;

// handle manager is like a set of linked lists coexisting in one pool
// a handle is a member of a particular list of handles.  Each handle has
// a singly-linked node pointing to the next handle until nullHandle is
// reached
class HandleManager
{
  friend HandleBase;
  
public:
  static const uint8_t nullHandle = 0;
  typedef uint8_t handle;

protected:
  class Handle
  {
    friend HandleManager;

  protected:
    void* data;
    handle next;
  public:
    void* getData() {return data;}
    handle getNext() {return next;}
  };

private:
  //Handle* handles;
  Handle handles[HANDLEMANAGER_CAPACITY];

protected:
  // allocate a data handle
  static handle alloc(Handle* handles, uint8_t capacity, void* data)
  {
    // look for any free ones (somewhat CPU expensive, but we shouldn't be adding/removing)
    // events so often - mainly firing them
    for(uint8_t i = 0; i < capacity; i++)
    {
      Handle& hEval = handles[i];

      if(hEval.getData() == NULL)
      {
        hEval.data = data;
        hEval.next = nullHandle;
        return i + 1;
      }
    }

    return nullHandle;
  }

  // allocate a data handle
  handle alloc(void* data)
  {
    return alloc(handles, HANDLEMANAGER_CAPACITY, data);
  }

  // remove handle with "data" as its data from within a handle chain
  // (not the entire handle pool).  Returns false if startNode itself
  // needs to be removed (external remove must happen), otherwise 
  // true
  static bool remove(Handle* handles, Handle* startNode, void* data);

  static Handle* getHandle(Handle* handles, handle h)
  {
    return &handles[h - 1];
  }
  
public:
  // clears and initializes handle list overall
  void init();
  uint8_t available();

  // initializes a new handle list
  handle init(void* data);
  // appends a handle to an existing handle list
  handle add(handle handle, void* data);
  void remove(handle handle);
  // clears this and all subsequent handles in the
  // list.  note does not gauruntee any handles preceding
  // this handle in the list are cleared
  void clear(handle handle);
  Handle* getHandle(handle handle) { return getHandle(handles, handle); }
  bool remove(handle startNode, void* data)
  {
    return remove(handles, getHandle(startNode), data);
  }
};

//template <uint8_t NMEMB>
class EventManager : public HandleManager
{
  // NOTE: be sure to not change the memory size from Handle, 
  // so that handles map properly onto HandleManager::Handle
  class Event : Handle
  {
  public:
    eventCallback getCallback() { return (eventCallback) data; }
    handle getNext() { return next; }
  };

  Event* getEvent(handle event) { return (Event*) getHandle(event); }

public:
  void invoke(handle event, void* parameter);
};

extern EventManager eventManager;


class HandleBase
{
protected:
  HandleManager::handle handle = HandleManager::nullHandle;

  void add(HandleManager* manager, void* data);
  void clear(HandleManager* manager)
  {
    manager->clear(handle);
    handle = HandleManager::nullHandle;
  }
  void remove(HandleManager* manager, void* data);
};


// be sure sizeof(T) == sizeof(void*)
template <class T>
class Event : public HandleBase
{
public:
  void add(void (*callback)(T parameter))
  {
    HandleBase::add(&eventManager, (void*)callback);
  }

  Event& operator+=(void (*callback)(T parameter))
  {
    add(callback);
    return *this;
  }
  
  Event& operator-=(void (*callback)(T parameter))
  {
    HandleBase::remove(&eventManager, (void*)callback);
    return *this;
  }

  void invoke(T parameter)
  {
    eventManager.invoke(handle, (void*) parameter);
  }

  Event& operator()(T parameter)
  {
    invoke(parameter);
    return *this;
  }

  void clear() { HandleBase::clear(&eventManager); }
};


// wrapper class to more comfortably expose raw event class without
// exposing invoke() ability
template <class T>
class EventWrapper
{
  Event<T> events;

protected:
  // protected constructor makes re-assigning  more difficult
  // (which is what we want)
  EventWrapper() {}

  void invoke(T parameter)
  {
    events.invoke(parameter);
  }

  EventWrapper operator()(T parameter)
  {
    invoke(parameter);
    return *this;
  }

public:
  operator Event<T>&() const
  {
    return events;
  }

  EventWrapper& operator+=(void (*callback)(T parameter))
  {
    events += callback;
    return *this;
  }
  
  EventWrapper& operator-=(void (*callback)(T parameter))
  {
    events -= callback;
    return *this;
  }

  void clear() { events.clear(); }
};


#define LOCAL_EVENT(T) class Event : public EventWrapper<T*> { friend T; }
//#define DECLARE_EVENT(P) class : public EventWrapper<P*> { friend P; }

template <class T>
class PropertyWithEvents
{
  LOCAL_EVENT(PropertyWithEvents);

  T value;

protected:
  void setValue(T value)
  {
    this->value = value;
#ifdef SERVICE_FEATURE_EVENTS
    updated.invoke(this);
#endif
  }

public:
#ifdef SERVICE_FEATURE_EVENTS
  // fired when state or status message changes
  Event updated;
#endif

  PropertyWithEvents(T value) : value(value) {}
  PropertyWithEvents() {}

  T getValue() { return value; }
  operator T() { return getValue(); }

  PropertyWithEvents<T>& operator = (PropertyWithEvents<T> source)
  {
    setValue(source);
    return *this;
  }


  void addUpdatedEvent(void (*callback)(PropertyWithEvents* parameter))
  {
#ifdef SERVICE_FEATURE_EVENTS
    updated += callback;
#endif
  }
};


class PSTR_Property : public PropertyWithEvents<const __FlashStringHelper*> {};
class STR_Property : public PropertyWithEvents<const char*>
{
public:
  STR_Property() {}
  STR_Property(const char* value) : PropertyWithEvents(value) {}
};

class PubSTR_Property : public STR_Property
{
public:
  PubSTR_Property() {}
  PubSTR_Property(const char* value) : STR_Property(value) {}
  void setValue(const char* value) { STR_Property::setValue(value); }
};
