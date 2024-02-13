#include "GUIApp.hpp"


#ifdef WXWIDGETS


#include <wx/wx.h>


class App : public wxApp {
public:
    bool OnInit() override {
        return true;
    }
};

GUIApp::GUIApp(int argc, char** argv) {
    wxApp::SetInstance(new App);
    wxEntryStart(argc, argv);
    wxTheApp->OnInit();
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

bool GUIApp::yield() {}


#endif
