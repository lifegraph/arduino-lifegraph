/*
 * Lifegraph Facebook Demo
 * This sketch is released to the public domain.
 */

#include <WiFlyHQ.h>
#include <WiKnot.h>

#include <SoftwareSerial.h>
SoftwareSerial wifiSerial(2,3);


/**
 * Configuration
 */
 
const char mySSID[] = "...";
const char myPassword[] = "...";

// To make a request, you'll need an access token to replace the "..." in this string.
// For temporary one, use the Graph API Explorer: https://developers.facebook.com/tools/explorer
// and request a token with "manage_notifications". Note that this access token expires every hour.
const char endpoint[] = "http://lifegraph-proxy-facebook.herokuapp.com/me/notifications?limit=1&access_token=...";
 
// Pin our LED is connected to.
int light = 12;
 
 
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


/**
 * Loop
 */

// We have to initialize a static buffer for parsing JSON keys
// and strings. This only needs to be as big as your largest key/string.
uint8_t buf[300];

// State the light should be in before/after a request. 
int nextLightState = LOW, state = 0;

// Our callback for parsing JSON.
// NOTE: Don't perform any serial tasks (such as logging) in this function.
// They are slow and will overflow the JSON parsing. Instead, we create
// a state variable that indicates the notification state.
int notifications_cb ( uint8_t *cursor, uint16_t length, uint16_t level )
{
  // "error", perhaps access token is expired
  if (strncmp((char *) cursor, "error", length) == 0) {
    state = 1;
  }
  // "data", we succesfully made the request.
  if (strncmp((char *) cursor, "data", length) == 0) {
    state = 2;
  }
  // "title", there is at least one notification
  if (strncmp((char *) cursor, "title", length) == 0) {
    state = 3;
    nextLightState = HIGH;
  }
}
 
void loop()
{
  // Make the GET request to the Facebook API.
  getRequestByUrl((char *) endpoint);  
  
  // Reset light state, parse the JSON stream with a custom callback, then reset the light.
  nextLightState = LOW;
  state = 0;
  int res = parseResponse ( buf, sizeof(buf), notifications_cb );
  digitalWrite(light, nextLightState);

  // Notify terminal of parsing status.
  Serial.print("Done. Parser state: ");
  Serial.print(res, HEX);
  Serial.print(", notification state: ");
  Serial.println(state, HEX);
}