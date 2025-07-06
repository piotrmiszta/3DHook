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

int main(void)
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_parsing_first_line),
    };
    return cmocka_run_group_tests(tests, NULL, NULL);
}
