cmake_minimum_required(VERSION 2.8)
project(main)

set(QMAKE_CXXFLAGS "-std=c++11")
#自己编译的openblas和opencv没有配置环境变量需要手动找
#include_directories(/home/software/openblas/include)
#link_directories(/home/software/openblas/lib)
#opencv4.0
#set(OpenCV_DIR /home/software/opencv/lib/cmake/opencv4)

find_package(OpenCV REQUIRED)
#set(CMAKE_BUILD_TYPE Debug)
AUX_SOURCE_DIRECTORY(./src DIR_SRCS)
add_executable(main ${DIR_SRCS})
target_link_libraries(main ${OpenCV_LIBS})
target_link_libraries(main openblas)
