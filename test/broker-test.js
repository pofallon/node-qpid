var nconf = require('nconf');
var Messenger = require('..').proton.Messenger;

var url = "amqp://localhost:5672/queue1";

var max = 5;
var sent = max;
var received = max;

var doSend = function(msgr, count) {

  if (count > 0) {
    var message = "Hello " + Date.now();
    msgr.send({address: url, body: message}, function(err) {
      if (err) {
        console.log("Error! + " + err.getMessage());
      } else {
        console.log("Message sent: " + message);
        doSend(msgr, --count);
      }
    });
  }

};

var m = new Messenger().subscribe(url);

m.on('subscribed', function(url) {
  console.log("Successfully subscribed to URL: " + url);
  doSend(m, sent);
});

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

m.receive();
