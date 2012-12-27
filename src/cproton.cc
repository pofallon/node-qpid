#define BUILDING_NODE_EXTENSION
#include <node.h>
#include "messenger.h"

using namespace v8;

void InitAll(Handle<Object> target) {
  Messenger::Init(target);
}

NODE_MODULE(cproton, InitAll)
