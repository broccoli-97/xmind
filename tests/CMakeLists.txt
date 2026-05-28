find_package(Qt6 REQUIRED COMPONENTS Test)

function(add_ymind_test TEST_NAME)
    add_executable(${TEST_NAME} ${TEST_NAME}.cpp)
    target_link_libraries(${TEST_NAME} PRIVATE ymind_lib Qt6::Test)
    target_compile_definitions(${TEST_NAME} PRIVATE
        YMIND_SOURCE_DIR="${CMAKE_SOURCE_DIR}"
    )
    add_test(NAME ${TEST_NAME} COMMAND ${TEST_NAME})
    # Allow headless execution (needed for tests that create QApplication)
    set_tests_properties(${TEST_NAME} PROPERTIES
        ENVIRONMENT "QT_QPA_PLATFORM=offscreen"
    )
endfunction()

# Tier 1 -- pure data, no QApplication needed
add_ymind_test(tst_TemplateDescriptor)
add_ymind_test(tst_ThemeDescriptor)
add_ymind_test(tst_LayoutStyle)
add_ymind_test(tst_TranslationCoverage)
add_ymind_test(tst_ViewGeometry)

# Tier 2 -- singleton registries
add_ymind_test(tst_TemplateRegistry)
add_ymind_test(tst_ThemeRegistry)
add_ymind_test(tst_LayoutAlgorithmRegistry)
add_ymind_test(tst_AppSettings)

# Tier 3 -- requires QApplication
add_ymind_test(tst_MindMapSceneSerialization)
add_ymind_test(tst_MindMapSceneKeys)
add_ymind_test(tst_OutlineWidget)
add_ymind_test(tst_RecoveryManager)
add_ymind_test(tst_MarkdownImport)
add_ymind_test(tst_MarkdownImportStrict)
