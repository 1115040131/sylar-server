file(GLOB SYLAR_TEST_SOURCES "${PROJECT_SOURCE_DIR}/tests/test*.cpp")

##########################################
# "make test_XYZ"
##########################################
foreach (sylar_test_source ${SYLAR_TEST_SOURCES})
    # Create a human readable name.
    get_filename_component(sylar_test_filename ${sylar_test_source} NAME)
    string(REPLACE ".cpp" "" sylar_test_name ${sylar_test_filename})

    # Add the test target separately and as part of "make check-tests".
    add_executable(${sylar_test_name} ${sylar_test_source})
    add_dependencies(${sylar_test_name} sylar)
    redefine_file_macro(${sylar_test_name}) #__FILE__
    target_link_libraries(${sylar_test_name} ${LIBS})

endforeach(sylar_test_source ${SYLAR_TEST_SOURCES})

# add_executable(test test.cpp)
# add_dependencies(test sylar)
# redefine_file_macro(test) #__FILE__
# target_link_libraries(test ${LIBS})

# add_executable(test_config test_config.cpp)
# add_dependencies(test_config sylar)
# redefine_file_macro(test_config) #__FILE__
# target_link_libraries(test_config ${LIBS})

# add_executable(test_thread test_thread.cpp)
# add_dependencies(test_thread sylar)
# redefine_file_macro(test_thread) #__FILE__
# target_link_libraries(test_thread ${LIBS})

# add_executable(test_util test_util.cpp)
# add_dependencies(test_util sylar)
# redefine_file_macro(test_util) #__FILE__
# target_link_libraries(test_util ${LIBS})

# add_executable(test_fiber test_fiber.cpp)
# add_dependencies(test_fiber sylar)
# redefine_file_macro(test_fiber) #__FILE__
# target_link_libraries(test_fiber ${LIBS})

SET(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR}/tests)
