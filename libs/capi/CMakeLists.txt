FILE(GLOB_RECURSE capi_sources src/*)

add_library(ouverium_capi ${capi_sources})
target_include_directories(ouverium_capi PUBLIC include)
target_link_libraries(ouverium_capi PRIVATE ouverium_options)

target_link_libraries(ouverium_capi PUBLIC ouverium_types)
