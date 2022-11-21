cmake_minimum_required(VERSION 3.20)
project(pcmrtp)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)
set(INCLUDE "/opt/Qt5.9.9/5.9.9/gcc_64/include/QtMultimedia") #    根据自己的情况书写路径
set(CMAKE_PREFIX_PATH "/opt/Qt5.9.9/5.9.9/gcc_64/lib/cmake")

#可以自己添加ffmpeg库路径和opencv 路径，由于本人环境安装在系统库内所以为写出
set(LINK_LIB "/opt/Qt5.9.9/5.9.9/gcc_64/lib")   # QT动态库路径

#链接库
link_directories(${LINK_LIB})

link_libraries(Qt5Multimedia)
link_libraries(swresample)
link_libraries(swscale)
link_libraries(avutil)
link_libraries(avformat)
link_libraries(avcodec)
link_libraries(opencv_core)
link_libraries(opencv_highgui)
link_libraries(opencv_imgcodecs)
link_libraries(opencv_videoio)
link_libraries(opencv_imgproc)
link_libraries(Qt5Widgets)
#头文件
INCLUDE_DIRECTORIES(${INCLUDE})
find_package(Qt5 COMPONENTS
        Core
        REQUIRED)
add_executable(pcmrtp main.cpp XMediaEncode.cpp XMediaEncode.h Xrtmp.cpp Xrtmp.h XAudioRecord.h XAudioRecord.cpp
        XVideoCaptur.cpp XVideoCaptur.h XData.h XData.cpp XDataThread.cpp XDataThread.h XFilter.cpp XFilter.h XBilateralFilter.cpp XBilateralFilter.h xrtmpstreamer.cpp xrtmpstreamer.h xrtmpstreamer.ui ui_xrtmpstreamer.h XController.h XController.cpp)

target_link_libraries(pcmrtp
        Qt5::Core
        )

