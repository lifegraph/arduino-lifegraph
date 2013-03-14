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
  // Ports.
  Serial.begin(9600);
  wifiSerial.begin(9600);
  pinMode(light, OUTPUT);
  
  Serial.println("Connecting...");
 
  // Setup network.
  if (!connectWifi(&wifiSerial, mySSID, myPassword)) {
    Serial.println("Failed to join network.");
  } else {
    Serial.println("Joined wifi network.");
  }
}

uint8_t buf[300];
int newState = LOW;

int notifications_cb ( uint8_t *cursor, uint16_t length, uint16_t level )
{
  if (strncmp((char *) cursor, "title", length) == 0) {
    newState = HIGH;
  }
}
 
void loop()
{
  getRequestByUrl("http://lifegraph-proxy-facebook.herokuapp.com/me/notifications?limit=1&access_token=...");  
  
  // Get response.
  int content_len;
  readResponse(&content_len);

  Serial.println( "parsing input..." );
  
  newState = LOW;
  parse_json_stream ( &wifly, buf, content_len, notifications_cb );
  digitalWrite(light, newState);

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
