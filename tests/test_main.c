#include "wrt_sdl.h"

#include<stdbool.h>
#include<stdio.h>

bool test_single_deserialize()
{
    struct {
        int a;
        int b;
        int c;
    } x, y;
    x.a = 7;
    x.b = 99;
    x.c = 194;

    char buffer[100] = { 0 };
    size_t len = wsdl_serialize((uint8_t*) &x, sizeof(x), buffer, sizeof(buffer));

    size_t len_out = wsdl_deserialize((uint8_t*) &y, sizeof(y), buffer, len);

    if (y.a != x.a || y.b != x.b || y.c != x.c) {
        return false;
    } else {
        return true;
    }
}

bool test_single_deserialize_byte_by_byte()
{
    struct {
        int a;
        int b;
        int c;
    } x, y;
    x.a = 7;
    x.b = 99;
    x.c = 194;

    char buffer[100] = { 0 };
    char *b = buffer;
    size_t len = wsdl_serialize((uint8_t*) &x, sizeof(x), buffer, sizeof(buffer));

    wsdl_ctx_t ctx;
    wsdl_begin_deserialization(&ctx, (uint8_t*) &y, sizeof(y));
    while(wsdl_deserialize_byte(&ctx, *(b++))) ;

    if (y.a != x.a || y.b != x.b || y.c != x.c) {
        return false;
    } else {
        return true;
    }
}

bool test_deserialize_single_byte()
{
    /*
     * There was a bug in the early version of this library where if you tried
     * to deserialize a single byte (which is 2 base 64 encoded characters),
     * the library would overwrite memory adjacent to where you wanted it to
     * (because it thought it wanted to decode two full base64 characters,
     * which is 12 bytes of data, which it tried to put in 2 bytes of memory,
     * when it should only touch 1).  This is real bad, since overwriting
     * memory you aren't supposed to is real bad. This test makes sure you
     * don't overwrite adjacent memory. Padding 1,2,3 shouldn't be modified by
     * decoding into test_out.
     */
    uint8_t padding1 = 0xff;
    uint8_t test_in = 0xca;
    uint8_t padding2 = 0xff;
    uint8_t test_out = 0;
    uint8_t padding3 = 0xff;
    
    char buffer[2];
    char *b = buffer;
    size_t len = wsdl_serialize(&test_in, sizeof(test_in), buffer, sizeof(buffer));
    if (len == 0)
        return false;

    wsdl_deserialize(&test_out, sizeof(test_out), buffer, len);

    //printf("p1: %p, ti: %p, p2: %p, to: %p, p3: %p\n", &padding1, &test_in, &padding2, &test_out, &padding3);
    //printf("p1: %d p2: %d p3: %d\n", padding1, padding2, padding3);
    //printf("test_in: %d, test_out: %d\n", test_in, test_out);

    if (test_out != test_in || padding1 != 0xff || padding2 != 0xff || padding3 != 0xff)
        return false;
    return true;
}

#define TEST(x) \
    if (! (x) ) { \
        printf("test failed! " #x "\n"); \
    } else { \
        printf("test passed! " #x "\n"); \
    }

int main()
{
    TEST(test_single_deserialize());
    TEST(test_single_deserialize_byte_by_byte());
    TEST(test_deserialize_single_byte());
}
