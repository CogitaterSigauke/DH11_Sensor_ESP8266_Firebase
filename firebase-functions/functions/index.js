const functions = require('firebase-functions');
const admin = require("firebase-admin");
const app = require('express')();
const cors = require('cors');

app.use(cors());
var serviceAccount = require("./key.json");
admin.initializeApp({
  credential: admin.credential.cert(serviceAccount),
  databaseURL: "https://energyresearch-43949.firebaseio.com"
});

const db = admin.firestore();
const rdb = admin.database();

exports.createWeatherData = functions.database.ref('/DHT11/HumidTemp/{id}')
    .onCreate((snapshot, context) => {
        var docID = context.params.id;
        const original = snapshot.val();
        var dataArray = original.split(";");
        var IDs = dataArray[2].split("-");
        const newData = {

            Date: new Date().toISOString(),
            House_ID : IDs[0],
            Room_ID : IDs[1],
            Humidity : dataArray[0],
            Temperature : dataArray[1]
    
        }
        
        db.collection('RealTimeInnerTemperatureAndHumidityData')
        .add(newData)
        .then((doc) => {
            const resData = {
                dataId : doc.id
            };

            res.json(resData);
            

        })
        .catch((err) => {
            res.status(500).json({error: 'something went wrong'});
            console.error(err);
        });

        rdb.ref(`/DHT11/HumidTemp/${docID}`).set(null)
        .catch((err) => {
            res.status(500).json({error: 'something went wrong'});
            console.error(err);
        });

    });


postData = (req, res) => {
    
    const newData = {
        
        Date: new Date().toISOString(),
        House_ID : req.params.houseID,
        Room_ID : req.params.roomID,
        Humidity : req.params.humidity,
        Temperature : req.params.temperature
    
    }

    db.collection('RealTimeInnerTemperatureAndHumidityData')
        .add(newData)
        .then((doc) => {
            const resData = {
                dataId : doc.id
            };

            res.json(resData);
     
        })
        .catch((err) => {
            res.status(500).json({error: 'something went wrong'});
            console.error(err);
    });

}


app.get('/data/:temperature/:humidity/:houseID', postData);

exports.api = functions.region('asia-northeast1').https.onRequest(app);
