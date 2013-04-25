#include "demo_base.h"

#include "views/view.h"
#include "demo_main.h"

namespace
{
class ContainerView : public views::View
{
public:
    explicit ContainerView(DemoBase* base) : demo_view_created_(false),demo_base_(base) {}
protected: //views::View overrides:
    virtual void ViewHierarchyChanged(bool is_add, views::View* parent, views::View* child)
    {
        views::View::ViewHierarchyChanged(is_add, parent, child);
        if(is_add && GetWidget() && !demo_view_created_)
        {
            demo_view_created_ = true;
            demo_base_->CreateDemoView(this);
        }
    }

private:
    bool demo_view_created_;

    DemoBase* demo_base_;

    DISALLOW_COPY_AND_ASSIGN(ContainerView);
};

}   //namespace

DemoBase::DemoBase(DemoMain* main) : main_(main)
{
    container_ = new ContainerView(this);
}

void DemoBase::PrintStatus(const std::wstring& status)
{
    main_->SetStatus(status);
}
