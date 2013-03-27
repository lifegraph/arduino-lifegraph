#include <Arduino.h>
#include <stdint.h>

/* user callback type */
typedef int ( * js0n_user_cb_t ) ( struct js0n_parser * parser );

typedef enum {
    JSON_START_MAP,
    JSON_END_MAP,
    JSON_START_ARRAY,
    JSON_END_ARRAY,
    JSON_MAP_KEY,
    JSON_NULL,
    JSON_TRUE,
    JSON_FALSE,
    JSON_STRING,
    JSON_DOUBLE
} JSON_TOKEN;

typedef struct js0n_parser
{
    // public:
    Stream *stream;
    js0n_user_cb_t user_cb;
    uint8_t *buffer;
    uint16_t token_length;
    JSON_TOKEN token_type;

    // private:
    uint8_t current;
    uint16_t cursor;
    uint8_t depth;
    uint16_t length;
    uint8_t utf8_remain;
    uint16_t gostate;
    uint16_t mark;
    uint8_t live;
} js0n_parser_t;

int js0n_parse ( js0n_parser_t * parser );