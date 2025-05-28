cmake_minimum_required(VERSION 3.30)
project(MapProxy)
project(MapProxy_test)

set(CMAKE_CXX_STANDARD 20)

include(FetchContent)
FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG v1.14.0
)
FetchContent_MakeAvailable(googletest)

add_executable(MapProxy main.cpp
        Auditor.h
        ProxiedMap.h
        ProxiedMap.cpp
        Auditor.cpp
)

add_executable(MapProxy_test
        Auditor.h
        ProxiedMap.h
        ProxiedMap.cpp
        Auditor.cpp
        gtest.cpp
)

target_link_libraries(MapProxy_test GTest::gtest_main)

# Автоматически добавляем тесты
include(GoogleTest)
gtest_discover_tests(MapProxy_test)