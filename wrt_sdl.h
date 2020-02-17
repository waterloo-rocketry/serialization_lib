#ifndef WRT_SDL_H_
#define WRT_SDL_H_

#include <stdint.h>
#include <stddef.h>

// Waterloo rocketry Serialization/Deserialization Library Context type
typedef struct {
    size_t chars_decoded;
    size_t chars_to_decode;
    uint8_t *data;
    uint8_t offset;
    uint8_t finished;
} wsdl_ctx_t;

/*
 * Initialize a new deserialization session.
 *
 * Must be called before deserializing a new string.
 *
 * returns 0 on success, negative on failure
 */
int wsdl_begin_deserialization(wsdl_ctx_t *ctx,
                               uint8_t *data_out,
                               size_t data_len);

/*
 * Returns negative on error,
 * 0 on completed transaction,
 * positive on continuation
 */
int wsdl_deserialize_byte(wsdl_ctx_t *ctx, char encoded);

/*
 * Returns number of bytes deserialized
 */
size_t wsdl_deserialize(uint8_t *data,
                        size_t data_len,
                        const char *encoded,
                        size_t encoded_len);

/*
 * Returns number of bytes put into encoded
 */
size_t wsdl_serialize(const uint8_t *data,
                      size_t data_len,
                      char *encoded,
                      size_t max_encoded_len);

#endif
