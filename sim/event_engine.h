#ifndef EVENT_H
#define EVENT_H

#include "inc_all.h"

#include <map>

using namespace std;

struct Event;
class EventEngine;

/*
 * 优先级计算，系统当中有多种事件，为了灵活定义event的优先级，我们
 * 为每一个event定义一个priority value，该value的计算公式如下：
 * PV = (-1024) * cycle + (64) * type + (1) * reserve
 * PV越大，则优先进行处理，原则如下:
 * 1. cycle越小，优先级越大
 * 2. event type根据用户定义优先级处理，例如可保证在每个cycle中，
 * 总是memory on arrive 事件先处理，其次是memory on access事件，
 * 或者，保证Memory事件先处理，CPU事件后处理
 * 3. reserve可以保证，相同cycle，相同event type情况下，由用户自定义
 * 优先级，比如可保证memory on arrive事件，先处理L1, 其次L2，最后L3
 * 4. 如果PV相同，则先注册的先处理
 */

// ranked by priority
enum EventType {
  // memory event type
  MemoryOnAccess,
  MemoryOnArrive,
  
  // CPU event type
  // CPU event priority is the reverse order of the pipeline
  ReorderBufferRetire,
  InstExecution,
  InstIssue,
  InstDispatch,
  InstFetch
};

// base class for callbackdata
struct EventDataBase {};

class EventHandler {
 protected:
  virtual void proc(EventDataBase* data) = 0;
  virtual EventType get_type() = 0;

 public:
  void proc_event(Event *e);
};

struct Event {
  EventType         type;
  EventHandler*     handler;
  EventDataBase*    callbackdata;

  void execute();
};


class EventEngine {
 private:
  multimap<s64, Event*>     _queue;
  s64                       _tick;

 public:
  EventEngine() : _tick(0) {};
  EventEngine(const EventEngine &) = delete;
  EventEngine & operator= (const EventEngine &) = delete;
  EventEngine(EventEngine &&) = delete;

  void register_after_now(Event* e, u32 ticks, u32 priority);
  int loop();
};

#endif
