# Lifegraph Arduino library

**VERSION 0.7**

## Installation

This library depends on the WiFlyHQ and (optionally) the sm130 library.

```
$ mkdir -p ~/Documents/Arduino/libraries
$ git clone https://github.com/harlequin-tech/WiFlyHQ.git ~/Documents/Arduino/libraries/WiFlyHQ
$ git clone https://github.com/lifegraph/sm130.git ~/Documents/Arduino/libraries/sm130
$ git clone https://github.com/lifegraph/arduino-lifegraph.git ~/Documents/Arduino/libraries/Lifegraph
```

## Examples

The [Notification Light](https://raw.github.com/lifegraph/arduino-lifegraph/master/examples/notificationlight.ino) example turns on an LED whenever you have unread messages in your inbox.

1. Follow the steps in [**arduino-wifi-setup**](https://github.com/lifegraph/arduino-wifi-setup) 
2. Copy the source from [`examples/notificationlight.ino`](https://raw.github.com/lifegraph/arduino-lifegraph/master/examples/notificationlight/notificationlight.ino) into a new sketch.
3. Set the SSID, Password, and Access Token at the top of the file.
4. Connect an LED from pin 13 to ground.
5. Run. Press Command+Shift+M after it uploads to monitor the progress of the notification light.
6. To post an example status update, uncomment the relevant code from the `setup()` function.

## Low-level request API

Start a request with the following:

```
Facebook.get(char *path, char *access_token);
Facebook.post(char *path, char *access_token);
```

You can write content to a POST request with:

```
Facebook.form(char *name, char *value);
```

Finally, make a request with:

```
int status_code = Facebook.request(function *callback);
```

The callback function is called for each key or basic value in the return JSON, as it is being streamed to the device. Detecting for the presence of keys in the returned body is usually enough, for example, looking for `"error"` to detect an error or `"title"` to detect a new notification.
