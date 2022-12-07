#include <WiFi.h>
#include "ThingSpeak.h"
#include <DFRobot_MAX30102.h>
#include <LiquidCrystal_I2C.h>
#include "DHT.h"

#define DHTPIN 4 

#define DHTTYPE DHT11

DHT dht(DHTPIN, DHTTYPE);

DFRobot_MAX30102 particleSensor;

LiquidCrystal_I2C lcd(0x3F,16,2);




const char* ssid = "your wifi name";   // your network SSID (name) 
const char* password = "your wifi password";   // your network password

WiFiClient  client;

unsigned long myChannelNumber = xxxxxxxxx;
const char * myWriteAPIKey = "xxxxxxxxxxxxxxxxxxxxxxx";

// Timer variables
unsigned long lastTime = 0;
unsigned long timerDelay = 1000;


 
#define LM35_Sensor1    35

 
 

 
void setup()
{
  Serial.begin(115200);

  
  dht.begin();

    

  lcd.init();         // initialize the lcd

  lcd.backlight();    // open the backlight

  WiFi.mode(WIFI_STA);   

   // Connect or reconnect to WiFi
    if(WiFi.status() != WL_CONNECTED){
      Serial.print("Attempting to connect");
      while(WiFi.status() != WL_CONNECTED){
        WiFi.begin(ssid, password); 
        delay(5000);     
      } 
      Serial.println("\nConnected.");
    }
  
  ThingSpeak.begin(client);  // Initialize ThingSpeak

  while (!particleSensor.begin()) {
    lcd.print("MAX30102 was not found");
    Serial.println("MAX30102 was not found");
    delay(100);
  }

    particleSensor.sensorConfiguration(/*ledBrightness=*/50, /*sampleAverage=*/SAMPLEAVG_4, \
                        /*ledMode=*/MODE_MULTILED, /*sampleRate=*/SAMPLERATE_100, \
                        /*pulseWidth=*/PULSEWIDTH_411, /*adcRange=*/ADCRANGE_16384);

}
 
void loop()
{


  

  if ((millis() - lastTime) > timerDelay) {
    
   

  // Get the voltage reading from the LM35
  int reading = analogRead(LM35_Sensor1);

  // Convert that reading into voltage
  float voltage = reading * (2.0 / 1024.0);

  // Convert the voltage into the temperature in Celsius
  float temperatureC = voltage * 25;

  int32_t SPO2; //SPO2
  int8_t SPO2Valid; //Flag to display if SPO2 calculation is valid
  int32_t heartRate; //Heart-rate
  int8_t heartRateValid; //Flag to display if heart-rate calculation is valid 

  // Reading temperature or humidity takes about 250 milliseconds!
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  float h = dht.readHumidity();
  // Read temperature as Celsius (the default)
  float t = dht.readTemperature();
  // Read temperature as Fahrenheit (isFahrenheit = true)
  float f = dht.readTemperature(true);

   // Check if any reads failed and exit early (to try again).
  if (isnan(h) || isnan(t) || isnan(f)) {
    Serial.println(F("Failed to read from DHT sensor!"));
    return;
  }

  // Compute heat index in Fahrenheit (the default)
  int hif = dht.computeHeatIndex(f, h);
  // Compute heat index in Celsius (isFahreheit = false)
  float hic = dht.computeHeatIndex(t, h, false);

  Serial.println(F("Wait about four seconds"));
  particleSensor.heartrateAndOxygenSaturation(/**SPO2=*/&SPO2, /**SPO2Valid=*/&SPO2Valid, /**heartRate=*/&heartRate, /**heartRateValid=*/&heartRateValid);
  //Print result 
  Serial.print(F("heartRate="));
  Serial.print(heartRate, DEC);
  Serial.print(F("|"));  
  
  Serial.print(F("|"));
  delay(1000);
  Serial.print(F("SPO2="));
  Serial.print(hif, DEC);
  Serial.print(F("|"));
  
  Serial.println(F("|"));
  delay(1000);

    // Print the temperature in Celsius
  Serial.print("Body Temperature: ");
  Serial.print(temperatureC);
  Serial.println("c |");
  delay(1000);

  Serial.print(F("Room Humidity: "));
  Serial.print(h);
  Serial.println("% |");  
  delay(1000);
  Serial.print(F("Room Temperature: "));
  Serial.print(f);
  Serial.println(F("F |"));
  delay(1000);
  
  Serial.print(F(" Heat index: "));
  Serial.print(hic);
  Serial.print(F("C "));
  
  delay(1000);
  

  delay(1000); // wait a second between readings

  if (isnan(temperatureC) || isnan(heartRate) || isnan(SPO2) || isnan(h) || isnan(f)) {
    lcd.setCursor(0, 0);
    lcd.print("Failed");
  } else {
    lcd.setCursor(1, 1);  // display position
    lcd.print("BodyTemp ");
    lcd.print(temperatureC);     // display the temperature
    lcd.print("c");
    delay(2000);
    lcd.clear();

    lcd.setCursor(2, 1);  // display position
    lcd.print("HR ");
    lcd.print(heartRate);      // display the humidity
    lcd.print("BPM");
    delay(2000);
    lcd.clear();
    
    lcd.setCursor(1, 1);  // display position
    lcd.print("SPO2 ");
    lcd.print(hif);      // display the humidity
    lcd.print("%");
    delay(2000);
    lcd.clear();

    lcd.setCursor(2, 1);  // display position
    lcd.print("RoomHum ");
    lcd.print(h);      // display the humidity
    lcd.print("%");
    delay(2000);
    lcd.clear();

    lcd.setCursor(1, 1);  // display position
    lcd.print("RoomTemp ");
    lcd.print(f);      // display the humidity
    lcd.print("f");
    delay(2000);
    lcd.clear();

  }

  delay(1000);

 


 // set the fields with the values
    ThingSpeak.setField(1, temperatureC);
    ThingSpeak.setField(2, heartRate);
    ThingSpeak.setField(3, SPO2);
    ThingSpeak.setField(4, h);
    ThingSpeak.setField(5, f);
    
    
    
    // Write to ThingSpeak. There are up to 8 fields in a channel, allowing you to store up to 8 different
    // pieces of information in a channel.  Here, we write to field 1.
    int x = ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey);

    if(x == 200){
      Serial.println("Channel update successful.");
    }
    else{
      Serial.println("Problem updating channel. HTTP error code " + String(x));
    }
    lastTime = millis();
    
  }
  
}
