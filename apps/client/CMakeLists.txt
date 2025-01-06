add_library(fsp_client_applib private/src/client_entry.cpp)
target_link_libraries(fsp_client_applib PUBLIC engine)

add_executable(fsp_client private/src/client.cpp)
target_link_libraries(fsp_client fsp_client_applib)