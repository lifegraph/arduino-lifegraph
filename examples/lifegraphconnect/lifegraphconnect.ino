/*
 * Lifegraph Facebook Demo
 * This sketch is released to the public domain.
 */

#include <SoftwareSerial.h>
#include <sm130.h>

#include <WiFlyHQ.h>
#include <Lifegraph.h>

// serial for talking to the internet.
SoftwareSerial wifiSerial(9, 10);

// Reader for RFID tags
NFCReader rfid(7, 8);


/**
 * Configuration
 */
 
// Wifi configuration for a WPA network.
const char mySSID[] = "...";
const char myPassword[] = "...";

// Lifegraph Connect (http://lifegraphconnect.com) manages access tokens for us
// so we can exchange a physical RFID for a Facebook access_token.
// Create an empty access token we can populate once an RFID card is tagged in.
char access_token[32] = { 0 };

// Let's grab our Facebook ID to test.
char fbid[16] = { 0 };

// We need an application's credentials (namespace, key, and secret)
// to request a user's access tokens from Facebook.
// Get these from one of your apps on https://developers.facebook.com/apps
// Make a Facebook App by following this step of this tutorial: https://developers.facebook.com/docs/opengraph/getting-started/#create-app
const char app_namespace[] = "...";
const char app_key[] = "...";
const char app_secret[] = "...";
 
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
      // Hang forever. In shame.
    }
  } else {
    Serial.println(F("Joined wifi network."));
  }
  
  // Configure Lifegraph so it knows that we are using our app.
  Lifegraph.configure(app_namespace, app_key, app_secret);
}

void loop()
{ 
  Serial.println("reading identity");
  // remember that this is a blocking call until an ID is found.
  Lifegraph.readIdentity(rfid, &wifiSerial, access_token);
  Serial.println("Access token is now obtainable if you want to do your own calls");
  Serial.println(access_token);
  
  // Now that we have an access token, we can make general graph API calls
  // Why don't we get the name of the person and see if it matches our name.
  // We do this by doing a findString to see if the /me?fields=name Facebook Graph API
  // enpoint contains one occurrence of our target string, which will be our name
  // Lastly, we provide a place for the returned number of matches we get.
  int num_found;
  
  int status_code = Facebook.findString ( access_token, "me?fields=name", "<YOUR_NAME>" ,&num_found );
  
  // We don't have to just do names. If you comment out the above line and uncomment
  // the next, we can light up if the person tagging likes The Social Network Movie on Facebook
//  int status_code = Facebook.findString ( access_token, "me/movies", "The Social Network Movie" ,&num_found );
  
  // We can even see if we are friends with a particular FBID to
  // see if this is one of our Facebook Friends!
//  int status_code = Facebook.findString ( access_token, "me/friends?fields=id", "MYFBIDGOESHERE" ,&num_found );
  
  // If the request is successful (HTTP OK), update the light accordingly.
  if (status_code == 200) {
    digitalWrite(light, num_found > 0 ? HIGH : LOW);
  }

  // Notify terminal of our status.
  Serial.print(F("HTTP Status Code: "));
  Serial.print(status_code);
  Serial.print(" Number matched: ");
  Serial.println(num_found);

  // We can also use any of the convenience functions, such as grabbing our Facebook ID
  status_code = Facebook.fbid(access_token, fbid);
  Serial.print(F("HTTP Status Code: "));
  Serial.print(status_code);
  Serial.print(" Facebook ID: ");
  Serial.println(fbid);
}
