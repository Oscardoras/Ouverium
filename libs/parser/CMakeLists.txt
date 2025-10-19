FILE(GLOB_RECURSE parser_sources src/*)

add_library(ouverium_parser ${parser_sources})
target_include_directories(ouverium_parser PUBLIC include)
target_link_libraries(ouverium_parser PRIVATE ouverium_options)

target_link_libraries(ouverium_parser PUBLIC ouverium_types)
