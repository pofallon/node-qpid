#define BUILDING_NODE_EXTENSION
#include <node.h>
#include <iostream>
#include "messenger.h"

using namespace v8;
using namespace node;
using namespace std;

Messenger::Messenger() {};
Messenger::~Messenger() {};

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
  
};

void AsyncWork(uv_work_t* req) {

  Baton* baton = static_cast<Baton*>(req->data);
  int ret;
  
  switch(baton->command) {

    case CONNECT: 
      ret = pn_messenger_start(baton->messenger_);
      cerr << "AsyncWork (CONNECT):  Started messenger with return value of " << ret << "\n";
      if (pn_messenger_errno(baton->messenger_)) {
        baton->error_code = pn_messenger_errno(baton->messenger_);
        baton->error_message = pn_messenger_error(baton->messenger_);
      } else {
        cerr << "AsyncWork (CONNECT): Subscribing to " << baton->address_ << "\n";
        pn_messenger_subscribe(baton->messenger_, baton->address_.c_str());
        if (pn_messenger_errno(baton->messenger_)) {
          baton->error_code = pn_messenger_errno(baton->messenger_);
          baton->error_message = pn_messenger_error(baton->messenger_);
        } else {
          baton->error_code = 0;
        }
      }
      break;

    case SEND:
      pn_message_t* message = pn_message();
      pn_message_set_address(message, baton->address_.c_str());
      pn_data_t* body = pn_message_body(message);
      pn_data_put_string(body, pn_bytes(baton->msgtext.size(), const_cast<char*>(baton->msgtext.c_str())));
      pn_messenger_put(baton->messenger_, message);
      cerr << "AsyncWork (SEND): Sending message '" << pn_data_get_string(pn_message_body(message)).start << "' to address " << pn_message_get_address(message) << " (" << pn_messenger_outgoing(baton->messenger_) << " outgoing)\n";
      if (pn_messenger_errno(baton->messenger_)) {
        baton->error_code = pn_messenger_errno(baton->messenger_);
        baton->error_message = pn_messenger_error(baton->messenger_);
      } else {
        pn_messenger_send(baton->messenger_);
        if (pn_messenger_errno(baton->messenger_)) {
          baton->error_code = pn_messenger_errno(baton->messenger_);
          baton->error_message = pn_messenger_error(baton->messenger_);
        } else { 
          cerr << "AsyncWork (SEND): Message sent (" << pn_messenger_outgoing(baton->messenger_) << " outgoing)\n";
          baton->error_code = 0;
          // pn_messenger_recv(baton->messenger_,1);
          // pn_message_t* testmsg = pn_message();
          // pn_messenger_get(baton->messenger_,testmsg);
          // cerr << "AsyncWork (SEND):  Test recv/get body = '" << pn_data_get_string(pn_message_body(testmsg)).start << "'\n";
        }
      }
      break;
  }

}

void AsyncAfter(uv_work_t* req) {
  HandleScope scope;
  Baton* baton = static_cast<Baton*>(req->data);

  switch(baton->command) {
    case CONNECT:
      if (baton->error_code > 0) {
        Local<Value> err = Exception::Error(
        String::New(baton->error_message.c_str()));
        Local<Value> argv[2] = { Local<Value>::New(String::New("error")), err };
        MakeCallback(baton->obj, "emit", 2, argv);
      } else {
        cerr << "AsyncAfter (CONNECT): Emitting 'connected' event\n";
        Local<Value> argv[1] = { Local<Value>::New(String::New("connected")) };
        MakeCallback(baton->obj, "emit", 1, argv);
      }
      baton->obj.Dispose();
      break;

    case SEND:
      if (baton->error_code > 0) {
        Local<Value> err = Exception::Error(String::New(baton->error_message.c_str()));
        Local<Value> argv[] = { err };
        baton->callback->Call(Context::GetCurrent()->Global(), 1, argv);
      } else {
        cerr << "AsyncAfter (SEND): Invoking callback on success\n";
        Local<Value> argv[] = {};
        baton->callback->Call(Context::GetCurrent()->Global(), 0, argv);
      }
      baton->callback.Dispose();
      break;
     
  }

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
  tpl->PrototypeTemplate()->Set(String::NewSymbol("send"),
      FunctionTemplate::New(Send)->GetFunction());

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
    obj->address_ = *String::AsciiValue(args[0]->ToString());
    
    Baton* baton = new Baton();
    baton->command = CONNECT;
    baton->obj = Persistent<Object>::New(args.This());
    baton->messenger_ = obj->messenger_;
    baton->address_ = obj->address_;

    cerr << "Messenger::New: Connecting to " << baton->address_ << "\n";

    baton->request.data = baton;
    uv_queue_work(uv_default_loop(), &baton->request, AsyncWork, AsyncAfter);
  }
  obj->Wrap(args.This());

  return args.This();
}

Handle<Value> Messenger::Send(const Arguments& args) {
  HandleScope scope;
  
  Messenger* obj = ObjectWrap::Unwrap<Messenger>(args.This());

  if (!args[0]->IsUndefined()) {
    
    Baton* baton = new Baton();
    baton->command = SEND;
    baton->messenger_ = obj->messenger_;
    baton->address_ = obj->address_;
    baton->msgtext = *String::Utf8Value(args[0]->ToString());
    
    if (args[1]->IsFunction()) {

      Local<Function> callback = Local<Function>::Cast(args[1]);
      baton->callback = Persistent<Function>::New(callback);

    }

    baton->request.data = baton;
    uv_queue_work(uv_default_loop(), &baton->request, AsyncWork, AsyncAfter);

  }

  return Undefined();
    
}

Handle<Value> Messenger::PlusOne(const Arguments& args) {
  HandleScope scope;

  Messenger* obj = ObjectWrap::Unwrap<Messenger>(args.This());
  obj->counter_ += 1;

  return scope.Close(Number::New(obj->counter_));
}
