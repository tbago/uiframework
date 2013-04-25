#ifndef UIDEMOS_DEMO_LABEL_H_
#define UIDEMOS_DEMO_LABEL_H_

#include "views/controls/link_listener.h"
#include "demo_base.h"

namespace views
{
    class Label;
    class Separator;
}

class DemoLabel : public DemoBase, public views::LinkListener
{
public:
    explicit DemoLabel(DemoMain* main);
    virtual ~DemoLabel();

    // Overridden from DemoBase:
    virtual std::wstring GetDemoTitle();
    virtual void CreateDemoView(views::View* container);

    // Overridden from view::LinkListener:
    virtual void LinkClicked(views::Link* source, int event_flags);

private:
    views::Link* link_;
    views::Link* link_custom_;
    views::Link* link_disable_;
    views::Link* link_disable_custom_;

    views::Separator* separator_;

    views::Label* label_;
    views::Label* label_align_left_;
    views::Label* label_align_right_;
    views::Label* label_multi_line_;

    DISALLOW_COPY_AND_ASSIGN(DemoLabel);
};

#endif      //UIDEMOS_DEMO_LABEL_H_
