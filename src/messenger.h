#ifndef MESSENGER_H
#define MESSENGER_H

#include <string>

#include <node.h>

#include "proton/message.h"
#include "proton/messenger.h"

using namespace v8;
using namespace node;
using namespace std;

enum async_commands_t {CONNECT, SEND};

struct Baton {

  uv_work_t request;
  Persistent<Object> obj;
  Persistent<Function> callback;

  int error_code;
  string error_message;

  async_commands_t command;
  pn_messenger_t* messenger_;
  string address_;

  string msgtext;
  pn_tracker_t tracker;

};

class Messenger : public node::ObjectWrap {
 public:
  static void Init(v8::Handle<v8::Object> target);

 private:
  Messenger();
  ~Messenger();

  static void AsyncWork(uv_work_t* req);
  static void AsyncAfter(uv_work_t* req);

  static v8::Handle<v8::Value> New(const v8::Arguments& args);
  static v8::Handle<v8::Value> Send(const v8::Arguments& args);
  static v8::Handle<v8::Value> PlusOne(const v8::Arguments& args);
  // static v8::Handle<v8::Value> Incoming(const v8::Arguments& args);
  double counter_;
  std::string address_;
  pn_messenger_t * messenger_;
};

#endif
