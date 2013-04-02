/*
 * Lifegraph Facebook Demo
 * This sketch is released to the public domain.
 */

#include <WiFlyHQ.h>
#include <sm130.h>
#include <Lifegraph.h>

#include <SoftwareSerial.h>
SoftwareSerial wifiSerial(9, 10);

NFCReader rfid(7, 8);

/**
 * Configuration
 */
 
const char mySSID[] = "...";
const char myPassword[] = "...";

char access_token[128] = { 0 };
const char app_namespace[] = "...";
const char app_key[] = "...";
const char app_secret[] = "...";
 
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
    Serial.println("Failed to join network.");
  } else {
    Serial.println("Joined wifi network.");
  }
  
  // Initialize access tokens.
  Lifegraph.configure(app_namespace, app_key, app_secret);
  Lifegraph.readIdentity(rfid, &wifiSerial, access_token);
}

void loop() {
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