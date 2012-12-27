#ifndef MESSENGER_H
#define MESSENGER_H

#include <string>

#include <node.h>

extern "C" {
  #include "proton/message.h"
  #include "proton/messenger.h"
}

class Messenger : public node::ObjectWrap {
 public:
  static void Init(v8::Handle<v8::Object> target);

 private:
  Messenger();
  ~Messenger();

  static v8::Handle<v8::Value> New(const v8::Arguments& args);
  static v8::Handle<v8::Value> PlusOne(const v8::Arguments& args);
  // static v8::Handle<v8::Value> Incoming(const v8::Arguments& args);
  double counter_;
  char * address_;
  pn_messenger_t * messenger_;
};

#endif
