const fs = require('fs');
const util = require('util');
var SerialPort = require('serialport');
var Readline = SerialPort.parsers.Readline;

var out = fs.createWriteStream(__dirname + '/index.out.log', { flags: 'a' }), 
    error = fs.createWriteStream(__dirname + '/index.error.log', { flags: 'a' });
// redirect stdout / stderr

console.log = function(d) { //
  out.write('['+new Date().toISOString()+']' + util.format(d) + '\n');
};

console.error = function(d) {
  error.write('['+new Date().toISOString()+']' + util.format(d) + '\n');
};


var serialPort = new SerialPort('/dev/ttyAMA0', {
  baudRate: 115200
}, false);
var parser = new Readline();
serialPort.pipe(parser);

var MongoClient = require('mongodb').MongoClient;
var url = "mongodb://localhost:27017/hubdb";

parser.on('data', function (data) {
  try {
  if (data.startsWith('#')) {
    var json = data.substr(1);
    console.log(json);
    

    MongoClient.connect(url, function(err, db) {
      if (err) throw err;
      var col = db.collection('events');
      var jsonObj = JSON.parse(json);
      jsonObj.createdAt = Date.now();
      col.insert(jsonObj);
      db.close();
    });
  }
  }
  catch (err)
  {
    console.error('Error: ' + err);
  }
});

serialPort.on('open', function () {
  console.log('Communication is on!')
});
