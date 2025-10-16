add_test(NAME ouverium_test_hello_world COMMAND $<TARGET_FILE:ouverium> ${CMAKE_CURRENT_SOURCE_DIR}/hello_world.fl)
add_test(NAME ouverium_test_string COMMAND $<TARGET_FILE:ouverium> ${CMAKE_CURRENT_SOURCE_DIR}/string.fl)
