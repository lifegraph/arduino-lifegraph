#ifndef _JS0N_H_
#define _JS0N_H_

#include <Arduino.h>
#include <stdint.h>
#include <stdbool.h>
#include <SoftwareSerial.h>

#ifdef __cplusplus
extern "C" {
#endif

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

void getRequestByUrl (char *url);
void getRequest (char *host, char *path);

void postRequestByUrl (char *url);
void postRequest (char *host, char *path);

void requestBody(int len, char *str);
void requestEnd();

void readResponseHeaders (int *content_len);
void readResponse (char *buf, int max_len, int content_len);

int parseResponse ( uint8_t *buf, uint16_t bufSize, js0n_user_cb_t cb );

#ifdef __cplusplus
}
#endif

#endif
