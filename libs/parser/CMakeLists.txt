FILE(GLOB_RECURSE parser_sources src/*)

add_library(ouverium_parser ${parser_sources})
target_include_directories(ouverium_parser PUBLIC include)
target_compile_features(ouverium_parser PRIVATE cxx_std_20)

target_link_libraries(ouverium_parser PUBLIC ouverium_types)
