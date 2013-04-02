#include <js0n.h>
#include <Arduino.h>
#include <WiFlyHQ.h>
#include <stdint.h>
#include <stdbool.h>
#include <SoftwareSerial.h>
#include <sm130.h>


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
    int readCard (NFCReader rfid, uint8_t uid[8]);
    void readIdentity (NFCReader rfid, SoftwareSerial *wifiSerial, char access_token[128]);
    int connect (uint8_t uid[], int uidLength, char access_token[128]);
};


// Facebook API.
class FacebookAPI : public JSONAPI {

  public:
    FacebookAPI (uint8_t *buf, int bufferSize);

    int postStatus (const char *access_token, const char *status);
    int unreadNotifications (const char *access_token, boolean *notifications_flag_ret);

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