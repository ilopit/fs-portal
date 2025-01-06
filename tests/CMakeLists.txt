add_executable(ll-bridge-tests 
    test_llbridge.cpp
    utils_tests.cpp
    fileapi_test.cpp
    test_binary_stream.cpp
    file_cache_test.cpp
    test_cli.cpp
    file_recombinator_test.cpp
    test_helpers.h
    test_helpers.cpp
    test_secure_session.cpp
    test_messages.cpp
    testing_utils.h
)

target_link_libraries(ll-bridge-tests PUBLIC engine gtest gtest_main botan fsp_server_applib fsp_client_applib)
target_include_directories(ll-bridge-tests PUBLIC "${PROJECT_SOURCE_DIR}/libs/engine/private/include"  )