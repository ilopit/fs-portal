add_library(fsp_server_applib private/src/server_entry.cpp)
target_link_libraries(fsp_server_applib PUBLIC engine)

add_executable(fsp_server private/src/server.cpp)
target_link_libraries(fsp_server fsp_server_applib)