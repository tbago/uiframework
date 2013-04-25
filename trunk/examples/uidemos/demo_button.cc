#include "demo_button.h"

#include "base/utf_string_conversions.h"

#include "gfx/image/image.h"

#include "uibase/models/simple_menu_model.h"
#include "uibase/resource/resource_bundle.h"

#include "views/controls/button/button_dropdown.h"
#include "views/controls/button/checkbox.h"
#include "views/controls/button/image_button.h"
#include "views/controls/button/menu_button.h"
#include "views/controls/button/radio_button.h"
#include "views/controls/button/text_button.h"
#include "views/controls/menu/menu_2.h"
#include "views/controls/menu/menu_delegate.h"
#include "views/layout/box_layout.h"

#include "demo_main.h"

#include "grit/ui_resources.h"

namespace
{
static const int kTextButtonText = 0;
static const int kTextButtonIcon = 1;
static const int kButtonDropDown = 2;

class DemoMenuModel : public ui::SimpleMenuModel, public ui::SimpleMenuModel::Delegate
{
public:
    DemoMenuModel();

    void RunMenuAt(const gfx::Point& point);

    // Overridden from ui::SimpleMenuModel::Delegate:
    virtual bool IsCommandIdChecked(int command_id) const;
    virtual bool IsCommandIdEnabled(int command_id) const;
    virtual bool GetAcceleratorForCommandId(int command_id,
                                            ui::Accelerator* accelerator);
    virtual void ExecuteCommand(int command_id);

private:
    enum
    {
        kGroupMakeDecision,
    };

    enum
    {
        kCommandDoSomething,
        kCommandSelectAscii,
        kCommandSelectUtf8,
        kCommandSelectUtf16,
        kCommandCheckApple,
        kCommandCheckOrange,
        kCommandCheckKiwi,
        kCommandGoHome,
    };

    scoped_ptr<views::Menu2> menu_;
    scoped_ptr<ui::SimpleMenuModel> submenu_;
    std::set<int> checked_fruits_;
    int current_encoding_command_id_;

    DISALLOW_COPY_AND_ASSIGN(DemoMenuModel);
};

// DemoMenuModel ---------------------------------------------------------
DemoMenuModel::DemoMenuModel() : ui::SimpleMenuModel(this),
    current_encoding_command_id_(kCommandSelectAscii)
{
    AddItem(kCommandDoSomething, WideToUTF16(L"Do Noting"));
    AddSeparator(ui::NORMAL_SEPARATOR);
    AddRadioItem(kCommandSelectAscii,
        WideToUTF16(L"ASCII"), kGroupMakeDecision);
    AddRadioItem(kCommandSelectUtf8,
        WideToUTF16(L"UTF-8"), kGroupMakeDecision);
    AddRadioItem(kCommandSelectUtf16,
        WideToUTF16(L"UTF-16"), kGroupMakeDecision);
    AddSeparator(ui::NORMAL_SEPARATOR);
    AddCheckItem(kCommandCheckApple, WideToUTF16(L"Apple"));
    AddCheckItem(kCommandCheckOrange, WideToUTF16(L"Orange"));
    AddCheckItem(kCommandCheckKiwi, WideToUTF16(L"Kiwi"));
    AddSeparator(ui::NORMAL_SEPARATOR);
    AddItem(kCommandGoHome, WideToUTF16(L"Go Home"));

    submenu_.reset(new ui::SimpleMenuModel(this));
    submenu_->AddItem(kCommandDoSomething, WideToUTF16(L"Do Something 2"));
    AddSubMenu(0, ASCIIToUTF16("Submenu"), submenu_.get());
    menu_.reset(new views::Menu2(this));
}

void DemoMenuModel::RunMenuAt(const gfx::Point& point)
{
    menu_->RunMenuAt(point, views::Menu2::ALIGN_TOPRIGHT);
}

bool DemoMenuModel::IsCommandIdChecked(int command_id) const
{
    // Radio items.
    if(command_id == current_encoding_command_id_)
    {
        return true;
    }

    // Check items.
    if(checked_fruits_.find(command_id) != checked_fruits_.end())
    {
        return true;
    }

    return false;
}

bool DemoMenuModel::IsCommandIdEnabled(int command_id) const
{
    // All commands are enabled except for kCommandGoHome.
    return command_id != kCommandGoHome;
}

bool DemoMenuModel::GetAcceleratorForCommandId(
    int command_id, ui::Accelerator* accelerator)
{
    // We don't use this in the example.
    return false;
}

void DemoMenuModel::ExecuteCommand(int command_id)
{
    switch(command_id)
    {
    case kCommandDoSomething:
        {
            LOG(INFO) << "Done something";
            break;
        }

        // Radio items.
    case kCommandSelectAscii:
        {
            current_encoding_command_id_ = kCommandSelectAscii;
            LOG(INFO) << "Selected ASCII";
            break;
        }
    case kCommandSelectUtf8:
        {
            current_encoding_command_id_ = kCommandSelectUtf8;
            LOG(INFO) << "Selected UTF-8";
            break;
        }
    case kCommandSelectUtf16:
        {
            current_encoding_command_id_ = kCommandSelectUtf16;
            LOG(INFO) << "Selected UTF-16";
            break;
        }

        // Check items.
    case kCommandCheckApple:
    case kCommandCheckOrange:
    case kCommandCheckKiwi:
        {
            // Print what fruit is checked.
            const char* checked_fruit = "";
            if(command_id == kCommandCheckApple)
            {
                checked_fruit = "Apple";
            }
            else if(command_id == kCommandCheckOrange)
            {
                checked_fruit = "Orange";
            }
            else if(command_id == kCommandCheckKiwi)
            {
                checked_fruit = "Kiwi";
            }
            LOG(INFO) << "Checked " << checked_fruit;

            // Update the check status.
            std::set<int>::iterator iter = checked_fruits_.find(command_id);
            if(iter == checked_fruits_.end())
            {
                checked_fruits_.insert(command_id);
            }
            else
            {
                checked_fruits_.erase(iter);
            }
            break;
        }
    }
}


class DemoMenuButton : public views::MenuButton, public views::MenuDelegate
{
public:
    DemoMenuButton(const std::wstring& test, bool show_menu_marker);
    virtual ~DemoMenuButton();

private:
    // Overridden from views::ViewMenuDelegate:
    virtual void RunMenu(views::View* source, const gfx::Point& point);

    scoped_ptr<DemoMenuModel> menu_model_;
    DISALLOW_COPY_AND_ASSIGN(DemoMenuButton);
};

// DemoMenuButton -----------------------------------------------------------

DemoMenuButton::DemoMenuButton(const std::wstring& test, bool show_menu_marker)
    : views::MenuButton(NULL, test, NULL, show_menu_marker)
{

}

DemoMenuButton::~DemoMenuButton() {}

void DemoMenuButton::RunMenu(views::View* source, const gfx::Point& point)
{
    if(menu_model_ == NULL)
    {
        menu_model_.reset(new DemoMenuModel());
    }
    menu_model_->RunMenuAt(point);
}

}

DemoButton::DemoButton(DemoMain* main) : DemoBase(main) {}

DemoButton::~DemoButton() {}

std::wstring DemoButton::GetDemoTitle()
{
    return std::wstring(L"Button");
}

void DemoButton::CreateDemoView(views::View* container)
{
    checkbox_ = new views::Checkbox(L"Checkbox");
    checkbox_->set_listener(this);
    checkbox_->SetChecked(true);
    checkbox_->SetFont(ui::ResourceBundle::GetSharedInstance().GetFont(
        ui::ResourceBundle::BoldFont));

    views::View* radio_container = new views::View();
    radio_container->SetLayoutManager(new views::BoxLayout(
        views::BoxLayout::kHorizontal, 0, 0, 10));
    radio1_ = new views::RadioButton(L"猫", 1);
    radio1_->set_listener(this);
    radio2_ = new views::RadioButton(L"猪", 1);
    radio2_->set_listener(this);
    radio3_ = new views::RadioButton(L"狗", 1);
    radio3_->set_listener(this);
    radio3_->SetChecked(true);
    radio3_->SetFont(ui::ResourceBundle::GetSharedInstance().GetFont(
        ui::ResourceBundle::BoldFont));
    radio_container->AddChildView(radio1_);
    radio_container->AddChildView(radio2_);
    radio_container->AddChildView(radio3_);

    //views::View* text_button_container = new views::View();
    //text_button_container->SetLayoutManager(new views::BoxLayout(
    //    views::BoxLayout::kHorizontal, 0, 0, 10));
    //{
    //    views::TextButton* text_button = new views::TextButton(this, L"文本按钮\n换行");
    //    text_button->set_tag(kTextButtonText);
    //    text_button->SetTextShadowColors(SK_ColorLTGRAY, SK_ColorDKGRAY);
    //    text_button->SetTextShadowOffset(1, -1);
    //    text_button->set_focusable(true);
    //    text_button_container->AddChildView(text_button);
    //}
    //const gfx::ImageSkia *image_skia;
    //{
    //    views::TextButton* text_button = new views::TextButton(this, L"(&I)图标按钮");
    //    image_skia = ui::ResourceBundle::GetSharedInstance().GetImageNamed(IDR_FOLDER_CLOSED).ToImageSkia();
    //    text_button->SetIcon(*image_skia);
    //    image_skia = ui::ResourceBundle::GetSharedInstance().GetImageNamed(IDR_FOLDER_OPEN).ToImageSkia();
    //    text_button->SetHoverIcon(*image_skia);
    //    image_skia = ui::ResourceBundle::GetSharedInstance().GetImageNamed(IDR_FOLDER_CLOSED).ToImageSkia();
    //    text_button->SetPushedIcon(*image_skia);
    //    text_button->set_tag(kTextButtonIcon);
    //    text_button->set_icon_text_spacing(10);
    //    text_button->SetFont(ui::ResourceBundle::GetSharedInstance().GetFont(
    //        ui::ResourceBundle::BoldFont));
    //    text_button->set_focusable(true);
    //    text_button->SetEnabledColor(SK_ColorRED);
    //    text_button_container->AddChildView(text_button);
    //}

    //views::View* image_button_container = new views::View();
    //image_button_container->SetLayoutManager(new views::BoxLayout(
    //    views::BoxLayout::kHorizontal, 0, 0, 0));
    //{
    //    views::ImageButton* image_button = new views::ImageButton(NULL);
    //    image_skia = ui::ResourceBundle::GetSharedInstance().GetImageNamed(IDR_MINIMIZE).ToImageSkia();
    //    image_button->SetImage(views::CustomButton::STATE_NORMAL, image_skia);
    //    image_skia = ui::ResourceBundle::GetSharedInstance().GetImageNamed(IDR_MINIMIZE_H).ToImageSkia();
    //    image_button->SetImage(views::CustomButton::STATE_HOVERED, image_skia);
    //    image_skia = ui::ResourceBundle::GetSharedInstance().GetImageNamed(IDR_MINIMIZE_P).ToImageSkia();
    //    image_button->SetImage(views::CustomButton::STATE_PRESSED, image_skia);
    //    image_button_container->AddChildView(image_button);
    //}
    //{
    //    views::ImageButton* image_button = new views::ImageButton(NULL);
    //    image_skia = ui::ResourceBundle::GetSharedInstance().GetImageNamed(IDR_MAXIMIZE).ToImageSkia();
    //    image_button->SetImage(views::CustomButton::STATE_NORMAL, image_skia);
    //    image_skia = ui::ResourceBundle::GetSharedInstance().GetImageNamed(IDR_MAXIMIZE_H).ToImageSkia();
    //    image_button->SetImage(views::CustomButton::STATE_HOVERED, image_skia);
    //    image_skia = ui::ResourceBundle::GetSharedInstance().GetImageNamed(IDR_MAXIMIZE_P).ToImageSkia();
    //    image_button->SetImage(views::CustomButton::STATE_PRESSED, image_skia);
    //    image_button_container->AddChildView(image_button);
    //}
    //{
    //    views::ImageButton* image_button = new views::ImageButton(NULL);
    //    image_skia = ui::ResourceBundle::GetSharedInstance().GetImageNamed(IDR_CLOSE).ToImageSkia();
    //    image_button->SetImage(views::CustomButton::STATE_NORMAL, image_skia);
    //    image_skia = ui::ResourceBundle::GetSharedInstance().GetImageNamed(IDR_CLOSE_H).ToImageSkia();
    //    image_button->SetImage(views::CustomButton::STATE_HOVERED, image_skia);
    //    image_skia = ui::ResourceBundle::GetSharedInstance().GetImageNamed(IDR_CLOSE_P).ToImageSkia();
    //    image_button->SetImage(views::CustomButton::STATE_PRESSED, image_skia);
    //    image_button_container->AddChildView(image_button);
    //}

    //views::View* menu_button_container = new views::View();
    //menu_button_container->SetLayoutManager(new views::BoxLayout(
    //    views::BoxLayout::kHorizontal, 0, 0, 0));
    //{
    //    DemoMenuButton* menu_button = new DemoMenuButton(L"MenuButton", true);
    //    menu_button->set_focusable(true);
    //    menu_button_container->AddChildView(menu_button);
    //}
    //{
    //    static DemoMenuModel meun_model;
    //    views::ButtonDropDown* button_dropdown =
    //        new views::ButtonDropDown(this, &meun_model);
    //    image_skia = ui::ResourceBundle::GetSharedInstance().GetImageNamed(IDR_CLOSE).ToImageSkia();
    //    button_dropdown->SetImage(views::CustomButton::STATE_NORMAL, image_skia);
    //    image_skia = ui::ResourceBundle::GetSharedInstance().GetImageNamed(IDR_CLOSE_H).ToImageSkia();
    //    button_dropdown->SetImage(views::CustomButton::STATE_HOVERED, image_skia);
    //    image_skia = ui::ResourceBundle::GetSharedInstance().GetImageNamed(IDR_CLOSE_P).ToImageSkia();
    //    button_dropdown->SetImage(views::CustomButton::STATE_PRESSED, image_skia);
    //    button_dropdown->set_tag(kButtonDropDown);
    //    menu_button_container->AddChildView(button_dropdown);
    //}

    views::LayoutManager *layout_manager = new views::BoxLayout(views::BoxLayout::kVertical, 5, 5 ,5);
    container->SetLayoutManager(layout_manager);

    container->AddChildView(checkbox_);
    container->AddChildView(radio_container);
    //container->AddChildView(text_button_container);
    //container->AddChildView(image_button_container);
    //container->AddChildView(menu_button_container);

    demo_main()->SetTrayIconContextMenu(new DemoMenuModel());
}

void DemoButton::ButtonPressed(views::Button* sender, const ui::Event& event)
{
    if(checkbox_ == sender)
    {
        PrintStatus(L"Checkbox clicked");
    }
    else if(radio1_==sender || radio2_==sender || radio3_==sender)
    {
        PrintStatus(static_cast<views::RadioButton*>(sender)->text()
            + L" clicked");
    }
    else
    {
        switch(sender->tag())
        {
        case kTextButtonText:
            PrintStatus(static_cast<views::TextButton*>(sender)->text()
                + L" clicked");
            break;
        case kTextButtonIcon:
            PrintStatus(L"Icon TextButton clicked");
            {
                views::TextButton* text_button =
                    static_cast<views::TextButton*>(sender);
                if(text_button->icon_placement() == views::TextButton::ICON_ON_LEFT)
                {
                    text_button->set_icon_placement(views::TextButton::ICON_ON_RIGHT);
                }
                else
                {
                    text_button->set_icon_placement(views::TextButton::ICON_ON_LEFT);
                }
            }
            break;
        case kButtonDropDown:
            PrintStatus(L"ButtonDropDown clicked");
            break;
        }
    }
}
