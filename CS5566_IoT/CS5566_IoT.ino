#include <ESP8266WiFi.h>                                                    // esp8266 library
#include <FirebaseArduino.h>                                                // firebase library
#include "DHT.h"                                                            // dht11 temperature and humidity sensor library
#define FIREBASE_HOST "cs5566-iot-temp-humidity.firebaseio.com"             // the project name address from firebase id
#define FIREBASE_AUTH "oeyVgZOzdQKm8tKc8LC0MoojAVqCu6iZVBUAoBux"            // the secret key generated from firebase
#define WIFI_SSID "TampaBae"                                             // input your home or public wifi name 
#define WIFI_PASSWORD "lealupin"                                             //password of wifi ssid
char server[] = "mail.smtpcorp.com";
 
#define DHTPIN D4                                                           // what digital pin we're connected to
#define DHTTYPE DHT11                                                       // select dht type as DHT 11 or DHT22
DHT dht(DHTPIN, DHTTYPE);  

WiFiClient client;

void setup() {
  Serial.begin(115200);
  delay(1000);                
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);                                     //try to connect with wifi
  Serial.print("Connecting to ");
  Serial.print(WIFI_SSID);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.println();
  Serial.print("Connected to ");
  Serial.println(WIFI_SSID);
  Serial.print("IP Address is : ");
  Serial.println(WiFi.localIP());                                            //print local IP address
  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);                              // connect to firebase
  dht.begin();  //Start reading dht sensor
}

void loop() {            // wait for a second
  float h = dht.readHumidity();                                              // Reading temperature or humidity takes about 250 milliseconds!
  float t = dht.readTemperature(true);                                       // Read temperature as Fahrenheit (the default)
    
  if (isnan(h) || isnan(t)) {                                                // Check if any reads failed and exit early (to try again).
    Serial.println(F("Failed to read from DHT sensor!"));
    return;
  }
  
  Serial.print("Humidity: ");  Serial.print(h);
  String fireHumid = String(h) + String("%");                                         //convert integer humidity to string humidity 
  Serial.print("%  Temperature: ");  Serial.print(t);  Serial.println("°F ");
  String fireTemp = String(t) + String("°F");                                        //convert integer temperature to string temperature
  delay(4000);
  
  Firebase.pushString("/DHT11/Humidity", fireHumid);                                  //setup path and send readings
  Firebase.pushString("/DHT11/Temperature", fireTemp);                                //setup path and send readings

  float recH = 50;              //Set this number yourself
  float recT = 80;              //Set this number yourself

  if (h < recH || t > recT)
  {
    byte ret = sendEmail(h, t, recH, recT);
  }
}

byte sendEmail(float h, float t, float recH, float recT)
{
  byte thisByte = 0;
  byte respCode;

  if (client.connect(server, 2525) == 1) {
    Serial.println(F("connected"));
  } else {
    Serial.println(F("connection failed"));
    return 0;
  }
  if (!eRcv()) return 0;

  Serial.println(F("Sending EHLO"));
  client.println("EHLO www.example.com");
  if (!eRcv()) return 0;
  Serial.println(F("Sending auth login"));
  client.println("auth login");
  if (!eRcv()) return 0;
  Serial.println(F("Sending User"));
  // Change to your base64, ASCII encoded user
  client.println("a2tjNkB2dC5lZHU="); // SMTP UserID
  if (!eRcv()) return 0;
  Serial.println(F("Sending Password"));
  // change to your base64, ASCII encoded password
  client.println("Q1M1NTY2SU9U");//  SMTP Passw
     if (!eRcv()) return 0;
    Serial.println(F("Sending From"));   // change to your email address (sender)
   client.println(F("MAIL From: ashishshk@gmail.com"));// not important 
   if (!eRcv()) return 0;   // change to recipient address
    Serial.println(F("Sending To"));
    client.println(F("RCPT To: kkc6@gmail.com"));
    if (!eRcv()) return 0;
    Serial.println(F("Sending DATA"));
    client.println(F("DATA"));
    
    if (!eRcv()) return 0;
    
    Serial.println(F("Sending email"));   // change to recipient address
   client.println(F("To: kkc6@gmail.com"));   // change to your address
   client.println(F("From: ashishshk@gmail.com"));
   client.println(F("Subject: Emails from ESp8266\r\n"));
 
    client.println(F("Warning: the temperature and/or humidity has passed the acceptable threshold"));
    client.print(F("Recommended temperature (F): "));
    client.println(recT);
    client.print(F("Current temperature (F): "));
    client.println(t);
    client.print(F("Recommended humidity (%): "));
    client.println(recH);
    client.print(F("Current humidity (%): "));
    client.println(h);
    client.println();
    client.println(F("We encourage you to remediate the temperature and/or humidity within a timely manner."));
    
    if (!eRcv()) return 0;
    
    Serial.println(F("Sending QUIT"));
    client.println(F("QUIT"));
    
    if (!eRcv()) return 0;
    
    client.stop();
    Serial.println(F("disconnected"));
    return 1;
  }
  byte eRcv()
  {
    byte respCode;
    byte thisByte;
    int loopCount = 0;
    while (!client.available())
    {
      delay(1);
      loopCount++;     // if nothing received for 10 seconds, timeout
      if (loopCount > 10000) {
      client.stop();
      Serial.println(F("\r\nTimeout"));
      return 0;
    }
  }

  respCode = client.peek();
  while (client.available())
  {
    thisByte = client.read();
    Serial.write(thisByte);
  }

  if (respCode >= '4')
  {
    //  efail();
    return 0;
  }
  return 1;
}
