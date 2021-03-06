#ifndef WRT_SDL_H_
#define WRT_SDL_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stddef.h>

#define WSDL_VERSION "v1.0.0"

// Waterloo rocketry Serialization/Deserialization Library Context type
typedef struct {
    size_t bytes_decoded;
    size_t bytes_to_decode;
    uint8_t checksum_incremental;
    uint8_t *data;
    uint8_t offset;
    uint8_t finished;
} wsdl_ctx_t;

/*
 * Macro for determining how long a buffer is needed to hold the serialized
 * form of a given data structure.
 *
 * Declared as a macro instead of a function so that you can declare a buffer
 * at compile time using something like `char
 * buffer[WSDL_SER_LEN(actuator_struct_t)` or something. Yay embedded programs
 * and not having access to dynamic memory allocation!
 *
 * Every 6 bits of data becomes a bute of output, and there's one byte at the
 * end for the checksum, so the length of the output string is
 * ceil(sizeof(type) * 8/6) + 1
 */
#define WSDL_SER_LEN(type) ((sizeof(type) * 8 + 5) / 6 + 1)

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

#ifdef __cplusplus
} // extern "C"
#endif

#endif
