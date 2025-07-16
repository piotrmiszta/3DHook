#include "http_gen.h"
#include "logger.h"
#include "str.h"

str_view_t WELCOME_PAGE =
    CONST_STRING_VIEW_CSTR("<!DOCTYPE html>\n"
                           "<html>\n"
                           "<body>\n"
                           "<h1>My First Heading</h1>\n"
                           "<p>My first paragraph.</p>\n"
                           "<a href=\"database.html\"> Database </a>\n"
                           "</body>\n"
                           "</html>\n");

str_view_t DATABASE_BEGIN = CONST_STRING_VIEW_CSTR("<!DOCTYPE html>\n"
                                                   "<html>\n"
                                                   "<body>\n"
                                                   "<h1>Database page</h1>\n");

str_view_t DATABASE_END =
    CONST_STRING_VIEW_CSTR("<a href=\"home.html\"> Main page </a>\n"
                           "</body>\n"
                           "</html>\n");

str_t http_gen_get_page(str_view_t path)
{
    if (string_view_equal(&path, &STRING_VIEW_CSTR("/")) == true)
    {
        log_debug("Returned welcome page!\n");
        str_t string = string_from_str_view(&WELCOME_PAGE);
        return string;
    }
    else if (string_view_equal(&path, &STRING_VIEW_CSTR("/home.html")) == true)
    {
        log_debug("Returned welcome page!\n");
        str_t string = string_from_str_view(&WELCOME_PAGE);
        return string;
    }
    else if (string_view_equal(&path, &STRING_VIEW_CSTR("/database.html")) ==
             true)
    {
        log_debug("Returned database page!\n");
        str_t string = string_from_str_view(&DATABASE_BEGIN);
        string_join_str_view(&string, &DATABASE_END);
        return string;
    }
    else
    {
        log_error("Unknown url!\n");
        return (str_t){nullptr, 0, false};
    }
}
