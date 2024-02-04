#include <iostream>

#include <wx/wx.h>

#include "UI.hpp"


extern int global_argc;
extern char** global_argv;


namespace Interpreter::SystemFunctions {

    namespace UI {

        class App : public wxApp {
        public:
            bool OnInit() override {
                std::cerr << "init" << std::endl;
                auto frame = new wxFrame(NULL, wxID_ANY, "Hello wxWidgets", wxPoint(50, 50), wxSize(450, 340));
                frame->CreateStatusBar();
                frame->SetStatusText("Welcome to wxWidgets!");
                frame->Show(true);
                return true;
            }
        };

        Reference ui_start(FunctionContext&) {
            wxApp::SetInstance(new App);
            wxEntryStart(global_argc, global_argv);
            wxTheApp->CallOnInit();
            wxTheApp->OnRun();
            wxEntryCleanup();

            return Data{};
        }


        void init(GlobalContext& context) {
            auto& s = *context.get_global().system;

            s["ui_start"].to_data(context).get<Object*>()->functions.push_front(SystemFunction{ std::make_shared<Parser::Tuple>(), ui_start });
        }

    }

}
