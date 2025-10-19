FILE(GLOB_RECURSE interpreter_sources src/*)

add_library(ouverium_interpreter ${interpreter_sources})
target_include_directories(ouverium_interpreter PUBLIC include)
target_compile_features(ouverium_interpreter PRIVATE cxx_std_20)

target_link_libraries(ouverium_interpreter PUBLIC ouverium_types)
target_link_libraries(ouverium_interpreter PUBLIC ouverium_parser)
target_link_libraries(ouverium_interpreter PRIVATE Boost::asio Boost::dll)
if(OUVERIUM_WITH_WXWIDGET)
    target_link_libraries(ouverium_interpreter PRIVATE wx::core wx::base wx::webview)
    target_compile_definitions(ouverium_interpreter PRIVATE OUVERIUM_WXWIDGETS)
endif()
