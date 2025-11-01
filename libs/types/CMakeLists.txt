add_library(ouverium_types INTERFACE)
target_include_directories(ouverium_types INTERFACE include)

file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/include DESTINATION ${CMAKE_BINARY_DIR})

install(DIRECTORY include/
    DESTINATION include
)
