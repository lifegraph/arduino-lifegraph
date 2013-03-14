#include <Arduino.h>
#include "WiKnot.h"
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

void readResponseHeaders (int *content_len) {
  boolean line_start = true;
  char toss[1], buf[64];
  int len;
  *content_len = 0;
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
 * GET
 */
 
void getHeader (char *host, char *path) {
  wifly.print("GET ");
  wifly.print(path);
  wifly.println(" HTTP/1.1"); // paste your number here
  wifly.print("Host: ");
  wifly.println(host);
  wifly.println("User-Agent: lifegraph/0.0.1");
  wifly.println();
}

void getRequestByUrl (char *url) {
  // TODO not include 256 byte buffer here.
  char host[100], *path;
  parseUrl(url, host, &path);
  getRequest(host, path);
}
 
void getRequest (char *host, char *path) {
  // If an old connection is active, close.
  if (wifly.isConnected()) {
    wifly.close();
  }
  
  if (!wifly.open(host, 80)) {
    Serial.println("Failed to connect to host.");
  }
  
  getHeader(host, path);
}

/**
 * POST
 */
 
void postHeader(char *host, char *path) {
  wifly.print("POST ");
  wifly.print(path);
  wifly.println(" HTTP/1.1"); // paste your number here
  wifly.print("Host: ");
  wifly.println(host);
  wifly.println("User-Agent: lifegraph/0.0.1");
  wifly.println("Content-Type: application/x-www-form-urlencoded");
  wifly.println("Transfer-Encoding: chunked");
  wifly.println();
}
 
void postRequestByUrl (char *url) {
  // TODO not include 256 byte buffer here.
  char host[256], *path;
  parseUrl(url, host, &path);
  postRequest(host, path);
}
 
void postRequest (char *host, char *path) {
  // If an old connection is active, close.
  if (wifly.isConnected()) {
    wifly.close();
  }
  
  if (wifly.open(host, 80)) {
    Serial.println("Connected.");
  } else {
    Serial.println("Failed to connect");
    return;
  }
  
  postHeader(host, path);
}

/**
 * Body
 */

void requestBody(int len, char *str) {
  char lenstr[12];
  sprintf(lenstr, "%X", len);
  wifly.println(lenstr);
  wifly.println(str);
}

void requestBody() {
  wifly.println("0");
  wifly.println();
}

/**
 * JSON parsing
 */

int parseResponse ( uint8_t *buf, uint16_t bufSize, js0n_user_cb_t cb )
{ 
  js0n_parser_t parser;
  parser.buf = buf;
  parser.stream = &wifly;
  parser.user_cb = cb;
  
  readResponseHeaders((int *) &parser.length);
  return js0n_parse ( &parser );
}
