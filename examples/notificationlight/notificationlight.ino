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

// To make a request, you'll need an access token.
// For temporary one, use the Graph API Explorer: https://developers.facebook.com/tools/explorer
// and request a token with the "manage_notifications" permission.
// Take note: Graph API Explorer access tokens expires every hour.
char access_token[256] = "...";
 
// Pin our LED is connected to.
int light = 13;
 
 
/**
 * Setup
 */
 
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
