/*
 * Lifegraph Facebook Demo
 * This sketch is released to the public domain.
 */

#include <WiFlyHQ.h>
#include <Lifegraph.h>

#include <SoftwareSerial.h>
SoftwareSerial wifiSerial(2,3);


/**
 * Configuration
 */
 
const char mySSID[] = "...";
const char myPassword[] = "...";

// To make a request, you'll need an access token.
// For temporary one, use the Graph API Explorer: https://developers.facebook.com/tools/explorer
// and request a token with "manage_notifications" and "publish_stream" permissions.
// But take note: Graph API Explorer access tokens expires every hour.
const char access_token[] = "...";
 
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
  
  Serial.println("Connecting...");
 
  // Setup network connection.
  if (!connectWifi(&wifiSerial, mySSID, myPassword)) {
    Serial.println("Failed to join network.");
  } else {
    Serial.println("Joined wifi network.");
  }
}

void loop()
{ 
  // Read if there are unread notifications on the server.
  boolean notifications_flag;
  int status_code = Facebook.unreadNotifications ( access_token, &notifications_flag );
  
  // If the request is successful, update the light.
  if (status_code == 200) {
    digitalWrite(light, notifications_flag ? HIGH : LOW);
  }

  // Notify terminal of our success.
  Serial.print("Response: ");
  Serial.print(status_code);
  Serial.print(" Unread notifications:");
  Serial.println(notifications_flag, HEX);
}