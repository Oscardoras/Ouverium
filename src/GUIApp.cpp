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
}

GUIApp::~GUIApp() {
    wxEntryCleanup();
}

int GUIApp::run() {
    wxTheApp->CallOnInit();
        return wxTheApp->OnRun();
}


#else


GUIApp::GUIApp(int argc, char** argv) {}

GUIApp::~GUIApp() {}

int GUIApp::run() {}


#endif
