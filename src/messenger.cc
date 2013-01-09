#define BUILDING_NODE_EXTENSION
#include <node.h>
#include <iostream>
#include "messenger.h"

using namespace v8;
using namespace node;
using namespace std;

Messenger::Messenger() { };

Messenger::~Messenger() { };

Persistent<FunctionTemplate> Messenger::constructor_template;

void Messenger::Init(Handle<Object> target) {
  HandleScope scope;

  Local<FunctionTemplate> t = FunctionTemplate::New(New);

  constructor_template = Persistent<FunctionTemplate>::New(t);
  constructor_template->InstanceTemplate()->SetInternalFieldCount(1);
  constructor_template->SetClassName(String::NewSymbol("Messenger"));

  NODE_SET_PROTOTYPE_METHOD(constructor_template, "send", Send);
  NODE_SET_PROTOTYPE_METHOD(constructor_template, "subscribe", Subscribe);

  target->Set(String::NewSymbol("Messenger"),
    constructor_template->GetFunction());
 
}

Handle<Value> Messenger::New(const Arguments& args) {
  HandleScope scope;

  Messenger* msgr = new Messenger();
  msgr->messenger = pn_messenger(NULL);

  // Does this work?
  // (not like this, no)
  if (!args[0]->IsUndefined()) {
    Subscribe(args);
  }

  msgr->Wrap(args.This());

  return args.This();
}

Handle<Value> Messenger::Subscribe(const Arguments& args) {
  HandleScope scope;

  Messenger* msgr = ObjectWrap::Unwrap<Messenger>(args.This());

  REQUIRE_ARGUMENT_STRING(0, address);
  OPTIONAL_ARGUMENT_FUNCTION(1, callback);

  // This assumes that a messenger can only subscribe to one address at a time
  // (which may not be true)
  msgr->address = *address;

  SubscribeBaton* baton = new SubscribeBaton(msgr, callback, *address);
  
  cerr << "Messenger::Subscribe: Subscribing to " << baton->address << "\n";

  Work_BeginSubscribe(baton);

  return args.This();
  
}

void Messenger::Work_BeginSubscribe(Baton* baton) {
  int status = uv_queue_work(uv_default_loop(),
    &baton->request, Work_Subscribe, Work_AfterSubscribe);

  assert(status == 0);

}

void Messenger::Work_Subscribe(uv_work_t* req) {

  SubscribeBaton* baton = static_cast<SubscribeBaton*>(req->data);

  cerr << "Work_Subscribe: Subscribing to " << baton->address << "\n";
  pn_messenger_subscribe(baton->msgr->messenger, baton->address.c_str());

}

void Messenger::Work_AfterSubscribe(uv_work_t* req) {

  SubscribeBaton* baton = static_cast<SubscribeBaton*>(req->data);

  if (baton->error_code > 0) {
    /* Local<Value> err = Exception::Error(
    String::New(baton->error_message.c_str()));
    Local<Value> argv[2] = { Local<Value>::New(String::New("error")), err };
    MakeCallback(baton->obj, "emit", 2, argv); */
  } else {
    cerr << "Work_AfterSubscribe: Emitting 'subscribed' event\n";
    Local<Value> args[] = { String::New("subscribed") };
    // MakeCallback(baton->msgr, "emit", 1, argv);
    EMIT_EVENT(baton->msgr->handle_, 1, args);
  }

  delete baton;

}

Handle<Value> Messenger::Send(const Arguments& args) {
  HandleScope scope;
  
  Messenger* msgr = ObjectWrap::Unwrap<Messenger>(args.This());

  REQUIRE_ARGUMENT_STRING(0, msg);
  OPTIONAL_ARGUMENT_FUNCTION(1, callback);

  SendBaton* baton = new SendBaton(msgr, callback, *msg);
  
  Work_BeginSend(baton);

  return Undefined();
    
}

void Messenger::Work_BeginSend(Baton* baton) {
  int status = uv_queue_work(uv_default_loop(),
    &baton->request, Work_Send, Work_AfterSend);

  assert(status == 0);

}

void Messenger::Work_Send(uv_work_t* req) {

  int ret = 0;
  SendBaton* baton = static_cast<SendBaton*>(req->data);
  pn_messenger_t* messenger = baton->msgr->messenger;

  pn_message_t* message = pn_message();

  // For now, it defaults to sending a message to the (one) subscribed address
  pn_message_set_address(message, baton->msgr->address.c_str());
  pn_data_t* body = pn_message_body(message);

  pn_data_put_string(body, pn_bytes(baton->msgtext.size(), const_cast<char*>(baton->msgtext.c_str())));

  assert(!pn_messenger_put(messenger, message));
  baton->tracker = pn_messenger_outgoing_tracker(messenger);
  cerr << "Work_Send: Put message '" << pn_data_get_string(pn_message_body(message)).start << "' (return value: " << ret << ", tracker: " << baton->tracker << ", status: " << pn_messenger_status(messenger,baton->tracker) << ", outgoing: " << pn_messenger_outgoing(messenger) << ")\n";
  
  assert(!pn_messenger_start(messenger));

  assert(!pn_messenger_send(messenger));
  cerr << "Work_Send: Sent message (return value: " << ret << ", tracker: " << baton->tracker << ", status: " << pn_messenger_status(messenger,baton->tracker) << ", outgoing: " << pn_messenger_outgoing(messenger) << "\n";
  
  pn_messenger_stop(messenger);

  pn_message_free(message);

  // Where to put this?!?
  // pn_messenger_free(messenger);
}

void Messenger::Work_AfterSend(uv_work_t* req) {
  HandleScope scope;
  SendBaton* baton = static_cast<SendBaton*>(req->data);

  if (baton->error_code > 0) {
    Local<Value> err = Exception::Error(String::New(baton->error_message.c_str()));
    Local<Value> argv[] = { err };
    baton->callback->Call(Context::GetCurrent()->Global(), 1, argv);
  } else {
    cerr << "Work_AfterSend: Invoking callback on success (tracker: " << baton->tracker << ")\n";
    Local<Value> argv[] = {};
    baton->callback->Call(Context::GetCurrent()->Global(), 0, argv);
  }

  delete baton;

}
