#include "SystemFunction.hpp"


#ifdef OUVERIUM_WXWIDGETS


#include <iostream>

#include <wx/wx.h>
#include <wx/activityindicator.h>
#include <wx/calctrl.h>
#include <wx/webview.h>


namespace Interpreter::SystemFunctions::UI {

    template<typename Args>
    void add_event(IndirectReference reference, wxEventTypeTag<Args> const& type) {
        struct EventHandler {
            wxEventTypeTag<Args> const& type;

            EventHandler(wxEventTypeTag<Args> const& type) : type(type) {}

            Reference operator()(FunctionContext& context) {
                try {
                    auto window = dynamic_cast<wxWindow*>(context["window"].to_data(context).get<ObjectPtr>()->c_obj.get<wxWindow*>());
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

        auto parameters = std::make_shared<Parser::Tuple>(Parser::Tuple(
            {
                std::make_shared<Parser::Symbol>("window"),
                std::make_shared<Parser::Symbol>("callback")
            }
        ));
        get_object(reference)->functions.push_front(SystemFunction{ parameters, EventHandler(type) });
    }

    namespace Window {

        Reference ui_position_get(wxWindow* window) {
            auto point = window->GetPosition();

            return TupleReference{ Data(static_cast<OV_INT>(point.x)), Data(static_cast<OV_INT>(point.y)) };
        }
        Reference ui_position_set(wxWindow* window, OV_INT x, OV_INT y) {
            window->SetPosition(wxPoint(x, y));

            return Data{};
        }

        Reference ui_size_get(wxWindow* window) {
            auto point = window->GetSize();

            return TupleReference{ Data(static_cast<OV_INT>(point.x)), Data(static_cast<OV_INT>(point.y)) };
        }
        Reference ui_size_set(wxWindow* window, OV_INT x, OV_INT y) {
            window->SetSize(wxSize(x, y));

            return Data{};
        }

        namespace Frame {

            Reference ui_frame_new(std::string const& title) {
                auto frame = new wxFrame(nullptr, wxID_ANY, title);
                frame->Show(true);

                auto obj = GC::new_object();
                obj->c_obj.set(std::make_unique<wxWindow*>(frame));
                return Data(obj);
            }

        }

        namespace Control {

            namespace Panel {

                Reference ui_panel_new(wxWindow* parent) {
                    auto panel = new wxPanel(parent, wxID_ANY);

                    auto obj = GC::new_object();
                    obj->c_obj.set(std::make_unique<wxWindow*>(panel));
                    return Data(obj);
                }

            }

            namespace ActivityIndicator {

                Reference ui_activityindicator_new(wxWindow* parent) {
                    auto activityIndicator = new wxActivityIndicator(parent);

                    auto obj = GC::new_object();
                    obj->c_obj.set(std::make_unique<wxWindow*>(activityIndicator));
                    return Data(obj);
                }

                Reference ui_activityindicator_running_get(wxWindow* window) {
                    auto activityIndicator = dynamic_cast<wxActivityIndicator*>(window);

                    return Data(activityIndicator->IsRunning());
                }

                Reference ui_activityindicator_running_set(wxWindow* window, bool state) {
                    auto activityIndicator = dynamic_cast<wxActivityIndicator*>(window);

                    if (state)
                        activityIndicator->Start();
                    else
                        activityIndicator->Stop();

                    return Data{};
                }

            }

            namespace Button {

                Reference ui_button_new(wxWindow* parent) {
                    auto button = new wxButton(parent, wxID_ANY);

                    auto obj = GC::new_object();
                    obj->c_obj.set(std::make_unique<wxWindow*>(button));
                    return Data(obj);
                }

                Reference ui_button_label_get(wxWindow* window) {
                    auto button = dynamic_cast<wxButton*>(window);

                    return Data(GC::new_object(button->GetLabel().ToStdString()));
                }

                Reference ui_button_label_set(wxWindow* window, std::string const& label) {
                    auto button = dynamic_cast<wxButton*>(window);

                    button->SetLabel(label);

                    return Data{};
                }

            }

            namespace Calendar {

                Reference ui_calendar_new(wxWindow* parent) {
                    auto calendar = new wxCalendarCtrl(parent, wxID_ANY);

                    auto obj = GC::new_object();
                    obj->c_obj.set(std::make_unique<wxWindow*>(calendar));
                    return Data(obj);
                }

                Reference ui_calendar_date_get(wxWindow* window) {
                    auto calendar = dynamic_cast<wxCalendarCtrl*>(window);

                    return Data(static_cast<OV_INT>(calendar->GetDate().GetTicks()));
                }

                Reference ui_calendar_date_set(wxWindow* window, OV_INT time) {
                    auto calendar = dynamic_cast<wxCalendarCtrl*>(window);

                    calendar->SetDate(wxDateTime(static_cast<time_t>(time)));

                    return Data{};
                }

            }

            namespace Checkbox {

                Reference ui_checkbox_new(wxWindow* parent) {
                    auto checkbox = new wxCheckBox(parent, wxID_ANY, "");

                    auto obj = GC::new_object();
                    obj->c_obj.set(std::make_unique<wxWindow*>(checkbox));
                    return Data(obj);
                }

                Reference ui_checkbox_value_get(wxWindow* window) {
                    auto checkbox = dynamic_cast<wxCheckBox*>(window);

                    return Data(checkbox->GetValue());
                }

                Reference ui_checkbox_value_set(wxWindow* window, bool value) {
                    auto checkbox = dynamic_cast<wxCheckBox*>(window);

                    checkbox->SetValue(value);

                    return Data{};
                }

            }

            namespace Choice {

                Reference ui_choice_new(wxWindow* parent) {
                    auto choice = new wxChoice(parent, wxID_ANY);

                    auto obj = GC::new_object();
                    obj->c_obj.set(std::make_unique<wxWindow*>(choice));
                    return Data(obj);
                }

                Reference ui_choice_size_get(wxWindow* window) {
                    auto choice = dynamic_cast<wxChoice*>(window);

                    return Data(static_cast<OV_INT>(choice->GetColumns()));
                }

                Reference ui_choice_size_set(wxWindow* window, OV_INT size) {
                    auto choice = dynamic_cast<wxChoice*>(window);

                    choice->SetColumns(static_cast<int>(size));

                    return Data{};
                }

                Reference ui_choice_label_get(wxWindow* window, OV_INT i) {
                    auto choice = dynamic_cast<wxChoice*>(window);

                    return Data(GC::new_object(choice->GetString(static_cast<unsigned int>(i)).ToStdString()));
                }

                Reference ui_choice_label_set(wxWindow* window, OV_INT i, std::string const& text) {
                    auto choice = dynamic_cast<wxChoice*>(window);

                    choice->SetString(static_cast<unsigned int>(i), text);

                    return Data{};
                }

                Reference ui_choice_selection_get(wxWindow* window) {
                    auto choice = dynamic_cast<wxChoice*>(window);

                    return Data(static_cast<OV_INT>(choice->GetSelection()));
                }

                Reference ui_choice_selection_set(wxWindow* window, OV_INT size) {
                    auto choice = dynamic_cast<wxChoice*>(window);

                    choice->SetSelection(static_cast<int>(size));

                    return Data{};
                }

            }

            namespace Webview {

                Reference ui_webview_new(wxWindow* parent) {
                    auto webview = wxWebView::New(parent, wxID_ANY);

                    auto obj = GC::new_object();
                    obj->c_obj.set(std::make_unique<wxWindow*>(webview));
                    return Data(obj);
                }

                Reference ui_webview_title_get(wxWindow* window) {
                    auto webview = dynamic_cast<wxWebView*>(window);

                    return Data(GC::new_object(webview->GetCurrentTitle().ToStdString()));
                }

                Reference ui_webview_url_get(wxWindow* window) {
                    auto webview = dynamic_cast<wxWebView*>(window);

                    return Data(GC::new_object(webview->GetCurrentURL().ToStdString()));
                }

                Reference ui_webview_url_set(wxWindow* window, std::string const& url) {
                    auto webview = dynamic_cast<wxWebView*>(window);

                    webview->LoadURL(url);

                    return Data{};
                }

                Reference ui_webview_set_page(wxWindow* window, std::string const& html, std::string const& url) {
                    auto webview = dynamic_cast<wxWebView*>(window);

                    webview->SetPage(html, url);

                    return Data{};
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

            Reference ui_sizer_detach(wxSizer* sizer, wxWindow* child) {
                sizer->Detach(child);

                return Data{};
            }

            Reference ui_sizer_detach(wxSizer* sizer, wxSizer* child) {
                sizer->Detach(child);

                return Data{};
            }

            Reference ui_vertical_sizer() {
                auto boxsizer = new wxBoxSizer(wxVERTICAL);

                auto obj = GC::new_object();
                obj->c_obj.set(std::make_unique<wxSizer*>(boxsizer));
                return Data(obj);
            }

            Reference ui_horizontal_sizer() {
                auto boxsizer = new wxBoxSizer(wxHORIZONTAL);

                auto obj = GC::new_object();
                obj->c_obj.set(std::make_unique<wxSizer*>(boxsizer));
                return Data(obj);
            }

        }

    }


    void init(GlobalContext& context) {
        auto& s = context.get_global().system;

        add_function(s.get_property("ui_position_set"), Window::ui_position_get);
        add_function(s.get_property("ui_set_position"), Window::ui_position_set);
        add_function(s.get_property("ui_set_size"), Window::ui_size_set);

        add_function(s.get_property("ui_frame_new"), Window::Frame::ui_frame_new);

        add_function(s.get_property("ui_panel_new"), Window::Control::Panel::ui_panel_new);

        add_function(s.get_property("ui_activityindicator_new"), Window::Control::ActivityIndicator::ui_activityindicator_new);
        add_function(s.get_property("ui_activityindicator_running_get"), Window::Control::ActivityIndicator::ui_activityindicator_running_get);
        add_function(s.get_property("ui_activityindicator_running_set"), Window::Control::ActivityIndicator::ui_activityindicator_running_set);

        add_function(s.get_property("ui_button_new"), Window::Control::Button::ui_button_new);
        add_function(s.get_property("ui_button_label_get"), Window::Control::Button::ui_button_label_get);
        add_function(s.get_property("ui_button_label_set"), Window::Control::Button::ui_button_label_set);
        add_event(s.get_property("ui_event_button"), wxEVT_BUTTON);

        add_function(s.get_property("ui_calendar_new"), Window::Control::Calendar::ui_calendar_new);
        add_function(s.get_property("ui_calendar_date_get"), Window::Control::Calendar::ui_calendar_date_get);
        add_function(s.get_property("ui_calendar_date_set"), Window::Control::Calendar::ui_calendar_date_set);

        add_function(s.get_property("ui_checkbox_new"), Window::Control::Checkbox::ui_checkbox_new);
        add_function(s.get_property("ui_checkbox_value_get"), Window::Control::Checkbox::ui_checkbox_value_get);
        add_function(s.get_property("ui_checkbox_value_set"), Window::Control::Checkbox::ui_checkbox_value_set);

        add_function(s.get_property("ui_choice_new"), Window::Control::Choice::ui_choice_new);
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
    }

}


#else


namespace Interpreter::SystemFunctions {

    namespace UI {

        void init(GlobalContext&) {}

    }

}


#endif
