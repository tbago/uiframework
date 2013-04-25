#include <stdio.h>
#include "base/command_line.h"
#include "base/logging.h"
#include "uibase/accessibility/accessible_view_state.h"
#include <string>
#include "base/memory/scoped_ptr.h"
#include "base/auto_reset.h"
#include "base/string_util.h"
#include "base/file_path.h"
#include "base/base_paths_win.h"
#include "base/path_service.h"
#include "base/at_exit.h"
#include "base/lazy_instance.h"
#include "base/memory/ref_counted.h"

using namespace logging;

class Foo
{
public:
    Foo() {printf("constructor\n");}
    virtual ~Foo() {printf("destructor\n");}
    void fun() {printf("call fun\n");}
};

class MyFoo : public base::RefCounted<MyFoo>
{
public:
    MyFoo() {printf("create my foo\n");}
private:
    virtual ~MyFoo() {printf("destroy my foo\n");}
    friend class base::RefCounted<MyFoo>;
};

int main(int argv, char **argc)
{
    printf("*****************begin base test**************\n\n");
    base::AtExitManager exit_manager;

    CommandLine::Init(argv, argc);
    std::wstring wstr_path = L"e:\\log.log";
    bool res = logging::InitLogging(wstr_path.c_str(),
        logging::LOG_ONLY_TO_FILE,
        logging::DONT_LOCK_LOG_FILE,
        logging::DELETE_OLD_LOG_FILE,
        logging::ENABLE_DCHECK_FOR_NON_OFFICIAL_RELEASE_BUILDS);

    LOG(INFO) << "Found " << 1 << " cookies";

    LOG_IF(INFO, true) << "Got lots of cookies";
    {
        scoped_ptr<Foo> sp(new Foo());
    }
    int org_value= 100;
    {
        base::AutoReset<int> reset_int(&org_value, 200);
        printf("org_value:%d\n", org_value);
    }

    base::LazyInstance<Foo> lazy_foo = LAZY_INSTANCE_INITIALIZER;
    FilePath test_path;
    PathService::Get(base::DIR_TEMP, &test_path);
    printf("windows path:%s\n", UTF16ToASCII(test_path.value()).c_str());

    lazy_foo.Get().fun();
    {
        scoped_refptr<MyFoo> foo_test = new MyFoo();
    }
    printf("\n*********end base test*********\n");
}
