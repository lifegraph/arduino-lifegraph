#include <stdint.h>
#include <stdbool.h>

#include <js0n.h>

#include <Arduino.h>
#include <SoftwareSerial.h>
#include <WiFlyHQ.h>


#if defined(PN532_PREAMBLE)
#include <Adafruit_NFCShield_I2C.h>
typedef NFCReader Adafruit_NFCShield_I2C;
#endif


// Wifi.
extern WiFly wifly;
boolean connectWifi (SoftwareSerial *wifiSerial, const char *ssid, const char *pass);
void debugWifiState ();


// JSON API
class JSONAPI {
  
  public:
    const char *host;
    const char *base;
    uint8_t *buffer;
    int bufferSize;

    JSONAPI () { }
    JSONAPI (const char *host, const char *base, uint8_t *buf, int bufferSize);
    
    void get (const char *path);
    void post (const char *path);
    int request ( );
    int request ( js0n_user_cb_t cb );
    void form (const char *name, const char *value);
  
  protected:
    boolean hasBody;
    void _headerStart (const char *method);
    void _headerPath (const char *path);
    void _headerEnd ();
    void _chunk (const char *str, int len);
};


// Lifegraph API.
class LifegraphAPI : public JSONAPI {

  public:
    const char *ns;
    const char *key;
    const char *secret;

    LifegraphAPI (uint8_t *buf, int bufferSize);

    void configure (const char *app_namespace, const char *app_key, const char *app_secret);
    #ifdef sm130_h
        int readCard (NFCReader rfid, uint8_t uid[8]);
        void readIdentity (NFCReader rfid, SoftwareSerial *wifiSerial, char access_token[32]);
    #endif
    int connect (uint8_t uid[], int uidLength);
    void stringifyTag (uint8_t uid[], int uidLength, char output[]);
};


// Facebook API.
class FacebookAPI : public JSONAPI {

  public:
    FacebookAPI (uint8_t *buf, int bufferSize);

    int postStatus (const char *access_token, const char *status);
    int unreadNotifications (const char *access_token, int *unread_count);
    int fbid (const char *access_token, char fbid_ret[16]);
    int findString (const char *access_token, char *path, char *str_to_find, int *num_strings_found_ret);

    void get (const char *path, const char *access_token);
    void post (const char *path, const char *access_token);

  protected:
    void _headerPath (const char *path, const char *access_token);
};


// Reentrant functions
#define crBegin static int __state=0; switch(__state) { case 0:
#define crReturn(x) do { __state=__LINE__; return x; case __LINE__:; } while (0)
#define crFinish }

// Macros for writing JSON parsers.
#define CB_BEGIN crBegin; while (true) {
#define CB_END crReturn(0); } crFinish;
#define CB_GET_NEXT_TOKEN crReturn(0);
#define CB_MATCHES(x) (strncmp((char *) parser->buffer, x, parser->token_length) == 0)
#define CB_MATCHES_KEY(x) parser->token_type == JSON_MAP_KEY && CB_MATCHES(x)

// Buffer object.
#define LIFEGRAPH_BUFFER_SIZE 160
extern uint8_t LIFEGRAPH_BUFFER[LIFEGRAPH_BUFFER_SIZE];

// Global objects.
extern LifegraphAPI Lifegraph;
extern FacebookAPI Facebook;


#ifndef LG_RFID
#define LG_RFID 1


#if defined(PN532_PREAMBLE)
#include <Adafruit_NFCShield_I2C.h>
typedef NFCReader Adafruit_NFCShield_I2C;
#endif



#if defined(sm130_h) || defined(PN532_PREAMBLE)

int LifegraphAPI::readCard(NFCReader rfid, uint8_t uid[8]) {
  rfid.begin();
  
  // Grab the firmware version
  uint32_t versiondata = rfid.getFirmwareVersion();

  // If nothing is returned, it didn't work. Loop forever
  if (!versiondata) {
    Serial.print(F("Didn't find RFID Shield. Check your connection to the Arduino board."));
    while (1); 
  }
  
   // We will store the results of our tag reading in these vars
  uint8_t success = 0;
  uint8_t uidLength = 0;

  Serial.println(F("Waiting for tag..."));

  // Wait for an ISO14443A type cards (Mifare, etc.).  When one is found
  // 'uid' will be populated with the UID, and uidLength will indicate the length
  // If we succesfully received a tag and it has been greater than the time delay (in seconds)
  while (!(success)) {
    success = rfid.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength);
  }
  
  // Print the ID in hex format
  Serial.print(F("Read a tag: "));
  for (int i = 0; i < uidLength; i++) {
    Serial.print(uid[i], HEX);
  }
  Serial.println(F(""));

  return uidLength;
}

uint8_t _physicalid[8] = { 0 };

void LifegraphAPI::readIdentity (NFCReader rfid, SoftwareSerial *wifiSerial, char lg_access_token[32]) {
  // Read identity.
  int pidLength = 0;
  while (true) {
    pidLength = this->readCard(rfid, _physicalid);
    (*wifiSerial).listen();
    int res = this->connect(_physicalid, pidLength);
    if (res == 200) {
      break;
    }
    Serial.print(F("Got error response: "));
    Serial.println(res);
  }

  snprintf(lg_access_token, 5, "@LGC");
  for (int i = 0; i < pidLength; i++) {
    snprintf(&lg_access_token[i * 2 + 4], 3, "%02x", _physicalid[i]);
  }

  Serial.print(F("Read lg access token: "));
  Serial.println(lg_access_token);
}
#endif

#endif