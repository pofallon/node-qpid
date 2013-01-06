#ifndef MESSENGER_H
#define MESSENGER_H

#include <string>

#include <node.h>

#include "proton/message.h"
#include "proton/messenger.h"

#include "macros.h"

using namespace v8;
using namespace node;

class Messenger : public node::ObjectWrap {
 public:
  static Persistent<FunctionTemplate> constructor_template;
  static void Init(Handle<Object> target);

  struct Baton {

    uv_work_t request;
    Messenger* msgr;
    Persistent<Function> callback;

    int error_code;
    std::string error_message;

    // pn_messenger_t* messenger_;

    Baton(Messenger* msgr_, Handle<Function> cb_) :
        msgr(msgr_) {
      msgr->Ref();
      request.data = this;
      callback = Persistent<Function>::New(cb_);
      error_code = 0;
    }   

    virtual ~Baton() {
      msgr->Unref();
      callback.Dispose();
    }

  };

  struct SubscribeBaton : Baton {

    std::string address;

    SubscribeBaton(Messenger* msgr_, Handle<Function> cb_, const char* address_) :
      Baton(msgr_, cb_), address(address_) {}

  };

  struct SendBaton : Baton {

    std::string msgtext;
    pn_tracker_t tracker;

    SendBaton(Messenger* msgr_, Handle<Function> cb_, const char* msgtext_) :
      Baton(msgr_, cb_), msgtext(msgtext_) {}

  };

 private:
  Messenger();
  ~Messenger();

  WORK_DEFINITION(Send)
  WORK_DEFINITION(Subscribe)
  WORK_DEFINITION(Start)
  WORK_DEFINITION(Stop)
  WORK_DEFINITION(Receive)
  WORK_DEFINITION(Get)
  WORK_DEFINITION(Put)

  static Handle<Value> New(const Arguments& args);
  std::string address;
  pn_messenger_t * messenger;
};

#endif
