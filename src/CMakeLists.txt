qt5_add_resources(fritzmon_RESOURCES ../assets/fritzmon.qrc)

set(fritzmon_SRCS
    fritzmon.cpp
    Graph.cpp
    GraphModel.cpp
    MonitorApp.cpp
    Settings.cpp
    soap/IMessageBodyHandler.cpp
    soap/Request.cpp
    upnp/Action.cpp
    upnp/Device.cpp
    upnp/DeviceBuilder.cpp
    upnp/DeviceFinder.cpp
    upnp/Service.cpp
    upnp/ServiceBuilder.cpp
    upnp/StateVariable.cpp
    ${fritzmon_RESOURCES}
)

include_directories(${OPENGL_INCLUDE_DIR})

add_executable(fritzmon ${fritzmon_SRCS})
qt5_use_modules(fritzmon Core Gui Qml Quick Network)
