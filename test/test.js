var nconf = require('nconf');
var Messenger = require('..').proton.Messenger;

nconf.file(process.env.HOME + '/.cproton/config.json');

var url = 'amqps://' + nconf.get('servicebus:issuer') + ':' + nconf.get('servicebus:key') + '@' + nconf.get('servicebus:namespace') + '.servicebus.windows.net/' + nconf.get('servicebus:entity');

// console.log(url);

var max = 20;

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

m.on('subscribed', function() {
  console.log("Successfully subscribed!");
  doSend(m, max);
});

m.on('message', function(msg) {
  console.log("Message received: " + msg.body);
});

m.on('error', function(err) {
  console.log("Node: 'error' event (" + err.message + ")");
});

m.listen();
