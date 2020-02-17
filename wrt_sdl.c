#include "wrt_sdl.h"

static char encode(uint8_t binary)
{
    if (binary >= 0 && binary <= 25) {
        binary += 65; //map to 'A' (ascii value 65). So add 65
    } else if (binary >= 26 && binary <= 51) {
        binary += 71; //map to 'a' (ascii value 97). So add 71
    } else if (binary >= 52 && binary <= 61) {
        binary -= 4; //map to '0' (ascii value 48). So subtract 4
    } else if (binary == 62) {
        binary -= 19; //map to '+' (ascii value 43). So subtract 19
    } else if (binary == 63) {
        binary -= 16; //map to '/' (ascii value 47). So subtract 16
    } else {
        return -1; //we have failed. So return -1
    }
    return binary;
}

static uint8_t decode(char base64)
{
    if ( base64 >= 'A' && base64 <= 'Z' ) {
        base64 -= 65; //we need to map 'A' (ascii value 65) to 0. So subtract 65
    } else if (base64 >= 'a' && base64 <= 'z') {
        base64 -= 71; //we need to map 'a' (ascii value 97) to 26. So subtract 71
    } else if (base64 >= '0' && base64 <= '9') {
        base64 += 4; //we need to map '0' (ascii value 48) to 52. So add 4
    } else if (base64 == '+') {
        base64 += 19; //we need to map '+' (ascii value 43) to 62. So add 19
    } else if (base64 == '/') {
        base64 += 16; //we need to map '/' (ascii value 47) to 63. so add 16
    } else {
        return -1; //we have failed. So return -1
    }
    return base64;
}

static size_t chars_in_bytes(size_t data_len)
{
    return (data_len * 8 + 5) / 6;
}

int wsdl_begin_deserialization(wsdl_ctx_t *ctx, uint8_t *data_out, size_t data_len)
{
    if (ctx == NULL || data_out == NULL)
        return -1;

    ctx->bytes_decoded = 0;
    ctx->bytes_to_decode = data_len;
    ctx->data = data_out;
    ctx->offset = 8;
    ctx->finished = 0;

    return 0;
}

int wsdl_deserialize_byte(wsdl_ctx_t *ctx, char encoded)
{
    if (ctx == NULL || ctx->data == NULL) {
        return -1;
    }

    if (ctx->finished) {
        return -1;
    }

    switch (ctx->offset) {
        case 8:
            // decode char into data[7:2]
            *(ctx->data) = decode(encoded) << 2;
            ctx->offset = 2;
            break;
        case 6:
            // decode char into data[5:0]
            *(ctx->data) |= decode(encoded);
            ctx->data++;
            ctx->bytes_decoded++;
            ctx->offset = 8;
            break;
        case 4:
            // decode *iter into data[3:0] and (data+1)[7:6]
            *(ctx->data) |= decode(encoded) >> 2;
            ctx->data++;
            ctx->bytes_decoded++;
            if (ctx->bytes_decoded < ctx->bytes_to_decode) {
                *(ctx->data) = (decode(encoded) & 0x3) << 6;
                ctx->offset = 6;
            }
            break;
        case 2:
            // decode *iter into data[1:0] and (data+1)[7:4]
            *(ctx->data) |= decode(encoded) >> 4;
            ctx->data++;
            ctx->bytes_decoded++;
            if (ctx->bytes_decoded < ctx->bytes_to_decode) {
                *(ctx->data) = (decode(encoded) & 0xf) << 4;
                ctx->offset = 4;
            }
            break;
        default:
            return -1;
    }

    if (ctx->bytes_decoded == ctx->bytes_to_decode) {
        ctx->finished = 1;
        return 0;
    } else {
        return 1;
    }
}

size_t wsdl_deserialize(uint8_t *data, size_t data_len, const char *ser, size_t ser_len)
{
    wsdl_ctx_t ctx;
    wsdl_begin_deserialization(&ctx, data, data_len);
    if (ser_len < chars_in_bytes(data_len)) {
        return 0;
    } else {
        size_t counter = 1;
        int x;
        while ((x = wsdl_deserialize_byte(&ctx, *ser++))) {
            if (x < 0) {
                // we got an error, so return error condition
                return 0;
            }
            counter++;
        }
        return counter;
    }
}

size_t wsdl_serialize(const uint8_t *data, size_t data_len, char *out, size_t out_len)
{
    // every 6 bits of data becomes 8 bits of out. And we append
    // a 1 byte checksum to out. Therefore,
    // 6/8*data_len + 1 < out_len
    // data_len <= ceil(6/8 * out_len) - 1
    if (data_len > (((out_len * 6 + 7) / 8) - 1)) {
        return 0;
    }

    char *out_orig = out;
    const uint8_t *iter = data;
    uint8_t offset = 8;

    while (iter < data + data_len) {
        switch (offset) {
            case 8:
                // move *iter[7:2] into out. Do not increment iter.
                *out = encode( (*iter & 0xfc) >> 2 );
                out++;
                offset = 2;
                break;
            case 6:
                // move *iter[5:0] into out, increment iter.
                *out = encode(*iter & 0x3f);
                out++;
                iter++;
                offset = 8;
                break;
            case 4:
                // move *iter[3:0] into out[5:2], increment iter, move iter[7:6] into out[1:0]
                *out = ((*iter & 0xf) << 2);
                iter++;
                *out |= ((*iter & 0xc0) >> 6);
                *out = encode(*out);
                out++;
                offset = 6;
                break;
            case 2:
                // move *iter[1:0] into out[5:4], increment iter, move iter[7:4] into out[3:0]
                *out = ((*iter & 0x3) << 4);
                iter++;
                *out |= ((*iter & 0xf0) >> 4);
                *out = encode(*out);
                out++;
                offset = 4;
                break;
            default:
                return 0;
        }
    }
    return out - out_orig;
}
