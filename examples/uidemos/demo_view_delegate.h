#ifndef UIDEMOS_DEMOVIEW_DELEGATE_H_
#define UIDEMOS_DEMOVIEW_DELEGATE_H_

#include "base/memory/scoped_ptr.h"
#include "views/views_delegate.h"

class DemoViewDelegate : public views::ViewsDelegate
{
public: // Override from view::ViewDelegate
    virtual void SaveWindowPlacement(const views::Widget* widget, const std::string& window_name,
                                    const gfx::Rect& bounds, ui::WindowShowState show_state);
    virtual bool GetSavedWindowPlacement(const std::string& window_name, gfx::Rect* bounds,
                                        ui::WindowShowState* show_state) const;
    virtual void NotifyAccessibilityEvent(views::View* view, ui::AccessibilityTypes::Event event_type);
    virtual void NotifyMenuItemFocused(const string16& menu_name, const string16& menu_item_name,
                                        int item_index, int item_count, bool has_submenu);
    virtual HICON GetDefaultWindowIcon() const;
    virtual views::NonClientFrameView* CreateDefaultNonClientFrameView(views::Widget* widget);
    virtual bool UseTransparentWindows() const;
    virtual void AddRef();
    virtual void ReleaseRef();
    virtual int GetDispositionForEvent(int event_flags);
    virtual void OnBeforeWidgetInit(views::Widget::InitParams* params, views::internal::NativeWidgetDelegate* delegate);

    virtual views::View* GetDefaultParentView();
public:
    void set_default_parent_view(views::View* view) {default_parent_view_ = view;}
public:
    DemoViewDelegate();
    virtual ~DemoViewDelegate();
private:
    views::View* default_parent_view_;

    DISALLOW_COPY_AND_ASSIGN(DemoViewDelegate);
};

#endif      //UIDEMOS_DEMOVIEW_DELEGATE_H_
