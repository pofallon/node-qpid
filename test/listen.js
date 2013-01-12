var nconf = require('nconf');
var Messenger = require('../lib/cproton').Messenger;
// var Messenger = require('../lib/Messenger');

nconf.file(process.env.HOME + '/.cproton/config.json');

var url = 'amqps://' + nconf.get('servicebus:issuer') + ':' + nconf.get('servicebus:key') + '@' + nconf.get('servicebus:namespace') + '.servicebus.windows.net/' + nconf.get('servicebus:entity');

// console.log(url);

var max = 4;
var count = 0;

var m = new Messenger().subscribe(url);

m.on('subscribeed', function() {
  console.log("Node: subscribed!");
});

m.on('message', function(msg) {
  console.log("Node: msg => " + msg);
});

m.on('error', function(err) {
  console.log("Node: 'error' event (" + err.message + ")");
});

console.log("Node: about to listen()");
m.listen();
