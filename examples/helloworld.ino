/*
 * Lifegraph Facebook Demo
 * This sketch is released to the public domain.
 */

#include <WiFlyHQ.h>
#include <sm130.h>
#include <Lifegraph.h>

#include <SoftwareSerial.h>
SoftwareSerial wifiSerial(9, 10);

/**
 * Configuration
 */
 
const char mySSID[] = "...";
const char myPassword[] = "...";
 
// Pin our LED is connected to.
int light = 13;
 
/**
 * Setup
 */

// Store physical ID from RFID tag.
uint8_t physicalid[8] = { 0 };
 
void setup()
{
  // Setup ports.
  Serial.begin(9600);
  wifiSerial.begin(9600);
  pinMode(light, OUTPUT);
  
  Serial.println("Connecting...");
 
  // Setup network connection.
  if (!connectWifi(&wifiSerial, mySSID, myPassword)) {
    Serial.println(F("Failed to join network."));
  } else {
    Serial.println(F("Joined wifi network."));
  }
  
  int status_code = 0;
  
  do {
    // Read if there are unread notifications on the server.
    Facebook.get ( NULL, "lifegraphlabs" );
    status_code = Facebook.request();
    
    // If the request is successful, update the light.
    digitalWrite(light, status_code == 200);

    // Notify terminal of our success.
    Serial.print("HTTP Response: ");
    Serial.println(status_code);
   
    // Repeat until we make a successful HTTP request.
    // Wifly occasionally returns "0".
    delay(3000);
  } while (status_code != 200);
}

void loop () {
  return;
}