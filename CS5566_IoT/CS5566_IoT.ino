#include <ESP8266WiFi.h>                                                    // esp8266 library
#include <FirebaseArduino.h>                                                // firebase library
#include "DHT.h"                                                            // dht11 temperature and humidity sensor library
#define FIREBASE_HOST "cs5566-iot-temp-humidity.firebaseio.com"             // the project name address from firebase id
#define FIREBASE_AUTH "oeyVgZOzdQKm8tKc8LC0MoojAVqCu6iZVBUAoBux"            // the secret key generated from firebase
#define WIFI_SSID "TampaBae"                                                // input your home or public wifi name 
#define WIFI_PASSWORD "lealupin"                                            // password of wifi ssid
char server[] = "mail.smtpcorp.com";                                        // server for sending emails
int numEmailsSent = 0;                                                      // count of sent alert emails
WiFiClient client;
 
#define DHTPIN D4                                                           // what digital pin we're connected to
#define DHTTYPE DHT11                                                       // select dht type as DHT 11 or DHT22
DHT dht(DHTPIN, DHTTYPE);  

/**
 * Initial setup function - connect to WiFi, connect to Firebase, turn on sensor
 */
void setup() 
{
  Serial.begin(115200);
  delay(1000);    
              
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);                                     //try to connect with wifi
  Serial.print("Connecting to ");
  Serial.print(WIFI_SSID);
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(500);
  }
  Serial.println();
  Serial.print("Connected to ");
  Serial.println(WIFI_SSID);
  
  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);                              // connect to firebase
  dht.begin();  //Start reading dht sensor
}

void loop()
{
  float h = dht.readHumidity();                                              // Reading temperature or humidity takes about 250 milliseconds!
  float t = dht.readTemperature(true);                                       // Read temperature as Fahrenheit (the default)
    
  if (isnan(h) || isnan(t))                                                  // Check if any reads failed and exit early (to try again).
  {                                                
    Serial.println(F("Failed to read from DHT sensor!"));
    return;
  }
  
  Serial.print("Humidity: ");  
  Serial.print(h);
  String fireHumid = String(h) + String("%");                                //convert integer humidity to string humidity 
  Serial.print("%  Temperature: ");  
  Serial.print(t);  
  Serial.println("°F ");
  String fireTemp = String(t) + String("°F");                                //convert integer temperature to string temperature
  delay(5000);
  
  Firebase.pushString("/DHT11/Fridge/Humidity", fireHumid);         //setup humidity path and send readings
  Firebase.pushString("/DHT11/Fridge/Temperature", fireTemp);       //setup temperature path and send readings

  float lowHum = 20;                                                           //Set this number yourself - lowest allowed humidity
  float lowTemp = 20;                                                          //Set this number yourself - highest allowed humidity
  float highHum = 90;                                                          //Set this number yourself - lowest allowed temperature
  float highTemp = 40;                                                         //Set this number yourself - highest allowed temperature

  // Checks if humidity and/or temperature vals have fallen outside of the allowed range
  // Allows for a 5 percent and/or degree differ
  if ((((lowHum - h) > 5) || ((h - highHum) > 5) || ((lowTemp - t) > 5) || ((t - highTemp) > 5)))
  {
    // Only send a new email every 5 failures
    if ((numEmailsSent % 5) == 0)
    {
      byte ret = sendEmail(h, t, lowHum, highHum, lowTemp, highTemp); 
    }

    numEmailsSent++;
  }
}

/**
 * Sends an email alerting the owner that humidity and/or temperature is not acceptable
 * 
 * @param h - Current humidity
 * @param t - Current temperature
 * @param lowHum - Lowest allowed humidity
 * @param highHum - Highest allowed humidity
 * @param lowTemp - Lowest allowed temperature
 * @param highTemp - Highest allowed temperature
 * 
 * @return 1 if successful, 0 otherwise
 */
byte sendEmail(float h, float t, float lowHum, float highHum, float lowTemp, float highTemp)
{
  byte thisByte = 0;
  byte respCode;

  // Check if connection to SMTP server succeeded or failed
  if (client.connect(server, 2525) == 1) 
  {
    Serial.println(F("connected"));
  } 
  else 
  {
    Serial.println(F("connection failed"));
    return 0;                                                                                         
  }

  // Check for server response
  if (!eRcv())
    return 0;

  // Send Extended HELLO
  Serial.println(F("Sending EHLO"));                                            
  client.println("EHLO www.example.com");

  // Check for server response
  if (!eRcv())
    return 0;

  Serial.println(F("Sending auth login"));
  client.println("auth login");

  // Check for server response
  if (!eRcv())
    return 0;
  
  Serial.println(F("Sending User"));
  // your base64, ASCII encoded user
  client.println("a2tjNkB2dC5lZHU="); // SMTP UserID

  // Check for server response
  if (!eRcv())
    return 0;
  
  Serial.println(F("Sending Password"));
  // your base64, ASCII encoded password
  client.println("Q1M1NTY2SU9U");//  SMTP Password

  // Check for server response
  if (!eRcv())
    return 0;
     
  Serial.println(F("Sending From"));   // change to your email address (sender)
  client.println(F("MAIL From: kkc6@vt.edu"));// not important 

  // Check for server response
  if (!eRcv())
    return 0;   
  
  Serial.println(F("Sending To"));
  client.println(F("RCPT To: kkc6@vt.edu")); // change to recipient address

  // Check for server response
  if (!eRcv())
    return 0;

  // Alert server that email is to follow
  Serial.println(F("Sending DATA"));
  client.println(F("DATA"));

  // Check for server response
  if (!eRcv())
    return 0;
    
  Serial.println(F("Sending email"));   
  client.println(F("To: kkc6@vt.edu"));   // change to your address
  client.println(F("From: kkc6@vt.edu")); // change to recipient address
  client.println(F("Subject: Warning - ESP8266 Temperature and Humidity Alert\r\n"));
 
  client.println(F("Warning: the temperature and/or humidity has moved outside the acceptable threshold."));
  client.println();
  
  client.print(F("Recommended temperature range (F): "));
  client.print(lowTemp);
  client.print(F(" to "));
  client.println(highTemp);
  client.print(F("Current temperature (F): "));
  client.println(t);
  
  client.print(F("Recommended humidity range (%): "));
  client.print(lowHum);
  client.print(F(" to "));
  client.println(highHum);
  client.print(F("Current humidity (%): "));
  client.println(h);
  client.println();
  
  client.println(F("We encourage you to remediate the temperature and/or humidity within a timely manner."));
  client.println(F("."));

  // Check for server response
  if (!eRcv())
    return 0;

  // Alert server that connection is ending
  Serial.println(F("Sending QUIT"));
  client.println(F("QUIT"));

  // Check for server response
  if (!eRcv())
    return 0;
  
  client.stop();
  Serial.println(F("disconnected"));
  return 1;
}

/**
 * Checks for server response to ensure that message was receievd correctly and was accurate
 * 
 * @return 1 if successful, 0 otherwise
 */
byte eRcv()
{
  byte respCode;
  byte thisByte;
  int loopCount = 0;
  
  while (!client.available())
  {
    delay(1);
    loopCount++;     // if nothing received for 10 seconds, timeout
    if (loopCount > 10000)
    {
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
    return 0;
  }
  return 1;
}
