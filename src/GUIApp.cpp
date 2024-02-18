#include "GUIApp.hpp"


#ifdef WXWIDGETS


#include <wx/wx.h>
#include <wx/evtloop.h>


class App : public wxApp {
public:
    bool OnInit() override {
        m_mainLoop = CreateMainLoop();
        wxEventLoopBase::SetActive(m_mainLoop);

        return true;
    }
};

IMPLEMENT_APP_NO_MAIN(App);
IMPLEMENT_WX_THEME_SUPPORT;

GUIApp::GUIApp(int argc, char** argv) {
    wxEntryStart(argc, argv);
    wxTheApp->CallOnInit();
}

GUIApp::~GUIApp() {
    wxTheApp->OnExit();
    wxEntryCleanup();
}

bool GUIApp::yield() {
    wxYield();

    return wxTheApp->GetTopWindow() != nullptr;
}


#else


GUIApp::GUIApp(int, char**) {}

GUIApp::~GUIApp() {}

bool GUIApp::yield() {
    return false;
}


#endif
