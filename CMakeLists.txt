cmake_minimum_required(VERSION 3.16)
project(ymind VERSION 1.0.0 LANGUAGES CXX)

# ---- Global settings --------------------------------------------------------

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(BUILD_TESTING "Build unit tests" OFF)
option(ENABLE_CLANG_TIDY "Run clang-tidy during compilation" OFF)

# ---- Static analysis ---------------------------------------------------------

if(ENABLE_CLANG_TIDY)
    find_program(CLANG_TIDY_EXE NAMES clang-tidy REQUIRED)
    set(CMAKE_CXX_CLANG_TIDY
        ${CLANG_TIDY_EXE}
        --config-file=${CMAKE_SOURCE_DIR}/.clang-tidy
    )
endif()

# ---- Dependencies ------------------------------------------------------------

find_package(Qt6 REQUIRED COMPONENTS Widgets Svg PrintSupport)

# ---- Library (all sources except main.cpp) -----------------------------------

add_library(ymind_lib OBJECT
    # Core – application infrastructure
    src/core/AppSettings.h        src/core/AppSettings.cpp
    src/core/Commands.h           src/core/Commands.cpp
    src/core/FileManager.h        src/core/FileManager.cpp
    src/core/MainWindow.h         src/core/MainWindow.cpp
    src/core/SettingsDialog.h     src/core/SettingsDialog.cpp
    src/core/TemplateDescriptor.h src/core/TemplateDescriptor.cpp
    src/core/TemplateRegistry.h   src/core/TemplateRegistry.cpp

    # Scene – graphics-scene items
    src/scene/EdgeItem.h          src/scene/EdgeItem.cpp
    src/scene/MindMapScene.h      src/scene/MindMapScene.cpp
    src/scene/MindMapView.h       src/scene/MindMapView.cpp
    src/scene/NodeItem.h          src/scene/NodeItem.cpp

    # Layout – auto-layout algorithms
    src/layout/ILayoutAlgorithm.h
    src/layout/LayoutAlgorithmBase.h  src/layout/LayoutAlgorithmBase.cpp
    src/layout/BilateralLayout.h      src/layout/BilateralLayout.cpp
    src/layout/TopDownLayout.h        src/layout/TopDownLayout.cpp
    src/layout/RightTreeLayout.h      src/layout/RightTreeLayout.cpp
    src/layout/LayoutAlgorithmRegistry.h src/layout/LayoutAlgorithmRegistry.cpp
    src/layout/LayoutEngine.h     src/layout/LayoutEngine.cpp
    src/layout/LayoutStyle.h

    # UI – widgets and theming
    src/ui/IconFactory.h          src/ui/IconFactory.cpp
    src/ui/OutlineWidget.h        src/ui/OutlineWidget.cpp
    src/ui/StartPage.h            src/ui/StartPage.cpp
    src/ui/StyleSheetGenerator.h  src/ui/StyleSheetGenerator.cpp
    src/ui/TabManager.h           src/ui/TabManager.cpp
    src/ui/ThemeManager.h         src/ui/ThemeManager.cpp

    # Resources
    resources/resources.qrc
)

target_include_directories(ymind_lib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)

target_link_libraries(ymind_lib PUBLIC
    Qt6::Widgets
    Qt6::Svg
    Qt6::PrintSupport
)

# ---- Executable --------------------------------------------------------------

if(WIN32)
    add_executable(ymind WIN32)
else()
    add_executable(ymind)
endif()

target_sources(ymind PRIVATE src/main.cpp)
target_link_libraries(ymind PRIVATE ymind_lib)

# ---- Install -----------------------------------------------------------------

install(TARGETS ymind DESTINATION bin)

# ---- Testing -----------------------------------------------------------------

if(BUILD_TESTING)
    enable_testing()
    add_subdirectory(tests)
endif()
