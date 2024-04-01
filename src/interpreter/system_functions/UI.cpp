#include "../Interpreter.hpp"


#ifdef OUVERIUM_WXWIDGETS


#include <iostream>

#include <wx/wx.h>
#include <wx/evtloop.h>


namespace Interpreter::SystemFunctions {

    namespace UI {

        auto ui_new_frame_args = std::make_shared<Parser::Symbol>("title");
        Reference ui_new_frame(FunctionContext& context) {
            try {
                auto str = context["title"].to_data(context).get<ObjectPtr>()->to_string();

                auto frame = new wxFrame(nullptr, wxID_ANY, str);
                frame->Show(true);

                auto obj = context.new_object();
                obj->c_obj.set(std::make_unique<wxObjectPtr>(frame));
                return Data(obj);
            } catch (Data::BadAccess const&) {
                throw FunctionArgumentsError();
            }
        }

        auto ui_new_panel_args = std::make_shared<Parser::Symbol>("parent");
        Reference ui_new_panel(FunctionContext& context) {
            try {
                auto parent = dynamic_cast<wxWindow*>(context["parent"].to_data(context).get<ObjectPtr>()->c_obj.get<wxObjectPtr>());

                auto panel = new wxPanel(parent, wxID_ANY);

                auto obj = context.new_object();
                obj->c_obj.set(std::make_unique<wxObjectPtr>(panel));
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
                auto parent = dynamic_cast<wxWindow*>(context["parent"].to_data(context).get<ObjectPtr>()->c_obj.get<wxObjectPtr>());

                auto button = new wxButton(parent, wxID_ANY);

                auto obj = context.new_object();
                obj->c_obj.set(std::make_unique<wxObjectPtr>(button));
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
                    auto window = dynamic_cast<wxWindow*>(context["window"].to_data(context).get<ObjectPtr>()->c_obj.get<wxObjectPtr>());
                    auto callback = context["callback"].to_data(context).get<ObjectPtr>();

                    auto& global = context.get_global();

                    window->Bind(type, [&global, callback](Args&) {
                        Interpreter::call_function(global, nullptr, Data(callback), Data{});
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
                auto window = dynamic_cast<wxWindow*>(context["window"].to_data(context).get<ObjectPtr>()->c_obj.get<wxObjectPtr>());
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
                auto window = dynamic_cast<wxWindow*>(context["window"].to_data(context).get<ObjectPtr>()->c_obj.get<wxObjectPtr>());
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
            auto s = context.get_global().system;

            get_object(context, s->properties["ui_new_frame"])->functions.push_front(SystemFunction{ ui_new_frame_args, ui_new_frame });

            get_object(context, s->properties["ui_new_panel"])->functions.push_front(SystemFunction{ ui_new_panel_args, ui_new_panel });
            get_object(context, s->properties["ui_new_button"])->functions.push_front(SystemFunction{ ui_new_button_args, ui_new_button });

            get_object(context, s->properties["ui_add_event"])->functions.push_front(SystemFunction{ ui_add_event_args, EventHandler(wxEVT_BUTTON) });

            get_object(context, s->properties["ui_set_position"])->functions.push_front(SystemFunction{ ui_set_position_args, ui_set_position });
            get_object(context, s->properties["ui_set_size"])->functions.push_front(SystemFunction{ ui_set_size_args, ui_set_size });
        }

    }

}


#else


namespace Interpreter::SystemFunctions {

    namespace UI {

        void init(GlobalContext&) {}

    }

}


#endif
