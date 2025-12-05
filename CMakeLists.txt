cmake_minimum_required(VERSION 3.20)
project(Titan VERSION 8.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Tell CMake to look in the 'include' folder for .hpp files
include_directories(include)

# --- Dependencies Management ---
include(FetchContent)

# 1. Google Test (For OrderBook tests)
FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

# 2. GLFW (Windowing System)
FetchContent_Declare(
        glfw
        GIT_REPOSITORY https://github.com/glfw/glfw.git
        GIT_TAG        3.3.8
)
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(glfw)

# 3. OpenGL (Graphics Driver)
find_package(OpenGL REQUIRED)

# 4. Dear ImGui (UI Library - Docking Branch)
FetchContent_Declare(
        imgui
        GIT_REPOSITORY https://github.com/ocornut/imgui.git
        GIT_TAG        docking
)
FetchContent_MakeAvailable(imgui)

# 5. ImPlot (Graphing Library)
FetchContent_Declare(
        implot
        GIT_REPOSITORY https://github.com/epezent/implot.git
        GIT_TAG        master
)
FetchContent_MakeAvailable(implot)

# 6. STB Image (Image Loading for Rocket)
FetchContent_Declare(
        stb
        GIT_REPOSITORY https://github.com/nothings/stb.git
        GIT_TAG        master
)
FetchContent_MakeAvailable(stb)

# --- Build the UI Library ---
# We bundle ImGui, ImPlot, and the OpenGL/GLFW backends into one library named 'imgui_lib'
add_library(imgui_lib STATIC
        ${imgui_SOURCE_DIR}/imgui.cpp
        ${imgui_SOURCE_DIR}/imgui_demo.cpp
        ${imgui_SOURCE_DIR}/imgui_draw.cpp
        ${imgui_SOURCE_DIR}/imgui_tables.cpp
        ${imgui_SOURCE_DIR}/imgui_widgets.cpp
        ${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp
        ${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp
        ${implot_SOURCE_DIR}/implot.cpp
        ${implot_SOURCE_DIR}/implot_items.cpp
        ${implot_SOURCE_DIR}/implot_demo.cpp
)

# Tell the compiler where to find the headers for these libraries
target_include_directories(imgui_lib PUBLIC
        ${imgui_SOURCE_DIR}
        ${imgui_SOURCE_DIR}/backends
        ${implot_SOURCE_DIR}
        ${stb_SOURCE_DIR}
)

# Link the system graphics libraries to our UI library
target_link_libraries(imgui_lib PUBLIC glfw OpenGL::GL)

# --- Main Executable (Titan) ---
# This builds your game using the new split structure
add_executable(Titan
        src/main.cpp
        src/TitanEngine.cpp
        src/TitanGUI.cpp
        src/OrderBook.cpp
)

# Link your app to the UI library we built above
target_link_libraries(Titan PRIVATE imgui_lib)

# --- Test Suite ---
enable_testing()
add_executable(TitanTests
        tests/OrderBookTests.cpp
        src/OrderBook.cpp
)
target_link_libraries(TitanTests GTest::gtest_main)
include(GoogleTest)
gtest_discover_tests(TitanTests)