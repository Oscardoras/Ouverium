#include "../Interpreter.hpp"


#ifdef OUVERIUM_WXWIDGETS


#include <wx/wx.h>
#include <wx/activityindicator.h>
#include <wx/calctrl.h>
#include <wx/webview.h>


namespace Interpreter::SystemFunctions {
    template<>
    inline wxWindow* get_arg<wxWindow*>(FunctionContext& /*context*/, Data const& data) {
        return data.get<ObjectPtr>()->c_obj.get<wxWeakRef<wxWindow>>().get();
    }

    template<>
    inline wxSizer* get_arg<wxSizer*>(FunctionContext& /*context*/, Data const& data) {
        return data.get<ObjectPtr>()->c_obj.get<wxSizer*>();
    }
}

namespace Interpreter::SystemFunctions::UI {

    template<typename Args>
    void add_event(IndirectReference const& reference, wxEventTypeTag<Args> const& type) {
        struct EventHandler {
            wxEventTypeTag<Args> const& type;

            EventHandler(wxEventTypeTag<Args> const& type) : type(type) {}

            Reference operator()(FunctionContext& context) {
                try {
                    auto* window = context["window"].to_data(context).get<ObjectPtr>()->c_obj.get<wxWeakRef<wxWindow>>().get();
                    auto callback = context["callback"];

                    auto& global = context.get_global();
                    auto function_context = std::make_shared<FunctionContext>(global, nullptr);
                    function_context->add_symbol("callback", callback);

                    window->Bind(type, [function_context](Args&) {
                        try {
                            Interpreter::try_call_function(*function_context, nullptr, (*function_context)["callback"], std::make_shared<Parser::Tuple>());
                        } catch (Interpreter::Exception const& ex) {
                            ex.print_stack_trace(*function_context);
                        }
                    });

                    return Data{};
                } catch (Data::BadAccess const&) {
                    throw FunctionArgumentsError();
                } catch (std::bad_any_cast const&) {
                    throw FunctionArgumentsError();
                }
            }
        };

        auto parameters = std::make_shared<Parser::Tuple>(Parser::Tuple(
            {
                std::make_shared<Parser::Symbol>("window"),
                std::make_shared<Parser::Symbol>("callback")
            }
        ));
        get_object(reference)->functions.emplace_front(SystemFunction{ parameters, EventHandler(type) });
    }

    namespace Window {

        template<typename T>
        Reference ui_new() {
            T* window = new T;

            auto obj = GC::new_object();
            obj->c_obj.set(std::make_unique<wxWeakRef<wxWindow>>(window));
            return Data(obj);
        }

        Reference ui_create(wxWindow* window, wxWindow* parent) {
            if (auto* activityIndicator = dynamic_cast<wxActivityIndicator*>(window))
                activityIndicator->Create(parent, wxID_ANY);
            else if (auto* button = dynamic_cast<wxButton*>(window))
                button->Create(parent, wxID_ANY);
            else if (auto* webview = dynamic_cast<wxWebView*>(window))
                webview->Create(parent, wxID_ANY);
            else
                window->Create(parent, wxID_ANY);

            return Data{};
        }

        Reference ui_position_get(wxWindow* window) {
            auto point = window->GetPosition();

            return TupleReference{ Data(static_cast<OV_INT>(point.x)), Data(static_cast<OV_INT>(point.y)) };
        }
        Reference ui_position_set(wxWindow* window, OV_INT x, OV_INT y) {
            window->SetPosition(wxPoint(static_cast<int>(x), static_cast<int>(y)));

            return Data{};
        }

        Reference ui_size_get(wxWindow* window) {
            auto point = window->GetSize();

            return TupleReference{ Data(static_cast<OV_INT>(point.x)), Data(static_cast<OV_INT>(point.y)) };
        }
        Reference ui_size_set(wxWindow* window, OV_INT x, OV_INT y) {
            window->SetSize(wxSize(static_cast<int>(x), static_cast<int>(y)));

            return Data{};
        }

        Reference ui_sizer_get(wxWindow* window) {
            auto* sizer = window->GetSizer();

            auto obj = GC::new_object();
            obj->c_obj.set(std::make_unique<wxSizer*>(sizer));
            return Data(obj);
        }
        Reference ui_sizer_set(wxWindow* window, wxSizer* sizer) {
            window->SetSizerAndFit(sizer);

            return Data{};
        }

        namespace TopLevelWindow {

            Reference ui_frame_new(std::string const& title) {
                auto* frame = new wxFrame(nullptr, wxID_ANY, title);
                frame->Show(true);

                auto obj = GC::new_object();
                obj->c_obj.set(std::make_unique<wxWindow*>(frame));
                return Data(obj);
            }

        }

        namespace Frame {

            Reference ui_frame_new(std::string const& title) {
                auto* frame = new wxFrame(nullptr, wxID_ANY, title);
                frame->Show(true);

                auto obj = GC::new_object();
                obj->c_obj.set(std::make_unique<wxWeakRef<wxWindow>>(frame));
                return Data(obj);
            }

        }

        namespace Control {

            namespace Panel {

            }

            namespace ActivityIndicator {

                Reference ui_activityindicator_running_get(wxWindow* window) {
                    auto* activityIndicator = dynamic_cast<wxActivityIndicator*>(window);

                    return Data(activityIndicator->IsRunning());
                }

                Reference ui_activityindicator_running_set(wxWindow* window, bool state) {
                    auto* activityIndicator = dynamic_cast<wxActivityIndicator*>(window);

                    if (state)
                        activityIndicator->Start();
                    else
                        activityIndicator->Stop();

                    return Data{};
                }

            }

            namespace Button {

                Reference ui_button_label_get(wxWindow* window) {
                    auto* button = dynamic_cast<wxButton*>(window);

                    return Data(GC::new_object(button->GetLabel().ToStdString()));
                }

                Reference ui_button_label_set(wxWindow* window, std::string const& label) {
                    auto* button = dynamic_cast<wxButton*>(window);

                    button->SetLabel(label);

                    return Data{};
                }

            }

            namespace Calendar {

                Reference ui_calendar_date_get(wxWindow* window) {
                    auto* calendar = dynamic_cast<wxCalendarCtrl*>(window);

                    return Data(static_cast<OV_INT>(calendar->GetDate().GetTicks()));
                }

                Reference ui_calendar_date_set(wxWindow* window, OV_INT time) {
                    auto* calendar = dynamic_cast<wxCalendarCtrl*>(window);

                    calendar->SetDate(wxDateTime(static_cast<time_t>(time)));

                    return Data{};
                }

            }

            namespace Checkbox {

                Reference ui_checkbox_value_get(wxWindow* window) {
                    auto* checkbox = dynamic_cast<wxCheckBox*>(window);

                    return Data(checkbox->GetValue());
                }

                Reference ui_checkbox_value_set(wxWindow* window, bool value) {
                    auto* checkbox = dynamic_cast<wxCheckBox*>(window);

                    checkbox->SetValue(value);

                    return Data{};
                }

            }

            namespace Choice {

                Reference ui_choice_size_get(wxWindow* window) {
                    auto* choice = dynamic_cast<wxChoice*>(window);

                    return Data(static_cast<OV_INT>(choice->GetColumns()));
                }

                Reference ui_choice_size_set(wxWindow* window, OV_INT size) {
                    auto* choice = dynamic_cast<wxChoice*>(window);

                    choice->SetColumns(static_cast<int>(size));

                    return Data{};
                }

                Reference ui_choice_label_get(wxWindow* window, OV_INT i) {
                    auto* choice = dynamic_cast<wxChoice*>(window);

                    return Data(GC::new_object(choice->GetString(static_cast<unsigned int>(i)).ToStdString()));
                }

                Reference ui_choice_label_set(wxWindow* window, OV_INT i, std::string const& text) {
                    auto* choice = dynamic_cast<wxChoice*>(window);

                    choice->SetString(static_cast<unsigned int>(i), text);

                    return Data{};
                }

                Reference ui_choice_selection_get(wxWindow* window) {
                    auto* choice = dynamic_cast<wxChoice*>(window);

                    return Data(static_cast<OV_INT>(choice->GetSelection()));
                }

                Reference ui_choice_selection_set(wxWindow* window, OV_INT size) {
                    auto* choice = dynamic_cast<wxChoice*>(window);

                    choice->SetSelection(static_cast<int>(size));

                    return Data{};
                }

            }

            namespace Webview {

                Reference ui_webview_new() {
                    auto* webview = wxWebView::New();

                    auto obj = GC::new_object();
                    obj->c_obj.set(std::make_unique<wxWeakRef<wxWindow>>(webview));
                    return Data(obj);
                }

                Reference ui_webview_title_get(wxWindow* window) {
                    auto* webview = dynamic_cast<wxWebView*>(window);

                    return Data(GC::new_object(webview->GetCurrentTitle().ToStdString()));
                }

                Reference ui_webview_url_get(wxWindow* window) {
                    auto* webview = dynamic_cast<wxWebView*>(window);

                    return Data(GC::new_object(webview->GetCurrentURL().ToStdString()));
                }

                Reference ui_webview_url_set(wxWindow* window, std::string const& url) {
                    auto* webview = dynamic_cast<wxWebView*>(window);

                    webview->LoadURL(url);

                    return Data{};
                }

                Reference ui_webview_set_page(wxWindow* window, std::string const& html, std::string const& url) {
                    auto* webview = dynamic_cast<wxWebView*>(window);

                    webview->SetPage(html, url);

                    return Data{};
                }

            }

        }

    }

    namespace Sizer {

        Reference ui_sizer_attach(wxSizer* sizer, wxWindow* child) {
            sizer->Add(child);

            return Data{};
        }

        Reference ui_sizer_attach(wxSizer* sizer, wxSizer* child) {
            sizer->Add(child);

            return Data{};
        }

        Reference ui_sizer_remove(wxSizer* sizer, wxWindow* child) {
            sizer->Detach(child);

            return Data{};
        }

        Reference ui_sizer_remove(wxSizer* sizer, wxSizer* child) {
            sizer->Remove(child);

            return Data{};
        }

        Reference ui_vertical_sizer() {
            auto* boxsizer = new wxBoxSizer(wxVERTICAL);

            auto obj = GC::new_object();
            obj->c_obj.set(std::make_unique<wxSizer*>(boxsizer));
            return Data(obj);
        }

        Reference ui_horizontal_sizer() {
            auto* boxsizer = new wxBoxSizer(wxHORIZONTAL);

            auto obj = GC::new_object();
            obj->c_obj.set(std::make_unique<wxSizer*>(boxsizer));
            return Data(obj);
        }

    }


    void init(GlobalContext& context) {
        auto& s = context.get_global().system;

        add_function(s.get_property("ui_create"), Window::ui_create);

        add_function(s.get_property("ui_position_set"), Window::ui_position_get);
        add_function(s.get_property("ui_position_set"), Window::ui_position_set);
        add_function(s.get_property("ui_size_get"), Window::ui_size_get);
        add_function(s.get_property("ui_size_set"), Window::ui_size_set);
        add_function(s.get_property("ui_sizer_get"), Window::ui_sizer_get);
        add_function(s.get_property("ui_sizer_set"), Window::ui_sizer_set);

        add_function(s.get_property("ui_frame_new"), Window::Frame::ui_frame_new);

        add_function(s.get_property("ui_panel_new"), Window::ui_new<wxPanel>);

        add_function(s.get_property("ui_activityindicator_new"), Window::ui_new<wxActivityIndicator>);
        add_function(s.get_property("ui_activityindicator_running_get"), Window::Control::ActivityIndicator::ui_activityindicator_running_get);
        add_function(s.get_property("ui_activityindicator_running_set"), Window::Control::ActivityIndicator::ui_activityindicator_running_set);

        add_function(s.get_property("ui_button_new"), Window::ui_new<wxButton>);
        add_function(s.get_property("ui_button_label_get"), Window::Control::Button::ui_button_label_get);
        add_function(s.get_property("ui_button_label_set"), Window::Control::Button::ui_button_label_set);
        add_event(s.get_property("ui_event_button"), wxEVT_BUTTON);

        add_function(s.get_property("ui_calendar_new"), Window::ui_new<wxCalendarCtrl>);
        add_function(s.get_property("ui_calendar_date_get"), Window::Control::Calendar::ui_calendar_date_get);
        add_function(s.get_property("ui_calendar_date_set"), Window::Control::Calendar::ui_calendar_date_set);
        add_event(s.get_property("ui_event_calendar"), wxEVT_CALENDAR_SEL_CHANGED);

        add_function(s.get_property("ui_checkbox_new"), Window::ui_new<wxCheckBox>);
        add_function(s.get_property("ui_checkbox_value_get"), Window::Control::Checkbox::ui_checkbox_value_get);
        add_function(s.get_property("ui_checkbox_value_set"), Window::Control::Checkbox::ui_checkbox_value_set);

        add_function(s.get_property("ui_choice_new"), Window::ui_new<wxChoice>);
        add_function(s.get_property("ui_choice_size_get"), Window::Control::Choice::ui_choice_size_get);
        add_function(s.get_property("ui_choice_size_set"), Window::Control::Choice::ui_choice_size_set);
        add_function(s.get_property("ui_choice_label_get"), Window::Control::Choice::ui_choice_label_get);
        add_function(s.get_property("ui_choice_label_get"), Window::Control::Choice::ui_choice_label_get);
        add_function(s.get_property("ui_choice_selection_get"), Window::Control::Choice::ui_choice_selection_get);
        add_function(s.get_property("ui_choice_selection_set"), Window::Control::Choice::ui_choice_selection_set);

        add_function(s.get_property("ui_webview_new"), Window::Control::Webview::ui_webview_new);
        add_function(s.get_property("ui_webview_title_get"), Window::Control::Webview::ui_webview_title_get);
        add_function(s.get_property("ui_webview_url_get"), Window::Control::Webview::ui_webview_url_get);
        add_function(s.get_property("ui_webview_url_set"), Window::Control::Webview::ui_webview_url_set);
        add_function(s.get_property("ui_webview_set_page"), Window::Control::Webview::ui_webview_set_page);

        add_function(s.get_property("ui_sizer_attach"), (Reference(*)(wxSizer*, wxWindow*))Sizer::ui_sizer_attach);
        add_function(s.get_property("ui_sizer_attach"), (Reference(*)(wxSizer*, wxSizer*))Sizer::ui_sizer_attach);
        add_function(s.get_property("ui_sizer_remove"), (Reference(*)(wxSizer*, wxWindow*))Sizer::ui_sizer_remove);
        add_function(s.get_property("ui_sizer_remove"), (Reference(*)(wxSizer*, wxSizer*))Sizer::ui_sizer_remove);

        add_function(s.get_property("ui_horizontal_sizer"), Sizer::ui_horizontal_sizer);
        add_function(s.get_property("ui_vertical_sizer"), Sizer::ui_vertical_sizer);
    }

}


#else


namespace Interpreter::SystemFunctions {

    namespace UI {

        void init(GlobalContext& /*unused*/) {}

    }

}


#endif
