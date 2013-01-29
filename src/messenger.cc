#define BUILDING_NODE_EXTENSION
#include <node.h>
#include <iostream>
#include <vector>
#include "messenger.h"
#include "async.h"


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
  NODE_SET_PROTOTYPE_METHOD(constructor_template, "receive", Receive);
  NODE_SET_PROTOTYPE_METHOD(constructor_template, "stop", Stop);

  target->Set(String::NewSymbol("Messenger"),
    constructor_template->GetFunction());
 
}

Handle<Value> Messenger::New(const Arguments& args) {
  HandleScope scope;

  Messenger* msgr = new Messenger();
  msgr->messenger = pn_messenger(NULL);

  // Temporary fix 
  pn_messenger_set_outgoing_window(msgr->messenger, 1);

  msgr->receiver = pn_messenger(NULL);

  // How long to block while receiving.  Should surface this as an option
  pn_messenger_set_timeout(msgr->receiver, 50);

  msgr->receiving = false;
  msgr->receiveWait = false;
  msgr->subscriptions = 0;

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
  
  // cerr << "Messenger::Subscribe: Subscribing to " << baton->address << "\n";

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

  // cerr << "Work_Subscribe: Subscribing to " << baton->address << "\n";
  pn_messenger_subscribe(baton->msgr->receiver, baton->address.c_str());

}

void Messenger::Work_AfterSubscribe(uv_work_t* req) {

  SubscribeBaton* baton = static_cast<SubscribeBaton*>(req->data);

  baton->msgr->subscriptions++;

  if (baton->error_code > 0) {
    /* Local<Value> err = Exception::Error(
    String::New(baton->error_message.c_str()));
    Local<Value> argv[2] = { Local<Value>::New(String::New("error")), err };
    MakeCallback(baton->obj, "emit", 2, argv); */
  } else {
    // cerr << "Work_AfterSubscribe: Emitting 'subscribed' event\n";
    Local<Value> args[] = { String::New("subscribed"), String::New(baton->address.c_str()) };
    EMIT_EVENT(baton->msgr->handle_, 2, args);
  }

  if (baton->msgr->receiveWait) {

    // cerr << "Receive Wait is TRUE\n";
 
    Work_BeginReceive(baton->msgr->receiveWaitBaton);

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

  SendBaton* baton = static_cast<SendBaton*>(req->data);
  pn_messenger_t* messenger = baton->msgr->messenger;

  pn_message_t* message = pn_message();

  // For now, it defaults to sending a message to the (one) subscribed address
  pn_message_set_address(message, baton->msgr->address.c_str());
  pn_data_t* body = pn_message_body(message);

  pn_data_put_string(body, pn_bytes(baton->msgtext.size(), const_cast<char*>(baton->msgtext.c_str())));

  assert(!pn_messenger_put(messenger, message));
  baton->tracker = pn_messenger_outgoing_tracker(messenger);
  // cerr << "Work_Send: Put message '" << pn_data_get_string(pn_message_body(message)).start << "', tracker: " << baton->tracker << ", status: " << pn_messenger_status(messenger,baton->tracker) << ", outgoing: " << pn_messenger_outgoing(messenger) << ")\n";
  
  assert(!pn_messenger_start(messenger));

  assert(!pn_messenger_send(messenger));
  // cerr << "Work_Send: Sent message (tracker: " << baton->tracker << ", status: " << pn_messenger_status(messenger,baton->tracker) << ", outgoing: " << pn_messenger_outgoing(messenger) << "\n";
  
  // pn_messenger_stop(messenger);

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
    // cerr << "Work_AfterSend: Invoking callback on success (tracker: " << baton->tracker << ")\n";
    Local<Value> argv[] = {};
    baton->callback->Call(Context::GetCurrent()->Global(), 0, argv);
  }

  delete baton;

}

Handle<Value> Messenger::Receive(const Arguments& args) {
  HandleScope scope;

  Messenger* msgr = ObjectWrap::Unwrap<Messenger>(args.This());

  // cerr << "Messenger::Receive: About to check receiving (which is " << msgr->receiving << ")\n";

  if (!msgr->receiving) {

    Local<Function> emitter = Local<Function>::Cast((msgr->handle_)->Get(String::NewSymbol("emit")));
    ReceiveBaton* baton = new ReceiveBaton(msgr, emitter);

    if (msgr->subscriptions > 0) {

      // cerr << "Messenger::Receive: About to BeginReceive\n";
      Work_BeginReceive(baton);

    } else {

      msgr->receiveWait = true;
      msgr->receiveWaitBaton = baton;

    }

  }

  return Undefined();

}

void Messenger::Work_BeginReceive(Baton *baton) {

  ReceiveBaton* receive_baton = static_cast<ReceiveBaton*>(baton);

  receive_baton->async = new Async(receive_baton->msgr, AsyncReceive);
  receive_baton->async->emitter = Persistent<Function>::New(receive_baton->callback);

  receive_baton->msgr->receiving = true;

  // cerr << "Receive Wait is FALSE\n";
  receive_baton->msgr->receiveWait = false;

  // cerr << "Work_BeginReceive: About to uv_queue_work\n";
  
  int status = uv_queue_work(uv_default_loop(),
    &baton->request, Work_Receive, Work_AfterReceive);

  assert(status == 0);

} 

void Messenger::CloseEmitter(uv_handle_t* handle) {

  assert(handle != NULL);
  assert(handle->data != NULL);
  Async* async = static_cast<Async*>(handle->data);
  delete async;
  handle->data = NULL;

}

void Messenger::Work_Receive(uv_work_t* req) {

  ReceiveBaton* baton = static_cast<ReceiveBaton*>(req->data);
  pn_messenger_t* receiver = baton->msgr->receiver;
  Async* async = baton->async;

  while (baton->msgr->receiving) {

    // cerr << "Work_Receive: About to block on recv\n";

    pn_messenger_recv(receiver, 1024);

    // cerr << "Work_Receive: Leaving blocking recv\n";

    while(pn_messenger_incoming(receiver)) {

      // cerr << "Work_Receive: Iterating over incoming messages\n";

      pn_message_t* message = pn_message();
      pn_messenger_get(receiver, message);

      NODE_CPROTON_MUTEX_LOCK(&async->mutex)
      async->data.push_back(message);
      NODE_CPROTON_MUTEX_UNLOCK(&async->mutex)

      uv_async_send(&async->watcher);

    } 

  }

  async->completed = true;
  uv_async_send(&async->watcher);

}

void Messenger::AsyncReceive(uv_async_t* handle, int status) {
  HandleScope scope;
  Async* async = static_cast<Async*>(handle->data);

  // cerr << "Messenger::AsyncReceive: entering while loop\n";

  while (true) {

    Messages messages;
    NODE_CPROTON_MUTEX_LOCK(&async->mutex)
    messages.swap(async->data);
    NODE_CPROTON_MUTEX_UNLOCK(&async->mutex)

    if (messages.empty()) {
      break;
    }

    Local<Value> argv[2];

    Messages::const_iterator it = messages.begin();
    Messages::const_iterator end = messages.end();
    for (int i = 0; it < end; it++, i++) {

      // cerr << "Messenger::AsyncReceive: iterating over vector\n";

      argv[0] = String::NewSymbol("message");
      argv[1] = MessageToJS(*it);
      TRY_CATCH_CALL(async->msgr->handle_, async->emitter, 2, argv)
      pn_message_free(*it);
      // delete *it;

    }

  }

  if (async->completed) {
    uv_close((uv_handle_t*)handle, CloseEmitter);
  }

}

void Messenger::Work_AfterReceive(uv_work_t* req) {

  HandleScope scope;
  SendBaton* baton = static_cast<SendBaton*>(req->data);

  // cerr << "Work_AfterReceive:  cleaning up\n";

  delete baton;

}

Handle<Value> Messenger::Stop(const Arguments& args) {
  HandleScope scope;

  OPTIONAL_ARGUMENT_FUNCTION(0, callback);

  Messenger* msgr = ObjectWrap::Unwrap<Messenger>(args.This());

  Baton* baton = new Baton(msgr, callback);

  if (baton->msgr->receiving || baton->msgr->receiveWait) {

    baton->msgr->receiving = false;
    baton->msgr->receiveWait = false;

    Work_BeginStop(baton);

  }

  return Undefined();

}

void Messenger::Work_BeginStop(Baton *baton) {

  int status = uv_queue_work(uv_default_loop(),
    &baton->request, Work_Stop, Work_AfterStop);

  assert(status == 0);

}

void Messenger::Work_Stop(uv_work_t* req) {

  Baton* baton = static_cast<Baton*>(req->data);

  baton->msgr->receiving = false;

}

void Messenger::Work_AfterStop(uv_work_t* req) {

}

Local<Object> Messenger::MessageToJS(pn_message_t* message) {

  Local<Object> result(Object::New());

  size_t buffsize = 1024;
  char buffer[buffsize];
  pn_data_t *body = pn_message_body(message);
  pn_data_format(body, buffer, &buffsize);

  result->Set(String::NewSymbol("body"), Local<Value>(String::New(buffer)));

  return result;

}
