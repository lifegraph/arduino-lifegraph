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

// To make a request, you'll need an access token.
// For temporary one, use the Graph API Explorer: https://developers.facebook.com/tools/explorer
// and request a token with the "manage_notifications" permission.
// Take note: Graph API Explorer access tokens expires every hour.
char access_token[128] = "...";
 
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

void loop()
{ 
  // Request if there are unread notifications on Facebook.
  int unread_count;
  int status_code = Facebook.unreadNotifications ( access_token, &unread_count );
  
  // If the request is successful (HTTP OK), update the light accordingly.
  if (status_code == 200) {
    digitalWrite(light, unread_count > 0 ? HIGH : LOW);
  }

  // Notify terminal of our status.
  Serial.print("HTTP Status Code: ");
  Serial.print(status_code);
  Serial.print(" Unread notifications: ");
  Serial.println(unread_count);
}