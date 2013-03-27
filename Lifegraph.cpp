#include <Arduino.h>
#include "Lifegraph.h"
#include <WiFlyHQ.h>
#include <SoftwareSerial.h>

WiFly wifly;

boolean connectWifi (SoftwareSerial *wifiSerial, const char *ssid, const char *pass) {
  if (!wifly.begin(wifiSerial, &Serial)) {
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
 * Facebook
 */

void FacebookAPI::get (const char *access_token, const char *path) {
  this->hasBody = false;
  this->_headers("GET", path, access_token);
}

void FacebookAPI::post (const char *access_token, const char *path) {
  this->hasBody = true;
  this->_headers("POST", path, access_token);
}

int FacebookAPI::request ( ) {
  return this->request(NULL);
}

int FacebookAPI::request ( js0n_user_cb_t cb ) {
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

void FacebookAPI::form (const char *name, const char *value) {
  this->chunk(name, strlen(name));
  this->chunk("=", 1);
  this->chunk(value, strlen(value));
  this->chunk("&", 1);
}

void FacebookAPI::chunk (const char *str, int len) {
  char lenstr[12];
  sprintf(lenstr, "%X", len);
  wifly.println(lenstr);
  wifly.println(str);
}

void FacebookAPI::_headers (const char *method, const char *path, const char *access_token) {
  // If an old connection is active, close.
  if (wifly.isConnected()) {
    wifly.close();
  }
  
  if (!wifly.open(this->host, 80)) {
    Serial.println("Failed to connect to host.");
  }
  
  wifly.print(method);
  wifly.print(" ");
  wifly.print("http://");
  wifly.print(this->host);
  wifly.print(this->base);
  wifly.print("/");
  wifly.print(path);
  wifly.print(strstr(path, "?") == 0 ? "?" : "&");
  wifly.print("access_token=");
  wifly.print(access_token);
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
 * Convenience methods
 */

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
 * First-class object
 */

// We have to initialize a static buffer for parsing JSON keys
// and strings. This only needs to be as big as your largest key/string.
uint8_t buf[300];

char *FacebookAPI::host = "lifegraph-proxy-facebook.herokuapp.com";
char *FacebookAPI::base = "";
uint8_t *FacebookAPI::buffer = buf;
int FacebookAPI::bufferSize = sizeof(buf);

FacebookAPI Facebook;