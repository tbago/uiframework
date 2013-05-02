
#include <tchar.h>
#include <windows.h>
#include <initguid.h>
#include <oleacc.h>

#include <atlbase.h>

#include "base/basictypes.h"
#include "base/compiler_specific.h"
#include "base/command_line.h"
#include "base/utf_string_conversions.h"
#include "base/path_service.h"
#include "base/string_util.h"
#include "base/at_exit.h"
#include "base/run_loop.h"
#include "gfx/canvas.h"
#include "views/controls/label.h"
#include "views/view.h"
#include "views/widget/widget.h"
#include "views/widget/widget_delegate.h"
#include "uibase/resource/resource_bundle.h"
#include "uibase/resource/resource_bundle_win.h"

class WindowView : public views::WidgetDelegateView {
public:
    WindowView() : label_(NULL) {
        Init();
    }

    virtual ~WindowView() {}

private:
    // Overridden from views::View:
    virtual void OnPaint(gfx::Canvas* canvas) OVERRIDE {
        canvas->FillRect(GetLocalBounds(), SK_ColorWHITE);
    }
    virtual void Layout() OVERRIDE {
        gfx::Size ps = label_->GetPreferredSize();
        label_->SetBounds((width() - ps.width()) / 2, (height() - ps.height()) / 2, ps.width(), ps.height());
    }
    virtual gfx::Size GetPreferredSize() OVERRIDE {
        gfx::Size ps = label_->GetPreferredSize();
        ps.set_width(ps.width() + 200);
        ps.set_height(ps.height() + 200);
        return ps;
    }

    // Overridden from views::WidgetDelegate:
    virtual string16 GetWindowTitle() const OVERRIDE{
        return ASCIIToUTF16("Hello World Window");
    }
    virtual bool CanResize() const OVERRIDE {
        return true;
    }
    virtual bool CanMaximize() const OVERRIDE {
        return false;
    }
    virtual views::View* GetContentsView() OVERRIDE {
        return this;
    }
    virtual void WindowClosing()
    {
        MessageLoopForUI::current()->Quit();
    }
    void Init() {
        label_ = new views::Label(ASCIIToUTF16("Hello, World!"));
        AddChildView(label_);
    }

    views::Label* label_;

    DISALLOW_COPY_AND_ASSIGN(WindowView);
};

//CComModule _Module;

// 程序入口.
int APIENTRY _tWinMain(HINSTANCE hInstance,
                       HINSTANCE hPrevInstance,
                       LPTSTR    lpCmdLine,
                       int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    HRESULT hRes = OleInitialize(NULL);

    base::AtExitManager exit_manager;

    CommandLine::Init(0, NULL);

    std::wstring wstr_path = L"e:\\log.log";
    bool res = logging::InitLogging(wstr_path.c_str(),
        logging::LOG_ONLY_TO_FILE,
        logging::DONT_LOCK_LOG_FILE,
        logging::DELETE_OLD_LOG_FILE,
        logging::ENABLE_DCHECK_FOR_NON_OFFICIAL_RELEASE_BUILDS);

    // this resolves ATL window thunking problem when Microsoft Layer
    // for Unicode (MSLU) is used.
//    ::DefWindowProc(NULL, 0, 0, 0L);
//    _Module.Init(NULL, hInstance);

    FilePath res_path;
    PathService::Get(base::DIR_EXE, &res_path);
    res_path = res_path.Append(L"default_skin.dll");

    //HINSTANCE hinst = ::LoadLibraryW(res_path.value().c_str());

    //ui::SetResourcesDataDLL(hinst);

    ui::ResourceBundle::InitSharedInstanceWithLocale(UTF16ToASCII(res_path.value()), NULL);

    MessageLoop main_message_loop(MessageLoop::TYPE_UI);

    views::Widget::CreateWindow(new WindowView)->Show();

    base::RunLoop run_loop;
    run_loop.Run();

//    _Module.Term();
    OleUninitialize();
    return 0;
}


// 提升公共控件样式.
#ifdef _UNICODE
#if defined _M_IX86
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='x86' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_IA64
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='ia64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_X64
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='amd64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#else
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#endif
#endif
