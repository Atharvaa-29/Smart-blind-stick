#include <TinyGPS++.h>

#include <SoftwareSerial.h>

#include <ESP8266WiFi.h>

#include <FirebaseESP8266.h>

#define FIREBASE_HOST "https://esp8266demo1-34e55-default-rtdb.firebaseio.com"
#define FIREBASE_AUTH "CjBjqvuWWyVNqxZn0TN4HBwezAlUuZCt4MNLs6CC"
const String PHONE = "+919820232184";

//GSM Module RX pin to NodeMCU D6
//GSM Module TX pin to NodeMCU D5
#define rxGSM D5
#define txGSM D6
SoftwareSerial sim800(rxGSM, txGSM);


#define rxGPS D2
#define txGPS D3
SoftwareSerial neogps(rxGPS, txGPS);
TinyGPSPlus gps;

String smsStatus, senderNumber, receivedDate, msg;
const char* ssid = "Galaxy M318476";
const char* password = "ndyz8798";

float latitude , longitude;
FirebaseData firebaseData;

FirebaseJson json;

void setup()

{

  Serial.begin(115200);

  neogps.begin(9600);
  sim800.begin(9600);
  Serial.println("SIM800L serial initialize");


  Serial.println();

  Serial.print("Connecting to ");

  Serial.println(ssid);


  WiFi.begin(ssid, password);


  while (WiFi.status() != WL_CONNECTED)

  {

    delay(500);

    Serial.print(".");

  }

  Serial.println("");

  Serial.println("WiFi connected");

  sim800.listen();
  neogps.listen();

  smsStatus = "";
  senderNumber = "";
  receivedDate = "";
  msg = "";

  sim800.print("AT+CMGF=1\r"); //SMS text mode
  delay(1000);


  // firebase
 // Serial.println("Connecting Firebase.....");
  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
  Firebase.reconnectWiFi(true);
  //Serial.println("Firebase OK.");
}

void loop()

{
  while (neogps.available() > 0)

    if (gps.encode(neogps.read()))

    {

      if (gps.location.isValid())

      {

        latitude = gps.location.lat();

        String lat_str = String(latitude , 6);
       // Serial.println(lat_str);

        longitude = gps.location.lng();


        String lng_str = String(longitude , 6);
        //Serial.println(lng_str);


        if (Firebase.setFloat(firebaseData, "/GPS/f_latitude", latitude))
        { //Serial.println('Done lat');
          //Serial.println("PATH: " + firebaseData.dataPath());
          //Serial.println("TYPE: " + firebaseData.dataType());
          //Serial.println("ETag: " + firebaseData.ETag());

        }
        else
        {
          Serial.println('Not Done lat');
        }
        //-------------------------------------------------------------
        if (Firebase.setFloat(firebaseData, "/GPS/f_longitude", longitude))
        { //Serial.println('Done log');
          //Serial.println("PATH: " + firebaseData.dataPath());
          //Serial.println("TYPE: " + firebaseData.dataType());
          //Serial.println("ETag: " + firebaseData.ETag());
        }
        else
        { Serial.println('Not Done log');
        }

      }

      while (sim800.available()) {
        parseData(sim800.readString());
      }


      while (Serial.available())
      {
        //String s = Serial.readString();
        //Serial.println(s);
        sim800.println(Serial.readString());
      }

    }
}
//main loop ends


void parseData(String buff) {
  Serial.println(buff);

  unsigned int len, index;
  index = buff.indexOf("\r");
  buff.remove(0, index + 2);
  buff.trim();

  if (buff != "OK") {
    index = buff.indexOf(":");
    String cmd = buff.substring(0, index);
    cmd.trim();

    buff.remove(0, index + 2);

    if (cmd == "+CMTI") {
      //get newly arrived memory location and store it in temp
      index = buff.indexOf(",");
      String temp = buff.substring(index + 1, buff.length());
      temp = "AT+CMGR=" + temp + "\r";
      //get the message stored at memory location "temp"
      sim800.println(temp);
    }
    else if (cmd == "+CMGR") {
      extractSms(buff);

      if (senderNumber == PHONE) {
        if (msg == "get location") {
          sendLocation();
        }
      }
      else {
        Serial.println(String(senderNumber) + ": Phone not registered");
      }
    }

  }
  else {
    //The result of AT Command is "OK"
  }
}

void extractSms(String buff) {
  unsigned int index;

  index = buff.indexOf(",");
  smsStatus = buff.substring(1, index - 1);
  buff.remove(0, index + 2);

  senderNumber = buff.substring(0, 13);
  buff.remove(0, 19);

  receivedDate = buff.substring(0, 20);
  buff.remove(0, buff.indexOf("\r"));
  buff.trim();

  index = buff.indexOf("\n\r");
  buff = buff.substring(0, index);
  buff.trim();
  msg = buff;
  buff = "";
  msg.toLowerCase();

}

void sendLocation()
{

  // Can take up to 60 seconds
  boolean newData = false;

  for (unsigned long start = millis(); millis() - start < 2000;)
  {
    while (neogps.available())
    {
      if (gps.encode(neogps.read()))
      {
        newData = true;
      }
    }
  }

  //If newData is true
  if (newData)
  {
    Serial.print("Latitude= ");
    Serial.print(gps.location.lat(), 6);
    Serial.print(" Longitude= ");
    Serial.println(gps.location.lng(), 6);
    newData = false;
    delay(300);
    ///*
    sim800.print("AT+CMGF=1\r");
    delay(100);
    sim800.print("AT+CMGS=\"" + PHONE + "\"\r");
    delay(1000);
    sim800.print("http://maps.google.com/maps?q=loc:");
    sim800.print(gps.location.lat(), 6);
    sim800.print(",");
    sim800.print(gps.location.lng(), 6);
    delay(1000);
    sim800.write(0x1A); //ascii code for ctrl-26 //sim800.println((char)26); //ascii code for ctrl-26
    delay(1000);
    Serial.println("GPS Location SMS Sent Successfully.");
    //*/
  }
  else {
    Serial.println("Invalid GPS data");
  }

}
