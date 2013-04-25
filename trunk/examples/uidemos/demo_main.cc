#include "demo_main.h"

#include "base/at_exit.h"
#include "base/message_loop.h"
#include "base/path_service.h"
#include "base/process_util.h"
#include "base/stringprintf.h"
#include "base/string_util.h"
#include "base/run_loop.h"
#include "gfx/image/image.h"
#include "uibase/resource/resource_bundle.h"
#include "views/controls/label.h"
#include "views/controls/tabbed_pane/tabbed_pane.h"
#include "views/focus/accelerator_handler.h"
#include "views/layout/grid_layout.h"
#include "views/widget/widget.h"
#include "uibase/resource/resource_bundle_win.h"
#include "uibase/resource/resource_data_dll_win.h"
#include "grit/ui_resources.h"

#include "demo_label.h"
#include "demo_button.h"


DemoMain::DemoMain() : contents_(NULL), status_label_(NULL)
{

}

DemoMain::~DemoMain()
{

}


void DemoMain::SetTrayIconContextMenu(ui::MenuModel* menu)
{

}

bool DemoMain::CanResize() const
{
    return true;
}

std::wstring DemoMain::GetWindowTitle() const
{
    return std::wstring(L"View Demo");
}

views::View* DemoMain::GetContentsView()
{
    return contents_;
}

void DemoMain::WindowClosing()
{
    MessageLoopForUI::current()->Quit();
}

views::Widget* DemoMain::GetWidget()
{
    return contents_->GetWidget();
}

const views::Widget* DemoMain::GetWidget() const
{
    return contents_->GetWidget();
}

void DemoMain::TabSelectedAt(int index)
{
    SetStatus(base::StringPrintf(L"Select tab: %d", index));
}

void DemoMain::SetStatus(const std::wstring& status)
{
    status_label_->SetText(status);
}

void DemoMain::Run()
{
    base::EnableTerminationOnHeapCorruption();

    base::AtExitManager exit_manager;

    FilePath res_path;
    PathService::Get(base::DIR_EXE, &res_path);
    res_path = res_path.Append(L"default_skin.dll");

    HINSTANCE hinst = ::LoadLibraryW(res_path.value().c_str());

    ui::SetResourcesDataDLL(hinst);

    ui::ResourceBundle::InitSharedInstanceWithLocale(UTF16ToASCII(res_path.value()), NULL);

    ui::ResourceBundle &res_bundle = ui::ResourceBundle::GetSharedInstance();

    MessageLoop main_message_loop(MessageLoop::TYPE_UI);

    DCHECK(contents_ == NULL) << "Run called more than once.";
    contents_ = new views::View();
    contents_->set_background(views::Background::CreateStandardPanelBackground());
    views::GridLayout* layout = new views::GridLayout(contents_);
    contents_->SetLayoutManager(layout);

    views::ColumnSet* column_set = layout->AddColumnSet(0);
    column_set->AddColumn(views::GridLayout::FILL, views::GridLayout::FILL, 1,
        views::GridLayout::USE_PREF, 0, 0);

    views::TabbedPane* tabbed_pane = new views::TabbedPane();
    tabbed_pane->set_listener(this);
    status_label_ = new views::Label();

    layout->StartRow(1, 0);
    layout->AddView(tabbed_pane);
    layout->StartRow(0, 0);
    layout->AddView(status_label_);

    views::Widget* window = views::Widget::CreateWindowWithBounds(this,
        gfx::Rect(100, 100, 800, 400));

    DemoLabel   demo_label(this);
    DemoButton  demo_button(this);
    tabbed_pane->AddTab(demo_label.GetDemoTitle(), demo_label.GetDemoView());
    tabbed_pane->AddTab(demo_button.GetDemoTitle(), demo_button.GetDemoView());

    window->Show();
    views::AcceleratorHandler accelerator_handler;

    base::RunLoop run_loop;
    run_loop.Run();

    ui::ResourceBundle::CleanupSharedInstance();
}
