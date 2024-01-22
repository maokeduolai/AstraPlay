cmake_minimum_required(VERSION 3.5)

project(AstraPlay VERSION 0.2 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

# 设置C++标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 添加Qt5依赖
find_package(QT NAMES Qt5 REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets)

# 寻找 mpv 库
find_library(MPV_LIBRARY NAMES libmpv.dll.a libmpv-2.dll PATHS ${CMAKE_CURRENT_SOURCE_DIR}/libs/mpv/)

# 指定编译源文件
set(SOURCE_FILES
        src/main.cpp
        src/application.cpp
        src/application.h
        src/controller.cpp
        src/controller.h
        src/screen_capture.cpp
        src/screen_capture.h
        resources/application.ui
        resources/icons.qrc
        include/mpv/client.h
        include/mpv/render.h
        include/mpv/render_gl.h
        include/mpv/stream_cb.h
        include/mpv/qthelper.hpp
)
add_executable(AstraPlay ${SOURCE_FILES})

# 链接Qt5
target_link_libraries(AstraPlay PRIVATE Qt${QT_VERSION_MAJOR}::Widgets)

# 链接libmpv
target_link_libraries(AstraPlay PRIVATE ${MPV_LIBRARY})

# 包含头文件目录
target_include_directories(AstraPlay PUBLIC ${PROJECT_SOURCE_DIR}/include)

set_target_properties(AstraPlay PROPERTIES
        ${BUNDLE_ID_OPTION}
        MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
        MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
        MACOSX_BUNDLE TRUE
        WIN32_EXECUTABLE TRUE
)

include(GNUInstallDirs)
install(TARGETS AstraPlay
        BUNDLE DESTINATION .
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
