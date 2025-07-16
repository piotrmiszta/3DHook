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

str_view_t http_gen_get_page(str_view_t path)
{
    if (string_view_equal(&path, &STRING_VIEW_CSTR("/")) == true)
    {
        log_debug("Returned welcome page!");
        return WELCOME_PAGE;
    }
    else
    {
        return (str_view_t){nullptr, 0, false};
    }
}
