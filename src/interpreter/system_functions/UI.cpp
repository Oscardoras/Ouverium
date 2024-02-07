#include "UI.hpp"


#ifdef WXWIDGETS


#include <iostream>

#include <wx/wx.h>


namespace Interpreter::SystemFunctions {

    namespace UI {

        Reference ui_start(FunctionContext&) {
            wxTheApp->CallOnInit();
            return Data(wxTheApp->OnRun());
        }

        auto ui_new_frame_args = std::make_shared<Parser::Symbol>("title");
        Reference ui_new_frame(FunctionContext& context) {
            try {
                auto str = context["title"].to_data(context).get<Object*>()->to_string();

                auto frame = new wxFrame(nullptr, wxID_ANY, str);
                frame->Show(true);

                auto obj = context.new_object();
                obj->c_obj = static_cast<wxObject*>(frame);
                return Data(obj);
            } catch (Data::BadAccess const&) {
                throw FunctionArgumentsError();
            }
        }

        auto ui_new_button_args = std::make_shared<Parser::Symbol>("parent");
        Reference ui_new_button(FunctionContext& context) {
            try {
                auto parent = dynamic_cast<wxWindow*>(context["parent"].to_data(context).get<Object*>()->c_obj.get<wxObject*>());

                auto button = new wxButton(parent, wxID_ANY, "button_test");
                button->Show(true);

                auto obj = context.new_object();
                obj->c_obj = static_cast<wxObject*>(button);
                return Data(obj);
            } catch (Data::BadAccess const&) {
                throw FunctionArgumentsError();
            } catch (std::bad_any_cast const&) {
                throw FunctionArgumentsError();
            }
        }


        void init(GlobalContext& context) {
            auto& s = *context.get_global().system;

            s["ui_start"].to_data(context).get<Object*>()->functions.push_front(SystemFunction{ std::make_shared<Parser::Tuple>(), ui_start });
            s["ui_new_frame"].to_data(context).get<Object*>()->functions.push_front(SystemFunction{ ui_new_frame_args, ui_new_frame });
            s["ui_new_button"].to_data(context).get<Object*>()->functions.push_front(SystemFunction{ ui_new_button_args, ui_new_button });
        }

    }

}


#else


namespace Interpreter::SystemFunctions {

    namespace UI {

        void init(GlobalContext& context) {}

    }

}


#endif
