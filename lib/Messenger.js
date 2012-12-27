var Messenger = require('../build/Release/cproton.node').Messenger;
var events = require('events');

inherits(Messenger, events.EventEmitter);
module.exports = Messenger;

function inherits(target, source) {
  for (var k in source.prototype)
    target.prototype[k] = source.prototype[k];
}
