#ifndef UIDEMOS_DEMO_MAIN_H_
#define UIDEMOS_DEMO_MAIN_H_

#include "views/controls/tabbed_pane/tabbed_pane_listener.h"
#include "views/widget/widget_delegate.h"

class StatusIcon;
class StatusTray;

namespace ui
{
    class MenuModel;
}

namespace views
{
    class Label;
    class View;
}

class DemoMain : public views::WidgetDelegate, public views::TabbedPaneListener
{
public:
    DemoMain();
    virtual ~DemoMain();

    void DisplayBalloon();
    void SetTrayIconContextMenu(ui::MenuModel* menu);

    // Overridden from view::WidgetDelegate:
    virtual bool CanResize() const;
    virtual bool CanMaximize() const;
    virtual std::wstring GetWindowTitle() const;
    virtual views::View* GetContentsView();
    virtual void WindowClosing();
    virtual views::Widget* GetWidget();
    virtual const views::Widget* GetWidget() const;

    // Overridden from view::TabbedPaneListener:
    virtual void TabSelectedAt(int index);

    void SetStatus(const std::wstring& status);
    void Run();

private:
    views::View* contents_;

    views::Label* status_label_;

    DISALLOW_COPY_AND_ASSIGN(DemoMain);
};

#endif      //UIDEMOS_DEMO_MAIN_H_
