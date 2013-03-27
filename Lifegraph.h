#include <js0n.h>
#include <Arduino.h>
#include <WiFlyHQ.h>
#include <stdint.h>
#include <stdbool.h>
#include <SoftwareSerial.h>

extern WiFly wifly;

// wifi

boolean connectWifi (SoftwareSerial *wifiSerial, const char *ssid, const char *pass);
void debugWifiState ();

// Facebook

class FacebookAPI {
  
  public:
    static char *host;
    static char *base;
    static uint8_t *buffer;
    static int bufferSize;
    
    void get (const char *path, const char *access_token);
    void post (const char *path, const char *access_token);
    int request ( );
    int request ( js0n_user_cb_t cb );
    void form (const char *name, const char *value);
    void chunk (const char *str, int len);

    int postStatus (const char *access_token, const char *status);
    int unreadNotifications (const char *access_token, boolean *notifications_flag_ret);
  
  private:
    boolean hasBody;
    void _headers (const char *method, const char *path, const char *access_token);
};

#define crBegin static int __state=0; switch(__state) { case 0:
#define crReturn(x) do { __state=__LINE__; return x; \
                         case __LINE__:; } while (0)
#define crFinish }

#define CB_BEGIN crBegin; while (true) {
#define CB_END crReturn(0); } crFinish;
#define CB_GET_NEXT_TOKEN crReturn(0);
#define CB_MATCHES(x) (strncmp((char *) parser->buffer, x, parser->token_length) == 0)
#define CB_MATCHES_KEY(x) parser->token_type == JSON_MAP_KEY && CB_MATCHES(x)

extern FacebookAPI Facebook;