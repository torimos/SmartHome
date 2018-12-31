const MongoClient = require('mongodb').MongoClient;

let db;

const loadDB = async (url, dbname) => {
    if (db) {
        return db;
    }
    try {
        const client = await MongoClient.connect(`${url}${dbname}`);
        db = client.db(dbname);
    } catch (err) {
        Raven.captureException(err);
    }
    return db;
};

module.exports = loadDB;