/*
 * Lifegraph Facebook Demo
 * This sketch is released to the public domain.
 */

#include <SoftwareSerial.h>
#include <WiFlyHQ.h>
#include <Lifegraph.h>

SoftwareSerial wifiSerial(9, 10);


/**
 * Configuration
 */
 
// Wifi configuration for a WPA network.
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
  
  // Setup network connection.
  Serial.println(F("Connecting to Wifi..."));
  if (!connectWifi(&wifiSerial, mySSID, myPassword)) {
    Serial.println(F("Failed to join network."));
    while (true) {
      // Hang forever.
    }
  } else {
    Serial.println(F("Joined wifi network."));
  }
}

void loop () {  
  // Make an HTTP request for graph.facebook.com/lifegraphlabs
  Facebook.get ( NULL, "lifegraphlabs" );
  int status_code = Facebook.request();
  
  // The request is successful if it returns an HTTP status code
  // of "200" (HTTP OK). Update the light accordingly.
  digitalWrite(light, status_code == 200 ? HIGH : LOW);

  // Notify terminal of our status.
  Serial.print("HTTP Status Code: ");
  Serial.println(status_code);
 
  // If successful, stop making requests.
  // (Wifly occasionally returns "0" instead of "200".)
  while (status_code == 200) {
    // Hang forever
  }
  
  // Otherwise, delay, and repeat until we make a successful HTTP request.
  delay(3000);
}