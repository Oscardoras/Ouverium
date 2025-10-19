# Boost
if (OUVERIUM_BUILD_BOOST)
    set(BOOST_INCLUDE_LIBRARIES asio dll)
    FetchContent_Declare(
        Boost
        URL https://github.com/boostorg/boost/releases/download/boost-1.85.0/boost-1.85.0-cmake.tar.gz
        SYSTEM
    )
    FetchContent_MakeAvailable(Boost)
endif()

# Readline
if (UNIX)
    find_library(READLINE_LIB readline)
    if (NOT READLINE_LIB_FOUND)
        message(WARNING "Readline is not installed.")
    endif()
endif()

# WxWidgets
if (OUVERIUM_BUILD_WXWIDGET)
    set(wxBUILD_SHARED OFF)
    set(wxUSE_WEBVIEW ON)
    set(wxBUILD_INSTALL OFF)
    FetchContent_Declare(
        wxWidgets
        URL https://github.com/wxWidgets/wxWidgets/releases/download/v3.2.4/wxWidgets-3.2.4.zip
        SYSTEM
    )
    FetchContent_MakeAvailable(wxWidgets)
elseif(OUVERIUM_WITH_WXWIDGET)
    find_package(wxWidgets COMPONENTS core base webview)
endif()
