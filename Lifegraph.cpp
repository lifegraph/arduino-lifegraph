#include <Arduino.h>
#include "Lifegraph.h"
#include <SPI.h>
#include <WiFly.h>
#include <Wire.h>
#include <Adafruit_NFCShield_I2C.h>


WiFlyClient client("example.com", 80);

void readResponseHeaders (int *status_code, int *content_len) {
  boolean line_start = true;
  char toss[1], buf[64];
  int len;
  *content_len = *status_code = 0;

  // HTTP/1.1 xxx
  if (client.readBytes(buf, 9) == 0) {
    return;
  }
  if (client.readBytes(buf, 3) == 0) {
    return;
  }
  buf[3] = '\0';
  *status_code = strtol(buf, NULL, 10);
  if (client.readBytesUntil('\n', buf, sizeof(buf)) == 0) {
    return;
  }

  while (true) {
    // Read first character of line.
    if (client.readBytes(buf, 1) == 0) {
      return;
    }
    char ch = buf[0];
 
    if (line_start) {
      // Beginning content.
      if (ch == '\r') {
        // read ...\n\r\n
        if (client.readBytes(buf, 1) == 0) { 
          return;
        }
        break;
      }
    }
     
    // Read header name. 
    len = client.readBytesUntil(':', &buf[1], sizeof(buf) - 2);
    if (len == 0) {
      return;
    }
    len += 1;
    buf[len + 1] = '\0';

    // space
    if (client.readBytes(toss, 1) == 0) { 
      return;
    }
    
    if (strncmp(buf, "Content-Length", len) == 0) {
      len = client.readBytesUntil('\n', buf, sizeof(buf));
      if (len == 0) {
        return;
      }
      buf[len] = '\0';
      *content_len = atoi(buf);
    } else {   
      if (client.readBytesUntil('\n', buf, sizeof(buf)) == 0) {
        return;
      }
    }
  }
}

void readResponse (char *buf, int max_len, int content_len) {
  // Read content.
  int len = client.readBytes(buf, max_len > content_len ? content_len : max_len);
  buf[len] = '\0'; // insurance
  
  // Flush buffer.
  while (client.available() > 0) {
    client.read();
  }
}
 
void parseUrl (char *url, char *host, char **path) {
  *path = url;
  int slash = 0, hoststart = 0, hostlen = 0;
  while (**path != '\0') {
    if (slash == 2) {
      if (hostlen < 100) {
        hostlen++;
      }
    } else {
      hoststart++;
    }
    if (**path == '/') {
      slash++;
      if (slash == 3) {
        break;
      }
    }
    (*path)++;
  }
  strncpy(host, &url[hoststart], hostlen);
  host[hostlen > 0 ? hostlen - 1 : hostlen] = '\0';
}
 
// void debugWifiState () {
//   char buf[32];
//   Serial.print("MAC: ");
//   Serial.println(client.getMAC(buf, sizeof(buf)));
//   Serial.print("IP: ");
//   Serial.println(client.getIP(buf, sizeof(buf)));
//   Serial.print("Netmask: ");
//   Serial.println(client.getNetmask(buf, sizeof(buf)));
//   Serial.print("Gateway: ");
//   Serial.println(client.getGateway(buf, sizeof(buf)));
//   Serial.print("DeviceID: ");
//   Serial.println(client.getDeviceID(buf, sizeof(buf)));
// }


/**
 * JSON API
 */

JSONAPI::JSONAPI (const char *host, const char *base, uint8_t *buf, int bufferSize)
{
  this->host = host;
  this->base = base;
  this->buffer = buf;
  this->bufferSize = bufferSize;
}

void JSONAPI::get (const char *path) {
  this->hasBody = false;
  this->_headerStart("GET");
  this->_headerPath(path);
  this->_headerEnd();
}

void JSONAPI::post (const char *path) {
  this->hasBody = true;
  this->_headerStart("POST");
  this->_headerPath(path);
  this->_headerEnd();
}

int json_debug_cb ( js0n_parser_t * parser )
{
  CB_BEGIN;
  Serial.print(parser->token_type);
  Serial.print(" ");
  Serial.print(parser->token_length);
  Serial.print(" ");
  Serial.print("\"");
  for (int i = 0; i < parser->token_length; i++) {
    Serial.print((char) parser->buffer[i]);
  }
  Serial.print("\"");
  Serial.println();
  CB_END;
}

int JSONAPI::request ( ) {
  return this->request(NULL);
}

int JSONAPI::request ( js0n_user_cb_t cb ) {
  if (this->hasBody) {
    client.println("0");
    client.println();
  }
  
  js0n_parser_t parser;
  parser.buffer = this->buffer;
  parser.stream = &client;
  parser.user_cb = cb;
  
  int status_code = 0;
  readResponseHeaders(&status_code, (int *) &parser.length);

  if (status_code != 0 && status_code < 500) {
    int parse_status = js0n_parse ( &parser );
  }
  return status_code;
}

void JSONAPI::form (const char *name, const char *value) {
  this->_chunk(name, strlen(name));
  this->_chunk("=", 1);
  this->_chunk(value, strlen(value));
  this->_chunk("&", 1);
}

void JSONAPI::_chunk (const char *str, int len) {
  char lenstr[12];
  sprintf(lenstr, "%X", len);
  client.println(lenstr);
  client.println(str);
}

void JSONAPI::_headerStart (const char *method) {
  client.stop();
  client._domain = this->host;
  if (!client.connect()) {
    Serial.println("Failed to connect to host.");
  }
  
  client.print(method);
  client.print(" ");
  // client.print("http://");
  // client.print(this->host);
}

void JSONAPI::_headerPath (const char *path) {
  client.print(this->base);
  client.print("/");
  client.print(path);
}

void JSONAPI::_headerEnd () {
  client.println(" HTTP/1.1"); // paste your number here
  client.print("Host: ");
  client.println(this->host);
  client.println("User-Agent: lifegraph/0.0.1");
  if (this->hasBody) {
    client.println("Content-Type: application/x-www-form-urlencoded");
    client.println("Transfer-Encoding: chunked");
  }
  client.println();
}


/** 
 * Lifegraph API
 * Extends the JSONAPI with custom methods to connect to the Facebook API.
 */

LifegraphAPI::LifegraphAPI (uint8_t *buf, int bufferSize)
{
  this->host = "www.lifegraphconnect.com";
  this->base = "/api";
  this->buffer = buf;
  this->bufferSize = bufferSize;
}

int LifegraphAPI::readCard(Adafruit_NFCShield_I2C rfid, uint8_t uid[8]) {
  rfid.begin();

  Serial.println("RFID BEGAN");
  
  // Grab the firmware version
  uint32_t versiondata = rfid.getFirmwareVersion();


  Serial.println("THATS SOME FIRM WARES");

  // If nothing is returned, it didn't work. Loop forever
  if (!versiondata) {
    Serial.print("Didn't find RFID Shield. Check your connection to the Arduino board.");
    while (1); 
  }
  
   // We will store the results of our tag reading in these vars
  uint8_t success = 0;
  uint8_t uidLength = 0;

  Serial.println("Waiting for tag...");

  // Wait for an ISO14443A type cards (Mifare, etc.).  When one is found
  // 'uid' will be populated with the UID, and uidLength will indicate the length
  // If we succesfully received a tag and it has been greater than the time delay (in seconds)
  while (!(success)) {
    success = rfid.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength);
  }
  
  // Print the ID in hex format
  Serial.print("Read a tag: ");
  for (int i = 0; i < uidLength; i++) {
    Serial.print(uid[i], HEX);
  }
  Serial.println("");

  return uidLength;
}

uint8_t _physicalid[8] = { 0 };

void LifegraphAPI::readIdentity (Adafruit_NFCShield_I2C rfid, char access_token[128]) {
  // Read identity.
  while (true) {
    int pidLength = this->readCard(rfid, _physicalid);
    if (this->connect(_physicalid, pidLength, access_token) == 200) {
      break;
    }
    Serial.println("No access tokens found. Retrying with tag...");
  }

  Serial.print("Read access token: ");
  Serial.println(access_token);
}

void LifegraphAPI::configure (const char *app_namespace, const char *app_key, const char *app_secret) {
  this->ns = app_namespace;
  this->key = app_key;
  this->secret = app_secret;
}

char *lifegraph_connect_buffer = NULL;

int lifegraph_connect_cb ( js0n_parser_t * parser )
{
  CB_BEGIN;
  if (CB_MATCHES_KEY("oauthAccessToken")) {
    CB_GET_NEXT_TOKEN;
    int len = parser->token_length > 127 ? 127 : parser->token_length;
    memcpy(lifegraph_connect_buffer, parser->buffer, len);
  }
  CB_END;
}

int LifegraphAPI::connect (uint8_t uid[], int uidLength, char access_token[128])
{
  lifegraph_connect_buffer = access_token;
  memset(access_token, '\0', 128);

  this->hasBody = false;
  this->_headerStart("GET");
  this->_headerPath("tokens");
  client.print("/");
  char token[3];
  for (int i = 0; i < uidLength; i++) {
    snprintf(token, 3, "%02x", uid[i]);
    client.print(token);
  }
  client.print("?namespace=");
  client.print(this->ns);
  client.print("&key=");
  client.print(this->key);
  client.print("&secret=");
  client.print(this->secret);
  this->_headerEnd();
  return this->request( lifegraph_connect_cb );
}


/** 
 * Facebook API
 * Extends the JSONAPI with custom methods to connect to the Facebook API.
 */

FacebookAPI::FacebookAPI (uint8_t *buf, int bufferSize)
{
  this->host = "lifegraph-proxy-facebook.herokuapp.com";
  this->base = "";
  this->buffer = buf;
  this->bufferSize = bufferSize;
}

void FacebookAPI::get (const char *access_token, const char *path) {
  Serial.print("GET http://graph.facebook.com/");
  Serial.println(path);

  this->hasBody = false;
  this->_headerStart("GET");
  this->_headerPath(path, access_token);
  this->_headerEnd();
}

void FacebookAPI::post (const char *access_token, const char *path) {
  Serial.print("POST http://graph.facebook.com/");
  Serial.println(path);

  this->hasBody = true;
  this->_headerStart("POST");
  this->_headerPath(path, access_token);
  this->_headerEnd();
}

void FacebookAPI::_headerPath (const char *path, const char *access_token) {
  client.print(this->base);
  client.print("/");
  client.print(path);
  if (access_token != 0) {
    client.print(strstr(path, "?") == 0 ? "?" : "&");
    client.print("access_token=");
    client.print(access_token);
  }
}

// postStatus

int FacebookAPI::postStatus (const char *access_token, const char *status) {
  Facebook.post(access_token, "me/feed");
  Facebook.form("message", status);
  return Facebook.request();
}

// unreadNotifications

int notifications_count;

int notifications_cb ( js0n_parser_t * parser )
{
  CB_BEGIN;
  if (CB_MATCHES_KEY("unseen_count")) {
    CB_GET_NEXT_TOKEN;
    notifications_count = atoi((char *) parser->buffer);
  }
  CB_END;
}

int FacebookAPI::unreadNotifications (const char *access_token, int *notifications_count_ret) {
  notifications_count = 0;
  Facebook.get(access_token, "me/notifications?limit=1");
  int status_code = Facebook.request(notifications_cb);
  *notifications_count_ret = notifications_count;
  return status_code;
}


/**
 * Globals object
 */

uint8_t LIFEGRAPH_BUFFER[LIFEGRAPH_BUFFER_SIZE];

FacebookAPI Facebook(LIFEGRAPH_BUFFER, LIFEGRAPH_BUFFER_SIZE);
LifegraphAPI Lifegraph(LIFEGRAPH_BUFFER, LIFEGRAPH_BUFFER_SIZE);