find_package(Qt6 REQUIRED COMPONENTS Test)

function(add_xmind_test TEST_NAME)
    add_executable(${TEST_NAME} ${TEST_NAME}.cpp)
    target_link_libraries(${TEST_NAME} PRIVATE xmind_lib Qt6::Test)
    add_test(NAME ${TEST_NAME} COMMAND ${TEST_NAME})
    # Allow headless execution (needed for tests that create QApplication)
    set_tests_properties(${TEST_NAME} PROPERTIES
        ENVIRONMENT "QT_QPA_PLATFORM=offscreen"
    )
endfunction()

# Tier 1 -- pure data, no QApplication needed
add_xmind_test(tst_TemplateDescriptor)
add_xmind_test(tst_LayoutStyle)

# Tier 2 -- singleton registries
add_xmind_test(tst_TemplateRegistry)
add_xmind_test(tst_LayoutAlgorithmRegistry)
add_xmind_test(tst_AppSettings)

# Tier 3 -- requires QApplication
add_xmind_test(tst_MindMapSceneSerialization)
