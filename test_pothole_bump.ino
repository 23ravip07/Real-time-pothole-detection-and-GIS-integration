
#include <WiFi.h>
#include <HTTPClient.h>
#include <TinyGPS++.h>
TinyGPSPlus gps;

#define RXD2 16
#define TXD2 17
HardwareSerial neogps(1);

//ENTER_GOOGLE_DEPLOYMENT_ID
const char * ssid = "OnePlus 8 Pro";
const char * password = "21032002";
String GOOGLE_SCRIPT_ID = "AKfycbx8HuFvvGxDYHje-Hcrm7rO8t3gVxQ5pkGDCDg12pKHawsaKq3W5RRsPjrQXQqdVY2D";


const int sendInterval = 2000;

//for ultrasonic sensor
const int trigPin = 5;
const int echoPin = 18;

//define sound speed in cm/uS
#define SOUND_SPEED 0.034
#define CM_TO_INCH 0.393701

long duration;
float distanceCm;


 void setup() {

  Serial.begin(115200);
  //start serial communication with Serial Monitor
  
  //start serial communication with Neo6mGPS
  neogps.begin(9600, SERIAL_8N1, RXD2, TXD2);
  delay(10);
  
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  Serial.print("Connecting to WiFi: ");
  Serial.println(ssid);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  // Print the IP address
  Serial.println("");
  Serial.println("Wi-Fi connected!");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  // for ultrasonic sensor
  pinMode(trigPin, OUTPUT); // Sets the trigPin as an Output
  pinMode(echoPin, INPUT); // Sets the echoPin as an Input
  
}

//for ultrasonic sensor
float ultrasonic(){
  // Clears the trigPin
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  // Sets the trigPin on HIGH state for 10 micro seconds
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  
  // Reads the echoPin, returns the sound wave travel time in microseconds
  duration = pulseIn(echoPin, HIGH);
  
  // Calculate the distance
  distanceCm = duration * SOUND_SPEED/2;
  
  // // Convert to inches
  // distanceInch = distanceCm * CM_TO_INCH;

  return distanceCm;
  
}

void loop() {

  float distance =ultrasonic();
  
  // Prints the distance in the Serial Monitor
  Serial.print("Distance (cm): ");
  Serial.println(distance);
    
  boolean newData = false;
  for (unsigned long start = millis(); millis() - start < 1000;)
  {
    while (neogps.available())
    {
      if (gps.encode(neogps.read()))
        {newData = true;}
    }
  }

  //If newData is true
  if(newData == true)
  {
    newData = false;
    Serial.println(gps.satellites.value());
    print_speed(distance);
  }
  else
  {
    Serial.println("No new data is received.");
  }  

  //delay(sendInterval);
}


// loop function starts

void print_speed(float distance)
{
  if (gps.location.isValid() == 1)
  {
    //String gps_speed = String(gps.speed.kmph());
    Serial.println(gps.location.lat(),6);
    Serial.println(gps.location.lng(),6);
    Serial.println(gps.speed.kmph());
    Serial.println(gps.satellites.value());
    Serial.println(gps.altitude.meters(), 0);

    if (distance < 12) { // Speed breaker
      String param;
      param  = "latitude="+String(gps.location.lat(), 6);
      param += "&longitude="+String(gps.location.lng(), 6);
      param += "&speed="+String(gps.speed.kmph());
      param += "&satellites="+String(gps.satellites.value());
      param += "&altitude="+String(gps.altitude.meters());
      param += "&gps_time="+String(gps.time.value());
      param += "&gps_date="+String(gps.date.value());

      Serial.println(param);
      write_to_google_sheet(param);
    }
    else if (distance > 20) { // Pothole
      String param;
      param  = "latitude="+String(gps.location.lat(), 6);
      param += "&longitude="+String(gps.location.lng(), 6);
      param += "&speed="+String(gps.speed.kmph());
      param += "&satellites="+String(gps.satellites.value());
      param += "&altitude="+String(gps.altitude.meters());
      param += "&gps_time="+String(gps.time.value());
      param += "&gps_date="+String(gps.date.value());

      Serial.println(param);
      write_to_google_sheet(param);
    }
  }
  else
  {
    Serial.println("No any valid GPS data.");
  }  
}



// loop function starts
void write_to_google_sheet(String params) {
   float distance =ultrasonic();
   HTTPClient http;
   String url="https://script.google.com/macros/s/"+GOOGLE_SCRIPT_ID+"/exec?" + params + "&distance=" + String(distance);  
  //  + "&distance=" + String(distance);
   //Serial.print(url);
    Serial.println("Postring GPS data to Google Sheet");
    
    //starts posting data to google sheet
    http.begin(url.c_str());
    http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);
    int httpCode = http.GET();  
    Serial.print("HTTP Status Code: ");
    Serial.println(httpCode);
    
    //getting response from google sheet
    String payload;
    if (httpCode > 0) {
        payload = http.getString();
        Serial.println("Payload: "+payload);     
    }
    
    http.end();
}