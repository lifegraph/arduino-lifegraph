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
// and request a token with "read_mailbox" and "publish_stream" permissions.
// But take note: Graph API Explorer access tokens expires every hour.
const char access_token[] = "...";
 
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

  // Uncomment these lines to post a status update:
//  Facebook.post("me/feed", access_token);
//  Facebook.form("message", "Posting a status update from my Arduino!");
//  int status_code = Facebook.request();
//  
//  Serial.print("Status: ");
//  Serial.println(status_code);
}


/**
 * Loop
 */

// Our response parsing is a state machine.
// 0 = no content
// 1 = error
// 2 = unread parsing
// 3 = no unread messages
// 4 = unread messages!
int state = 0;

// Our callback for parsing JSON.
// NOTE: Don't perform any serial tasks (such as logging) in this function.
// They are slow and will overflow the JSON parsing. Instead, we create
// a state variable that indicates the notification state.
int notifications_cb ( uint8_t *cursor, uint16_t length, uint16_t level )
{
  // after "unread", we should immediately read the next value.
  // if it's non-0, we've received unread notifications.
  if (state == 2) {
    if (cursor[0] != '0') {
      state = 4;
    } else {
      state = 3;
    }
  }
  // "error", perhaps access token is expired
  if (strncmp((char *) cursor, "error", length) == 0) {
    state = 1;
  }
  // "unread", we are reading the # of unread messages
  if (strncmp((char *) cursor, "unread", length) == 0) {
    state = 2;
  }
}
 
void loop()
{ 
  // Reset light state.
  state = 0;
  
  // Make the GET request to the Facebook API.
  Facebook.get("me/inbox?limit=1&fields=unread", access_token);
  int status_code = Facebook.request ( notifications_cb );
  
  // Update light state.
  if (state > 0) {
    digitalWrite(light, state == 4 ? HIGH : LOW);
  }

  // Notify terminal of parsing status.
  Serial.print("Done. Response: ");
  Serial.print(status_code);
  Serial.print(", notification state: ");
  Serial.println(state, HEX);
}