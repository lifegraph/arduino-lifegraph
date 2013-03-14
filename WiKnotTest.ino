/*
 * Lifegraph Facebook Demo
 * This sketch is released to the public domain.
 */

#include "WiKnot.h"

#include <SoftwareSerial.h>
SoftwareSerial wifiSerial(2,3);

/**
 * Configuration
 */
 
const char mySSID[] = "...";
const char myPassword[] = "...";
 
int light = 12; // define our light pin
 
 
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

// We have to initialize a static buffer for parsing JSON keys
// and strings. This only needs to be as big as your largest key/string.
uint8_t buf[300];

// State the light should be in before/after a request. 
int nextLightState = LOW;

// Our callback for while we are parsing JSON.
int notifications_cb ( uint8_t *cursor, uint16_t length, uint16_t level )
{
  // If we see the "title" key in JSON, that means we have a notification.
  if (strncmp((char *) cursor, "title", length) == 0) {
    nextLightState = HIGH;
  }
}
 
void loop()
{
  // Make the GET request to the Facebook API.
  getRequestByUrl("http://lifegraph-proxy-facebook.herokuapp.com/me/notifications?limit=1&access_token=...");  
  
  // Get response.
  int content_len;
  readResponse(&content_len);

  Serial.println( "parsing input..." );
  
  // Reset light state, parse the JSON stream with a custom callback, then reset the light.
  nextLightState = LOW;
  parse_json_stream ( &wifly, buf, content_len, notifications_cb );
  digitalWrite(light, nextLightState);

}







  
//  if (wifiSerial.overflow()) {
//    Serial.println("~~~~~~OVERFLOWING~~~~~");
//  }
//    Serial.println("\n----");
//    Serial.print("Length: ");
//    Serial.println(length);
//    Serial.print("Level: ");
//    Serial.println(level);
//    Serial.print("Key: \"");
//    for (uint16_t l = 0; l < length; l++) {
//      Serial.print((char) cursor[l]);
//    }
//    Serial.println("\"");
//    Serial.println("----");
//    return 0;

//void loop() {
  // Make request.
//  postRequestByUrl("http://lifegraph-proxy-facebook.herokuapp.com/4570044049285/comments?access_token=AAACEdEose0cBAKLvuKbqqGytazVbfcZATxw4fSbkiC41DZBCyvIGGIuaG938zE1UtCmgY0ShagHBPzZA6Fx2ZAhyuiOvl4EwwQVJwFBtHglpx7jLUBEH");
//  char *body = "message=Testing";
//  postBody(strlen(body), body);
//  postEnd();
//}
