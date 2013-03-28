#include <Arduino.h>
#include "Lifegraph.h"
#include <WiFlyHQ.h>
#include <SoftwareSerial.h>
#include <sm130.h>

WiFly wifly;

/* Connect the WiFly serial to the serial monitor. */
void terminal()
{
    while (1) {
  if (wifly.available() > 0) {
      Serial.write(wifly.read());
  }


  if (Serial.available() > 0) {
      wifly.write(Serial.read());
  }
    }
}

boolean connectWifi (SoftwareSerial *wifiSerial, const char *ssid, const char *pass) {
  if (!wifly.begin(wifiSerial, &Serial)) {
    Serial.println("Could not start Wifly module.");
    terminal();
    return false;
  }
 
  // Join wifi network if not already associated
  if (!wifly.isAssociated()) {
    wifly.setSSID(ssid);
    wifly.setPassphrase(pass);
    wifly.enableDHCP();
    wifly.setDeviceID("Wifly-WebClient");
 
    if (!wifly.join()) {
      Serial.println("Failed to join wifi network");
      terminal();
      return false;
    }
  }
  return true;
}

void readResponseHeaders (int *status_code, int *content_len) {
  boolean line_start = true;
  char toss[1], buf[64];
  int len;
  *content_len = *status_code = 0;

  // HTTP/1.1 xxx
  if (wifly.readBytes(buf, 9) == 0) {
    return;
  }
  if (wifly.readBytes(buf, 3) == 0) {
    return;
  }
  buf[3] = '\0';
  *status_code = strtol(buf, NULL, 10);
  if (wifly.readBytesUntil('\n', buf, sizeof(buf)) == 0) {
    return;
  }

  while (true) {
    // Read first character of line.
    if (wifly.readBytes(buf, 1) == 0) {
      return;
    }
    char ch = buf[0];
 
    if (line_start) {
      // Beginning content.
      if (ch == '\r') {
        // read ...\n\r\n
        if (wifly.readBytes(buf, 1) == 0) { 
          return;
        }
        break;
      }
    }
     
    // Read header name. 
    len = wifly.readBytesUntil(':', &buf[1], sizeof(buf) - 2);
    if (len == 0) {
      return;
    }
    buf[len + 1] = '\0';
    // space
    if (wifly.readBytes(toss, 1) == 0) { 
      return;
    }
    
    if (strncmp(buf, "Content-Length", sizeof(buf)) == 0) {
      len = wifly.readBytesUntil('\n', buf, sizeof(buf));
      if (len == 0) {
        return;
      }
      buf[len] = '\0';
      *content_len = atoi(buf);
    } else {   
      if (wifly.readBytesUntil('\n', buf, sizeof(buf)) == 0) {
        return;
      }
    }
  }
}

void readResponse (char *buf, int max_len, int content_len) {
  // Read content.
  int len = wifly.readBytes(buf, max_len > content_len ? content_len : max_len);
  buf[len] = '\0'; // insurance
  
  // Flush buffer.
  while (wifly.available() > 0) {
    wifly.read();
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
 
void debugWifiState () {
  char buf[32];
  Serial.print("MAC: ");
  Serial.println(wifly.getMAC(buf, sizeof(buf)));
  Serial.print("IP: ");
  Serial.println(wifly.getIP(buf, sizeof(buf)));
  Serial.print("Netmask: ");
  Serial.println(wifly.getNetmask(buf, sizeof(buf)));
  Serial.print("Gateway: ");
  Serial.println(wifly.getGateway(buf, sizeof(buf)));
  Serial.print("DeviceID: ");
  Serial.println(wifly.getDeviceID(buf, sizeof(buf)));
}


/**
 * JSON API
 */

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

int JSONAPI::request ( ) {
  return this->request(NULL);
}

int JSONAPI::request ( js0n_user_cb_t cb ) {
  if (this->hasBody) {
    wifly.println("0");
    wifly.println();
  }
  
  js0n_parser_t parser;
  parser.buffer = this->buffer;
  parser.stream = &wifly;
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
  wifly.println(lenstr);
  wifly.println(str);
}

void JSONAPI::_headerStart (const char *method) {
  // If an old connection is active, close.
  if (wifly.isConnected()) {
    wifly.close();
  }

  if (!wifly.open(this->host, 80)) {
    Serial.println("Failed to connect to host.");
  }
  
  wifly.print(method);
  wifly.print(" ");
  // wifly.print("http://");
  // wifly.print(this->host);
}

void JSONAPI::_headerPath (const char *path) {
  wifly.print(this->base);
  wifly.print("/");
  wifly.print(path);
}

void JSONAPI::_headerEnd () {
  wifly.println(" HTTP/1.1"); // paste your number here
  wifly.print("Host: ");
  wifly.println(this->host);
  wifly.println("User-Agent: lifegraph/0.0.1");
  if (this->hasBody) {
    wifly.println("Content-Type: application/x-www-form-urlencoded");
    wifly.println("Transfer-Encoding: chunked");
  }
  wifly.println();
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

int LifegraphAPI::readCard(NFCReader rfid, uint8_t uid[8]) {
  rfid.begin();
  
  // Grab the firmware version
  uint32_t versiondata = rfid.getFirmwareVersion();

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

void LifegraphAPI::readIdentity (NFCReader rfid, SoftwareSerial *wifiSerial, char access_token[128]) {
  // Read identity.
  while (true) {
    int pidLength = this->readCard(rfid, _physicalid);
    (*wifiSerial).listen();
    if (this->connect(_physicalid, pidLength, access_token) == 200) {
      break;
    }
  }
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

int LifegraphAPI::connect (uint8_t uid[], int uidLength, char access_token[128]) {
  lifegraph_connect_buffer = access_token;
  memset(access_token, '\0', 128);

  this->hasBody = false;
  this->_headerStart("GET");
  this->_headerPath("tokens");
  wifly.print("/");
  char token[3];
  for (int i = 0; i < uidLength; i++) {
    snprintf(token, 3, "%02x", uid[i]);
    wifly.print(token);
  }
  wifly.print("?namespace=");
  wifly.print(this->ns);
  wifly.print("&key=");
  wifly.print(this->key);
  wifly.print("&secret=");
  wifly.print(this->secret);
  this->_headerEnd();
  int ret = this->request( lifegraph_connect_cb );
  
  if (access_token[0] != 0) {
    Serial.print("Read access token: ");
    Serial.println(access_token);
  } else {
    Serial.println("Error: no access token.");
  }
  return ret;
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
  this->hasBody = false;
  this->_headerStart("GET");
  this->_headerPath(path, access_token);
  this->_headerEnd();
}

void FacebookAPI::post (const char *access_token, const char *path) {
  this->hasBody = true;
  this->_headerStart("POST");
  this->_headerPath(path, access_token);
  this->_headerEnd();
}

void FacebookAPI::_headerPath (const char *path, const char *access_token) {
  wifly.print(this->base);
  wifly.print("/");
  wifly.print(path);
  wifly.print(strstr(path, "?") == 0 ? "?" : "&");
  wifly.print("access_token=");
  wifly.print(access_token);
}

// postStatus

int FacebookAPI::postStatus (const char *access_token, const char *status) {
  Facebook.post(access_token, "me/feed");
  Facebook.form("message", status);
  return Facebook.request();
}

// unreadNotifications

boolean notifications_flag;

int notifications_cb ( js0n_parser_t * parser )
{
  CB_BEGIN;
  if (CB_MATCHES_KEY("unread")) {
    CB_GET_NEXT_TOKEN;
    if (parser->buffer[0] != '0') {
      notifications_flag = 1;
    }
  }
  CB_END;
}

int FacebookAPI::unreadNotifications (const char *access_token, boolean *notifications_flag_ret) {
  notifications_flag = 0;
  Facebook.get(access_token, "me/notifications?limit=1");
  int status_code = Facebook.request(notifications_cb);
  *notifications_flag_ret = notifications_flag;
  return status_code;
}


/**
 * Globals object
 */

uint8_t buf[160];
FacebookAPI Facebook(buf, sizeof(buf));
LifegraphAPI Lifegraph(buf, sizeof(buf));