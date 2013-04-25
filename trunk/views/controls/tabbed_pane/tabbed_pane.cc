// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "views/controls/tabbed_pane/tabbed_pane.h"

#include "base/logging.h"
#include "uibase/accessibility/accessible_view_state.h"
#include "uibase/keycodes/keyboard_codes.h"
#include "views/controls/native/native_view_host.h"
#include "views/controls/tabbed_pane/native_tabbed_pane_views.h"
#include "views/controls/tabbed_pane/native_tabbed_pane_wrapper.h"
#include "views/controls/tabbed_pane/tabbed_pane_listener.h"
#include "views/widget/widget.h"

// TODO(markusheintz): This should be removed once the native Windows tabbed
// pane is not used anymore (http://crbug.com/138059).
#if defined(OS_WIN) && !defined(USE_AURA)
#include "views/controls/tabbed_pane/native_tabbed_pane_win.h"
#endif

namespace {

views::NativeTabbedPaneWrapper* CreateNativeWrapper(
    views::TabbedPane* tabbed_pane) {
#if defined(OS_WIN) && !defined(USE_AURA)
  if (tabbed_pane->use_native_win_control())
    return new views::NativeTabbedPaneWin(tabbed_pane);
#endif
  return new views::NativeTabbedPaneViews(tabbed_pane);
}

}  // namespace

namespace views {

// static
const char TabbedPane::kViewClassName[] = "views/TabbedPane";

TabbedPane::TabbedPane()
  : native_tabbed_pane_(NULL),
#if defined(OS_WIN) && !defined(USE_AURA)
    use_native_win_control_(false),
#endif
    listener_(NULL) {
  set_focusable(true);
}

TabbedPane::~TabbedPane() {
}

int TabbedPane::GetTabCount() {
  return native_tabbed_pane_->GetTabCount();
}

int TabbedPane::GetSelectedTabIndex() {
  return native_tabbed_pane_->GetSelectedTabIndex();
}

View* TabbedPane::GetSelectedTab() {
  return native_tabbed_pane_->GetSelectedTab();
}

void TabbedPane::AddTab(const string16& title, View* contents) {
  native_tabbed_pane_->AddTab(title, contents);
  PreferredSizeChanged();
}

void TabbedPane::AddTabAtIndex(int index,
                               const string16& title,
                               View* contents,
                               bool select_if_first_tab) {
  native_tabbed_pane_->AddTabAtIndex(index, title, contents,
                                     select_if_first_tab);
  PreferredSizeChanged();
}

View* TabbedPane::RemoveTabAtIndex(int index) {
  View* tab = native_tabbed_pane_->RemoveTabAtIndex(index);
  PreferredSizeChanged();
  return tab;
}

void TabbedPane::SelectTabAt(int index) {
  native_tabbed_pane_->SelectTabAt(index);
}

void TabbedPane::SetAccessibleName(const string16& name) {
  accessible_name_ = name;
}

gfx::Size TabbedPane::GetPreferredSize() {
  return native_tabbed_pane_ ?
      native_tabbed_pane_->GetPreferredSize() : gfx::Size();
}

void TabbedPane::LoadAccelerators() {
  // Ctrl+Shift+Tab
  AddAccelerator(ui::Accelerator(ui::VKEY_TAB,
                                 ui::EF_CONTROL_DOWN | ui::EF_SHIFT_DOWN));
  // Ctrl+Tab
  AddAccelerator(ui::Accelerator(ui::VKEY_TAB, ui::EF_CONTROL_DOWN));
}

void TabbedPane::Layout() {
  if (native_tabbed_pane_)
    native_tabbed_pane_->GetView()->SetBounds(0, 0, width(), height());
}

void TabbedPane::ViewHierarchyChanged(bool is_add, View* parent, View* child) {
  if (is_add && !native_tabbed_pane_) {
    // The native wrapper's lifetime will be managed by the view hierarchy after
    // we call AddChildView.
    native_tabbed_pane_ = CreateNativeWrapper(this);
    AddChildView(native_tabbed_pane_->GetView());
    LoadAccelerators();
  }
}

bool TabbedPane::AcceleratorPressed(const ui::Accelerator& accelerator) {
  // We only accept Ctrl+Tab keyboard events.
  DCHECK(accelerator.key_code() == ui::VKEY_TAB && accelerator.IsCtrlDown());

  int tab_count = GetTabCount();
  if (tab_count <= 1)
    return false;
  int selected_tab_index = GetSelectedTabIndex();
  int next_tab_index = accelerator.IsShiftDown() ?
      (selected_tab_index - 1) % tab_count :
      (selected_tab_index + 1) % tab_count;
  // Wrap around.
  if (next_tab_index < 0)
    next_tab_index += tab_count;
  SelectTabAt(next_tab_index);
  return true;
}

std::string TabbedPane::GetClassName() const {
  return kViewClassName;
}

void TabbedPane::OnFocus() {
  // Forward the focus to the wrapper.
  if (native_tabbed_pane_) {
    native_tabbed_pane_->SetFocus();

    View* selected_tab = GetSelectedTab();
    if (selected_tab) {
      selected_tab->GetWidget()->NotifyAccessibilityEvent(
          selected_tab, ui::AccessibilityTypes::EVENT_FOCUS, true);
    }
  } else {
    View::OnFocus();  // Will focus the RootView window (so we still get
                      // keyboard messages).
  }
}

void TabbedPane::OnPaintFocusBorder(gfx::Canvas* canvas) {
  if (NativeViewHost::kRenderNativeControlFocus)
    View::OnPaintFocusBorder(canvas);
}

void TabbedPane::GetAccessibleState(ui::AccessibleViewState* state) {
  state->role = ui::AccessibilityTypes::ROLE_PAGETABLIST;
  state->name = accessible_name_;
}

}  // namespace views
