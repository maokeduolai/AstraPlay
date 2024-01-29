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
        src/controller.cpp
        src/func/screen_capture.cpp
        src/func/video_downloader.cpp
        src/func/output_window.cpp
        src/func/media_info.cpp
        src/func/subtitle.cpp
        resources/application.ui
        resources/resources.qrc
        src/include/mpv/client.h
        src/include/mpv/render.h
        src/include/mpv/render_gl.h
        src/include/mpv/stream_cb.h
        src/include/mpv/qthelper.hpp
        src/include/screen_capture.h
        src/include/video_downloader.h
        src/include/application.h
        src/include/controller.h
        src/include/output_window.h
        src/include/media_info.h
        src/include/subtitle.h
        resources/icon.rc
)
add_executable(AstraPlay ${SOURCE_FILES})

# 链接Qt5
target_link_libraries(AstraPlay PRIVATE Qt${QT_VERSION_MAJOR}::Widgets)

# 链接libmpv
target_link_libraries(AstraPlay PRIVATE ${MPV_LIBRARY})

# 包含头文件目录
target_include_directories(AstraPlay PUBLIC src/include)

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
