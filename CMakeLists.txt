cmake_minimum_required(VERSION 3.16)
project(WxCam VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# -- wxWidgets 
find_package(wxWidgets REQUIRED COMPONENTS core base net)
include(${wxWidgets_USE_FILE})

#  OpenCV 
find_package(OpenCV REQUIRED COMPONENTS core videoio imgproc imgcodecs)

# -Sources 
set(SOURCES
    src/App.cpp
    src/CameraPanel.cpp
    src/MainFrame.cpp
)

if(WIN32)
    add_executable(WxCam WIN32 ${SOURCES})
elseif(APPLE)
    add_executable(WxCam MACOSX_BUNDLE ${SOURCES})
else()
    add_executable(WxCam ${SOURCES})
endif()

target_include_directories(WxCam PRIVATE include)

target_link_libraries(WxCam
    PRIVATE
        ${wxWidgets_LIBRARIES}
        ${OpenCV_LIBS}
)

# Threads requis sur Linux pour std::thread
find_package(Threads REQUIRED)
target_link_libraries(WxCam PRIVATE Threads::Threads)

install(TARGETS WxCam DESTINATION bin)
