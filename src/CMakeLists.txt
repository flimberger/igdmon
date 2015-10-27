find_package(OpenGL REQUIRED)
find_package(Qt5 COMPONENTS Core Gui Qml Network)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)

set(fritzmon_SRCS
    fritzmon.cpp
    upnp/Action.cpp
    upnp/Device.cpp
    upnp/DeviceBuilder.cpp
    upnp/DeviceFinder.cpp
    upnp/Service.cpp
    upnp/ServiceBuilder.cpp
)

include_directories(${OPENGL_INCLUDE_DIR})

add_executable(fritzmon ${fritzmon_SRCS})
qt5_use_modules(fritzmon Core Gui Qml Network)
