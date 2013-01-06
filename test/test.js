var nconf = require('nconf');
var Messenger = require('../lib/cproton').Messenger;
// var Messenger = require('../lib/Messenger');

nconf.file(process.env.HOME + '/.cproton/config.json');

var url = 'amqps://' + nconf.get('servicebus:issuer') + ':' + nconf.get('servicebus:key') + '@' + nconf.get('servicebus:namespace') + '.servicebus.windows.net/' + nconf.get('servicebus:entity');

// console.log(url);

var max = 4;
var count = 0;

var m = new Messenger().subscribe(url);
m.on('subscribed', function() {
  console.log("Node: 'subscribed' event");
  for (var i = 0; i <= max-1; i++) {
    m.send('Testing: ' + i + ' - ' + Date.now(), function(err) {
      count++;
      if (err) {
        console.log("Node: send callback count #" + count + " (error: " + err.message + ")");
      } else { 
        console.log("Node: send callback count #" + count + " (success)");
        if (count === max) {
          console.log("Done!");
        }
      }
    });
  }
});
m.on('error', function(err) {
  console.log("Node: 'error' event (" + err.message + ")");
});
// console.log( m.plusOne() ); // 11
//console.log( m.plusOne() ); // 12
//console.log( m.plusOne() ); // 13
