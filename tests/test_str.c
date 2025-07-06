#include "str.h"

// disable clang-format becouse cmocka need to be included after
// setjmp.h, stdarg.h, stddef.h, strint.h
// clang-foramt off
#include <setjmp.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>

#include <cmocka.h>
// clang-format on

static void test_string_create(void **state)
{
    str_t string = string_create_from_cstr("TEST");
    assert_int_equal(4, string.size);
    assert_int_equal(4, string.capacity);
    assert_int_equal(true, string.valid);
    assert_int_equal(string_equal_cstr(&string, "TEST", 4), true);
    string_free(&string);
}

static void test_string_to_lower(void **state)
{
    str_t string = string_create_from_cstr("TEST");
    str_t expected = string_create_from_cstr("test");
    string_to_lower(&string);
    assert_int_equal(string_equal(&string, &expected), true);
    string_free(&string);
    string_free(&expected);
}

static void test_string_to_upper(void **state)
{
    str_t expected = string_create_from_cstr("TEST");
    str_t string = string_create_from_cstr("test");
    string_to_upper(&string);
    assert_int_equal(string_equal(&string, &expected), true);
    string_free(&string);
    string_free(&expected);
}

static void test_string_find(void **state)
{
    str_t string = string_create_from_cstr("This is simple str!");
    assert_int_equal(string_find(&string, 'T'), 0);
    assert_int_equal(string_find(&string, 'i'), 2);
    assert_int_equal(string_find(&string, '!'), 18);
    assert_int_equal(string_find(&string, 'a'), STRING_NOT_FOUND);
    string_free(&string);
}

static void test_string_find_substr(void **state)
{
    str_t string = string_create_from_cstr("This is simple str!");
    str_t substr1 = string_create_from_cstr("This ");
    str_t substr2 = string_create_from_cstr("simple str!");

    assert_int_equal(string_substr(&string, &substr1), 0);
    assert_int_equal(string_substr(&string, &substr2), 8);

    string_free(&string);
    string_free(&substr1);
    string_free(&substr2);
}

static void test_tokenizer(void **state)
{
    str_t string =
        string_create_from_cstr("This is string;with-multiple:delims");
    str_view_t str_view = string_view_create_from_string(&string);
    str_view_t delims = STRING_VIEW_CSTR(" ;:-");
    auto token = string_tokenizer_init(&str_view, &delims);
    auto str = string_tokenizer_next(&token);
    auto this = STRING_VIEW_CSTR("This");
    assert_int_equal(string_view_equal(&str, &this), true);
    str = string_tokenizer_next(&token);
    this = STRING_VIEW_CSTR("is");
    assert_int_equal(string_view_equal(&str, &this), true);
    str = string_tokenizer_next(&token);
    this = STRING_VIEW_CSTR("string");
    assert_int_equal(string_view_equal(&str, &this), true);
    str = string_tokenizer_next(&token);
    this = STRING_VIEW_CSTR("with");
    assert_int_equal(string_view_equal(&str, &this), true);
    str = string_tokenizer_next(&token);
    this = STRING_VIEW_CSTR("multiple");
    assert_int_equal(string_view_equal(&str, &this), true);
    str = string_tokenizer_next(&token);
    this = STRING_VIEW_CSTR("delims");
    assert_int_equal(string_view_equal(&str, &this), true);
    str = string_tokenizer_next(&token);
    assert_int_equal(token.valid, false);
    assert_int_equal(str.size, 0);
    assert_null(str.data);
    assert_int_equal(str.valid, false);
}

int main(void)
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_string_create),
        cmocka_unit_test(test_string_to_lower),
        cmocka_unit_test(test_string_to_upper),
        cmocka_unit_test(test_string_find),
        cmocka_unit_test(test_string_find_substr),
        cmocka_unit_test(test_tokenizer),
    };
    return cmocka_run_group_tests(tests, NULL, NULL);
}
