add_executable(test_str test_str.c)
target_link_libraries(test_str PRIVATE http_server_lib cmocka)

add_test(NAME test_str COMMAND test_str)
