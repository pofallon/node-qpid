var Messenger = require('..').proton.Messenger;

var send_url = "amqp://0.0.0.0/";

var max = 5;
var sent = max;
var received = max;

var doSend = function(msgr, count) {

  if (count > 0) {
    var message = "Hello " + Date.now();
    msgr.send({address: send_url, body: message}, function(err) {
      if (err) {
        console.log("Error! + " + err.getMessage());
      } else {
        console.log("Message sent: " + message);
        doSend(msgr, --count);
      }
    });
  }

};

var m = new Messenger();

m.on('message', function(msg) {
  console.log("Message received: " + msg.body);
  if (--received === 0) {
    console.log("Stopping...");
    m.stop();
  }
});

m.on('error', function(err) {
  console.log("Error: " + err.message);
});

m.listen();

doSend(m, 5);
