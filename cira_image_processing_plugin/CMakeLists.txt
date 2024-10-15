cmake_minimum_required(VERSION 2.8.3)
project(cira_image_processing_plugin)

add_definitions(-std=c++11)

set(CMAKE_AUTOUIC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

add_definitions(${QT_DEFINITIONS})
add_definitions(-DQT_PLUGIN)

set(qt_version qt511)

find_package(catkin REQUIRED
    roscpp
    cira_lib_bernoulli
    cira_lib_yolo
    cira_lib_darknet_widget
    )

catkin_package(
  INCLUDE_DIRS include
  LIBRARIES ${PROJECT_NAME}
  CATKIN_DEPENDS
        roscpp
        cira_lib_bernoulli
        cira_lib_yolo
        cira_lib_darknet_widget
)

include_directories(
 include
 models
 ${catkin_INCLUDE_DIRS}
)


find_package(OpenCV REQUIRED
#NO_MODULE
#PATHS /usr/local
#NO_DEFAULT_PATH
)
include_directories(${OpenCV_INCLUDE_DIRS})

set(CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH} /opt/${qt_version})
set(Qt5Core_DIR /opt/${qt_version}/lib/cmake/Qt5Core)
set(Qt5Widgets_DIR /opt/${qt_version}/lib/cmake/Qt5Widgets)
set(Qt5Gui_DIR /opt/${qt_version}/lib/cmake/Qt5Gui)
set(Qt5_DIR /opt/${qt_version}/lib/cmake/Qt5)
set(QT5_LIBRARIES /opt/${qt_version}/lib)

find_package(Qt5 COMPONENTS
             Core
             Widgets
             Gui
             )

file(GLOB QT_FORMS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
    ui/*.ui
    models/*.ui
    )
file(GLOB QT_RESOURCES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
    resources/*.qrc
    )
file(GLOB_RECURSE QT_MOC RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} FOLLOW_SYMLINKS
    include/*.hpp
    include/*.h

    models/*.hpp
    models/*.h
    )

QT5_ADD_RESOURCES(QT_RESOURCES_CPP ${QT_RESOURCES})
QT5_WRAP_UI(QT_FORMS_HPP ${QT_FORMS})
QT5_WRAP_CPP(QT_MOC_HPP ${QT_MOC})

file(GLOB_RECURSE QT_SOURCES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} FOLLOW_SYMLINKS
    include/*.cpp
    src/*.cpp
    models/*.cpp
    )

add_library(${PROJECT_NAME}
    ${QT_SOURCES}
    ${QT_RESOURCES_CPP}
    ${QT_FORMS_HPP}
    ${QT_MOC_HPP}
    )

target_link_libraries(${PROJECT_NAME}
    ${QT_LIBRARIES} ${catkin_LIBRARIES}
    ${OpenCV_LIBS}
    Qt5::Core Qt5::Widgets Qt5::Gui
    )

install(TARGETS ${PROJECT_NAME}
    LIBRARY DESTINATION $ENV{HOME}/CiRA_CORE_Plugins)
