var Messenger = require('..').proton.Messenger;
var optimist = require('optimist')
    .options('a', {
        alias : 'address',
        default : 'amqp://0.0.0.0',
    })
    .usage("$0 [-a address] message")
;

var message;
if (optimist.argv._.length < 1) {
  console.log(optimist.help());
} else {
  message = optimist.argv._[optimist.argv._.length-1];
}

var m = new Messenger();

m.on('error', function(err) {
  console.log("Error: " + err.message);
});

m.send({address: optimist.argv.address, body: message}, function(err) {
  if (err) {
    console.log("Error sending message: " + err.message);
  }
});
