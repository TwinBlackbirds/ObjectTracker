cmake_minimum_required(VERSION 3.30)
project(ObjectTracker)

set(CMAKE_CXX_STANDARD 11)
find_package(OpenCV REQUIRED)
find_package(X11 REQUIRED)

add_executable(ObjectTracker main.cpp)

target_include_directories(ObjectTracker PRIVATE ${X11_INCLUDE_DIR})
target_link_libraries(ObjectTracker PRIVATE ${X11_LIBRARIES} Xrandr)
target_include_directories(ObjectTracker PRIVATE ${OpenCV_INCLUDE_DIRS})
target_link_libraries(ObjectTracker PRIVATE ${OpenCV_LIBS})