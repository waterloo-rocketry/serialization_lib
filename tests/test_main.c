#include "wrt_sdl.h"

#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>

static void test_single_deserialize()
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

    assert_int_equal(x.a, y.a);
    assert_int_equal(x.b, y.b);
    assert_int_equal(x.c, y.c);
    assert_int_equal(len, len_out);
}

static void test_single_deserialize_byte_by_byte()
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

    assert_int_equal(x.a, y.a);
    assert_int_equal(x.b, y.b);
    assert_int_equal(x.c, y.c);
}

static void test_deserialize_single_byte()
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
    assert_int_equal(len, 2);

    wsdl_deserialize(&test_out, sizeof(test_out), buffer, len);

    assert_int_equal(test_out, test_in);
    assert_int_equal(padding1, 0xff);
    assert_int_equal(padding2, 0xff);
    assert_int_equal(padding3, 0xff);
}

int main()
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_single_deserialize),
        cmocka_unit_test(test_single_deserialize_byte_by_byte),
        cmocka_unit_test(test_deserialize_single_byte),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
