#include "http_gen.h"
#include "database.h"
#include "logger.h"
#include "str.h"
#include <stdio.h>

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
                                                   "<h1>Produkty</h1>\n");

str_view_t DATABASE_END =
    CONST_STRING_VIEW_CSTR("<a href=\"home.html\"> Main page </a>\n"
                           "</body>\n"
                           "</html>\n");

str_t http_gen_get_page(str_view_t path, void *arg)
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
        Database *database = database_get_ptr();
        log_debug("Returned database page!\n");
        str_t string = string_from_str_view(&DATABASE_BEGIN);
        string_join_str_view(
            &string,
            &STRING_VIEW_CSTR("<table border='1'><tr><th>ID</th><th>Price</"
                              "th><th>Category</th></tr>"));
        for (s32 i = 0; i < database->size; i++)
        {
            string_join_str_view(&string, &STRING_VIEW_CSTR("<tr><th>"));
            char price[10];
            char category[10];
            char id[10];
            u32 price_size = sprintf(price, "%f", database->rows[i].price);
            u32 cat_size = sprintf(category, "%d", database->rows[i].category);
            u32 id_size = sprintf(id, "%d", database->rows[i].id);
            string_join_str_view(
                &string,
                &(str_view_t)string_view_create_from_cstr(id, id_size));
            string_join_str_view(&string, &STRING_VIEW_CSTR("</th><th>"));
            string_join_str_view(
                &string,
                &(str_view_t)string_view_create_from_cstr(price, price_size));
            string_join_str_view(&string, &STRING_VIEW_CSTR("</th><th>"));
            string_join_str_view(
                &string,
                &(str_view_t)string_view_create_from_cstr(category, cat_size));
            string_join_str_view(&string, &STRING_VIEW_CSTR("</th></tr>"));
        }

        string_join_str_view(&string, &DATABASE_END);
        return string;
    }
    else
    {
        return (str_t){nullptr, 0, false};
    }
}
