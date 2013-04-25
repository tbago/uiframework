
#include "demo_view_delegate.h"

#include "base/logging.h"
#include "gfx/icon_util.h"
#include "gfx/image/image.h"
#include "uibase/clipboard/clipboard.h"
#include "uibase/resource/resource_bundle.h"

#include "grit/ui_resources.h"

void DemoViewDelegate::SaveWindowPlacement( const views::Widget* widget, const std::string& window_name, const gfx::Rect& bounds, ui::WindowShowState show_state )
{

}

bool DemoViewDelegate::GetSavedWindowPlacement( const std::string& window_name, gfx::Rect* bounds, ui::WindowShowState* show_state ) const
{
    return false;
}

void DemoViewDelegate::NotifyAccessibilityEvent( views::View* view, ui::AccessibilityTypes::Event event_type )
{

}

void DemoViewDelegate::NotifyMenuItemFocused( const string16& menu_name, const string16& menu_item_name, int item_index, int item_count, bool has_submenu )
{

}

HICON DemoViewDelegate::GetDefaultWindowIcon() const
{
    const SkBitmap *sk_bmp = ui::ResourceBundle::GetSharedInstance().GetImageNamed(IDR_PRODUCT_LOGO_16).ToSkBitmap();
    return IconUtil::CreateHICONFromSkBitmap(*sk_bmp);
}

views::NonClientFrameView* DemoViewDelegate::CreateDefaultNonClientFrameView( views::Widget* widget )
{
    return NULL;
}

bool DemoViewDelegate::UseTransparentWindows() const
{
    return false;
}

void DemoViewDelegate::AddRef()
{

}

void DemoViewDelegate::ReleaseRef()
{

}

int DemoViewDelegate::GetDispositionForEvent(int event_flags)
{
    return 0;
}

void DemoViewDelegate::OnBeforeWidgetInit( views::Widget::InitParams* params, views::internal::NativeWidgetDelegate* delegate )
{

}

DemoViewDelegate::DemoViewDelegate() : default_parent_view_(NULL)
{
    DCHECK(!views::ViewsDelegate::views_delegate);
    views::ViewsDelegate::views_delegate = this;
}

DemoViewDelegate::~DemoViewDelegate()
{
    views::ViewsDelegate::views_delegate = NULL;
}

views::View* DemoViewDelegate::GetDefaultParentView()
{
    return default_parent_view_;
}
