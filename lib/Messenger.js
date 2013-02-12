var Messenger = require('../build/Release/cproton.node').Messenger;
var events = require('events');

inherits(Messenger, events.EventEmitter);
module.exports = Messenger;

function inherits(target, source) {
  for (var k in source.prototype)
    target.prototype[k] = source.prototype[k];
}

Messenger.prototype.listen = function(url) {

  var that = this;

  if (!url) {
    url = "amqp://~0.0.0.0";
  }

  console.log("About to subscribe and receive from " + url); 

  this.subscribe(url, function(err, subscribed_url) {
    that.receive();
    console.log("Listening for incoming messages on " + subscribed_url);
  });

}
