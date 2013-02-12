var Messenger = require('..').proton.Messenger;

var optimist = require('optimist')
    .options('a', {
        alias : 'address',
        default : 'amqp://~0.0.0.0',
    })
    .usage("$0 [-a address]")
;

var m = new Messenger();

m.on('error', function(err) {
  console.log("Error: " + err.message);
});

m.on('subscribed', function(url) {
  console.log("Subscribed to " + url);
});

m.on('message', function(message) {
  console.log(message.body);
});

m.subscribe(optimist.argv.address).receive();
