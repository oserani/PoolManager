#include <DFRobot_ORP_PRO.h>
#include <Adafruit_SSD1351.h>
#include <SPI.h>
#include "MemoryFree.h"
//#include <gfxfont.h>      //do we really need this one? What about the other adafruitgx one
#include <time.h>
#include <TimeLib.h>       //time conversions... how does this jive with regular time.h?
#include "secrets.h"      //credentials (WiFi, MQTT hosts, etc)
//PMUtility.ino is compiled along with this file.  All utility validated routines are moved there.  This only works with the IDE taking care of stuff.  Else, need to do includes of .cpp and .h files

#include <ArduinoMqttClient.h>
//includes the appropriate WiFi NINA library for the arduino
#if defined(ARDUINO_SAMD_MKRWIFI1010) || defined(ARDUINO_SAMD_NANO_33_IOT) || defined(ARDUINO_AVR_UNO_WIFI_REV2)
  #include <WiFiNINA.h>
#elif defined(ARDUINO_SAMD_MKR1000)
  #include <WiFi101.h>
#elif defined(ARDUINO_ARCH_ESP8266)
  #include <ESP8266WiFi.h>
#elif defined(ARDUINO_PORTENTA_H7_M7) || defined(ARDUINO_NICLA_VISION) || defined(ARDUINO_ARCH_ESP32) || defined(ARDUINO_GIGA)
  #include <WiFi.h>
#endif

//Serial debugging on/off                //there may be some other ways to optimize this, or make it more elegant/best practice.  Idea is to be able to debug tricky sections as needed (i.e. MQTT)
#define SERIALDEBUGGING       false     //generic serial debugging
#define SERIALDEBUGGINGISR    false     //dedicated to debug display and interrupt service request
#define SERIALDEBUGGINGpH     false     //dedicated to debug pH measuring as library and sensor are a little funky, and smoothing out of the values
#define SERIALDEBUGGINGORP    false     //dedicated to debug ORP measuring and smoothing out of the values
#define SERIALDEBUGGINGMQTT   false     //dedicated to debug MQTT
#define SERIALDEBUGGINGRELAYS false     //dedicated to valve and relays flipping, delaying

//MQTT stuff (using self-hosted on Synology DOCKER)
#define MQTT_INTERVAL 10000    //how often should it send data to the MQTT queue  
#define MQTT_retry    5000    //how often should it retry to reconnect if dropped.  Hasn't ever worked
WiFiClient MyWiFiClient;              //instantiated from WiFiNINA.h
MqttClient MymqttClient(MyWiFiClient);  //instantiated from ArduinomqttClient.h
const char broker[]           = MY_MQTT_HOST;
int port                      = MY_MQTT_PORT;
const char receivetopic[]     = MY_MQTT_RX;
const char sendtopic[]        = MY_MQTT_TX;
const char sendtopicCtemp[]   = MY_MQTT_TX_CTEMP;
const char sendtopicPtemp[]   = MY_MQTT_TX_PTEMP;
const char sendtopicValve[]   = MY_MQTT_TX_VALVE;
const char sendtopicpH[]      = MY_MQTT_TX_PH;
const char sendtopicORP[]     = MY_MQTT_TX_ORP;
const char sendtopicReed[]    = MY_MQTT_TX_REED;
const char sendtopicCount[]   = MY_MQTT_TX_COUNT;
const char sendtopicMem[]     = MY_MQTT_TX_MEM;
unsigned long previousMQTTmillis = 0;
unsigned long LastMQTTAttempt = 0;
//loop counter and used in some for displaying MQTT messages since startup.  Need to keep global as the value is displayed in a couple of places
int count = 0;

//Temperature Vars
float TempPool = 0; //need to globally expose this one as it's needed in the pH Calibration for temp

//Relay and valve handling variables, including relay delay to avoid false firing
//Booleans and millis longs to handle relay delay
//do we need all this stuff to manage a simple delay on the relay?  Maybe? Becoming a state machine?
//Can we use the state of the digital pins on the relay to reliable know their status?
unsigned long RelayStartTime = 0;
unsigned long RelayElapsedTime = 0;
const long Relay_delay = 15000;
bool Relay_delay_finished = false; 
bool Heat_mode = false; //use this as a semaphore to simplify the wait x secons for temp to settle and avoid unncessary checks.

//OLED Stuff
// Screen dimensions
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 128  // Change this to 96 for 1.27" OLED.
//OLED Pin Definitions
#define OLED_CS 13
#define OLED_DC 12  //#define OLED_RES 8 On the Arduino UNO WIFI Rev 2 using RST on the ICSP header.  Magically figures it out?
//OLED Color definitions - can I use a PROGMEM trick here?
#define BLACK   0x0000
#define BLUE    0x001F
#define RED     0xF800
#define GREEN   0x07E0
#define CYAN    0x07FF
#define MAGENTA 0xF81F
#define YELLOW  0xFFE0
#define WHITE   0xFFFF
// Initial text and background colors.  Flipping back and forth alllows you to print old value ("erasing") and then paint new value
uint16_t OLED_Text_Color = WHITE;
uint16_t OLED_Background_Color = BLACK;

//Relay Module
#define Relay_1 4
#define Relay_2 5
#define Relay_3 6
#define Relay_4 7
#define Relay_5 8
#define Relay_6 9
#define Relay_7 10
#define Relay_8 11

//Reed switch to detect closure/position/proximity of something - say a valve handle for example
#define REED_PIN 3
//#define LED_PIN   //find out how to use LED_BUILTIN to make it more portable

//Button, screen blanking handling, clock  
// assume the display is off until configured in setup()
#define MILLIS_BLANK_SCREEN_BLANK 600000  //180 second blank after button is not pressed
#define screen_blank_enabled true         //set to false during debugging/troubleshooting
#define TZOFFSET               -3         //Timezone offset -3 Chile summer time/US winter time
#define DISPLAYREFRESHRATE    15000        //Measure temp, pH, ORP every xx millis (seconds). Whatever is inside the if check in the main loop is subservient to this rate
bool isDisplayVisible = false;
// connect a push button to ...
const uint8_t Button_pin = 2;
// the interrupt service routine affects this so the variable needs to be volatile, so it's not optimized away by the compiler, or cached
volatile bool isButtonPressed = false;
// millis vars for screen blanking
unsigned long previousMillisDisplay = 0;  //global var for screen blanking
unsigned long previousRefreshMillis = 0;  //global var for timing of the measuring loops
//RTC stuff - instantiate rtc and a buffer to hold time strings - not using RTC, using Network time 
//#include <RTClib.h>  //Real time clock
//RTC_DS3231 rtc;
/*
  //start RTC time and wire stuff
  rtc.begin();
  Wire.begin();
  // clear /EOSC bit to ensure that the clock runs on battery power - read that this is good practice, but....
  Wire.beginTransmission(0x68);  // address DS3231
  Wire.write(0x0E);              // select register
  Wire.write(0b00011100);        // write register bitmap, bit 7 is /EOSC
  Wire.endTransmission();
  //Following line needs to be run only the first time you set the time.  Commented after that.
  //rtc.adjust(DateTime(F(__DATE__),F(__TIME__)));
*/


//Buffers that need to be kept global from the measuring routines.  Mainly to decide whether to redraw on the OLED
// declare size of working string buffers, account for null terminated string
const size_t MaxString = 6;       //Should handle temps and. plus null
const size_t MaxStringValve = 4;  //"ON ", "OFF". plus null
const size_t MaxStringpH = 10;     // for pH xx.xx plus null.  However, it apparently needs 8 because there's some junk being written on display.  Investigate
// the string being displayed on the SSD1331 (initially empty)
char OldTempCollString[MaxString] = { 0 };
char OldTempPoolString[MaxString] = { 0 };
char OldValveString[MaxStringValve] = { 0 };
char OldpHString[MaxStringpH] = { 0 };
char OldORPString[MaxStringpH] = { 0 };

//Instantiate OLED from Adafruit_SSD1351.  Waveshare has their own libraries, .h, .cpp and sample codes but they are almost unreadable
//Adafruit_SSD1351 OLED = Adafruit_SSD1351(OLED_CS, OLED_DC, OLED_RES);
//Again assuming it only needs CS and DC to operate
//My understanding here is that I am using the SPI Hardware pins that should make this much faster (See 13451 example commentary)
//Don't know if I will ever try the native library that came from waveshare.  It seems unreadable from a common sense POV
Adafruit_SSD1351 OLED = Adafruit_SSD1351(SCREEN_WIDTH, SCREEN_HEIGHT, &SPI, OLED_CS, OLED_DC);  //should take 6-7 params in new projects....???

//Thermistor and temperature stuff
// which analog pin to connect
// for future improvement:  try running the voltage dividers off the 3.3V pin, connecting AREF to 3.3V and declaring the external AREF in code (See notebook)
#define THERMISTORPIN1 A0  // wired to solar collector thermistor.  Thermistor is not polarized but use consistent red/white to keep things clear
#define THERMISTORPIN2 A1  // wired to pool thermistor.  Thermistor is not polarized but use consistent red/white to keep things clear
// resistance at 25 degrees C
#define THERMISTORNOMINAL 10000
// temp. for nominal resistance (almost always 25 C)
#define TEMPERATURENOMINAL 25
// how many samples to take and average, more takes longer
// but is more 'smooth'
#define NUMTEMPSAMPLES 20
// The beta coefficient of the thermistor (usually 3000-4000.  Get actual number from datasheet.  Usually coded something like NTC specification: 10K3977 1%
#define BCOEFFICIENT 3977
// the value of the fixed resistor used in the thermistor voltage divider.  Their resistances are the same, usually 10K or 100K
#define SERIESRESISTOR 10000
int samples[NUMTEMPSAMPLES];
//May have to be tweaked.  Using 2 degrees celsius for starters.  Needs to be modified, particularly, if logic changes to Fahrenheit
//On temp we're doing just a regular average.  On pH we're being a bit fancier and doing an average excluding outliers on both ends of a sorted array
#define TEMPDELTAMIN 2     //Minimum delta between temperature at solar collector and pool to trigger the opening of the heating valve. This is done in Celsius, regardless of below.  Adjust accordingly
#define TEMPDISPDEGF true  //If set to false, degrees are displayed in Celsius, if true in Fahrenheit.  All temp calcs are done in C/K regardles to stay in line with Steinhart and thermistor specs
//pH Measuring tweaking parameters
#define NUMPHSAMPLES 20
//On ORP we're just being lazy again and doing a moving average, just like on temp
#define NUMORPSAMPLES 20

//pH Meter - DFR Robot Gravity Sensor v2
#include "DFRobot_PH.h"

//ORP Meter - DFR Robot Gravity ORP SEnsor Pro
#include "DFRobot_ORP_PRO.h"


//Setup: do things once.
void setup() {
  // button press pulls pin LOW so configure HIGH/PULLUP
  pinMode(Button_pin, INPUT_PULLUP);
  // use an interrupt to sense when the button is pressed
  attachInterrupt(digitalPinToInterrupt(Button_pin), senseButtonPressed, FALLING);
  
  //setup reeed switch

  pinMode(REED_PIN, INPUT_PULLUP);
  pinMode(LED_BUILTIN, OUTPUT);


//turn on serial port for debugging - general and surgical
#if (SERIALDEBUGGING)
  Serial.begin(115200);
  while (!Serial)
    ;
  Serial.println();
#endif
#if (SERIALDEBUGGINGISR)
  Serial.begin(115200);
  while (!Serial)
    ;
  Serial.println();
#endif
#if (SERIALDEBUGGINGMQTT)
  Serial.begin(115200);
  while (!Serial)
    ;
  Serial.println();
#endif
#if (SERIALDEBUGGINGRELAYS)
  Serial.begin(115200);
  while (!Serial)
    ;
  Serial.println();
#endif
  
  Serial.begin(115200);  //for any ad-hoc Serial.print()s

  // settling time for button power-on-reboot noise
  delay(250);
  isButtonPressed = false;
  SPI.begin();
  OLED.begin();
  // the display is now on
  isDisplayVisible = true;
  //WiFi Connection and Status
  SetupWiFi();
  //Setup static OLED characters
  SetupOLED();
  //Do anything you may need to bring relays to known state (like all off)
  SetupRelays();
  //connect to MQTT broker
  SetupMQTT();
  }


//loop: do things forever
/* includes a couple of  "learned concepts":
  millis() counting
  interrupt service requests including volatile boolean
  flipping boolean as semaphores to reflect physical state of hardware
  using #if #endif preprocessor parameters to inject broad and/or narrow debugging  
*/

void loop() 
{
  /*
  Four  independent speeds for loop
  Speed of loop
  Millis MQTT publishing
  Millis refresh display
  Millis perform measurements
*/

  
  
  //call display routines only if display interval has elapsed // these could be in the order of 15 minutes in commercial monitors / update time display on exit
  unsigned long currentRefreshMillis = millis();
  if (currentRefreshMillis - previousRefreshMillis >= DISPLAYREFRESHRATE) {
    MymqttClient.poll();
    measure_pH();
    MymqttClient.poll();
    measure_ORP();
    MymqttClient.poll();
    MeasureTemps();
    MymqttClient.poll();
    paint_UNIX_time();
    previousRefreshMillis = currentRefreshMillis;
  }
  
    MymqttClient.poll(); //poll MQTT often
  
  //call MQTT routines only if MQTT interval has elapsed.  These could be in the order of ms/secs in IOT applications
  unsigned long currentMQTTmillis = millis();
  char pub_payload[50] = { 0 };
    #if (SERIALDEBUGGINGMQTT)
    Serial.print(F("Pub interval MQTT:"));
    Serial.println(currentMQTTmillis - previousMQTTmillis);
    #endif 
  
  if (currentMQTTmillis - previousMQTTmillis >= MQTT_INTERVAL) {
    OLED.fillRect(75, 32, 40, 14, BLACK);  //create black bacground for count update
    MymqttClient.poll(); 
    sprintf(pub_payload, "C:%s P:%s V:%s pH:%s ORP:%s N:%i", OldTempCollString, OldTempPoolString, OldValveString, OldpHString, OldORPString, count);
    Pub_MQTT_args(sendtopic, pub_payload);                //send a relatively complete parsable string to main topic
    
    MymqttClient.poll(); 
    Pub_MQTT_args(sendtopicCtemp, OldTempCollString);     //send collector value to collector topic
    Pub_MQTT_args(sendtopicPtemp, OldTempPoolString);     //send pool value to pool topic
    
    MymqttClient.poll(); 
    Pub_MQTT_args(sendtopicValve, OldValveString);        //send valve status to valve topic
    
    MymqttClient.poll(); 
    Pub_MQTT_args(sendtopicpH,    OldpHString);           //send pH value to pH topic
    Pub_MQTT_args(sendtopicORP,   OldORPString);          //send ORP value to ORP topic
    
    MymqttClient.poll(); 
    sprintf(pub_payload, "%i", count);Pub_MQTT_args(sendtopicCount, pub_payload); //send count to count topic
   
    MymqttClient.poll(); 
    sprintf(pub_payload, "%i", freeMemory());Pub_MQTT_args(sendtopicMem, pub_payload); //send free memory mem topic
    Pub_MQTT_args(sendtopicMem, pub_payload); // log memory to check for leaks
    
    count++;
    previousMQTTmillis = currentMQTTmillis;
    MymqttClient.poll(); 
  }
  
  //These functions are executed at the speed of the microprocessor through loop(), no millis() counters
  
  OLED.setCursor (75,38);         //return OLED to normal text state
  OLED.setTextSize(1);
  OLED.print (count);
  OLED.setTextSize (2);
  OLED.setTextColor(OLED_Text_Color);
  

//Check MQTT status at speed of loop
  if (Check_MQTT_status()){
    OLED.fillRect(55, 32, 14, 14, GREEN);  // check health of MQYY connection
  } else {
    OLED.fillRect(55, 32, 14, 14, RED);  // check health of MQYY connection and do something.....
    MQTT_Reconnect();
  }
  MymqttClient.poll();            //poll often

  
  sprintf(pub_payload, "%s", CheckReed() ? "true" : "false"); Pub_MQTT_args(sendtopicReed, pub_payload); //Send reed switch status to Reed topic
  MymqttClient.poll(); 
  CheckButtonPress();             //Check that the phyiscal button hasn't been pressed
}

