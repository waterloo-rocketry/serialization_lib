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
}
