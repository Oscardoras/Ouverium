#include <any>
#include <cstdint>
#include <ctime>
#include <exception>
#include <memory>
#include <string>
#include <typeinfo>

#include "ouverium/types.h"

#include "SystemFunction.hpp"

#include "../Interpreter.hpp"

#include "../../parser/Expressions.hpp"


#ifdef OUVERIUM_WXWIDGETS


#include <wx/wx.h>
#include <wx/activityindicator.h>
#include <wx/calctrl.h>
#include <wx/control.h>
#include <wx/clrpicker.h>
#include <wx/datectrl.h>
#include <wx/dialog.h>
#include <wx/filepicker.h>
#include <wx/gauge.h>
#include <wx/hyperlink.h>
#include <wx/radiobox.h>
#include <wx/simplebook.h>
#include <wx/srchctrl.h>
#include <wx/webview.h>
#include <wx/wrapsizer.h>

namespace Interpreter::SystemFunctions {
    template<>
    inline std::optional<wxWindow*> get_arg<wxWindow*>(Data const& data) {
        auto const& obj = data.get<ObjectPtr>();
        auto it = obj->properties.find("_handle");
        if (it != obj->properties.end())
            return it->second.get<wxWeakRef<wxWindow>>();
        else
            return nullptr;
    }
}

namespace Interpreter::SystemFunctions::UI {

    struct ItemStyle {
        unsigned proportion = 0;
        enum class Align : uint8_t {
            Start,
            Center,
            End,
            Expand,
        } align = Align::Start;
        bool keep_ratio = false;
        int border{};
        int spacing{};
    };

    struct UserData : public wxClientData {
        ItemStyle item_style;
    };

    template<bool Skip = false, typename Args>
    void add_event(IndirectReference const& reference, wxEventTypeTag<Args> const& type) {
        struct EventHandler {
            wxEventTypeTag<Args> const& type;

            EventHandler(wxEventTypeTag<Args> const& type) : type(type) {}

            Reference operator()(FunctionContext& context) {
                try {
                    auto* window = get_arg<wxWindow*>(context["window"].to_data(context)).value();
                    auto callback = context["callback"];

                    auto& global = context.get_global();
                    auto function_context = std::make_shared<FunctionContext>(global, nullptr);
                    function_context->add_symbol("callback", callback);

                    window->Bind(type, [function_context](Args& ev) {
                        try {
                            Interpreter::try_call_function(*function_context, nullptr, (*function_context)["callback"], std::make_shared<Parser::Tuple>());
                        } catch (Interpreter::Exception const& ex) {
                            ex.print_stack_trace(*function_context);
                        }

                        if constexpr (Skip) {
                            ev.Skip();
                        }
                    });

                    return Data{};
                } catch (std::exception const&) {
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
            wxSizerFlags flags(item_style.proportion);

            {
                int orientation = 0;
                if (auto* box_sizer = dynamic_cast<wxBoxSizer*>(parent_sizer)) {
                    orientation = box_sizer->GetOrientation();
                } else if (auto* wrap_sizer = dynamic_cast<wxWrapSizer*>(parent_sizer)) {
                    orientation = wrap_sizer->GetOrientation();
                }

                int border = item_style.border;
                border = border >= 0 ? border : -border * wxSizerFlags::GetDefaultBorder();

                if (orientation == wxVERTICAL) {
                    switch (item_style.align) {
                        case ItemStyle::Align::Start:
                            flags.Left();
                            break;
                        case ItemStyle::Align::Center:
                            flags.CenterHorizontal();
                            break;
                        case ItemStyle::Align::End:
                            flags.Right();
                            break;
                        default:
                            break;
                    }

                    flags.Border(wxLEFT | wxRIGHT, border);
                }
                if (orientation == wxHORIZONTAL) {
                    switch (item_style.align) {
                        case ItemStyle::Align::Start:
                            flags.Top();
                            break;
                        case ItemStyle::Align::Center:
                            flags.CenterVertical();
                            break;
                        case ItemStyle::Align::End:
                            flags.Bottom();
                            break;
                        default:
                            break;
                    }

                    flags.Border(wxTOP | wxBOTTOM, border);
                }
            }

            if (item_style.align == ItemStyle::Align::Expand) {
                flags.Expand();
            }

            if (item_style.keep_ratio) {
                flags.Shaped();
            }

            return flags;
        }

        void set_style(wxWindow* window) {
            ItemStyle item_style;
            if (auto* user_data = dynamic_cast<UserData*>(window->GetClientObject()))
                item_style = user_data->item_style;

            auto* sizer = window->GetSizer();
            if (sizer) {
                sizer->Clear();
                auto const& children = window->GetChildren();
                for (size_t i = 0; i < children.size(); ++i) {
                    auto* child = children[i];

                    if (dynamic_cast<wxDialog*>(child))
                        continue;

                    wxSizerFlags flags;
                    if (auto* user_data = dynamic_cast<UserData*>(child->GetClientObject())) {
                        flags = get_flags(sizer, user_data->item_style);
                    }

                    if (i != 0 && item_style.spacing > 0) {
                        sizer->AddSpacer(item_style.spacing);
                    }
                    sizer->Add(child, flags);
                }
            }
            window->Fit();

            if (auto* parent = window->GetParent()) {
                if (auto* parent_sizer = parent->GetSizer()) {
                    size_t i = 0;
                    for (auto* child : parent_sizer->GetChildren()) {
                        if (child->GetWindow() == window)
                            break;

                        ++i;
                    }

                    parent_sizer->Detach(window);
                    parent_sizer->Insert(i, window, get_flags(parent_sizer, item_style));
                    parent->Fit();
                }
            }
        }

        template<typename T>
        Reference ui_new() {
            T* window = new T;

            auto obj = GC::new_object();
            obj->properties["_handle"] = Data(std::any(wxWeakRef<wxWindow>(window)));
            return Data(obj);
        }

        template<typename T>
        Reference ui_is(Data const& data) {
            try {
                auto window = get_arg<wxWindow*>(data);
                if (window.has_value())
                    if (dynamic_cast<T*>(window.value()))
                        return Data(true);
            } catch (Data::BadAccess const&) {
            } catch (std::bad_cast const&) {}

            return Data(false);
        }

        Reference ui_visible_get(wxWindow* window) {
            auto visible = window->IsShown();

            return Data(visible);
        }
        Reference ui_visible_set(wxWindow* window, bool visible) {
            if (auto* parent = window->GetParent()) {
                if (auto* parent_sizer = parent->GetSizer()) {
                    parent_sizer->Show(window, visible);
                    parent->Fit();
                }
            }
            window->Show(visible);

            return Data{};
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

        Reference ui_min_size_get(wxWindow* window) {
            auto point = window->GetMinSize();

            return Data(GC::new_object({ Data(static_cast<OV_INT>(point.x)), Data(static_cast<OV_INT>(point.y)) }));
        }
        Reference ui_min_size_set(wxWindow* window, OV_INT x, OV_INT y) {
            window->SetMinSize(wxSize(static_cast<int>(x), static_cast<int>(y)));

            return Data{};
        }

        Reference ui_max_size_get(wxWindow* window) {
            auto point = window->GetMaxSize();

            return Data(GC::new_object({ Data(static_cast<OV_INT>(point.x)), Data(static_cast<OV_INT>(point.y)) }));
        }
        Reference ui_max_size_set(wxWindow* window, OV_INT x, OV_INT y) {
            window->SetMaxSize(wxSize(static_cast<int>(x), static_cast<int>(y)));

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

            window->SetSizer(sizer);
            set_style(window);

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
            set_style(window);

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
            set_style(window);

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
            set_style(window);

            return Data();
        }

        Reference ui_border_get(wxWindow* window) {
            if (auto* user_data = dynamic_cast<UserData*>(window->GetClientObject())) {
                auto border = user_data->item_style.border;

                return Data(static_cast<OV_INT>(border));
            }

            return Data(static_cast<OV_INT>(ItemStyle{}.border));
        }
        Reference ui_border_set(wxWindow* window, OV_INT border) {
            if (window->GetClientObject() == nullptr) {
                window->SetClientObject(new UserData());
            }
            auto* user_data = dynamic_cast<UserData*>(window->GetClientObject());

            user_data->item_style.border = static_cast<int>(border);
            set_style(window);

            return Data();
        }

        Reference ui_spacing_get(wxWindow* window) {
            if (auto* user_data = dynamic_cast<UserData*>(window->GetClientObject())) {
                auto spacing = user_data->item_style.spacing;

                return Data(static_cast<OV_INT>(spacing));
            }

            return Data(static_cast<OV_INT>(ItemStyle{}.spacing));
        }
        Reference ui_spacing_set(wxWindow* window, OV_INT spacing) {
            if (window->GetClientObject() == nullptr) {
                window->SetClientObject(new UserData());
            }
            auto* user_data = dynamic_cast<UserData*>(window->GetClientObject());

            user_data->item_style.spacing = static_cast<int>(spacing);
            set_style(window);

            return Data();
        }

        Reference ui_background_color_get(wxWindow* window) {
            auto color = window->GetBackgroundColour();

            return Data(GC::new_object({
                Data(static_cast<OV_INT>(color.Red())),
                Data(static_cast<OV_INT>(color.Green())),
                Data(static_cast<OV_INT>(color.Blue())),
                Data(static_cast<OV_INT>(color.Alpha()))
                }));
        }
        Reference ui_background_color_set(wxWindow* window, ObjectPtr const& color) {
            window->SetBackgroundColour({
                static_cast<unsigned char>(color->array.at(0).get<OV_INT>()),
                static_cast<unsigned char>(color->array.at(1).get<OV_INT>()),
                static_cast<unsigned char>(color->array.at(2).get<OV_INT>()),
                static_cast<unsigned char>(color->array.at(3).get<OV_INT>())
                });

            return Data{};
        }

        Reference ui_foreground_color_get(wxWindow* window) {
            auto color = window->GetBackgroundColour();

            return Data(GC::new_object({
                Data(static_cast<OV_INT>(color.Red())),
                Data(static_cast<OV_INT>(color.Green())),
                Data(static_cast<OV_INT>(color.Blue())),
                Data(static_cast<OV_INT>(color.Alpha()))
                }));
        }
        Reference ui_foreground_color_set(wxWindow* window, ObjectPtr const& color) {
            window->SetForegroundColour({
                static_cast<unsigned char>(color->array.at(0).get<OV_INT>()),
                static_cast<unsigned char>(color->array.at(1).get<OV_INT>()),
                static_cast<unsigned char>(color->array.at(2).get<OV_INT>()),
                static_cast<unsigned char>(color->array.at(3).get<OV_INT>())
                });

            return Data{};
        }

        namespace Window {

            Reference ui_window_title_get(wxWindow* window) {
                auto* control = dynamic_cast<wxTopLevelWindow*>(window);

                return Data(GC::new_object(control->GetTitle().ToStdString()));
            }

            Reference ui_window_title_set(wxWindow* window, std::string const& title) {
                auto* control = dynamic_cast<wxTopLevelWindow*>(window);

                control->SetTitle(title);

                return Data{};
            }

            namespace Frame {

                Reference ui_frame_create(wxWindow* window, wxWindow* parent) {
                    dynamic_cast<wxFrame*>(window)->Create(parent, wxID_ANY, "");

                    return Data{};
                }

            }

            namespace Dialog {

                Reference ui_dialog_create(wxWindow* window, wxWindow* parent) {
                    dynamic_cast<wxDialog*>(window)->Create(parent, wxID_ANY, "");

                    return Data{};
                }

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

                Reference ui_activityindicator_value_get(wxWindow* window) {
                    auto* activityIndicator = dynamic_cast<wxActivityIndicator*>(window);

                    return Data(activityIndicator->IsRunning());
                }

                Reference ui_activityindicator_value_set(wxWindow* window, bool state) {
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

                Reference ui_calendar_value_set_silent(wxWindow* window, OV_INT time) {
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

                Reference ui_choice_options_get(wxWindow* window) {
                    auto* choice = dynamic_cast<wxChoice*>(window);

                    auto options = GC::new_object();
                    options->array.reserve(choice->GetCount());
                    for (int i = 0; i < (int) choice->GetCount(); ++i) {
                        options->array.emplace_back(GC::new_object(Object(choice->GetString(i).ToStdString())));
                    }

                    return Data(options);
                }

                Reference ui_choice_options_set(wxWindow* window, ObjectPtr const& options) {
                    auto* choice = dynamic_cast<wxChoice*>(window);

                    std::vector<wxString> v;
                    v.reserve(options->array.size());
                    for (auto const& o : options->array) {
                        v.emplace_back(o.get<ObjectPtr>()->to_string());
                    }

                    choice->Set(v);

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

                    auto color = colour_picker->GetColour();

                    return Data(GC::new_object({
                        Data(static_cast<OV_INT>(color.Red())),
                        Data(static_cast<OV_INT>(color.Green())),
                        Data(static_cast<OV_INT>(color.Blue())),
                        Data(static_cast<OV_INT>(color.Alpha()))
                        }));
                }

                Reference ui_colorpicker_value_set(wxWindow* window, ObjectPtr const& color) {
                    auto* colour_picker = dynamic_cast<wxColourPickerCtrl*>(window);

                    colour_picker->SetColour({
                        static_cast<unsigned char>(color->array.at(0).get<OV_INT>()),
                        static_cast<unsigned char>(color->array.at(1).get<OV_INT>()),
                        static_cast<unsigned char>(color->array.at(2).get<OV_INT>()),
                        static_cast<unsigned char>(color->array.at(3).get<OV_INT>())
                        });

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
                    dynamic_cast<wxHyperlinkCtrl*>(window)->Create(parent, wxID_ANY, "", "about:blank");

                    return Data{};
                }

                Reference ui_hyperlink_url_get(wxWindow* window) {
                    auto* hyperlink = dynamic_cast<wxHyperlinkCtrl*>(window);

                    return Data(GC::new_object(Object(std::string(hyperlink->GetURL()))));
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

            namespace Text {

                Reference ui_text_create(wxWindow* window, wxWindow* parent) {
                    dynamic_cast<wxTextCtrl*>(window)->Create(parent, wxID_ANY);

                    return Data{};
                }

                Reference ui_text_value_get(wxWindow* window) {
                    auto* text = dynamic_cast<wxTextEntry*>(window);

                    return Data(GC::new_object(Object(std::string(text->GetValue()))));
                }

                Reference ui_text_value_set(wxWindow* window, std::string const& value) {
                    auto* text = dynamic_cast<wxTextEntry*>(window);

                    text->SetValue(value);

                    return Data{};
                }

                Reference ui_text_value_set_silent(wxWindow* window, std::string const& value) {
                    auto* text = dynamic_cast<wxTextEntry*>(window);

                    text->ChangeValue(value);

                    return Data{};
                }

                Reference ui_text_editable_get(wxWindow* window) {
                    auto* text = dynamic_cast<wxTextEntry*>(window);

                    return Data(text->IsEditable());
                }

                Reference ui_text_editable_set(wxWindow* window, bool value) {
                    auto* text = dynamic_cast<wxTextEntry*>(window);

                    text->SetEditable(value);

                    return Data{};
                }

                namespace MultilineText {

                    Reference ui_multiline_is(Data const& data) {
                        try {
                            auto window = get_arg<wxWindow*>(data);
                            if (window.has_value())
                                if (auto const* text = dynamic_cast<wxTextCtrl*>(window.value()))
                                    return Data(text->IsMultiLine());
                        } catch (Data::BadAccess const&) {
                        } catch (std::bad_cast const&) {}

                        return Data(false);
                    }

                    Reference ui_multilinetext_create(wxWindow* window, wxWindow* parent) {
                        dynamic_cast<wxTextCtrl*>(window)->Create(parent, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE);

                        return Data{};
                    }

                }

                namespace Search {

                    Reference ui_search_create(wxWindow* window, wxWindow* parent) {
                        dynamic_cast<wxSearchCtrl*>(window)->Create(parent, wxID_ANY);

                        return Data{};
                    }

                }

                namespace Password {

                    Reference ui_password_is(Data const& data) {
                        try {
                            auto window = get_arg<wxWindow*>(data);
                            if (window.has_value())
                                if (auto const* text = dynamic_cast<wxTextCtrl*>(window.value()))
                                    return Data(bool(text->GetWindowStyle() & wxTE_PASSWORD));
                        } catch (Data::BadAccess const&) {
                        } catch (std::bad_cast const&) {}

                        return Data(false);
                    }

                    Reference ui_password_create(wxWindow* window, wxWindow* parent) {
                        dynamic_cast<wxTextCtrl*>(window)->Create(parent, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_PASSWORD);

                        return Data{};
                    }

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

            namespace FilePicker {

                Reference ui_filepicker_create(wxWindow* window, wxWindow* parent) {
                    dynamic_cast<wxFilePickerCtrl*>(window)->Create(parent, wxID_ANY);

                    return Data{};
                }

                Reference ui_filepicker_value_get(wxWindow* window) {
                    auto* text = dynamic_cast<wxFilePickerCtrl*>(window);

                    return Data(GC::new_object(Object(std::string(text->GetPath()))));
                }

                Reference ui_filepicker_value_set(wxWindow* window, std::string const& value) {
                    auto* text = dynamic_cast<wxFilePickerCtrl*>(window);

                    text->SetPath(value);

                    return Data{};
                }

            }

            namespace Book {

                Reference ui_book_create(wxWindow* window, wxWindow* parent) {
                    auto* book = dynamic_cast<wxSimplebook*>(window);

                    book->Create(parent, wxID_ANY);

                    return Data{};
                }

                Reference ui_reload(wxWindow* window) {
                    auto* book = dynamic_cast<wxBookCtrlBase*>(window);

                    book->DeleteAllPages();

                    for (auto* child : book->GetChildren())
                        book->AddPage(child, "");

                    return Data{};
                }

                Reference ui_book_value_get(wxWindow* window) {
                    auto* book = dynamic_cast<wxBookCtrlBase*>(window);

                    return Data(static_cast<OV_INT>(book->GetSelection()));
                }

                Reference ui_book_value_set(wxWindow* window, OV_INT value) {
                    auto* book = dynamic_cast<wxBookCtrlBase*>(window);

                    book->SetSelection(static_cast<int>(value));

                    return Data{};
                }

                Reference ui_book_get_text(wxWindow* window, OV_INT index) {
                    auto* book = dynamic_cast<wxBookCtrlBase*>(window);

                    return Data(GC::new_object(book->GetPageText(index).ToStdString()));
                }

                Reference ui_book_set_text(wxWindow* window, OV_INT index, std::string const& text) {
                    auto* book = dynamic_cast<wxBookCtrlBase*>(window);

                    book->SetPageText(index, text);

                    return Data{};
                }

                namespace Notebook {

                    Reference ui_notebook_create(wxWindow* window, wxWindow* parent) {
                        auto* book = dynamic_cast<wxNotebook*>(window);

                        book->Create(parent, wxID_ANY);

                        return Data{};
                    }

                }

            }

            namespace Webview {

                Reference ui_webview_new() {
                    auto* webview = wxWebView::New();

                    auto obj = GC::new_object();
                    obj->properties["_handle"] = Data(std::any(wxWeakRef<wxWindow>(webview)));
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

        add_function(s.get_property("ui_is"), Window::ui_is<wxWindow>);
        add_function(s.get_property("ui_visible_get"), Window::ui_visible_get);
        add_function(s.get_property("ui_visible_set"), Window::ui_visible_set);
        add_function(s.get_property("ui_position_set"), Window::ui_position_get);
        add_function(s.get_property("ui_position_set"), Window::ui_position_set);
        add_function(s.get_property("ui_size_get"), Window::ui_size_get);
        add_function(s.get_property("ui_size_set"), Window::ui_size_set);
        add_function(s.get_property("ui_min_size_get"), Window::ui_min_size_get);
        add_function(s.get_property("ui_min_size_set"), Window::ui_min_size_set);
        add_function(s.get_property("ui_max_size_get"), Window::ui_max_size_get);
        add_function(s.get_property("ui_max_size_set"), Window::ui_max_size_set);
        add_function(s.get_property("ui_layout_get"), Window::ui_layout_get);
        add_function(s.get_property("ui_layout_set"), Window::ui_layout_set);
        add_function(s.get_property("ui_proportion_get"), Window::ui_proportion_get);
        add_function(s.get_property("ui_proportion_set"), Window::ui_proportion_set);
        add_function(s.get_property("ui_align_get"), Window::ui_align_get);
        add_function(s.get_property("ui_align_set"), Window::ui_align_set);
        add_function(s.get_property("ui_keep_ratio_get"), Window::ui_keep_ratio_get);
        add_function(s.get_property("ui_keep_ratio_set"), Window::ui_keep_ratio_set);
        add_function(s.get_property("ui_border_get"), Window::ui_border_get);
        add_function(s.get_property("ui_border_set"), Window::ui_border_set);
        add_function(s.get_property("ui_spacing_get"), Window::ui_spacing_get);
        add_function(s.get_property("ui_spacing_set"), Window::ui_spacing_set);
        add_function(s.get_property("ui_background_color_get"), Window::ui_background_color_get);
        add_function(s.get_property("ui_background_color_set"), Window::ui_background_color_set);
        add_function(s.get_property("ui_foreground_color_get"), Window::ui_foreground_color_get);
        add_function(s.get_property("ui_foreground_color_set"), Window::ui_foreground_color_set);

        add_function(s.get_property("ui_window_is"), Window::ui_is<wxTopLevelWindow>);
        add_function(s.get_property("ui_window_title_get"), Window::Window::ui_window_title_get);
        add_function(s.get_property("ui_window_title_set"), Window::Window::ui_window_title_set);
        add_event<true>(s.get_property("ui_window_closed"), wxEVT_CLOSE_WINDOW);

        add_function(s.get_property("ui_frame_is"), Window::ui_is<wxFrame>);
        add_function(s.get_property("ui_frame_new"), Window::ui_new<wxFrame>);
        add_function(s.get_property("ui_frame_create"), Window::Window::Frame::ui_frame_create);

        add_function(s.get_property("ui_dialog_is"), Window::ui_is<wxDialog>);
        add_function(s.get_property("ui_dialog_new"), Window::ui_new<wxDialog>);
        add_function(s.get_property("ui_dialog_create"), Window::Window::Dialog::ui_dialog_create);

        add_function(s.get_property("ui_panel_is"), Window::ui_is<wxPanel>);
        add_function(s.get_property("ui_panel_new"), Window::ui_new<wxPanel>);
        add_function(s.get_property("ui_panel_create"), Window::Panel::ui_panel_create);

        add_function(s.get_property("ui_ctrl_is"), Window::ui_is<wxControl>);
        add_function(s.get_property("ui_ctrl_label_get"), Window::Control::ui_ctrl_label_get);
        add_function(s.get_property("ui_ctrl_label_set"), Window::Control::ui_ctrl_label_set);

        add_function(s.get_property("ui_activityindicator_is"), Window::ui_is<wxActivityIndicator>);
        add_function(s.get_property("ui_activityindicator_new"), Window::ui_new<wxActivityIndicator>);
        add_function(s.get_property("ui_activityindicator_create"), Window::Control::ActivityIndicator::ui_activityindicator_create);
        add_function(s.get_property("ui_activityindicator_value_get"), Window::Control::ActivityIndicator::ui_activityindicator_value_get);
        add_function(s.get_property("ui_activityindicator_value_set"), Window::Control::ActivityIndicator::ui_activityindicator_value_set);

        add_function(s.get_property("ui_button_is"), Window::ui_is<wxButton>);
        add_function(s.get_property("ui_button_new"), Window::ui_new<wxButton>);
        add_function(s.get_property("ui_button_create"), Window::Control::Button::ui_button_create);
        add_event(s.get_property("ui_button_event"), wxEVT_BUTTON);

        add_function(s.get_property("ui_calendar_is"), Window::ui_is<wxCalendarCtrl>);
        add_function(s.get_property("ui_calendar_new"), Window::ui_new<wxCalendarCtrl>);
        add_function(s.get_property("ui_calendar_create"), Window::Control::Calendar::ui_calendar_create);
        add_function(s.get_property("ui_calendar_value_get"), Window::Control::Calendar::ui_calendar_value_get);
        add_function(s.get_property("ui_calendar_value_set"), Window::Control::Calendar::ui_calendar_value_set);
        add_event(s.get_property("ui_calendar_value_event"), wxEVT_CALENDAR_SEL_CHANGED);

        add_function(s.get_property("ui_checkbox_is"), Window::ui_is<wxCheckBox>);
        add_function(s.get_property("ui_checkbox_new"), Window::ui_new<wxCheckBox>);
        add_function(s.get_property("ui_checkbox_create"), Window::Control::Checkbox::ui_checkbox_create);
        add_function(s.get_property("ui_checkbox_value_get"), Window::Control::Checkbox::ui_checkbox_value_get);
        add_function(s.get_property("ui_checkbox_value_set"), Window::Control::Checkbox::ui_checkbox_value_set);
        add_event(s.get_property("ui_checkbox_value_event"), wxEVT_CHECKBOX);

        add_function(s.get_property("ui_choice_is"), Window::ui_is<wxChoice>);
        add_function(s.get_property("ui_choice_new"), Window::ui_new<wxChoice>);
        add_function(s.get_property("ui_choice_create"), Window::Control::Choice::ui_choice_create);
        add_function(s.get_property("ui_choice_options_get"), Window::Control::Choice::ui_choice_options_get);
        add_function(s.get_property("ui_choice_options_set"), Window::Control::Choice::ui_choice_options_set);
        add_function(s.get_property("ui_choice_value_get"), Window::Control::Choice::ui_choice_value_get);
        add_function(s.get_property("ui_choice_value_set"), Window::Control::Choice::ui_choice_value_set);
        add_event(s.get_property("ui_choice_value_event"), wxEVT_CHOICE);

        add_function(s.get_property("ui_colorpicker_is"), Window::ui_is<wxColourPickerCtrl>);
        add_function(s.get_property("ui_colorpicker_new"), Window::ui_new<wxColourPickerCtrl>);
        add_function(s.get_property("ui_colorpicker_create"), Window::Control::ColorPicker::ui_colorpicker_create);
        add_function(s.get_property("ui_colorpicker_value_get"), Window::Control::ColorPicker::ui_colorpicker_value_get);
        add_function(s.get_property("ui_colorpicker_value_set"), Window::Control::ColorPicker::ui_colorpicker_value_set);
        add_event(s.get_property("ui_colorpicker_value_event"), wxEVT_COLOURPICKER_CHANGED);

        add_function(s.get_property("ui_datepicker_is"), Window::ui_is<wxDatePickerCtrl>);
        add_function(s.get_property("ui_datepicker_new"), Window::ui_new<wxDatePickerCtrl>);
        add_function(s.get_property("ui_datepicker_create"), Window::Control::DatePicker::ui_datepicker_create);
        add_function(s.get_property("ui_datepicker_value_get"), Window::Control::DatePicker::ui_datepicker_value_get);
        add_function(s.get_property("ui_datepicker_value_set"), Window::Control::DatePicker::ui_datepicker_value_set);
        add_event(s.get_property("ui_datepicker_value_event"), wxEVT_DATE_CHANGED);

        add_function(s.get_property("ui_gauge_is"), Window::ui_is<wxGauge>);
        add_function(s.get_property("ui_gauge_new"), Window::ui_new<wxGauge>);
        add_function(s.get_property("ui_gauge_create"), Window::Control::Gauge::ui_gauge_create);
        add_function(s.get_property("ui_gauge_range_get"), Window::Control::Gauge::ui_gauge_range_get);
        add_function(s.get_property("ui_gauge_range_set"), Window::Control::Gauge::ui_gauge_range_set);
        add_function(s.get_property("ui_gauge_value_get"), Window::Control::Gauge::ui_gauge_value_get);
        add_function(s.get_property("ui_gauge_value_set"), Window::Control::Gauge::ui_gauge_value_set);

        add_function(s.get_property("ui_hyperlink_is"), Window::ui_is<wxHyperlinkCtrl>);
        add_function(s.get_property("ui_hyperlink_new"), Window::ui_new<wxHyperlinkCtrl>);
        add_function(s.get_property("ui_hyperlink_create"), Window::Control::Hyperlink::ui_hyperlink_create);
        add_function(s.get_property("ui_hyperlink_url_get"), Window::Control::Hyperlink::ui_hyperlink_url_get);
        add_function(s.get_property("ui_hyperlink_url_set"), Window::Control::Hyperlink::ui_hyperlink_url_set);

        add_function(s.get_property("ui_label_is"), Window::ui_is<wxStaticText>);
        add_function(s.get_property("ui_label_new"), Window::ui_new<wxStaticText>);
        add_function(s.get_property("ui_label_create"), Window::Control::Label::ui_label_create);

        add_function(s.get_property("ui_radiobutton_is"), Window::ui_is<wxRadioButton>);
        add_function(s.get_property("ui_radiobutton_new"), Window::ui_new<wxRadioButton>);
        add_function(s.get_property("ui_radiobutton_create"), Window::Control::RadioButton::ui_radiobutton_create);
        add_function(s.get_property("ui_radiobutton_value_get"), Window::Control::RadioButton::ui_radiobutton_value_get);
        add_function(s.get_property("ui_radiobutton_value_set"), Window::Control::RadioButton::ui_radiobutton_value_set);
        add_event(s.get_property("ui_radiobutton_value_event"), wxEVT_RADIOBUTTON);

        add_function(s.get_property("ui_text_is"), Window::ui_is<wxTextEntry>);
        add_function(s.get_property("ui_text_new"), Window::ui_new<wxTextCtrl>);
        add_function(s.get_property("ui_text_create"), Window::Control::Text::ui_text_create);
        add_function(s.get_property("ui_text_value_get"), Window::Control::Text::ui_text_value_get);
        add_function(s.get_property("ui_text_value_set"), Window::Control::Text::ui_text_value_set);
        add_event(s.get_property("ui_text_value_event"), wxEVT_TEXT);
        add_function(s.get_property("ui_text_editable_get"), Window::Control::Text::ui_text_editable_get);
        add_function(s.get_property("ui_text_editable_set"), Window::Control::Text::ui_text_editable_set);
        add_function(s.get_property("ui_text_value_set_silent"), Window::Control::Text::ui_text_value_set_silent);

        add_function(s.get_property("ui_multilinetext_is"), Window::Control::Text::MultilineText::ui_multiline_is);
        add_function(s.get_property("ui_multilinetext_new"), Window::ui_new<wxTextCtrl>);
        add_function(s.get_property("ui_multilinetext_create"), Window::Control::Text::MultilineText::ui_multilinetext_create);

        add_function(s.get_property("ui_search_is"), Window::ui_is<wxSearchCtrl>);
        add_function(s.get_property("ui_search_new"), Window::ui_new<wxSearchCtrl>);
        add_function(s.get_property("ui_search_create"), Window::Control::Text::Search::ui_search_create);
        add_event(s.get_property("ui_search_cancel_event"), wxEVT_SEARCH_CANCEL);

        add_function(s.get_property("ui_password_is"), Window::Control::Text::Password::ui_password_is);
        add_function(s.get_property("ui_password_new"), Window::ui_new<wxTextCtrl>);
        add_function(s.get_property("ui_password_create"), Window::Control::Text::Password::ui_password_create);

        add_function(s.get_property("ui_box_is"), Window::ui_is<wxStaticBox>);
        add_function(s.get_property("ui_box_new"), Window::ui_new<wxStaticBox>);
        add_function(s.get_property("ui_box_create"), Window::Control::Box::ui_box_create);

        add_function(s.get_property("ui_slider_is"), Window::ui_is<wxSlider>);
        add_function(s.get_property("ui_slider_new"), Window::ui_new<wxSlider>);
        add_function(s.get_property("ui_slider_create"), Window::Control::Slider::ui_slider_create);
        add_function(s.get_property("ui_slider_value_get"), Window::Control::Slider::ui_slider_value_get);
        add_function(s.get_property("ui_slider_value_set"), Window::Control::Slider::ui_slider_value_set);
        add_event(s.get_property("ui_slider_value_event"), wxEVT_SLIDER);
        add_function(s.get_property("ui_slider_min_get"), Window::Control::Slider::ui_slider_min_get);
        add_function(s.get_property("ui_slider_min_set"), Window::Control::Slider::ui_slider_min_set);
        add_function(s.get_property("ui_slider_max_get"), Window::Control::Slider::ui_slider_max_get);
        add_function(s.get_property("ui_slider_max_set"), Window::Control::Slider::ui_slider_max_set);

        add_function(s.get_property("ui_filepicker_is"), Window::ui_is<wxFilePickerCtrl>);
        add_function(s.get_property("ui_filepicker_new"), Window::ui_new<wxFilePickerCtrl>);
        add_function(s.get_property("ui_filepicker_create"), Window::Control::FilePicker::ui_filepicker_create);
        add_function(s.get_property("ui_filepicker_value_get"), Window::Control::FilePicker::ui_filepicker_value_get);
        add_function(s.get_property("ui_filepicker_value_set"), Window::Control::FilePicker::ui_filepicker_value_set);
        add_event(s.get_property("ui_filepicker_value_event"), wxEVT_FILEPICKER_CHANGED);

        add_function(s.get_property("ui_book_is"), Window::ui_is<wxBookCtrlBase>);
        add_function(s.get_property("ui_book_new"), Window::ui_new<wxSimplebook>);
        add_function(s.get_property("ui_book_create"), Window::Control::Book::ui_book_create);
        add_function(s.get_property("ui_book_reload"), Window::Control::Book::ui_reload);
        add_function(s.get_property("ui_book_value_get"), Window::Control::Book::ui_book_value_get);
        add_function(s.get_property("ui_book_value_set"), Window::Control::Book::ui_book_value_set);
        add_function(s.get_property("ui_book_get_text"), Window::Control::Book::ui_book_get_text);
        add_function(s.get_property("ui_book_set_text"), Window::Control::Book::ui_book_set_text);

        add_function(s.get_property("ui_notebook_is"), Window::ui_is<wxNotebook>);
        add_function(s.get_property("ui_notebook_new"), Window::ui_new<wxNotebook>);
        add_function(s.get_property("ui_notebook_create"), Window::Control::Book::Notebook::ui_notebook_create);

        add_function(s.get_property("ui_webview_is"), Window::ui_is<wxWebView>);
        add_function(s.get_property("ui_webview_new"), Window::Control::Webview::ui_webview_new);
        add_function(s.get_property("ui_webview_create"), Window::Control::Webview::ui_webview_create);
        add_function(s.get_property("ui_webview_title_get"), Window::Control::Webview::ui_webview_title_get);
        add_function(s.get_property("ui_webview_url_get"), Window::Control::Webview::ui_webview_url_get);
        add_function(s.get_property("ui_webview_url_set"), Window::Control::Webview::ui_webview_url_set);
        add_function(s.get_property("ui_webview_set_page"), Window::Control::Webview::ui_webview_set_page);
        add_event(s.get_property("ui_webview_url_event"), wxEVT_WEBVIEW_NAVIGATED);
    }

}


#else


namespace Interpreter::SystemFunctions {

    namespace UI {

        void init(GlobalContext& /*unused*/) {}

    }

}


#endif
