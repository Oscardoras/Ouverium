FILE(GLOB_RECURSE capi_sources src/*)

add_library(ouverium_capi ${capi_sources})
target_include_directories(ouverium_capi PUBLIC include)
target_compile_features(ouverium_capi PRIVATE c_std_99)
target_compile_options(ouverium_capi PRIVATE $<$<CXX_COMPILER_ID:GNU>:-Wall -Wextra -Wpedantic>)

target_link_libraries(ouverium_capi PUBLIC ouverium_types)

file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/include DESTINATION ${CMAKE_BINARY_DIR})

install(TARGETS ouverium_capi)
install(DIRECTORY include/
    DESTINATION include
)
