#include <wx/wx.h>

#include "GUIApp.hpp"


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
