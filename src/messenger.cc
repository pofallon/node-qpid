#define BUILDING_NODE_EXTENSION
#include <node.h>
#include "messenger.h"

using namespace v8;
using namespace node;

Messenger::Messenger() {};
Messenger::~Messenger() {};

enum async_commands_t {CONNECT};

struct Baton {

  uv_work_t request;
  Persistent<Object> obj;

  int error_code;
  std::string error_message;

  async_commands_t command;
  pn_messenger_t* messenger_;
  char* address_;
  
};

void AsyncWork(uv_work_t* req) {

  Baton* baton = static_cast<Baton*>(req->data);
  
  switch(baton->command) {
    case CONNECT: 
      pn_messenger_start(baton->messenger_);
      pn_messenger_subscribe(baton->messenger_, baton->address_);
      if (pn_messenger_errno(baton->messenger_) > 0) {
        baton->error_code = pn_messenger_errno(baton->messenger_);
        baton->error_message = pn_messenger_error(baton->messenger_);
      } else {
        baton->error_code = 0;
      }
      break;
  }

}

void AsyncAfter(uv_work_t* req) {
  HandleScope scope;
  Baton* baton = static_cast<Baton*>(req->data);

  if (baton->error_code > 0) {
    Local<Value> err = Exception::Error(
      String::New(baton->error_message.c_str()));
    Local<Value> argv[2] = { Local<Value>::New(String::New("error")), err };
    MakeCallback(baton->obj, "emit", 2, argv);
  } else {
    switch(baton->command) {
      case CONNECT:
        Local<Value> argv[1] = { Local<Value>::New(String::New("connected")) };
        MakeCallback(baton->obj, "emit", 1, argv);
        break;
    }
  }

  baton->obj.Dispose();
  delete baton;
}


void Messenger::Init(Handle<Object> target) {
  // Prepare constructor template
  Local<FunctionTemplate> tpl = FunctionTemplate::New(New);
  tpl->SetClassName(String::NewSymbol("Messenger"));
  tpl->InstanceTemplate()->SetInternalFieldCount(2);
  // Prototype
  tpl->PrototypeTemplate()->Set(String::NewSymbol("plusOne"),
      FunctionTemplate::New(PlusOne)->GetFunction());
  /* tpl->PrototypeTemplate()->Set(String::NewSymbol("incoming"),
      FunctionTemplate::New(Incoming)->GetFunction()); */

  Persistent<Function> constructor = Persistent<Function>::New(tpl->GetFunction());
  target->Set(String::NewSymbol("Messenger"), constructor);
}

Handle<Value> Messenger::New(const Arguments& args) {
  HandleScope scope;

  Messenger* obj = new Messenger();
  // obj->counter_ = args[0]->IsUndefined() ? 0 : args[0]->NumberValue();
  obj->counter_ = 0;
  obj->messenger_ = pn_messenger(NULL);
  if (!args[0]->IsUndefined()) {
    obj->address_ = *String::Utf8Value(args[0]->ToString());
    
    Baton* baton = new Baton();
    baton->command = CONNECT;
    baton->obj = Persistent<Object>::New(args.This());
    baton->messenger_ = obj->messenger_;
    baton->address_ = obj->address_;

    baton->request.data = baton;
    uv_queue_work(uv_default_loop(), &baton->request, AsyncWork, AsyncAfter);
  }
  obj->Wrap(args.This());

  return args.This();
}

Handle<Value> Messenger::PlusOne(const Arguments& args) {
  HandleScope scope;

  Messenger* obj = ObjectWrap::Unwrap<Messenger>(args.This());
  obj->counter_ += 1;

  return scope.Close(Number::New(obj->counter_));
}
