#include <any>
#include <ctime>
#include <memory>
#include <string>

#include <ouverium/types.h>

#include "SystemFunction.hpp"

#include <ouverium/interpreter/Interpreter.hpp>

#include <ouverium/parser/Expressions.hpp>


#ifdef OUVERIUM_WXWIDGETS


#include <wx/wx.h>
#include <wx/activityindicator.h>
#include <wx/calctrl.h>
#include <wx/control.h>
#include <wx/clrpicker.h>
#include <wx/datectrl.h>
#include <wx/dialog.h>
#include <wx/gauge.h>
#include <wx/hyperlink.h>
#include <wx/radiobox.h>
#include <wx/srchctrl.h>
#include <wx/webview.h>
#include <wx/wrapsizer.h>

namespace Interpreter::SystemFunctions {
    template<>
    inline wxWindow* get_arg<wxWindow*>(FunctionContext& /*context*/, Data const& data) {
        return data.get<ObjectPtr>()->c_obj.get<wxWeakRef<wxWindow>>().get();
    }
}

namespace Interpreter::SystemFunctions::UI {

    struct ItemStyle {
        unsigned proportion = 0;
        enum class Align {
            Start,
            Center,
            End,
        } align = Align::Start;
        bool expand = false;
        bool keep_ratio = false;
        struct {
            int top{};
            int bottom{};
            int left{};
            int right{};
        } border;
    };

    struct UserData : public wxClientData {
        ItemStyle item_style;
    };

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

        wxSizerFlags get_flags(wxSizer* parent_sizer, const ItemStyle& item_style) {
            wxSizerFlags flags;

            flags = wxSizerFlags(item_style.proportion);

            {
                int orientation = 0;
                if (auto* box_sizer = dynamic_cast<wxBoxSizer*>(parent_sizer)) {
                    orientation = box_sizer->GetOrientation();
                } else if (auto* wrap_sizer = dynamic_cast<wxWrapSizer*>(parent_sizer)) {
                    orientation = wrap_sizer->GetOrientation();
                }

                if (orientation == wxVERTICAL) {
                    switch (item_style.align) {
                        case ItemStyle::Align::Start:
                            flags = flags.Left();
                            break;
                        case ItemStyle::Align::Center:
                            flags = flags.CenterHorizontal();
                            break;
                        case ItemStyle::Align::End:
                            flags = flags.Right();
                            break;
                        default:
                            break;
                    }
                }
                if (orientation == wxHORIZONTAL) {
                    switch (item_style.align) {
                        case ItemStyle::Align::Start:
                            flags = flags.Top();
                            break;
                        case ItemStyle::Align::Center:
                            flags = flags.CenterVertical();
                            break;
                        case ItemStyle::Align::End:
                            flags = flags.Bottom();
                            break;
                        default:
                            break;
                    }
                }
            }

            if (item_style.expand) {
                flags = flags.Expand();
            }

            if (item_style.keep_ratio) {
                flags = flags.Shaped();
            }

            {
                std::map<wxDirection, int> borders{
                    {wxTOP, item_style.border.top},
                    {wxBOTTOM, item_style.border.bottom},
                    {wxLEFT, item_style.border.left},
                    {wxRIGHT, item_style.border.right}
                };
                for (auto const [direction, val] : borders) {
                    if (val >= 0) {
                        flags = flags.Border(direction, val);
                    }
                }
            }

            return flags;
        }

        void set_style(wxWindow* window, const ItemStyle& item_style) {
            if (auto* parent = window->GetParent()) {
                if (auto* parent_sizer = parent->GetSizer()) {
                    size_t i = 0;
                    for (auto* child : parent_sizer->GetChildren()) {
                        if (child->GetWindow() == window) {
                            break;
                        }

                        ++i;
                    }

                    parent_sizer->Detach(window);
                    parent_sizer->Insert(i, window, get_flags(parent_sizer, item_style));
                }
            }
        }

        template<typename T>
        Reference ui_new() {
            T* window = new T;

            auto obj = GC::new_object();
            obj->c_obj.set(std::make_unique<wxWeakRef<wxWindow>>(window));
            return Data(obj);
        }

        Reference ui_position_get(wxWindow* window) {
            auto point = window->GetPosition();

            return Data(GC::new_object({ Data(static_cast<OV_INT>(point.x)), Data(static_cast<OV_INT>(point.y)) }));
        }
        Reference ui_position_set(wxWindow* window, OV_INT x, OV_INT y) {
            window->SetPosition(wxPoint(static_cast<int>(x), static_cast<int>(y)));

            return Data{};
        }

        Reference ui_size_get(wxWindow* window) {
            auto point = window->GetSize();

            return Data(GC::new_object({ Data(static_cast<OV_INT>(point.x)), Data(static_cast<OV_INT>(point.y)) }));
        }
        Reference ui_size_set(wxWindow* window, OV_INT x, OV_INT y) {
            window->SetSize(wxSize(static_cast<int>(x), static_cast<int>(y)));

            return Data{};
        }

        Reference ui_layout_get(wxWindow* window) {
            if (auto* sizer = window->GetSizer()) {
                int orientation = 0;
                if (auto* box_sizer = dynamic_cast<wxBoxSizer*>(sizer)) {
                    orientation = box_sizer->GetOrientation();
                } else if (auto* wrap_sizer = dynamic_cast<wxWrapSizer*>(sizer)) {
                    orientation = wrap_sizer->GetOrientation();
                }

                if (orientation == wxVERTICAL) {
                    return Data(static_cast<OV_INT>(1));
                }
                if (orientation == wxHORIZONTAL) {
                    return Data(static_cast<OV_INT>(2));
                }
            }

            return Data(static_cast<OV_INT>(-1));
        }
        Reference ui_layout_set(wxWindow* window, OV_INT layout) {
            wxSizer* sizer{};
            switch (layout) {
                case -1:
                    sizer = nullptr;
                    break;
                case 1:
                    sizer = new wxBoxSizer(wxVERTICAL);
                    break;
                case 2:
                    sizer = new wxBoxSizer(wxHORIZONTAL);
                    break;
                case 3:
                    sizer = new wxWrapSizer(wxVERTICAL);
                    break;
                case 4:
                    sizer = new wxWrapSizer(wxHORIZONTAL);
                    break;
                default:
                    break;
            }

            if (sizer) {
                for (auto* child : window->GetChildren()) {
                    wxSizerFlags flags;
                    if (auto* user_data = dynamic_cast<UserData*>(child->GetClientObject())) {
                        flags = get_flags(sizer, user_data->item_style);
                    }
                    sizer->Add(child, flags);
                }
            }
            window->SetSizerAndFit(sizer);

            return Data();
        }

        Reference ui_proportion_get(wxWindow* window) {
            if (auto* user_data = dynamic_cast<UserData*>(window->GetClientObject())) {
                return Data(static_cast<OV_INT>(user_data->item_style.proportion));
            }

            return Data(static_cast<OV_INT>(0));
        }
        Reference ui_proportion_set(wxWindow* window, OV_INT proportion) {
            if (window->GetClientObject() == nullptr) {
                window->SetClientObject(new UserData());
            }
            auto* user_data = dynamic_cast<UserData*>(window->GetClientObject());

            user_data->item_style.proportion = proportion;
            set_style(window, user_data->item_style);

            return Data();
        }

        Reference ui_align_get(wxWindow* window) {
            if (auto* user_data = dynamic_cast<UserData*>(window->GetClientObject())) {
                return Data(static_cast<OV_INT>(user_data->item_style.align));
            }

            return Data(static_cast<OV_INT>(0));
        }
        Reference ui_align_set(wxWindow* window, OV_INT align) {
            if (window->GetClientObject() == nullptr) {
                window->SetClientObject(new UserData());
            }
            auto* user_data = dynamic_cast<UserData*>(window->GetClientObject());

            user_data->item_style.align = static_cast<ItemStyle::Align>(align);
            set_style(window, user_data->item_style);

            return Data();
        }

        Reference ui_expand_get(wxWindow* window) {
            if (auto* user_data = dynamic_cast<UserData*>(window->GetClientObject())) {
                return Data(user_data->item_style.expand);
            }

            return Data(false);
        }
        Reference ui_expand_set(wxWindow* window, bool expand) {
            if (window->GetClientObject() == nullptr) {
                window->SetClientObject(new UserData());
            }
            auto* user_data = dynamic_cast<UserData*>(window->GetClientObject());

            user_data->item_style.expand = expand;
            set_style(window, user_data->item_style);

            return Data();
        }

        Reference ui_keep_ratio_get(wxWindow* window) {
            if (auto* user_data = dynamic_cast<UserData*>(window->GetClientObject())) {
                return Data(user_data->item_style.keep_ratio);
            }

            return Data(false);
        }
        Reference ui_keep_ratio_set(wxWindow* window, bool keep_ratio) {
            if (window->GetClientObject() == nullptr) {
                window->SetClientObject(new UserData());
            }
            auto* user_data = dynamic_cast<UserData*>(window->GetClientObject());

            user_data->item_style.keep_ratio = keep_ratio;
            set_style(window, user_data->item_style);

            return Data();
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

        namespace Dialog {

            Reference ui_dialog_new(std::string const& title) {
                auto* dialog = new wxDialog(nullptr, wxID_ANY, title);
                dialog->Show(true);

                auto obj = GC::new_object();
                obj->c_obj.set(std::make_unique<wxWeakRef<wxWindow>>(dialog));
                return Data(obj);
            }

        }

        namespace Panel {

            Reference ui_panel_create(wxWindow* window, wxWindow* parent) {
                dynamic_cast<wxPanel*>(window)->Create(parent, wxID_ANY);

                return Data{};
            }

        }

        namespace Control {

            Reference ui_ctrl_label_get(wxWindow* window) {
                auto* control = dynamic_cast<wxControl*>(window);

                return Data(GC::new_object(control->GetLabel().ToStdString()));
            }

            Reference ui_ctrl_label_set(wxWindow* window, std::string const& label) {
                auto* control = dynamic_cast<wxControl*>(window);

                control->SetLabel(label);

                return Data{};
            }

            namespace ActivityIndicator {

                Reference ui_activityindicator_create(wxWindow* window, wxWindow* parent) {
                    dynamic_cast<wxActivityIndicator*>(window)->Create(parent, wxID_ANY);

                    return Data{};
                }

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

                Reference ui_button_create(wxWindow* window, wxWindow* parent) {
                    dynamic_cast<wxButton*>(window)->Create(parent, wxID_ANY);

                    return Data{};
                }

            }

            namespace Calendar {

                Reference ui_calendar_create(wxWindow* window, wxWindow* parent) {
                    dynamic_cast<wxCalendarCtrl*>(window)->Create(parent, wxID_ANY);

                    return Data{};
                }

                Reference ui_calendar_value_get(wxWindow* window) {
                    auto* calendar = dynamic_cast<wxCalendarCtrl*>(window);

                    return Data(static_cast<OV_INT>(calendar->GetDate().GetTicks()));
                }

                Reference ui_calendar_value_set(wxWindow* window, OV_INT time) {
                    auto* calendar = dynamic_cast<wxCalendarCtrl*>(window);

                    calendar->SetDate(wxDateTime(static_cast<time_t>(time)));

                    return Data{};
                }

            }

            namespace Checkbox {

                Reference ui_checkbox_create(wxWindow* window, wxWindow* parent) {
                    dynamic_cast<wxCheckBox*>(window)->Create(parent, wxID_ANY, "");

                    return Data{};
                }

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

                Reference ui_choice_create(wxWindow* window, wxWindow* parent) {
                    dynamic_cast<wxChoice*>(window)->Create(parent, wxID_ANY);

                    return Data{};
                }

                Reference ui_choice_size_get(wxWindow* window) {
                    auto* choice = dynamic_cast<wxChoice*>(window);

                    return Data(static_cast<OV_INT>(choice->GetColumns()));
                }

                Reference ui_choice_size_set(wxWindow* window, OV_INT size) {
                    auto* choice = dynamic_cast<wxChoice*>(window);

                    choice->SetColumns(static_cast<int>(size));

                    return Data{};
                }

                Reference ui_choice_string_get(wxWindow* window, OV_INT i) {
                    auto* choice = dynamic_cast<wxChoice*>(window);

                    if (i < 0 || i >= choice->GetCount())
                        throw FunctionArgumentsError();

                    return Data(Object(std::string(choice->GetString(i))));
                }

                Reference ui_choice_string_set(wxWindow* window, OV_INT i, std::string const& string) {
                    auto* choice = dynamic_cast<wxChoice*>(window);

                    if (i < 0 || i >= choice->GetCount())
                        throw FunctionArgumentsError();

                    choice->SetString(i, string);

                    return Data{};
                }

                Reference ui_choice_value_get(wxWindow* window) {
                    auto* choice = dynamic_cast<wxChoice*>(window);

                    return Data(static_cast<OV_INT>(choice->GetSelection()));
                }

                Reference ui_choice_value_set(wxWindow* window, OV_INT i) {
                    auto* choice = dynamic_cast<wxChoice*>(window);

                    choice->SetSelection(i);

                    return Data{};
                }

            }

            namespace ColorPicker {

                Reference ui_colorpicker_create(wxWindow* window, wxWindow* parent) {
                    dynamic_cast<wxColourPickerCtrl*>(window)->Create(parent, wxID_ANY);

                    return Data{};
                }

                Reference ui_colorpicker_value_get(wxWindow* window) {
                    auto* colour_picker = dynamic_cast<wxColourPickerCtrl*>(window);

                    return Data(Object(std::string(colour_picker->GetColour().GetAsString())));
                }

                Reference ui_colorpicker_value_set(wxWindow* window, std::string const& color) {
                    auto* colour_picker = dynamic_cast<wxColourPickerCtrl*>(window);

                    colour_picker->SetColour(color);

                    return Data{};
                }

            }

            namespace DatePicker {

                Reference ui_datepicker_create(wxWindow* window, wxWindow* parent) {
                    dynamic_cast<wxDatePickerCtrl*>(window)->Create(parent, wxID_ANY);

                    return Data{};
                }

                Reference ui_datepicker_value_get(wxWindow* window) {
                    auto* date_picker = dynamic_cast<wxDatePickerCtrl*>(window);

                    return Data(static_cast<OV_INT>(date_picker->GetValue().GetTicks()));
                }

                Reference ui_datepicker_value_set(wxWindow* window, OV_INT time) {
                    auto* date_picker = dynamic_cast<wxDatePickerCtrl*>(window);

                    date_picker->SetValue(wxDateTime(static_cast<time_t>(time)));

                    return Data{};
                }

            }

            namespace Gauge {

                Reference ui_gauge_create(wxWindow* window, wxWindow* parent) {
                    dynamic_cast<wxGauge*>(window)->Create(parent, wxID_ANY, 100);

                    return Data{};
                }

                Reference ui_gauge_range_get(wxWindow* window) {
                    auto* gauge = dynamic_cast<wxGauge*>(window);

                    return Data(static_cast<OV_INT>(gauge->GetRange()));
                }

                Reference ui_gauge_range_set(wxWindow* window, OV_INT value) {
                    auto* gauge = dynamic_cast<wxGauge*>(window);

                    gauge->SetRange(value);

                    return Data{};
                }

                Reference ui_gauge_value_get(wxWindow* window) {
                    auto* gauge = dynamic_cast<wxGauge*>(window);

                    return Data(static_cast<OV_INT>(gauge->GetValue()));
                }

                Reference ui_gauge_value_set(wxWindow* window, OV_INT range) {
                    auto* gauge = dynamic_cast<wxGauge*>(window);

                    gauge->SetValue(range);

                    return Data{};
                }

            }

            namespace Hyperlink {

                Reference ui_hyperlink_create(wxWindow* window, wxWindow* parent) {
                    dynamic_cast<wxHyperlinkCtrl*>(window)->Create(parent, wxID_ANY, "", "");

                    return Data{};
                }

                Reference ui_hyperlink_url_get(wxWindow* window) {
                    auto* hyperlink = dynamic_cast<wxHyperlinkCtrl*>(window);

                    return Data(Object(std::string(hyperlink->GetURL())));
                }

                Reference ui_hyperlink_url_set(wxWindow* window, std::string const& url) {
                    auto* hyperlink = dynamic_cast<wxHyperlinkCtrl*>(window);

                    hyperlink->SetURL(url);

                    return Data{};
                }

            }

            namespace Label {

                Reference ui_label_create(wxWindow* window, wxWindow* parent) {
                    dynamic_cast<wxStaticText*>(window)->Create(parent, wxID_ANY, "");

                    return Data{};
                }

            }

            namespace RadioButton {

                Reference ui_radiobutton_create(wxWindow* window, wxWindow* parent) {
                    dynamic_cast<wxRadioButton*>(window)->Create(parent, wxID_ANY, "");

                    return Data{};
                }

                Reference ui_radiobutton_value_get(wxWindow* window) {
                    auto* radio_button = dynamic_cast<wxRadioButton*>(window);

                    return Data(radio_button->GetValue());
                }

                Reference ui_radiobutton_value_set(wxWindow* window, bool value) {
                    auto* radio_button = dynamic_cast<wxRadioButton*>(window);

                    radio_button->SetValue(value);

                    return Data{};
                }

            }

            namespace Search {

                Reference ui_search_create(wxWindow* window, wxWindow* parent) {
                    dynamic_cast<wxSearchCtrl*>(window)->Create(parent, wxID_ANY);

                    return Data{};
                }

                Reference ui_search_value_get(wxWindow* window) {
                    auto* search = dynamic_cast<wxSearchCtrl*>(window);

                    return Data(Object(std::string(search->GetValue())));
                }

                Reference ui_search_value_set(wxWindow* window, std::string const& value) {
                    auto* search = dynamic_cast<wxSearchCtrl*>(window);

                    search->SetValue(value);

                    return Data{};
                }

            }

            namespace Text {

                Reference ui_text_create(wxWindow* window, wxWindow* parent) {
                    dynamic_cast<wxTextCtrl*>(window)->Create(parent, wxID_ANY);

                    return Data{};
                }

                Reference ui_text_value_get(wxWindow* window) {
                    auto* text = dynamic_cast<wxTextCtrl*>(window);

                    return Data(Object(std::string(text->GetValue())));
                }

                Reference ui_text_value_set(wxWindow* window, std::string const& value) {
                    auto* text = dynamic_cast<wxTextCtrl*>(window);

                    text->SetValue(value);

                    return Data{};
                }

            }

            namespace Box {

                Reference ui_box_create(wxWindow* window, wxWindow* parent) {
                    dynamic_cast<wxStaticBox*>(window)->Create(parent, wxID_ANY, "");

                    return Data{};
                }

            }

            namespace Slider {

                Reference ui_slider_create(wxWindow* window, wxWindow* parent) {
                    dynamic_cast<wxSlider*>(window)->Create(parent, wxID_ANY, 0, 0, 1);

                    return Data{};
                }

                Reference ui_slider_value_get(wxWindow* window) {
                    auto* slider = dynamic_cast<wxSlider*>(window);

                    return Data(static_cast<OV_INT>(slider->GetValue()));
                }

                Reference ui_slider_value_set(wxWindow* window, OV_INT value) {
                    auto* slider = dynamic_cast<wxSlider*>(window);

                    slider->SetValue(value);

                    return Data{};
                }

                Reference ui_slider_min_get(wxWindow* window) {
                    auto* slider = dynamic_cast<wxSlider*>(window);

                    return Data(static_cast<OV_INT>(slider->GetMin()));
                }

                Reference ui_slider_min_set(wxWindow* window, OV_INT min) {
                    auto* slider = dynamic_cast<wxSlider*>(window);

                    slider->SetMin(min);

                    return Data{};
                }

                Reference ui_slider_max_get(wxWindow* window) {
                    auto* slider = dynamic_cast<wxSlider*>(window);

                    return Data(static_cast<OV_INT>(slider->GetMax()));
                }

                Reference ui_slider_max_set(wxWindow* window, OV_INT max) {
                    auto* slider = dynamic_cast<wxSlider*>(window);

                    slider->SetMax(max);

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

                Reference ui_webview_create(wxWindow* window, wxWindow* parent) {
                    dynamic_cast<wxWebView*>(window)->Create(parent, wxID_ANY);

                    return Data{};
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


    void init(GlobalContext& context) {
        auto& s = context.get_global().system;

        add_function(s.get_property("ui_position_set"), Window::ui_position_get);
        add_function(s.get_property("ui_position_set"), Window::ui_position_set);
        add_function(s.get_property("ui_size_get"), Window::ui_size_get);
        add_function(s.get_property("ui_size_set"), Window::ui_size_set);
        add_function(s.get_property("ui_layout_get"), Window::ui_layout_get);
        add_function(s.get_property("ui_layout_set"), Window::ui_layout_set);
        add_function(s.get_property("ui_proportion_get"), Window::ui_proportion_get);
        add_function(s.get_property("ui_proportion_set"), Window::ui_proportion_set);
        add_function(s.get_property("ui_align_get"), Window::ui_align_get);
        add_function(s.get_property("ui_align_set"), Window::ui_align_set);
        add_function(s.get_property("ui_expand_get"), Window::ui_expand_get);
        add_function(s.get_property("ui_expand_set"), Window::ui_expand_set);
        add_function(s.get_property("ui_keep_ratio_get"), Window::ui_keep_ratio_get);
        add_function(s.get_property("ui_keep_ratio_set"), Window::ui_keep_ratio_set);

        add_function(s.get_property("ui_frame_new"), Window::Frame::ui_frame_new);

        add_function(s.get_property("ui_panel_new"), Window::ui_new<wxPanel>);
        add_function(s.get_property("ui_panel_create"), Window::Panel::ui_panel_create);

        add_function(s.get_property("ui_ctrl_label_get"), Window::Control::ui_ctrl_label_get);
        add_function(s.get_property("ui_ctrl_label_set"), Window::Control::ui_ctrl_label_set);

        add_function(s.get_property("ui_activityindicator_new"), Window::ui_new<wxActivityIndicator>);
        add_function(s.get_property("ui_activityindicator_create"), Window::Control::ActivityIndicator::ui_activityindicator_create);
        add_function(s.get_property("ui_activityindicator_running_get"), Window::Control::ActivityIndicator::ui_activityindicator_running_get);
        add_function(s.get_property("ui_activityindicator_running_set"), Window::Control::ActivityIndicator::ui_activityindicator_running_set);

        add_function(s.get_property("ui_button_new"), Window::ui_new<wxButton>);
        add_function(s.get_property("ui_button_create"), Window::Control::Button::ui_button_create);
        add_event(s.get_property("ui_button_event"), wxEVT_BUTTON);

        add_function(s.get_property("ui_calendar_new"), Window::ui_new<wxCalendarCtrl>);
        add_function(s.get_property("ui_calendar_create"), Window::Control::Calendar::ui_calendar_create);
        add_function(s.get_property("ui_calendar_value_get"), Window::Control::Calendar::ui_calendar_value_get);
        add_function(s.get_property("ui_calendar_value_set"), Window::Control::Calendar::ui_calendar_value_set);
        add_event(s.get_property("ui_calendar_value_event"), wxEVT_CALENDAR_SEL_CHANGED);

        add_function(s.get_property("ui_checkbox_new"), Window::ui_new<wxCheckBox>);
        add_function(s.get_property("ui_checkbox_create"), Window::Control::Checkbox::ui_checkbox_create);
        add_function(s.get_property("ui_checkbox_value_get"), Window::Control::Checkbox::ui_checkbox_value_get);
        add_function(s.get_property("ui_checkbox_value_set"), Window::Control::Checkbox::ui_checkbox_value_set);
        add_event(s.get_property("ui_checkbox_value_event"), wxEVT_CHECKBOX);

        add_function(s.get_property("ui_choice_new"), Window::ui_new<wxChoice>);
        add_function(s.get_property("ui_choice_create"), Window::Control::Choice::ui_choice_create);
        add_function(s.get_property("ui_choice_size_get"), Window::Control::Choice::ui_choice_size_get);
        add_function(s.get_property("ui_choice_size_set"), Window::Control::Choice::ui_choice_size_set);
        add_function(s.get_property("ui_choice_string_get"), Window::Control::Choice::ui_choice_string_get);
        add_function(s.get_property("ui_choice_string_set"), Window::Control::Choice::ui_choice_string_set);
        add_function(s.get_property("ui_choice_value_get"), Window::Control::Choice::ui_choice_value_get);
        add_function(s.get_property("ui_choice_value_set"), Window::Control::Choice::ui_choice_value_set);
        add_event(s.get_property("ui_choice_value_event"), wxEVT_CHOICE);

        add_function(s.get_property("ui_colorpicker_new"), Window::ui_new<wxColourPickerCtrl>);
        add_function(s.get_property("ui_colorpicker_create"), Window::Control::ColorPicker::ui_colorpicker_create);
        add_function(s.get_property("ui_colorpicker_value_get"), Window::Control::ColorPicker::ui_colorpicker_value_get);
        add_function(s.get_property("ui_colorpicker_value_set"), Window::Control::ColorPicker::ui_colorpicker_value_set);
        add_event(s.get_property("ui_colorpicker_value_event"), wxEVT_COLOURPICKER_CHANGED);

        add_function(s.get_property("ui_datepicker_new"), Window::ui_new<wxDatePickerCtrl>);
        add_function(s.get_property("ui_datepicker_create"), Window::Control::DatePicker::ui_datepicker_create);
        add_function(s.get_property("ui_datepicker_value_get"), Window::Control::DatePicker::ui_datepicker_value_get);
        add_function(s.get_property("ui_datepicker_value_set"), Window::Control::DatePicker::ui_datepicker_value_set);
        add_event(s.get_property("ui_datepicker_value_event"), wxEVT_DATE_CHANGED);

        add_function(s.get_property("ui_gauge_new"), Window::ui_new<wxGauge>);
        add_function(s.get_property("ui_gauge_create"), Window::Control::Gauge::ui_gauge_create);
        add_function(s.get_property("ui_gauge_range_get"), Window::Control::Gauge::ui_gauge_range_get);
        add_function(s.get_property("ui_gauge_range_set"), Window::Control::Gauge::ui_gauge_range_set);
        add_function(s.get_property("ui_gauge_value_get"), Window::Control::Gauge::ui_gauge_value_get);
        add_function(s.get_property("ui_gauge_value_set"), Window::Control::Gauge::ui_gauge_value_set);

        add_function(s.get_property("ui_hyperlink_new"), Window::ui_new<wxHyperlinkCtrl>);
        add_function(s.get_property("ui_hyperlink_create"), Window::Control::Hyperlink::ui_hyperlink_create);
        add_function(s.get_property("ui_hyperlink_url_get"), Window::Control::Hyperlink::ui_hyperlink_url_get);
        add_function(s.get_property("ui_hyperlink_url_set"), Window::Control::Hyperlink::ui_hyperlink_url_set);

        add_function(s.get_property("ui_label_new"), Window::ui_new<wxStaticText>);
        add_function(s.get_property("ui_label_create"), Window::Control::Label::ui_label_create);

        add_function(s.get_property("ui_radiobutton_new"), Window::ui_new<wxRadioButton>);
        add_function(s.get_property("ui_radiobutton_create"), Window::Control::RadioButton::ui_radiobutton_create);
        add_function(s.get_property("ui_radiobutton_value_get"), Window::Control::RadioButton::ui_radiobutton_value_get);
        add_function(s.get_property("ui_radiobutton_value_set"), Window::Control::RadioButton::ui_radiobutton_value_set);
        add_event(s.get_property("ui_radiobutton_value_event"), wxEVT_RADIOBUTTON);

        add_function(s.get_property("ui_search_new"), Window::ui_new<wxSearchCtrl>);
        add_function(s.get_property("ui_search_create"), Window::Control::Search::ui_search_create);
        add_function(s.get_property("ui_search_value_get"), Window::Control::Search::ui_search_value_get);
        add_function(s.get_property("ui_search_value_set"), Window::Control::Search::ui_search_value_set);
        add_event(s.get_property("ui_search_value_event"), wxEVT_SEARCH);
        add_event(s.get_property("ui_search_cancel_event"), wxEVT_SEARCH_CANCEL);

        add_function(s.get_property("ui_text_new"), Window::ui_new<wxTextCtrl>);
        add_function(s.get_property("ui_text_create"), Window::Control::Text::ui_text_create);
        add_function(s.get_property("ui_text_value_get"), Window::Control::Text::ui_text_value_get);
        add_function(s.get_property("ui_text_value_set"), Window::Control::Text::ui_text_value_set);

        add_function(s.get_property("ui_box_new"), Window::ui_new<wxStaticText>);
        add_function(s.get_property("ui_box_create"), Window::Control::Box::ui_box_create);

        add_function(s.get_property("ui_slider_new"), Window::ui_new<wxSlider>);
        add_function(s.get_property("ui_slider_create"), Window::Control::Slider::ui_slider_create);
        add_function(s.get_property("ui_slider_value_get"), Window::Control::Slider::ui_slider_value_get);
        add_function(s.get_property("ui_slider_value_set"), Window::Control::Slider::ui_slider_value_set);
        add_event(s.get_property("ui_slider_value_event"), wxEVT_SLIDER);
        add_function(s.get_property("ui_slider_min_get"), Window::Control::Slider::ui_slider_min_get);
        add_function(s.get_property("ui_slider_min_set"), Window::Control::Slider::ui_slider_min_set);
        add_function(s.get_property("ui_slider_max_get"), Window::Control::Slider::ui_slider_max_get);
        add_function(s.get_property("ui_slider_max_set"), Window::Control::Slider::ui_slider_max_set);

        add_function(s.get_property("ui_webview_new"), Window::Control::Webview::ui_webview_new);
        add_function(s.get_property("ui_webview_create"), Window::Control::Webview::ui_webview_create);
        add_function(s.get_property("ui_webview_title_get"), Window::Control::Webview::ui_webview_title_get);
        add_function(s.get_property("ui_webview_url_get"), Window::Control::Webview::ui_webview_url_get);
        add_function(s.get_property("ui_webview_url_set"), Window::Control::Webview::ui_webview_url_set);
        add_function(s.get_property("ui_webview_set_page"), Window::Control::Webview::ui_webview_set_page);
    }

}


#else


namespace Interpreter::SystemFunctions {

    namespace UI {

        void init(GlobalContext& /*unused*/) {}

    }

}


#endif
