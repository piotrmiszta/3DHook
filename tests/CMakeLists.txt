add_executable(test_str test_str.c )
target_link_libraries(test_str PRIVATE http_server_lib cmocka)

add_test(NAME test_str COMMAND test_str)

add_executable(test_http_parser test_http_parser.c)
target_link_libraries(test_http_parser PRIVATE http_server_lib cmocka)

add_test(NAME test_http_parser COMMAND test_http_parser)
