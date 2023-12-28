void SetupMQTT() {
  /*Connect to MQTT
  You can provide a unique client ID, if not set the library uses Arduino-millis()
  Each client must have a unique client ID
  You can provide a username and password for authentication
  MymqttClient.setUsernamePassword("oserani", "sass9506");
  */
  //MymqttClient.setCleanSession(1);            //this made it fail all the time.  Lots to learn here, LWAT? QOS? Retain and recapture? Subscription?
  MymqttClient.setId(UNIQUE_CLIENT_ID);         //it seems that it is unique and it's working
  //MymqttClient.setKeepAliveInterval(90);       //this helps and the reconnect loop gets invoked less
  //MymqttClient.setConnectionTimeout(30);      //this made it fail all the time
  MymqttClient.stop();

  #if (SERIALDEBUGGINGMQTT)
  Serial.print(F("Trying first connection to broker: "));
  Serial.println(broker);
  #endif

  if (MymqttClient.connect(broker, port)) {
      #if (SERIALDEBUGGINGMQTT)
      Serial.print(F("Connected to the MQTT broker: ")); Serial.println(broker);
      Serial.print(F("Initial MQTT connection error code = "));Serial.println(MymqttClient.connectError());
      #endif
    char pub_payload[35] = { 0 };
    sprintf(pub_payload, "----- Initial connect.  ClientID: %s", UNIQUE_CLIENT_ID);
    MymqttClient.beginMessage(sendtopic);
    MymqttClient.print(pub_payload);
    MymqttClient.endMessage();
    OLED.fillRect(55, 32, 14, 14, GREEN);
    MymqttClient.subscribe(receivetopic);
    LastMQTTAttempt = 0;
  } else {
    OLED.fillRect(55, 32, 14, 14, RED);
      #if (SERIALDEBUGGINGMQTT)
      Serial.print(F("Initial MQTT connection failed! Error code = "));
      Serial.print(MymqttClient.connectError());
      Serial.println(F(" exiting first ConnectMQTT loop..."));
      #endif
    }
}

void Pub_MQTT_args(char *topic, char *str) {
  // send message, the Print interface can be used to set the message contents
  #if (SERIALDEBUGGINGMQTT)
      Serial.print(F("Sending message to topic: "));
      Serial.print(topic);
      Serial.println(str);
  #endif
  MymqttClient.beginMessage(topic);
  MymqttClient.print(str);
  MymqttClient.endMessage();
}

boolean Check_MQTT_status() {
    #if (SERIALDEBUGGINGMQTT)
      Serial.print(F("Check_MQTT_status reports MymqttClient.connectError(): "));
      Serial.print(MymqttClient.connectError());
      Serial.print(F(" and MymqttClient.connected: "));
      Serial.println(MymqttClient.connected());
    #endif
  return MymqttClient.connected();
}

boolean MQTT_Reconnect() {            //not sure that this ever worked.
    #if (SERIALDEBUGGINGMQTT)
    Serial.print(F("MQTT connection failed! Error code = "));
    Serial.println(MymqttClient.connectError());
    #endif
  if (MymqttClient.connect(broker, port)) {
    #if (SERIALDEBUGGINGMQTT)
    Serial.print(F("Reconnected in reconnect() loop. MQTT broker: "));
    Serial.println(broker);
    #endif
    char pub_payload[35] = { 0 };
    sprintf(pub_payload, "N:%i %s ---reconnect() succesful---", count);
    MymqttClient.beginMessage(sendtopic);
    MymqttClient.print(pub_payload);
    MymqttClient.endMessage();
    OLED.fillRect(55, 32, 14, 14, GREEN);
  }   
  return MymqttClient.connected();
}
