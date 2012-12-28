var nconf = require('nconf');
var Messenger = require('../lib/cproton').Messenger;
// var Messenger = require('../lib/Messenger');

nconf.file(process.env.HOME + '/.cproton/config.json');

var url = 'amqps://' + nconf.get('servicebus:issuer') + ':' + nconf.get('servicebus:key') + '@' + nconf.get('servicebus:namespace') + '.servicebus.windows.net/' + nconf.get('servicebus:entity');

console.log(url);

var m = new Messenger(url);
m.on('connected', function() {
  console.log("Connected!");
  m.send('Testing!', function(err) {
    if (err) {
      console.log("Error sending: " + err.message);
    } else { 
      console.log("Success?");
    }
  });
});
m.on('error', function(err) {
  console.log('Error: ' + err.message);
});
console.log( m.plusOne() ); // 11
console.log( m.plusOne() ); // 12
console.log( m.plusOne() ); // 13
