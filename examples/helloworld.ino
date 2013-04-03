/*
 * Lifegraph Facebook Demo
 * This sketch is released to the public domain.
 */

#include <SPI.h>
#include <WiFly.h>
#include <SoftwareSerial.h>
#include <Adafruit_NFCShield_I2C.h>
#include <Lifegraph.h>


/**
 * Configuration
 */

// Our wifi network credentials.
const char mySSID[] = "...";
const char myPassword[] = "...";
 
// Pin our LED is connected to.
int light = 6;
 

/**
 * Setup
 */
 
void setup()
{
  // Setup ports.
  Serial.begin(9600);
  pinMode(light, OUTPUT);
  
  Serial.println("Connecting to Wi-Fi...");
 
  // Setup WiFly and wireless network.
  WiFly.begin();
  if (!WiFly.join(mySSID, myPassword)) {
    Serial.println("Failed to join network.");
    while (true) {
      // Hang forever
    }
  }
  Serial.println("Joined wifi network.");
  WiFly.configure(WIFLY_BAUD, 9600);
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