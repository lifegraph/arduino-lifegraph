#ifndef _JS0N_H_
#define _JS0N_H_

#include <Arduino.h>
#include <WiFlyHQ.h>
#include <stdint.h>
#include <stdbool.h>
#include <SoftwareSerial.h>

#ifdef __cplusplus
extern "C" {
#endif

extern WiFly wifly;

typedef struct js0n_parser;

/* user callback type */
typedef int ( * js0n_user_cb_t ) ( uint8_t *buf, uint16_t length, uint16_t level );

typedef struct js0n_parser
{
    // public:
    Stream *stream;
    uint16_t length;
    js0n_user_cb_t user_cb;

    // private:
    uint8_t current;
    uint16_t cursor;
    uint8_t depth;
    uint8_t utf8_remain;
    uint16_t gostate;
    uint8_t *buf;
    uint16_t mark;
    uint8_t live;
} js0n_parser_t;

int js0n_parse ( js0n_parser_t * parser );

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
  
  private:
    boolean hasBody;
    void _headers (const char *method, const char *path, const char *access_token);
};

extern FacebookAPI Facebook;

#ifdef __cplusplus
}
#endif

#endif
