#include <TinyGPS++.h>

#include <SoftwareSerial.h>

#include <ESP8266WiFi.h>

#include <FirebaseESP8266.h>  

#define FIREBASE_HOST "https://esp8266demo1-34e55-default-rtdb.firebaseio.com"
#define FIREBASE_AUTH "CjBjqvuWWyVNqxZn0TN4HBwezAlUuZCt4MNLs6CC"

const int RXPin = 4, TXPin = 5;
SoftwareSerial neo6m(RXPin, TXPin);
TinyGPSPlus gps;


const char* ssid = "MI_29";
const char* password = "atharva29";

float latitude , longitude;
FirebaseData firebaseData;

FirebaseJson json;


void setup()

{

  Serial.begin(115200);

  neo6m.begin(9600);

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






  // firebase
  Serial.println("Connecting Firebase.....");
  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
  Firebase.reconnectWiFi(true);
  Serial.println("Firebase OK.");

  


}


void loop()

{


  while (neo6m.available() > 0)

    if (gps.encode(neo6m.read()))

    {

      if (gps.location.isValid())

      {

        latitude = gps.location.lat();

        String lat_str = String(latitude , 6);
      Serial.println(lat_str);

        longitude = gps.location.lng();
    

       String lng_str = String(longitude , 6);
        Serial.println(lng_str);


  if(Firebase.setFloat(firebaseData, "/GPS/f_latitude", latitude))
      {Serial.println('Done lat');
    Serial.println("PATH: " + firebaseData.dataPath());
    Serial.println("TYPE: " + firebaseData.dataType());
    Serial.println("ETag: " + firebaseData.ETag());

  }
    else
      {Serial.println('Not Done lat');}
    //-------------------------------------------------------------
    if(Firebase.setFloat(firebaseData, "/GPS/f_longitude", longitude))
      {Serial.println('Done log');
    Serial.println("PATH: " + firebaseData.dataPath());
    Serial.println("TYPE: " + firebaseData.dataType());
    Serial.println("ETag: " + firebaseData.ETag());
  }
    else
      {Serial.println('Not Done log');}

      }
  
    }
  
}
