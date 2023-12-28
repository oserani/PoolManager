void MeasureTemps() {
  //allocate local buffers for working and printing temps -- temps here -- valve further down as we may cut up the function
  char NewTempCollString[MaxString] = { 0 };
  char NewTempPoolString[MaxString] = { 0 };
  uint8_t i;
  float average_coll;
  float average_pool;

  // take N samples in a row from collector thermistor, with a slight delay - took the delay off from the example 
  for (i = 0; i < NUMTEMPSAMPLES; i++) {
    samples[i] = analogRead(THERMISTORPIN1);
    //delay(10);
  }
  // average all the samples out from collector thermistor
  average_coll = 0;
  for (i = 0; i < NUMTEMPSAMPLES; i++) {
    average_coll += samples[i];
  }
  average_coll /= NUMTEMPSAMPLES;

  // take N samples in a row from pool thermistor, with a slight delay - took delay off from the example
  for (i = 0; i < NUMTEMPSAMPLES; i++) {
    samples[i] = analogRead(THERMISTORPIN2);
    //delay(10);
  }
  // average all the samples out from pool thermistor
  average_pool = 0;
  for (i = 0; i < NUMTEMPSAMPLES; i++) {
    average_pool += samples[i];
  }
  average_pool /= NUMTEMPSAMPLES;

  #if (SERIALDEBUGGING)
  Serial.print(F("Average coll analog reading "));
  Serial.println(average_coll);
  Serial.print(F("Average pool analog reading "));
  Serial.println(average_pool);
  #endif

  // convert the values to resistance
  average_coll = 1023 / average_coll - 1;
  average_coll = SERIESRESISTOR / average_coll;
  average_pool = 1023 / average_pool - 1;
  average_pool = SERIESRESISTOR / average_pool;
    #if (SERIALDEBUGGING)
    Serial.print(F("Thermistor coll resistance "));
    Serial.println(average_coll);
    Serial.print(F("Thermistor pool resistance "));
    Serial.println(average_pool);
    #endif

  //perform steinhart conversions on both collector and pool thermistor values
  float steinhart_coll;
  steinhart_coll = average_coll / THERMISTORNOMINAL;      // (R/Ro)
  steinhart_coll = log(steinhart_coll);                   // ln(R/Ro)
  steinhart_coll /= BCOEFFICIENT;                         // 1/B * ln(R/Ro)
  steinhart_coll += 1.0 / (TEMPERATURENOMINAL + 273.15);  // + (1/To)
  steinhart_coll = 1.0 / steinhart_coll;                  // Invert
  steinhart_coll -= 273.15;                               // convert absolute temp to C
  

  float steinhart_pool;
  steinhart_pool = average_pool / THERMISTORNOMINAL;      // (R/Ro)
  steinhart_pool = log(steinhart_pool);                   // ln(R/Ro)
  steinhart_pool /= BCOEFFICIENT;                         // 1/B * ln(R/Ro)
  steinhart_pool += 1.0 / (TEMPERATURENOMINAL + 273.15);  // + (1/To)
  steinhart_pool = 1.0 / steinhart_pool;                  // Invert
  steinhart_pool -= 273.15;                               // convert absolute temp to C
  
  TempPool = steinhart_pool;                              // need to expose this one to make it avaiable to others, particularly for pH calibration

#if (SERIALDEBUGGING)
  Serial.print(F("Temperature Coll "));
  Serial.print(steinhart_coll);
  Serial.println(" *C");
  Serial.print(F("Temperature Pool "));
  Serial.print(steinhart_pool);
  Serial.println(F(" *C"));
#endif

  //convert both steinhart values to usable strings
  dtostrf(steinhart_coll, 3, 1, NewTempCollString);
  dtostrf(steinhart_pool, 3, 1, NewTempPoolString);
// change the text color to foreground color

//convert them to Fahrenheit if the #TEMPDISFDEG is true
#if (TEMPDISPDEGF)
  dtostrf(((steinhart_coll)*1.8000 + 32), 3, 1, NewTempCollString);
  dtostrf(((steinhart_pool)*1.8000 + 32), 3, 1, NewTempPoolString);
#endif


  //Has the temp coll string value changed? If yes, rease by redrawing reverse colors
  if (strcmp(NewTempCollString, OldTempCollString) != 0) {
      #if (SERIALDEBUGGING)
      Serial.print(F("Entered strcmp - Old Temp Coll String = "));
      Serial.print(OldTempCollString);
      Serial.print(F(" "));
      Serial.print(F("New Temp Coll String = ")_;
      Serial.print(NewTempCollString);
      Serial.print(F(" Strcmp New vs Old yields: "));
      Serial.println(strcmp(NewTempCollString, OldTempCollString));
      #endif
    //home the cursor
    OLED.setCursor(55, 0);
    // change the text color to the background color
    OLED.setTextColor(OLED_Background_Color);
    // redraw the old value to erase
    OLED.print(OldTempCollString);
    // home the cursor
    OLED.setCursor(55, 0);
    // change the text color to foreground color
    OLED.setTextColor(OLED_Text_Color);
    // draw the newvalue
    OLED.print(NewTempCollString);
      #if (SERIALDEBUGGING)
      Serial.print(F("Exiting strcmp - Old Temp Coll String = "));
      Serial.print(OldTempCollString);
      Serial.print(F("   New Temp Coll String = "));
      Serial.print(NewTempCollString);
      Serial.print(F(" Strcmp New vs Old yields: "));
      Serial.println(strcmp(NewTempCollString, OldTempCollString));
      #endif
  }
  //remember the new value
  strcpy(OldTempCollString, NewTempCollString);

  //Has the temp pool string value changed? If yes, rease by redrawing reverse colors
  if (strcmp(NewTempPoolString, OldTempPoolString) != 0) {
    #if (SERIALDEBUGGING)
    Serial.print(F("Entered strcmp - Old Temp Pool String = "));
    Serial.print(OldTempPoolString);
    Serial.print(F(" New Temp Pool String = "));
    Serial.print(NewTempPoolString);
    Serial.print(F(" Strcmp New vs Old yields: "));
    Serial.println(strcmp(NewTempPoolString, OldTempPoolString));
    #endif
    //home the cursor
    OLED.setCursor(55, 16);
    // change the text color to the background color
    OLED.setTextColor(OLED_Background_Color);
    // redraw the old value to erase
    OLED.print(OldTempPoolString);
    // home the cursor
    OLED.setCursor(55, 16);
    // change the text color to foreground color
    OLED.setTextColor(OLED_Text_Color);
    // draw the new value
    OLED.print(NewTempPoolString);
      #if (SERIALDEBUGGING)
      Serial.print("Exiting strcmp - Old Temp Pool String = ");
      Serial.print(OldTempPoolString);
      Serial.print(F("   New Temp Pool String = "));
      Serial.print(NewTempPoolString);
      Serial.print(F(" Strcmp New vs Old yields: "));
      Serial.println(strcmp(NewTempPoolString, OldTempPoolString));
      #endif
  }
  //remember the new value
  strcpy(OldTempPoolString, NewTempPoolString);

/* Valve stuff candidate for separate function

*/

  char NewValveString[MaxStringValve] = { 0 };
  float temp_delta = 0;
  bool valve_status = false;
  
  //check whether delta between collector and pool exceeds threshold #defined
  //Need to figure out an minimal time that delta has been sustained so that the relays don't bounce - some millis loop - rounding doesn't cut it
  temp_delta = (round(steinhart_coll) - round(steinhart_pool));  //the round is a cheat to take most of the relay bounce out
  //previous millis logic here
  if (temp_delta > TEMPDELTAMIN)  {
    valve_status = true;
    strcpy(NewValveString, "ON ");
    //digitalWrite(Relay_1,LOW);   //need to figure out how to take the noise when it's close enough so the relay doesn't bounce
    //OLED.fillRoundRect(55, 55, OLED.width()/9, OLED.width()/9, 5, YELLOW); // display valve 1 requested icon - relay not on yet
    OLED.fillRoundRect(55, 55, OLED.width()/9, OLED.width()/9, 5, RED); //display valve 1 requested icon - RELAY DELAY WAITING
      
  } else {
    valve_status = false;
    strcpy(NewValveString, "OFF");
    //digitalWrite(Relay_1,HIGH);    //need to figure out how to take the noise when it's close enough so the relay doesn't bounce
    //OLED.fillRoundRect(55, 55, OLED.width()/9, OLED.width()/9, 5, CYAN); //display valve 1 requested icon - relay not off yet
     OLED.fillRoundRect(55, 55, OLED.width()/9, OLED.width()/9, 5, CYAN); //display valve 1 requested icon - RELAY DELAY WAITING

  
      
  } 

// May send all relay management (delays, multiple relay patterns firing, etc. to a separate function)

  if(valve_status && !Heat_mode) {
    // valve requests heat, but physical valve is set to COLD, need to TURN ON relay after 15 seconds and if the HEAT request is still there
    OLED.fillRoundRect(55, 55, OLED.width()/9, OLED.width()/9, 5, WHITE); //display valve 1 requested icon - RELAY DELAY WAITING
      #if (SERIALDEBUGGINGRELAYS)
      Serial.print(F("Valve REQUESTS HEAT but relay is OFF - valve_status heat_mode relay_done: ")); Serial.print(valve_status);Serial.print(Heat_mode); Serial.println(Relay_delay_finished);
      #endif
    if (!Relay_delay_finished) {
      if (RelayElapsedTime == 0) {
        RelayStartTime = millis();
        #if (SERIALDEBUGGINGRELAYS)
        Serial.print(F("Relay delay NOT finished and elapsed time is 0 (first loop):  ")); Serial.print(valve_status);Serial.println(Relay_delay_finished); 
        #endif
        OLED.fillRoundRect(55, 55, OLED.width()/9, OLED.width()/9, 5, YELLOW); //display valve 1 requested icon - RELAY DELAY WAITING
      }
      RelayElapsedTime = millis() - RelayStartTime;
      if(RelayElapsedTime >= Relay_delay) {
        // delay has elapsed
        Relay_delay_finished = true;
          #if(SERIALDEBUGGINGRELAYS)
          Serial.println(F("Relay delay elapsed.  Flip the delay_finished boolean"));
          #endif
      }
    }
  }
    if (Relay_delay_finished) {
      digitalWrite(Relay_1,LOW);  //tuns on the relay... check electronics on pins high/low, resistors?
      OLED.fillRoundRect(55, 55, OLED.width()/9, OLED.width()/9, 5, RED); // display valve 1 requested icon - RELAY NOW ON
      RelayStartTime = 0;
      RelayElapsedTime = 0;
      Relay_delay_finished = false; 
      Heat_mode = true;
        #if(SERIALDEBUGGINGRELAYS)
        Serial.println(F("Relay TURNED ON.  counters and semaphores reset"));
        #endif
    }

  if (!valve_status && Heat_mode) {
    //valve requests cold, but physical valve is set to  HEAT, need to TURN OFF relay after 15 seconds and if the COLD request is still there
    OLED.fillRoundRect(55, 55, OLED.width()/9, OLED.width()/9, 5, WHITE); //display valve 1 requested icon - RELAY DELAY WAITING
      #if (SERIALDEBUGGINGRELAYS)
      Serial.print(F("Valve REQUESTS COLD but relay is ON - valve_status heat_mode relay_done: ")); Serial.print(valve_status);Serial.print(Heat_mode);Serial.println(Relay_delay_finished); 
      #endif
    if (!Relay_delay_finished) {
      if (RelayElapsedTime == 0) {
        RelayStartTime = millis();
          #if (SERIALDEBUGGINGRELAYS)
          Serial.print(F("Relay delay NOT finished and elapsed time is 0 (first loop):  ")); Serial.print(valve_status);Serial.println(Relay_delay_finished); 
          #endif
        OLED.fillRoundRect(55, 55, OLED.width()/9, OLED.width()/9, 5, YELLOW); //display valve 1 requested icon - RELAY DELAY WAITING
      }
      RelayElapsedTime = millis() - RelayStartTime;
      if (RelayElapsedTime >= Relay_delay) {
        Relay_delay_finished = true;
          #if (SERIALDEBUGGINGRELAYS)
          Serial.println(F("Relay delay elapsed.  Flip the delay_finished boolean"));
          #endif
      }
    } 
    if (Relay_delay_finished) {
      digitalWrite(Relay_1,HIGH);  //tuns off the relay... check electronics on pins high/low, resistors?
      OLED.fillRoundRect(55, 55, OLED.width()/9, OLED.width()/9, 5, CYAN); //display valve 1 requested icon - RELAY NOW OFF
      RelayStartTime = 0;
      RelayElapsedTime = 0;
      Heat_mode = false;
      Relay_delay_finished = false;
        #if(SERIALDEBUGGINGRELAYS)
        Serial.println(F("Relay TURNED OFF.  counters and semaphores reset"));
        #endif 
    }
  }


//and remember the new value
  strcpy(OldValveString, NewValveString);
}

void measure_pH() {
  //pH bounces down when the relay is powered on
  #define PH_PIN A2
  float voltage = 00.00, pHValue = 00.00, pH_buffer_arr[NUMPHSAMPLES], ph_average=0.0;
  char NewpHstring[MaxStringpH];
  DFRobot_PH pH;
  pH.begin();

  for (int i=0;i<NUMPHSAMPLES;i++) {
    voltage = analogRead(PH_PIN) / 1024.0 * 5000;  // read the voltage
    pHValue = pH.readPH(voltage, TempPool);     // convert voltage to pH with temperature compensation
    //pHValue = pH.readPH(voltage, 27);     // fudge to understand calibration
   
    pH_buffer_arr[i] = pHValue; 
    #if (SERIALDEBUGGINGpH)
    Serial.print(F("pH Array")); Serial.print(i); Serial.print(F(" : "));Serial.println(pH_buffer_arr[i]);
    #endif
  }
  
  ph_average=0.0; //reuse this to save
    for(int i=0;i<NUMPHSAMPLES;i++) {
      for(int j=i+1;j<NUMPHSAMPLES-2;j++) {
        if(pH_buffer_arr[i]>pH_buffer_arr[j]) {
        ph_average = pH_buffer_arr[i];
        pH_buffer_arr[i]=pH_buffer_arr[j];
        pH_buffer_arr[j]= ph_average;
        }
      }
      #if (SERIALDEBUGGINGpH)
      Serial.print(F("pH Array Sorted")); Serial.print(i); Serial.print(F(" : "));Serial.println(pH_buffer_arr[i]);
      #endif 
    }

  ph_average=0.0; //reuse this to save
  for (int i=2;i<(NUMPHSAMPLES-2);i++) {
    ph_average += pH_buffer_arr[i];
      #if (SERIALDEBUGGINGpH)
      Serial.print(F("pH center point ")); Serial.print(i); Serial.print(F(" : "));Serial.println(pH_buffer_arr[i]);
      #endif
  }

  //ph_average = ph_average/6; 
  ph_average = ph_average/(NUMPHSAMPLES-4);
    #if (SERIALDEBUGGINGpH)
    Serial.print(F("pH Average: "));Serial.println(ph_average);
    #endif 
  dtostrf(ph_average, 2, 1, NewpHstring);
    
  if (strcmp(NewpHstring, OldpHString) != 0) {
    //home the cursor
    OLED.setCursor(55, 75);
    // change the text color to the background color
    OLED.setTextColor(OLED_Background_Color);
    // redraw the old value to erase
    OLED.print(OldpHString);
    // home the cursor
    OLED.setCursor(55, 75);
    // change the text color to foreground color - let's try yellow for pH to make it a bit more fun or change colors on range
    if (ph_average < 7) {
      OLED.setTextColor(YELLOW);
    }
    if (ph_average > 7 && ph_average < 7.81) {
      OLED.setTextColor(GREEN);
    }
    // draw the new pH value
    OLED.print(NewpHstring);
    //return text color to assumed in the rest of the program
    OLED.setTextColor(OLED_Text_Color);
    }
    //and remember the new value
    strcpy(OldpHString, NewpHstring);
}


void measure_ORP() {
  char NewORPString[MaxStringpH];
  #define PIN_ORP A3
  #define ADC_RES 1024
  #define V_REF 5000
  float ADC_voltage, average_ADC, average_ORP;
  uint8_t i;
  DFRobot_ORP_PRO ORP(0);
  
  // take N samples in a row from ORP sensor
  for (i = 0; i < NUMORPSAMPLES; i++) {
    samples[i] = ((unsigned long)analogRead(PIN_ORP) * V_REF + ADC_RES / 2) / ADC_RES;
  }
  average_ADC = 0;
  //average them out - is there a better way to consolidate these two loops?
  for (i = 0; i < NUMORPSAMPLES; i++) {
    average_ADC += samples[i];
    #ifdef SerialDebuggingORP()
      Serial.print(F("ADC_voltage summed value is : "));
      Serial.print(average_ADC);
      Serial.print(F("mV  "   ));
      Serial.println(i);
    #endif
  }
  average_ADC /= NUMORPSAMPLES;
  //ADC_voltage = ((unsigned long)analogRead(PIN_ORP) * V_REF + ADC_RES / 2) / ADC_RES;
  
  dtostrf(ORP.getORP(round(average_ADC)),3,0, NewORPString);    
    #ifdef SerialDebuggingORP()
      Serial.print(F("ADC_voltage average value is : "));
      Serial.print(average_ADC);
      Serial.print(F("mV  "   ));
      Serial.print(F("Average ORP value is : "));
      Serial.print(ORP.getORP(average_ADC));
      Serial.println(F("mV"));
    #endif
    
  if (strcmp(NewORPString, OldORPString) != 0) {
    //home the cursor
    OLED.setCursor(55, 95);
    // change the text color to the background color
    OLED.setTextColor(OLED_Background_Color);
    // redraw the old value to erase
    OLED.print(OldORPString);
    // home the cursor
    OLED.setCursor(55, 95);
    // draw the new ORP value
    OLED.setTextColor(OLED_Text_Color);
    OLED.print(NewORPString);
    //return text color to assumed in the rest of the program
    OLED.setTextColor(OLED_Text_Color);
  }
    //and remember the new value
   
  strcpy(OldORPString, NewORPString);
}



// interrupt service routine
void senseButtonPressed() {
  if (!isButtonPressed) {
    isButtonPressed = true;
  }
}

void CheckButtonPress() {
  // has the button been pressed?
  // and get a millis() time mark regardless to calculate "screen saver blankout"
  unsigned long currentMillisDisplay = millis();
  if (isButtonPressed) {
    // yes! toggle display visibility
    isDisplayVisible = !isDisplayVisible;
    //and reset the timer;
    previousMillisDisplay = currentMillisDisplay;
    // apply display settings
    OLED.enableDisplay(isDisplayVisible);
    Pub_MQTT_args (sendtopic, "AR: Button Pressed");
#if (SERIALDEBUGGINGISR)
    Serial.print(F("button pressed @ "));
    Serial.print(millis());
    Serial.print(F(", display is now "));
    Serial.println((isDisplayVisible ? "ON" : "OFF"));
#endif
    // confirm button handled
    isButtonPressed = false;
  }
#if (SERIALDEBUGGINGISR)
  Serial.print(F("Checking screen blanking requirement"));
  Serial.print(", display is now ");
  Serial.println((isDisplayVisible ? "ON" : "OFF"));
  Serial.print(F("Screen blank countdown: "));
  Serial.println(currentMillis - previousMillisDisplay);
#endif

#if (screen_blank_enabled)
  if ((currentMillisDisplay - previousMillisDisplay >= MILLIS_BLANK_SCREEN_BLANK) && (isDisplayVisible)) {
//checking the time elapsed regardless of button press status.  Blank on an already blank won't hurt, but may want to make it cleaner
#if (SERIALDEBUGGINGISR)
    Serial.print(F("Screen blanking threshold exceeded AND Display is ON"));
    Serial.print(F(", display is now "));
    Serial.println((isDisplayVisible ? "ON" : "OFF"));
    delay(5000);
#endif
    OLED.enableDisplay(false);
    isDisplayVisible = false;
#endif
  }
}

void SetupOLED() {
  OLED.setRotation(2);
  OLED.fillScreen(BLACK);
  OLED.setTextSize(2);
  //setup initial display for loop()
  OLED.fillScreen(BLACK);
  OLED.setTextSize(2);
  OLED.setCursor(0, 0);  //Collector temp at 55,0
  OLED.print(F("Coll"));
  OLED.setCursor(0, 16);  //Pool temp at 55,16
  OLED.print(F("Pool"));
  OLED.setCursor(0, 32);  //MQTT status at 55,32
  OLED.print(F("MQTT"));
  OLED.setCursor(0, 55);  //Heating valve status at 55,55
  OLED.print(F("Val:"));
  OLED.setCursor(0, 75);  //pH Value at 55,75
  OLED.print(F("pH: "));
  OLED.setCursor(0, 95);  //ORP Value at 55,95
  OLED.print(F("ORP:"));
}

void SetupWiFi() {
// WiFi stuff                               //modifiy creds in secrets.h as necessary.  Notice secrets.h included with ""

  char ssid[] = SECRET_SSID;
  char pass[] = SECRET_PASS;
  int keyIndex = 0;
  int status = WL_IDLE_STATUS;
  //WiFiServer server(80);  // may use this for WiFi access one day
  //String readString;      //for server read???
  OLED.setRotation(2);
  OLED.fillScreen(YELLOW);
  OLED.setTextSize(2);
  WiFi.setHostname("LaMeryUnoWFR2");
  while (status != WL_CONNECTED) {
#if (SERIALDEBUGGING)
    Serial.print("Attempting to connect to Network named: ");
    Serial.println(ssid);
#endif
    status = WiFi.begin(ssid, pass);
    OLED.setCursor(1, 1);
    OLED.print(F("Connecting"));
  }
  IPAddress ip = WiFi.localIP();
#if (SERIALDEBUGGING)
  Serial.print(F("SSID: "));
  Serial.println(WiFi.SSID());
  Serial.print(F("IP Address: "));
  Serial.println(ip);
#endif
  //notify WIFI connection has been established
  OLED.setTextSize(1);
  OLED.setCursor(0, 18);
  OLED.print(WiFi.SSID());
  OLED.setCursor(0, 30);
  OLED.print(ip);
  delay(2000);
  OLED.setTextSize(2);
}

void SetupRelays(){
  //Setup Relays
  // Relays
    pinMode(Relay_1,OUTPUT);
    pinMode(Relay_2,OUTPUT);
    pinMode(Relay_3,OUTPUT);
    pinMode(Relay_4,OUTPUT);
    pinMode(Relay_5,OUTPUT);
    pinMode(Relay_6,OUTPUT);
    pinMode(Relay_7,OUTPUT);
    pinMode(Relay_8,OUTPUT);
  
//Initial Relay configuration -- see counterintuitive HIGH/LOW and relay board light logic above
    digitalWrite(Relay_1,HIGH);
    digitalWrite(Relay_2,HIGH);
    digitalWrite(Relay_3,HIGH);
    digitalWrite(Relay_4,HIGH);
    digitalWrite(Relay_5,HIGH);
    digitalWrite(Relay_6,HIGH);
    digitalWrite(Relay_7,HIGH);
    digitalWrite(Relay_8,HIGH);
//First time setup of "valve icons" on display - CYAN is valve OPEN - relay light not lit
    OLED.fillRoundRect(55, 55, OLED.width()/9, OLED.width()/9, 5, CYAN);
    OLED.fillRoundRect(70, 55, OLED.width()/9, OLED.width()/9, 5, CYAN);
    OLED.fillRoundRect(85, 55, OLED.width()/9, OLED.width()/9, 5, CYAN);
    OLED.fillRoundRect(100,55, OLED.width()/9, OLED.width()/9, 5, CYAN);
}


//Clock painting
void paint_UNIX_time() {
  OLED.setCursor(0, 115);
  OLED.fillRect(0,115, OLED.width(), 14, BLACK);
  OLED.setTextSize(1);
  OLED.setTextColor(MAGENTA);
  OLED.print(dayShortStr(weekday(WiFi.getTime())));
  OLED.print(" ");
  OLED.print(year(WiFi.getTime()));
  //OLED.print("-");
  OLED.print(month(WiFi.getTime()));
  //OLED.print("-");
  OLED.print(day(WiFi.getTime()));
  OLED.print(" ");
    
  if (hour(WiFi.getTime())<10){
    OLED.print("0");
  }
  OLED.print(hour(WiFi.getTime())+TZOFFSET); 
  OLED.print(":"); 
  if (minute(WiFi.getTime())<10){
    OLED.print("0");
  }
  OLED.print(minute(WiFi.getTime())); 
  
  //Taking out secs because of fit and too much refresh
  OLED.print(":"); 
  if (second(WiFi.getTime())<10){
    OLED.print("0");
  }
  OLED.print(second(WiFi.getTime())); 
  

  OLED.setTextColor(OLED_Text_Color);
  OLED.setTextSize(2); 
}

boolean CheckReed() 
{
  //No delay in reacting to the reed.  Relay - in this case 4 - goes LOW - aka closed and activated - immediately after reed swith is closed 
  int proximity = digitalRead(REED_PIN); // Read the state of the switch.  Pin is internally PULLED UP, so a short to ground will send it LOW, i.e switch closed
  if (proximity == LOW) // If the pin reads low, the switch is closed.  
  {
    digitalWrite(LED_BUILTIN, HIGH); // Turn the LED ON
    digitalWrite(Relay_4,LOW);  //tuns on the relay... 
    OLED.fillRoundRect(100, 55, OLED.width()/9, OLED.width()/9, 5, RED); // display valve 1 requested icon - RELAY NOW ON
  }
  else                 
  {
    digitalWrite(LED_BUILTIN, LOW); // Turn the LED OFF
    digitalWrite(Relay_4,HIGH);  //tuns on the relay
    OLED.fillRoundRect(100, 55, OLED.width()/9, OLED.width()/9, 5, CYAN); // display valve 1 requested icon - RELAY NOW ON
  }
  return (proximity);   
}
