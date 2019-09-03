cmake_minimum_required(VERSION 3.4.1)

project(a7zip C CXX)

option(EXTRACT "Only supports extracting archives" OFF)
option(LITE "Only supports 7z, rar, zip formats" OFF)

add_subdirectory(p7zip)

set(A_SEVEN_ZIP_SOURCES
        src/main/cpp/BlackHole.cpp
        src/main/cpp/InArchive.cpp
        src/main/cpp/InternalSeekableInputStream.cpp
        src/main/cpp/JavaEnv.cpp
        src/main/cpp/JavaHelper.cpp
        src/main/cpp/JavaInArchive.cpp
        src/main/cpp/JavaInitA7Zip.cpp
        src/main/cpp/JavaSeekableInputStream.cpp
        src/main/cpp/JavaInputStream.cpp
        src/main/cpp/InternalOutputStream.cpp
        src/main/cpp/SevenZip.cpp
)

set(A_SEVEN_ZIP_FLAGS -fvisibility=hidden)

if(EXTRACT)
    if (LITE)
        set(A_SEVEN_ZIP_NAME "a7zip-extract-lite")
    else()
        set(A_SEVEN_ZIP_NAME "a7zip-extract")
    endif()
else()
    if (LITE)
        set(A_SEVEN_ZIP_NAME "a7zip-lite")
    else()
        set(A_SEVEN_ZIP_NAME "a7zip-full")
    endif()
endif()

add_library(a7zip SHARED ${A_SEVEN_ZIP_SOURCES})
target_link_libraries(a7zip PUBLIC p7zip log)
set_target_properties(a7zip PROPERTIES OUTPUT_NAME ${A_SEVEN_ZIP_NAME})
target_compile_options(a7zip PRIVATE ${A_SEVEN_ZIP_FLAGS})

add_library(a7zip-loader SHARED src/main/c/JavaInitA7ZipLoader.c)

add_library(gtest STATIC IMPORTED)
set_target_properties(gtest PROPERTIES IMPORTED_LOCATION ${CMAKE_CURRENT_SOURCE_DIR}/googletest/lib/${ANDROID_ABI}/libgtest.a)

add_library(a7zip_test SHARED src/androidTest/cpp/JavaInit.cpp)
target_link_libraries(a7zip_test PRIVATE a7zip gtest)
target_include_directories(a7zip_test PRIVATE
        src/main/cpp
        googletest/include
)
