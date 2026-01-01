# Boost
if (OUVERIUM_BUILD_BOOST)
    set(BOOST_INCLUDE_LIBRARIES asio dll)
    FetchContent_Declare(
        Boost
        OVERRIDE_FIND_PACKAGE
        SYSTEM
        URL https://github.com/boostorg/boost/releases/download/boost-1.85.0/boost-1.85.0-cmake.tar.gz
    )
endif()

# WxWidgets
if (OUVERIUM_BUILD_WXWIDGET)
    set(wxBUILD_SHARED OFF)
    set(wxUSE_WEBVIEW ON)
    set(wxBUILD_INSTALL OFF)
    FetchContent_Declare(
        wxWidgets
        OVERRIDE_FIND_PACKAGE
        SYSTEM
        URL https://github.com/wxWidgets/wxWidgets/releases/download/v3.2.4/wxWidgets-3.2.4.zip
    )
endif()
