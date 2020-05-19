/* Sending Sensor Data  to Firebase Database by CircuitDigest(www.circuitdigest.com) */

#include <ESP8266WiFi.h>                                                    // esp8266 library
#include <FirebaseArduino.h>                                                // firebase library
#include <DHT.h>                                                            // dht11 temperature and humidity sensor library
#include <NTPClient.h>
#include <WiFiUdp.h> 
#include <EEPROM.h>
#include <ESP8266WebServer.h>

#define FIREBASE_HOST "energyresearch-43949.firebaseio.com"                          // the project name address from firebase id
#define FIREBASE_AUTH "DMKKOGUqLiqVeq5OMfMt0LDN9UcjlxyZbccES0Da"            // the secret key generated from firebase

#define WIFI_SSID ""                                             // input your home or public wifi name 
#define WIFI_PASSWORD ""                                    //password of wifi ssid
 
#define DHTPIN D1                                                           // what digital pin we're connected to
#define DHTTYPE DHT11                                                       // select dht type as DHT 11 or DHT22
DHT dht(DHTPIN, DHTTYPE);  




//Variables
int i = 0;
int statusCode;
const char* ssid     = WIFI_SSID;     // your network SSID (name of wifi network)
const char* password = WIFI_PASSWORD;
String houseID = "text";
String st;
String content;


//Function Decalration
bool testWifi(void);
void launchWeb(void);
void setupAP(void);
void createWebServer(void);

//Establishing Local server at port 80 whenever required
ESP8266WebServer server(80);


String path;

// Variables NTP
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "europe.pool.ntp.org", 32400, 60000);


void setup() {
  //Initialize serial and wait for port to open:
  Serial.begin(9600);
  delay(1000);
  
  Serial.println();
  Serial.println("Disconnecting previously connected WiFi");
  WiFi.disconnect();
  EEPROM.begin(512); //Initialasing EEPROM
  delay(10);
  
  //***  pinMode(LED_BUILTIN, OUTPUT);
  //-----Read eeprom for ssid and pass
  Serial.println("Reading EEPROM ssid");

  //----------------
  String esid;
  for (int i = 0; i < 32; ++i)
  {
    esid += char(EEPROM.read(i));
  }
  Serial.println();
  Serial.print("SSID: ");
  Serial.println(esid);
  Serial.println("Reading EEPROM pass");

  String epass = "";
  for (int i = 32; i < 96; ++i)
  {
    epass += char(EEPROM.read(i));
  }
  Serial.println("PASSWORD READ");
  Serial.println("Reading EEPROM HOUSE_ID");
  String ehouseId = "";
  for (int i = 96; i < 121; ++i)
  {
    ehouseId += char(EEPROM.read(i));
  }
  Serial.print("HOUSE_ID: ");
  Serial.println(ehouseId);

  houseID = ehouseId;
  Serial.println(houseID);

  WiFi.begin(esid.c_str(), epass.c_str());
  if (testWifi())
  {
    Serial.println("Succesfully Connected!!!");
      timeClient.begin();
    while(!timeClient.update()) {
      timeClient.forceUpdate();
    }
  
    Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);              // connect to firebase
    dht.begin(); 
    return;
  }
  else
  {
    Serial.println("Turning the HotSpot On");
    launchWeb();
    setupAP();// Setup HotSpot
  }

  Serial.println();
  Serial.println("Waiting.");
    
  while ((WiFi.status() != WL_CONNECTED))
  {
    Serial.print(".");
    delay(100);
    server.handleClient();
  }
  //================

  Serial.print("Connected to ");
  Serial.println(ssid);                                   //print local IP address

  timeClient.begin();
  //timeClient.setTimeOffset(32400);
  while(!timeClient.update()) {
    timeClient.forceUpdate();
  }
  
  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);             // connect to firebase
  dht.begin();                                              //Start reading dht sensor
}

void loop() { 
  float humidity = dht.readHumidity();        // Reading temperature or humidity takes about 250 milliseconds!
  float temperature = dht.readTemperature();  // Read temperature as Celsius (the default)

  if ((WiFi.status() == WL_CONNECTED)){
  if (isnan(humidity) || isnan(temperature)) {// Check if any reads failed and exit early (to try again).
    Serial.println(F("Failed to read from DHT sensor!"));
    return;
  }  
  String str = "Humidity: " + String(humidity) + String("%\t") + "Temperature: " + String(temperature)+ String("°C\t") + "HouseAndRoomID: " + houseID ;
  Serial.println(str);
  
  timeClient.update();
  String currTime = timeClient.getFormattedTime();
  String reading = String(humidity) + ";" + String(temperature) + ";" + houseID ;

  if (Firebase.pushString("/DHT11/HumidTemp", reading)) {
    //setup path and send readings

    Serial.println("SEND TIME, HUMIDITY, TEMPERATURE SUCCESS");
  } 
  else {
     Serial.println("SEND TIME, HUMIDITY, TEMPERATURE FAILED");
  }
  }
  delay(4000); 
}

//-------- Fuctions used for WiFi credentials saving and connecting to it which you do not need to change 
bool testWifi(void)
{
  int c = 0;
  Serial.println("Waiting for Wifi to connect");
  while ( c < 20 ) {
    if (WiFi.status() == WL_CONNECTED)
    {
      return true;
    }
    delay(500);
    Serial.print("*");
    c++;
  }
  Serial.println("");
  Serial.println("Connect timed out, opening AP");
  return false;
}

void launchWeb()
{
  Serial.println("");
  if (WiFi.status() == WL_CONNECTED)
    Serial.println("WiFi connected");
  Serial.print("Local IP: ");
  Serial.println(WiFi.localIP());
  Serial.print("SoftAP IP: ");
  Serial.println(WiFi.softAPIP());
  createWebServer();
  // Start the server
  server.begin();
  Serial.println("Server started");
}

void setupAP(void)
{
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);
  int n = WiFi.scanNetworks();
  Serial.println("scan done");
  if (n == 0)
    Serial.println("no networks found");
  else
  {
    Serial.print(n);
    Serial.println(" networks found");
    for (int i = 0; i < n; ++i)
    {
      // Print SSID and RSSI for each network found
      Serial.print(i + 1);
      Serial.print(": ");
      Serial.print(WiFi.SSID(i));
      Serial.print(" (");
      Serial.print(WiFi.RSSI(i));
      Serial.print(")");
      Serial.println((WiFi.encryptionType(i) == ENC_TYPE_NONE) ? " " : "*");
      delay(10);
    }
  }
  Serial.println("");
  st = "<ol>";
  for (int i = 0; i < n; ++i)
  {
    // Print SSID and RSSI for each network found
    st += "<li>";
    st += WiFi.SSID(i);
    st += " (";
    st += WiFi.RSSI(i);

    st += ")";
    st += (WiFi.encryptionType(i) == ENC_TYPE_NONE) ? " " : "*";
    st += "</li>";
  }
  st += "</ol>";
  delay(100);
  WiFi.softAP("WeatherSensor", "");
  Serial.println("softap");
  launchWeb();
  Serial.println("over");
}



void createWebServer()
{
 {
    server.on("/", []() {

      IPAddress ip = WiFi.softAPIP();
      String ipStr = String(ip[0]) + '.' + String(ip[1]) + '.' + String(ip[2]) + '.' + String(ip[3]);
      content = "<!DOCTYPE HTML>\r\n<html>Hello from ESP8266 at ";
      content += "<form action=\"/scan\" method=\"POST\"><input type=\"submit\" value=\"scan\"></form>";
      content += ipStr;
      content += "<p>";
      content += st;
      content += "</p><form method='get' action='setting'><label>SSID: </label> <input name='ssid' length=32> <br/> <label>Password: </label> <input name='pass' length=64 type='password' placeholder='password'> <br /> <label>House ID: </label> <input placeholder='HouseID-RoomID' name='houseId' length=25> <br /> <input type='submit'></form>";
      content += "</html>"; 
      server.send(200, "text/html", content);
    });
    server.on("/scan", []() {
      //setupAP();
      IPAddress ip = WiFi.softAPIP();
      String ipStr = String(ip[0]) + '.' + String(ip[1]) + '.' + String(ip[2]) + '.' + String(ip[3]);

      content = "<!DOCTYPE HTML>\r\n<html>go back";
      server.send(200, "text/html", content);
    });

    server.on("/setting", []() {
      String qsid = server.arg("ssid");
      String qpass = server.arg("pass");
      String qhouseId = server.arg("houseId");
      if (qsid.length() > 0 && qpass.length() > 0) {
        Serial.println("clearing eeprom");
        for (int i = 0; i < 121; ++i) {
          EEPROM.write(i, 0);
        }
        Serial.println(qsid);
        Serial.println("");
        Serial.println(qpass);
        Serial.println("");
        Serial.println(qhouseId);
        Serial.println("");

        Serial.println("writing eeprom ssid:");
        for (int i = 0; i < qsid.length(); ++i)
        {
          EEPROM.write(i, qsid[i]);
          Serial.print("Wrote: ");
          Serial.println(qsid[i]);
        }
        Serial.println("writing eeprom pass:");
        for (int i = 0; i < qpass.length(); ++i)
        {
          EEPROM.write(32 + i, qpass[i]);
          Serial.print("Wrote: ");
          Serial.println(qpass[i]);
        }
        Serial.println("writing eeprom houseId:");
        for (int i = 0; i < qhouseId.length(); ++i)
        {
          EEPROM.write(96 + i, qhouseId[i]);
          Serial.print("Wrote: ");
          Serial.println(qhouseId[i]);
        }
     
        EEPROM.commit();

        content = "{\"Success\":\"saved to eeprom... reset to boot into new wifi\"}";
        statusCode = 200;
        ESP.reset();
      } else {
        content = "{\"Error\":\"404 not found\"}";
        statusCode = 404;
        Serial.println("Sending 404");
      }
      server.sendHeader("Access-Control-Allow-Origin", "*");
      server.send(statusCode, "application/json", content);

    });
  } 
}
