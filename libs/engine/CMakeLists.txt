add_library(engine 
    "public/include/engine/client.h"
    "public/include/engine/files_list.h"
    "public/include/engine/message.h"
    "public/include/engine/message_bridge.h"
    "public/include/engine/server.h"
    "public/include/engine/communication_context.h"

    "private/include/engine/private/secure_session.h"
    "private/include/engine/private/binary_stream.h"
    "private/include/engine/private/client_transport_context.h"
    "private/include/engine/private/client_impl.h"
    "private/include/engine/private/file_recombinator.h"
    "private/include/engine/private/session.h"
    "private/include/engine/private/server_context.h"
    "private/include/engine/private/server_impl.h"
    "private/include/engine/private/utils.h"
    "private/include/engine/private/psx_file.h"

    "private/src/secure_session.cpp"
    "private/src/binary_stream.cpp"
    "private/src/client.cpp"
    "private/src/client_impl.cpp"
    "private/src/client_transport_context.cpp"
    "private/src/files_list.cpp"
    "private/src/file_read_cache.cpp"
    "private/src/psx_file.cpp"
    "private/src/file_recombinator.cpp"
    "private/src/message.cpp"
    "private/src/message_bridge.cpp"
    "private/src/server.cpp"
    "private/src/communication_context.cpp"
    "private/src/session.cpp"
    "private/src/server_context.cpp"
    "private/src/server_impl.cpp"
    "private/src/utils.cpp"
)

target_link_libraries(engine Boost::asio spdlog::spdlog botan)

target_include_directories(engine PUBLIC ${CMAKE_CURRENT_LIST_DIR}/public/include)
target_include_directories(engine PRIVATE ${CMAKE_CURRENT_LIST_DIR}/private/include)