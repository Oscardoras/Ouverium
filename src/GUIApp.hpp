#ifndef __GUIAPP_HPP__
#define __GUIAPP_HPP__

#include <wx/wx.h>


class GUIApp {

    class App : public wxApp {
    public:
        bool OnInit() override {
            return true;
        }
    };

public:

    GUIApp(int argc, char** argv) {
        wxApp::SetInstance(new App);
        wxEntryStart(argc, argv);
    }

    ~GUIApp() {
        wxEntryCleanup();
    }

};


#endif
