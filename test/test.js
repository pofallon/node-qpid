var nconf = require('nconf');
var Messenger = require('..').proton.Messenger;

nconf.file(process.env.HOME + '/.cproton/config.json');

// var url = 'amqps://' + nconf.get('servicebus:issuer') + ':' + nconf.get('servicebus:key') + '@' + nconf.get('servicebus:namespace') + '.servicebus.windows.net/' + nconf.get('servicebus:entity');

var url = "amqp://localhost:5672/queue1";

// console.log(url);

var max = 10;
var sent = max;
var received = max;

var doSend = function(msgr, count) {

  if (count > 0) {
    var message = "Hello " + Date.now();
    msgr.send(message, function() {
      console.log("Message sent: " + message);
      doSend(msgr, --count);
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
