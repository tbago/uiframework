#ifndef UIDEMOS_DEMO_BASE_H_
#define UIDEMOS_DEMO_BASE_H_

#include "base/basictypes.h"

#include <string>

namespace views
{
    class View;
}

class DemoMain;

class DemoBase
{
public:
    views::View* GetDemoView() { return container_; }

    virtual std::wstring GetDemoTitle() = 0;
    virtual void CreateDemoView(views::View* parent) = 0;

protected:
    explicit DemoBase(DemoMain* main);
    virtual ~DemoBase() {}

    void PrintStatus(const std::wstring& status);

    DemoMain* demo_main() const { return main_; }

private:
    DemoMain* main_;

    views::View* container_;

    DISALLOW_COPY_AND_ASSIGN(DemoBase);
};

#endif UIDEMOS_DEMO_BASE_H_
