#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <setjmp.h>
#include <cmocka.h>
#include "str.h"

static void test_string_create(void** state) {
    str_t string = string_create_from_cstr("TEST");
    assert_int_equal(4, string.size);
    assert_int_equal(4, string.capacity);
    assert_int_equal(true, string.valid);
    assert_int_equal(string_equal_cstr(&string, "TEST", 4), true);
    string_free(&string);
}

static void test_string_to_lower(void **state) {
    str_t string = string_create_from_cstr("TEST");
    str_t expected = string_create_from_cstr("test");
    string_to_lower(&string);
    assert_int_equal(string_equal(&string, &expected), true);
    string_free(&string);
    string_free(&expected);
}

static void test_string_to_upper(void **state) {
    str_t expected = string_create_from_cstr("TEST");
    str_t string = string_create_from_cstr("test");
    string_to_upper(&string);
    assert_int_equal(string_equal(&string, &expected), true);
    string_free(&string);
    string_free(&expected);
}

int main(void) {
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_string_create),
        cmocka_unit_test(test_string_to_lower),
        cmocka_unit_test(test_string_to_upper),
    };
    return cmocka_run_group_tests(tests, NULL, NULL);
}
