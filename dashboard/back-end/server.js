const loadDB = require('./db');
const http = require('http');
const url = require('url');
const path = require('path');
const fs = require('fs');

// maps file extention to MIME types
const mimeType = {
  '.ico': 'image/x-icon',
  '.html': 'text/html',
  '.js': 'text/javascript',
  '.json': 'application/json',
  '.css': 'text/css',
  '.png': 'image/png',
  '.jpg': 'image/jpeg',
  '.wav': 'audio/wav',
  '.mp3': 'audio/mpeg',
  '.svg': 'image/svg+xml',
  '.pdf': 'application/pdf',
  '.doc': 'application/msword',
  '.eot': 'appliaction/vnd.ms-fontobject',
  '.ttf': 'aplication/font-sfnt'
};
const hostname = '0.0.0.0';
const port = 8080;
var mongoUrl = "mongodb://smarthub.local:27017/";
loadDB(mongoUrl, 'hubdb').then((db)=>{
  const server = http.createServer(async (req, res) => {
    console.log(`${req.method} ${req.url}`);

    var parsedUrl = url.parse(req.url, true);
    var query = parsedUrl.query;
    if (req.url.startsWith('/api/events'))
    {
      let skip = parseInt(query.skip);
      let limit = parseInt(query.limit);
      let order = parseInt(query.order);
      skip = skip > 0 ? skip : 0
      limit = limit > 0 ? limit : 12
      order = order ? order : -1;
      const cnt = await db.collection("events").count();
      const records = await db.collection("events")
        .find({})
        .sort({createdAt: order})
        .skip(skip)
        .limit(limit)
        .toArray((err, result) => {
          if (err) throw err;
          res.statusCode = 200;
          res.setHeader('Content-Type', 'application/json');
          res.setHeader('Access-Control-Allow-Origin', '*');
          res.setHeader('Access-Control-Allow-Methods', 'GET,PUT,POST,DELETE');
          res.setHeader('Access-Control-Allow-Headers', 'Content-Type');
          var respJson = {};
          respJson.items = result.reverse();
          respJson.totalCount = cnt;
          res.end(JSON.stringify(respJson));
        });
    }
    else if (req.url.startsWith('/api/commands'))
    {
      if (query.cmd == "reboot") {
        require('child_process').exec('sudo /sbin/shutdown -r now', function (msg) { console.log(msg) });
      }
    }
    else {
      const sanitizePath = path.normalize(parsedUrl.pathname).replace(/^(\.\.[\/\\])+/, '');
      let pathname = path.join(__dirname + "/dist", sanitizePath);

      fs.exists(pathname, function (exist) {
        if(!exist) {
          // if the file is not found, return 404
          res.statusCode = 404;
          res.end(`File ${pathname} not found!`);
          return;
        }
        // if is a directory, then look for index.html
        if (fs.statSync(pathname).isDirectory()) {
          pathname += '/index.html';
        }
        // read file from file system
        fs.readFile(pathname, function(err, data){
          if(err){
            res.statusCode = 500;
            res.end(`Error getting the file: ${err}.`);
          } else {
            // based on the URL path, extract the file extention. e.g. .js, .doc, ...
            const ext = path.parse(pathname).ext;
            // if the file is found, set Content-type and send data
            res.setHeader('Content-type', mimeType[ext] || 'text/plain' );
            res.end(data);
          }
        });
      });
    }
  });
  server.listen(port, hostname, () => {
    console.log(`Server running at http://${hostname}:${port}/`);
  });
});

