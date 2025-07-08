set(TEST_TARGETS)

function(add_test_exe name source)
    add_executable(${name} ${source})
    target_link_libraries(${name} PRIVATE http_server_lib cmocka)
    add_test(NAME ${name} COMMAND ${name})
    list(APPEND TEST_TARGETS ${name})
    set(TEST_TARGETS "${TEST_TARGETS}" PARENT_SCOPE)
endfunction()

add_test_exe(test_str test_str.c)
add_test_exe(test_http_parser test_http_parser.c)

add_custom_target(run_all_tests
    COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure
    COMMENT "Running all unit tests after build"
)

add_dependencies(run_all_tests ${TEST_TARGETS})

add_custom_target(build_and_test ALL
    DEPENDS run_all_tests
)
