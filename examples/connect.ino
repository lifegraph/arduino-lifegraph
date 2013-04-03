/*
 * Lifegraph Facebook Demo
 * This sketch is released to the public domain.
 */

#include <SPI.h>
#include <WiFly.h>
#include <SoftwareSerial.h>
#include <Wire.h>
#include <Adafruit_NFCShield_I2C.h>
#include <Lifegraph.h>

Adafruit_NFCShield_I2C rfid(2, 3);


/**
 * Configuration
 */
 
const char mySSID[] = "...";
const char myPassword[] = "...";

// Lifegraph Connect manages access tokens for us.
// Create an empty access token we can populate once an RFID card is tagged in.
char access_token[128] = { 0 };

// We need an application's credentials (namespace, key, and secret)
// to request a user's access tokens from Facebook.
const char app_namespace[] = "...";
const char app_key[] = "...";
const char app_secret[] = "...";
 
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
  
  // Initialize Lifegraph. Then wait until the user tags an RFID
  // card, read it, and retrieve an access token for the given user.
  Lifegraph.configure(app_namespace, app_key, app_secret);
  Lifegraph.readIdentity(rfid, access_token);
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