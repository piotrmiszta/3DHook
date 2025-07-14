// disable clang-format becouse cmocka need to be included after
// setjmp.h, stdarg.h, stddef.h, strint.h
// clang-foramt off
#include <setjmp.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>

#include <cmocka.h>
// clang-format on

#include "err_codes.h"
#include "http_parser.h"
#include "str.h"
#include "worker.h"

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

static void test_http_parser_real_get_message(void **state)
{
    str_t message = string_create_from_cstr(
        "GET /home.html HTTP/1.1\n"
        "Host: developer.mozilla.org\n"
        "User-Agent: Mozilla/5.0 (Macintosh; Intel Mac OS X 10.9; rv:50.0) "
        "Gecko/20100101 Firefox/50.0\n"
        "Accept: "
        "text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8\n"
        "Accept-Language: en-US,en;q=0.5\n"
        "Accept-Encoding: gzip, deflate, br\n"
        "Referer: https://developer.mozilla.org/testpage.html\n"
        "Connection: keep-alive\n"
        "Upgrade-Insecure-Requests: 1\n"
        "If-Modified-Since: Mon, 18 Jul 2016 02:36:04 GMT\n"
        "If-None-Match: \"c561c68d0ba92bbeb8b0fff2a9199f722e3a621a\"\n"
        "Cache-Control: max-age=0\n");
    HttpMessage msg;
    http_message_parse(&msg, message);
    assert_int_equal(
        string_view_equal(&msg.url, &STRING_VIEW_CSTR("/home.html")), 1);
    assert_int_equal(msg.method, HTTP_METHOD_GET);
    assert_int_equal(msg.version, HTTP_VERSION_1_1);
    assert_int_equal(msg.elements_count, 11);
    str_t string;
    worker_process(&msg, 0, &string);
}

int main(void)
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_parsing_first_line),
        cmocka_unit_test(test_parse_http_header),
        cmocka_unit_test(test_http_parser_real_get_message)};

    return cmocka_run_group_tests(tests, NULL, NULL);
}
