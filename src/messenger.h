#ifndef MESSENGER_H
#define MESSENGER_H

#include <string>
#include <vector>

#include <node.h>

#include "proton/message.h"
#include "proton/messenger.h"

#include "macros.h"
#include "threading.h"

using namespace v8;
using namespace node;

typedef std::vector<pn_message_t*> Messages;

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

  struct Async;

  struct ReceiveBaton : Baton {

    Async* async;

    ReceiveBaton(Messenger* msgr_, Handle<Function> cb_) : 
      Baton(msgr_, cb_) {} 

  };

  struct Async {
    uv_async_t watcher;
    Messenger* msgr;
    Messages data;
    NODE_CPROTON_MUTEX_t;
    bool completed;
    int retrieved;

    // Store the emitter here because we don't have
    // access to the baton in the async callback.
    Persistent<Function> emitter;

    Async(Messenger* m, uv_async_cb async_cb) :
            msgr(m), completed(false), retrieved(0) {
        watcher.data = this;
        NODE_CPROTON_MUTEX_INIT
        msgr->Ref();
        uv_async_init(uv_default_loop(), &watcher, async_cb);
    }

    ~Async() {
        msgr->Unref();
        emitter.Dispose();
        NODE_CPROTON_MUTEX_DESTROY
    }
  };


 private:
  Messenger();
  ~Messenger();

  WORK_DEFINITION(Send)
  WORK_DEFINITION(Subscribe)
  WORK_DEFINITION(Stop)
  WORK_DEFINITION(Put)
  WORK_DEFINITION(Receive)

  static void AsyncReceive(uv_async_t* handle, int status);
  static void CloseEmitter(uv_handle_t* handle);
  static Local<Object> MessageToJS(pn_message_t* message);

  static Handle<Value> New(const Arguments& args);
  std::string address;
  pn_messenger_t * messenger;
  pn_messenger_t * receiver;
  bool receiving;
  bool receiveWait;
  ReceiveBaton * receiveWaitBaton;
  int subscriptions;
};

#endif
