#ifndef UIDEMOS_DEMO_BUTTON_H_
#define UIDEMOS_DEMO_BUTTON_H_

#include "views/controls/button/button.h"

#include "demo_base.h"

namespace views
{
    class Checkbox;
    class NativeCheckbox;
    class NativeRadioButton;
    class RadioButton;
}

class DemoButton : public DemoBase, public views::ButtonListener
{
public:
    explicit DemoButton(DemoMain* main);
    virtual ~DemoButton();

    // Overridden from DemoBase:
    virtual std::wstring GetDemoTitle();
    virtual void CreateDemoView(views::View* container);

    // Overridden from view::ButtonListener:
    virtual void ButtonPressed(views::Button* sender, const ui::Event& event);

private:
    views::Checkbox* checkbox_;

    views::RadioButton* radio1_;
    views::RadioButton* radio2_;
    views::RadioButton* radio3_;

    DISALLOW_COPY_AND_ASSIGN(DemoButton);
};
#endif      //UIDEMOS_DEMO_BUTTON_H_
