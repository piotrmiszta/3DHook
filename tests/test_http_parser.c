// disable clang-format becouse cmocka need to be included after
// setjmp.h, stdarg.h, stddef.h, strint.h
// clang-foramt off
#include <setjmp.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>

#include <cmocka.h>
// clang-format on

#include "http_parser.h"
#include "str.h"

static void test_parsing_first_line(void **state)
{
    str_t message = string_create_from_cstr("GET /home.html HTTP/1.1\n");
    HttpMessage msg;
    http_message_parse(&msg, message);
    assert_int_equal(
        string_view_equal(&msg.url, &STRING_VIEW_CSTR("/home.html")), 1);
    assert_int_equal(msg.method, HTTP_METHOD_GET);
    assert_int_equal(msg.version, HTTP_VERSION_1_1);
}

static void test_parse_http_header(void **state)
{
    str_t message = string_create_from_cstr("GET /home.html HTTP/1.1\n"
                                            "Host: developer\n"
                                            "User-Agent: Mozilla\n"
                                            "Accept: text/html\n"
                                            "Accept-Language: en-US\n");
    HttpMessage msg;
    http_message_parse(&msg, message);
    assert_int_equal(
        string_view_equal(&msg.url, &STRING_VIEW_CSTR("/home.html")), 1);
    assert_int_equal(msg.method, HTTP_METHOD_GET);
    assert_int_equal(msg.version, HTTP_VERSION_1_1);
    assert_int_equal(msg.elements_count, 4);
    assert_int_equal(
        string_view_equal(&STRING_VIEW_CSTR("Host"), &msg.elements[0].key),
        true);

    assert_int_equal(string_view_equal(&STRING_VIEW_CSTR("developer"),
                                       &msg.elements[0].value),
                     true);

    assert_int_equal(string_view_equal(&STRING_VIEW_CSTR("User-Agent"),
                                       &msg.elements[1].key),
                     true);

    assert_int_equal(
        string_view_equal(&STRING_VIEW_CSTR("Mozilla"), &msg.elements[1].value),
        true);

    assert_int_equal(
        string_view_equal(&STRING_VIEW_CSTR("Accept"), &msg.elements[2].key),
        true);

    assert_int_equal(string_view_equal(&STRING_VIEW_CSTR("text/html"),
                                       &msg.elements[2].value),
                     true);

    assert_int_equal(string_view_equal(&STRING_VIEW_CSTR("Accept-Language"),
                                       &msg.elements[3].key),
                     true);

    assert_int_equal(
        string_view_equal(&STRING_VIEW_CSTR("en-US"), &msg.elements[3].value),
        true);
}

int main(void)
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_parsing_first_line),
        cmocka_unit_test(test_parse_http_header)};
    return cmocka_run_group_tests(tests, NULL, NULL);
}
