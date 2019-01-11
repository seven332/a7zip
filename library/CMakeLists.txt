cmake_minimum_required(VERSION 3.4.1)

project(a7zip C CXX)

option(EXTRACT "Only supports extracting archives" OFF)
option(LITE "Only supports 7z, rar, zip formats" OFF)

add_subdirectory(p7zip)

set(A_SEVEN_ZIP_SOURCES
        src/main/cpp/BlackHole.cpp
        src/main/cpp/InArchive.cpp
        src/main/cpp/InStream.cpp
        src/main/cpp/JavaEnv.cpp
        src/main/cpp/JavaInArchive.cpp
        src/main/cpp/JavaInit.cpp
        src/main/cpp/OutputStreamOutStream.cpp
        src/main/cpp/P7Zip.cpp
)

add_library(a7zip SHARED ${A_SEVEN_ZIP_SOURCES})
target_link_libraries(a7zip PUBLIC p7zip log)

add_library(gtest STATIC IMPORTED)
set_target_properties(gtest PROPERTIES IMPORTED_LOCATION ${CMAKE_CURRENT_SOURCE_DIR}/googletest/lib/${ANDROID_ABI}/libgtest.a)

add_library(a7zip_test SHARED src/androidTest/cpp/JavaInit.cpp)
target_link_libraries(a7zip_test PRIVATE a7zip gtest)
target_include_directories(a7zip_test PRIVATE
        src/main/cpp
        googletest/include
)
