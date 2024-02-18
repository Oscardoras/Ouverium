#include "UI.hpp"


#ifdef WXWIDGETS


#include <iostream>

#include <wx/wx.h>
#include <wx/evtloop.h>


namespace Interpreter::SystemFunctions {

    namespace UI {

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

        auto ui_new_panel_args = std::make_shared<Parser::Symbol>("parent");
        Reference ui_new_panel(FunctionContext& context) {
            try {
                auto parent = dynamic_cast<wxWindow*>(context["parent"].to_data(context).get<Object*>()->c_obj.get<wxObject*>());

                auto panel = new wxPanel(parent, wxID_ANY);

                auto obj = context.new_object();
                obj->c_obj = static_cast<wxObject*>(panel);
                return Data(obj);
            } catch (Data::BadAccess const&) {
                throw FunctionArgumentsError();
            } catch (std::bad_any_cast const&) {
                throw FunctionArgumentsError();
            }
        }

        auto ui_new_button_args = std::make_shared<Parser::Symbol>("parent");
        Reference ui_new_button(FunctionContext& context) {
            try {
                auto parent = dynamic_cast<wxWindow*>(context["parent"].to_data(context).get<Object*>()->c_obj.get<wxObject*>());

                auto button = new wxButton(parent, wxID_ANY);

                auto obj = context.new_object();
                obj->c_obj = static_cast<wxObject*>(button);
                return Data(obj);
            } catch (Data::BadAccess const&) {
                throw FunctionArgumentsError();
            } catch (std::bad_any_cast const&) {
                throw FunctionArgumentsError();
            }
        }

        auto ui_add_event_args = std::make_shared<Parser::Tuple>(Parser::Tuple(
            {
                std::make_shared<Parser::Symbol>("window"),
                std::make_shared<Parser::Symbol>("callback")
            }
        ));
        template<typename Args>
        struct EventHandler {
            wxEventTypeTag<Args> const& type;

            EventHandler(wxEventTypeTag<Args> const& type) : type(type) {}

            Reference operator()(FunctionContext& context) {
                try {
                    auto window = dynamic_cast<wxWindow*>(context["window"].to_data(context).get<Object*>()->c_obj.get<wxObject*>());
                    auto callback = context["callback"].to_data(context).get<Object*>();

                    auto& global = context.get_global();
                    auto expression = context.expression;

                    window->Bind(type, [&global, expression, callback](Args&) {
                        Interpreter::call_function(global, expression, callback, Data{});
                    });

                    return Data{};
                } catch (Data::BadAccess const&) {
                    throw FunctionArgumentsError();
                } catch (std::bad_any_cast const&) {
                    throw FunctionArgumentsError();
                }
            }
        };

        auto ui_set_position_args = std::make_shared<Parser::Tuple>(Parser::Tuple(
            {
                std::make_shared<Parser::Symbol>("window"),
                std::make_shared<Parser::Symbol>("x"),
                std::make_shared<Parser::Symbol>("y")
            }
        ));
        Reference ui_set_position(FunctionContext& context) {
            try {
                auto window = dynamic_cast<wxWindow*>(context["window"].to_data(context).get<Object*>()->c_obj.get<wxObject*>());
                auto x = context["x"].to_data(context).get<OV_INT>();
                auto y = context["y"].to_data(context).get<OV_INT>();

                window->SetPosition(wxPoint(x, y));

                return Data{};
            } catch (Data::BadAccess const&) {
                throw FunctionArgumentsError();
            } catch (std::bad_any_cast const&) {
                throw FunctionArgumentsError();
            }
        }

        auto ui_set_size_args = std::make_shared<Parser::Tuple>(Parser::Tuple(
            {
                std::make_shared<Parser::Symbol>("window"),
                std::make_shared<Parser::Symbol>("x"),
                std::make_shared<Parser::Symbol>("y")
            }
        ));
        Reference ui_set_size(FunctionContext& context) {
            try {
                auto window = dynamic_cast<wxWindow*>(context["window"].to_data(context).get<Object*>()->c_obj.get<wxObject*>());
                auto x = context["x"].to_data(context).get<OV_INT>();
                auto y = context["y"].to_data(context).get<OV_INT>();

                window->SetSize(wxSize(x, y));

                return Data{};
            } catch (Data::BadAccess const&) {
                throw FunctionArgumentsError();
            } catch (std::bad_any_cast const&) {
                throw FunctionArgumentsError();
            }
        }


        void init(GlobalContext& context) {
            auto& s = *context.get_global().system;

            s["ui_new_frame"].to_data(context).get<Object*>()->functions.push_front(SystemFunction{ ui_new_frame_args, ui_new_frame });

            s["ui_new_panel"].to_data(context).get<Object*>()->functions.push_front(SystemFunction{ ui_new_panel_args, ui_new_panel });
            s["ui_new_button"].to_data(context).get<Object*>()->functions.push_front(SystemFunction{ ui_new_button_args, ui_new_button });

            s["ui_add_event"].to_data(context).get<Object*>()->functions.push_front(SystemFunction{ ui_add_event_args, EventHandler(wxEVT_BUTTON) });

            s["ui_set_position"].to_data(context).get<Object*>()->functions.push_front(SystemFunction{ ui_set_position_args, ui_set_position });
            s["ui_set_size"].to_data(context).get<Object*>()->functions.push_front(SystemFunction{ ui_set_size_args, ui_set_size });
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
