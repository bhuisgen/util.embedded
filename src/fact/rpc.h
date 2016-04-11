#pragma once

// NOTE: function references are available, but instance-member references are not
// (only instance-member pointers)
// I decided to use function references *when we can* to make consuming code more
// readable.  For why C++ doesn't have instance-member references:
//
// http://stackoverflow.com/questions/21952386/why-doesnt-reference-to-member-exist-in-c
//
// note also: TOut usage may not be compatible with ESP8266 compiler
#include "lib.h"
#include "../Console.h"

#include <cstdint>
#include <typeinfo>

namespace FactUtilEmbedded
{
  namespace rpc
  {
    template <class... Ts> struct tuple {};

    template <class T, class... Ts>
    struct tuple<T, Ts...> : tuple<Ts...> {
      tuple(T t, Ts... ts) : tuple<Ts...>(ts...), tail(t) {}

      T tail;
    };
    
template <class... TIn>
struct ParameterTuple {};


template <class T, class... TIn>
struct ParameterTuple<T, TIn...> : ParameterTuple<TIn...>
{
//public:
  ParameterTuple(T t, TIn... tin) : ParameterTuple<TIn...>(tin...), tail(t) {}
  
  //void debugPrint() {}
  
  T tail;

  void assign(T t, TIn... tin)
  {
    tail = t;
    assign(tin...);
  }

  void test(int val1, int val2) {}
  void test(int val1) {}

  // the stub works - cool!
  typedef void (*stub)(T, TIn...);
  
  void invoke(stub func)
  {
    // somehow this is doable...
    
    //test(std::forward<TIn>()...);
    //func(tail, std::forward<TIn>()...);
  }
};



template <size_t, class> struct elem_type_holder;

template <class T, class... Ts>
struct elem_type_holder<0, ParameterTuple<T, Ts...>> {
  typedef T type;
};


template <size_t k, class T, class... Ts>
struct elem_type_holder<k, ParameterTuple<T, Ts...>> {
  typedef typename elem_type_holder<k - 1, ParameterTuple<Ts...>>::type type;
};


template <size_t k, class... Ts>
typename std::enable_if<
    k == 0, typename elem_type_holder<0, ParameterTuple<Ts...>>::type&>::type
get(ParameterTuple<Ts...>& t) {
  return t.tail;
}

template <size_t k, class T, class... Ts>
typename std::enable_if<
    k != 0, typename elem_type_holder<k, ParameterTuple<T, Ts...>>::type&>::type
get(ParameterTuple<T, Ts...>& t) {
  ParameterTuple<Ts...>& base = t;
  return get<k - 1>(base);
}

#ifdef UNUSED
template <class TIn1>
class ParameterTuple<TIn1> : public ParameterTuple<Tin...>
{
public:
  template <class TOut> using stub_func = TOut (*)(TIn1);
  //template <class TOut> using stub_method = TOut (TIn1::*)();

  template <class TOut> 
  TOut invoke(stub_func<TOut> func)
  {
    return func(get<0>(*this));
  }

  /*
  template <class TOut> 
  TOut invoke(stub_method<TOut> func)
  {
    return ((*(get<0>(*this))).*func)();
  } */
};


template <class TIn1, class TIn2>
class ParameterTuple<TIn1, TIn2>
{
public:
  template <class TOut> using stub_func = TOut (*)(TIn1, TIn2);
  //template <class TOut> using stub_method = TOut (TIn1::*)(TIn2);

  template <class TOut> 
  TOut invoke(stub_func<TOut> func)
  {
    return func(get<0>(*this), get<1>(*this));
  }

  /*
  template <class TOut> 
  TOut invoke(stub_method<TOut> func)
  {
    return ((*(get<0>(*this))).*func)(get<1>(*this));
  } */
};


template <class TIn1, class TIn2, class TIn3>
class ParameterTuple<TIn1, TIn2, TIn3>
{
public:
  template <class TOut> using stub_func = TOut (*)(TIn1, TIn2, TIn3);
  //template <class TOut> using stub_method = TOut (TIn1::*)(TIn2, TIn3);

  template <class TOut> 
  TOut invoke(stub_func<TOut> func)
  {
    return func(get<0>(*this), get<1>(*this), get<2>(*this));
  }

  /*
  template <class TOut> 
  TOut invoke(stub_method<TOut> func)
  {
    return ((*(get<0>(*this))).*func)(get<1>(*this), get<2>(*this));
  } */
};
#endif


class ParameterClass_0
{
public:
  typedef void (&stub)();

  template <class TOut>
  TOut invoke(TOut (&func)()) const
  {
    return func();
  }

/*
  template <class TOut>
  TOut invoke(TOut (*func)()) const
  {
    return func();
  } */

  void debugPrint() const {}
};

template <class TIn>
class ParameterClass_1
{
protected:
public: // FIX: temporarily making these public as we iron out architecture & POC
  TIn param1;

  typedef void (&stub)(TIn);

public:
  ParameterClass_1(const TIn& param1) : param1(param1) {}
  ParameterClass_1() {}

  void invoke(stub func) { func(param1); }

  void invokeExp(void (*func)(TIn)) { func(param1); }

  template <class TOut>
  TOut invoke(TOut (&func)(TIn)) const
  {
    return func(param1);
  }

  // TClass and TIn1 must always match
  template <class TClass, class TOut>
  TOut invoke(TOut (TClass::*func)())
  {
    return ((*param1).*func)();
  }

  void debugPrint() const
  {
    cout << F("p1: ") << param1;
    cout.println();
  }

  template <class TOut> using stub_func = TOut (&)(TIn);
  template <class TOut> using stub_method = TOut (&)(TIn);
};

template <class TIn1, class TIn2>
class ParameterClass_2 : public ParameterClass_1<TIn1>
{
protected:
public: // FIX: temporarily making these public as we iron out architecture & POC
  TIn2 param2;

public:
  ParameterClass_2() {}
  ParameterClass_2(const TIn1& in1, const TIn2& in2) : ParameterClass_1<TIn1>(in1), param2(in2)
  {}

  template <class TOut>
  TOut invoke(TOut (&func)(TIn1, TIn2)) const
  {
    return func(ParameterClass_1<TIn1>::param1, param2);
  }


  // TClass and TIn1 must always match
  template <class TClass, class TOut>
  TOut invoke(TOut (TClass::*func)(TIn2))
  {
    return ((*ParameterClass_1<TIn1>::param1).*func)(param2);
  }

  void debugPrint() const
  {
    ParameterClass_1<TIn1>::debugPrint();
    cout << F("p2: ") << param2;
    cout.println();
  }

  typedef void (&stub)(TIn1, TIn2);
  
  template <class TOut> using stub_func = TOut (&)(TIn1, TIn2);
  template <class TOut> using stub_method = TOut (&)(TIn1, TIn2);
};

template <class TIn1, class TIn2, class TIn3>
class ParameterClass_3 : public ParameterClass_2<TIn1, TIn2>
{
protected:
public: // FIX: temporarily making these public as we iron out architecture & POC
  TIn3 param3;

public:
  template <class TOut>
  TOut invoke(TOut (&func)(TIn1, TIn2, TIn3)) const
  {
    return func(ParameterClass_1<TIn1>::param1, ParameterClass_2<TIn1, TIn2>::param2, param3);
  }
};


template <class TOut>
class ParameterReturnClass
{
  TOut returnValue;
};

class IInvoker
{
public:
  /* this doesn't work
  template <class TOut>
  virtual TOut invoke() = 0; */

  virtual void invoke() = 0;

#ifdef DEBUG
  virtual void debugPrint() const = 0;
#endif

  void operator()()
  {
    invoke();
  }
};


class CallHolderFactory;

template <class TParameters, class TFunc>
class CallHolder : public IInvoker
{
  friend CallHolderFactory;

protected:
  //typedef TParameters::
  const TFunc func;
  TParameters parameters;

public:
  CallHolder(TFunc func) : func(func) {}

  virtual void invoke() override
  {
    parameters.invoke(func);
  }

#ifdef DEBUG
  virtual void debugPrint() const override
  {
    parameters.debugPrint();
#ifdef UNIT_TEST
    //cout << F("Func ptr: ") << (uint32_t)(long)func;
#endif
    cout.println();
  }
#endif
};


template <class TParameters, class TOut>
class StubGen
{
public:
  typedef typename TParameters::template stub_func <TOut> stub;
};

template <class TParameters, class TOut>
class CallHolderFunction
  //: public CallHolder<TParameters, typename StubGen<TParameters, TOut>::stub>
  : public CallHolder<TParameters, typename TParameters::template stub_func <TOut>>
  //  CallHolderFunction<TParameters, TOut>::stub>
{
public:
  //TParameters::stub_func<TOut>* getStuff() { return nullptr; }
  typedef typename TParameters::template stub_func <TOut> stub;
  //typedef typename StubGen<TParameters, TOut>::stub stub;

  CallHolderFunction(stub func) : CallHolder<TParameters, stub>(func) {}
};


template <class TParameters, class TOut>
class CallHolderMethod
  : public CallHolder<TParameters, typename TParameters::template stub_method <TOut>>
{
public:
  typedef typename TParameters::template stub_method <TOut> stub;

  CallHolderMethod(stub func) : CallHolder<TParameters, stub>(func) {}
};


/*
template <class... TIn>
class CallHolder2 {  };

template <class T, class... TIn>
class CallHolder2<T, TIn...> : public CallHolder2<TIn...>
{
public:
  typedef ParameterTuple<T, TIn...> ParameterClass;

  //typedef typename ParameterClass::template stub_method <TOut> stub_method;
  //typedef typename ParameterClass::template stub_func <TOut> stub_func;
};
*/

class CallHolderFactory
{
public:
  // ditching template class TFunc trick because:
  // a) it doesn't gracefully resolve func ref vs func ptr
  // b) it forces TIn resolution to hang primarily off of create parameters
  //    vs actual function parameters, and we'd prefer function parameters

/*
  template <class TFunc, class TIn1>
  static CallHolder<ParameterClass_1<TIn1>, TFunc> _create(TFunc func, TIn1 in1)
  {
    auto m = createCallHolder(func);
    m.parameters.param1 = in1;
    return m;
  }*/
  
  //template <class TOut, class... TIn>
  //static CallHolder

/*
  template <class TOut, class... TIn>
  //typename TParameters::template stub_func <TOut> stub
  static CallHolderFunction<ParameterTuple<TIn...>, TOut>* create2_exp(void* mem, 
    typename ParameterTuple<TIn...>::template stub_func <TOut> func,
    //stub func, 
    TIn... tin)
  {
    auto ch = new (mem) CallHolderFunction<ParameterTuple<TIn...>, TOut>(func);
    ch->assign(tin...);
    return ch;
  }*/


  template <class TOut, class... TIn>
  //typename TParameters::template stub_func <TOut> stub
  static CallHolder<ParameterTuple<TIn...>, 
    typename ParameterTuple<TIn...>::template stub_func <TOut>>* create3(void* mem, 
    typename ParameterTuple<TIn...>::template stub_func <TOut> func,
    //stub func, 
    TIn... tin)
  {
    auto ch = new (mem) CallHolder<ParameterTuple<TIn...>, 
      typename ParameterTuple<TIn...>::template stub_func <TOut>>(func);
    ch->parameters.assign(tin...);
    return ch;
  }


  template <class TOut>
  static CallHolder<ParameterClass_0, TOut (&)()> create(TOut (&func)())
  {
    CallHolder<ParameterClass_0, TOut (&)()> m(func);
    return m;
  }

  template <class TOut>
  static CallHolder<ParameterClass_0, TOut (&)()>& createInPlace(void* mem, TOut (&func)())
  {
    return *(new (mem) CallHolder<ParameterClass_0, TOut (&)()>(func));
  }

/*
  template <class TOut>
  static CallHolderFunction<ParameterClass_0, TOut>& createInPlace(void* mem, TOut (&func)())
  {
    return *(new (mem) CallHolderFunction<ParameterClass_0, TOut>(func));
  }*/

  template <class TOut, class TIn1>
  static CallHolder<ParameterClass_1<TIn1>, TOut (&)(TIn1)> create(TOut (&func)(TIn1), TIn1 in1)
  {
    CallHolder<ParameterClass_1<TIn1>, TOut (&)(TIn1)> m(func);

    m.parameters.param1 = in1;
    return m;
  }
/*
  template <class TOut, class TIn1>
  static CallHolder<ParameterClass_1<TIn1>, TOut (&)(TIn1)>& createInPlace(void* mem, TOut (&func)(TIn1), TIn1 in1)
  {
    auto m = new (mem) CallHolder<ParameterClass_1<TIn1>, TOut (&)(TIn1)>(func);

    m->parameters.param1 = in1;
    return *m;
  }*/

  template <class TOut, class TIn1>
  static CallHolderFunction<ParameterClass_1<TIn1>, TOut>& createInPlace(void* mem, TOut (&func)(TIn1), TIn1 in1)
  {
    auto m = new (mem) CallHolderFunction<ParameterClass_1<TIn1>, TOut>(func);

    m->parameters.param1 = in1;
    return *m;
  }

  template <class TOut, class TClass>
  static CallHolder<ParameterClass_1<TClass*>, TOut (TClass::*)()> create(TOut (TClass::*func)(), TClass* in1)
  {
    CallHolder<ParameterClass_1<TClass*>, TOut (TClass::*)()> m(func);

    m.parameters.param1 = in1;
    return m;
  }


  template <class TOut, class TClass>
  static CallHolder<ParameterClass_1<TClass*>, TOut (TClass::*)()> createInPlace(void* mem, TOut (TClass::*func)(), TClass* in1)
  {
    auto m = new (mem) CallHolder<ParameterClass_1<TClass*>, TOut (TClass::*)()>(func);

    m->parameters.param1 = in1;
    return m;
  }

  template <class TOut, class TIn1, class TIn2>
  static CallHolder<ParameterClass_2<TIn1, TIn2>, TOut (&)(TIn1, TIn2)>
    create(TOut (&func)(TIn1, TIn2), TIn1 in1, TIn2 in2)
  {
    CallHolder<ParameterClass_2<TIn1, TIn2>, TOut (&)(TIn1, TIn2)> m(func);

    m.parameters.param1 = in1;
    m.parameters.param2 = in2;
    return m;
  }

  template <class TOut, class TClass, class TIn2>
  static CallHolder<ParameterClass_2<TClass*, TIn2>, TOut (TClass::*)(TIn2)>
    create(TOut (TClass::*func)(TIn2), TClass* in1, TIn2 in2)
  {
    CallHolder<ParameterClass_2<TClass*, TIn2>, TOut (TClass::*)(TIn2)> m(func);

    m.parameters.param1 = in1;
    m.parameters.param2 = in2;
    return m;
  }
};

// NOTE: we probably want elementSize of 2 or 4 byte increments for alignment issues
template <uint8_t elementSize, uint8_t bufferSize>
class CallQueue
{
#ifdef UNIT_TEST
public:
#endif
  union Element
  {
    uint8_t buffer[elementSize];
  };

  Element elements[bufferSize];
  CircularBuffer<Element> queue;

public:
  CallQueue() : queue(elements, bufferSize) {}

  inline void put(IInvoker& invoker, size_t size)
  {
    // TODO: put size sanity check here to make sure we don't go over
    // size limit
    queue.put((Element*)&invoker, size);
  }

  inline IInvoker& get()
  {
    const Element& invoker = queue.get();

    return (IInvoker&)invoker;
  }

  inline uint8_t available() { return queue.available(); }
};
}
}
