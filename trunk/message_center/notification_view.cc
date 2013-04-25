// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "message_center/notification_view.h"

#include "base/utf_string_conversions.h"
#include "grit/ui_resources.h"
#include "base/accessibility/accessible_view_state.h"
#include "base/resource/resource_bundle.h"
#include "base/text/text_elider.h"
#include "gfx/canvas.h"
#include "gfx/size.h"
#include "message_center/message_center_constants.h"
#include "native_theme/native_theme.h"
#include "views/controls/button/image_button.h"
#include "views/controls/image_view.h"
#include "views/controls/label.h"
#include "views/layout/box_layout.h"
#include "views/layout/grid_layout.h"

namespace {

// Notification dimensions.
const int kIconLeftPadding = 0;
const int kIconColumnWidth = message_center::kNotificationIconWidth;
const int kIconToTextPadding = 15;
const int kTextToClosePadding = 10;
const int kCloseColumnWidth = 8;
const int kCloseRightPadding = 6;
const int kIconTopPadding = 0;
const int kTextTopPadding = 9;
const int kCloseTopPadding = 6;
const int kIconBottomPadding = 0;
const int kTextBottomPadding = 12;
const int kItemTitleToDetailsPadding = 3;

// Notification colors. The text background colors below are used only to keep
// view::Label from modifying the text color and will not actually be drawn.
// See view::Label's SetEnabledColor() and SetBackgroundColor() for details.
const SkColor kBackgroundColor = SkColorSetRGB(255, 255, 255);
const SkColor kTitleColor = SkColorSetRGB(68, 68, 68);
const SkColor kTitleBackgroundColor = SK_ColorWHITE;
const SkColor kMessageColor = SkColorSetRGB(136, 136, 136);
const SkColor kMessageBackgroundColor = SK_ColorBLACK;

// Static function to create an empty border to be used as padding.
views::Border* MakePadding(int top, int left, int bottom, int right) {
  return views::Border::CreateEmptyBorder(top, left, bottom, right);
}

// ItemViews are responsible for drawing each NotificationView item's title and
// message next to each other within a single column.
class ItemView : public views::View {
 public:
  ItemView(const message_center::NotificationList::NotificationItem& item);
  virtual ~ItemView();

 private:
  DISALLOW_COPY_AND_ASSIGN(ItemView);
};

ItemView::ItemView(
    const message_center::NotificationList::NotificationItem& item) {
  views::BoxLayout* layout =
      new views::BoxLayout(views::BoxLayout::kHorizontal,
                           0, 0, kItemTitleToDetailsPadding);
  SetLayoutManager(layout);

  views::Label* title = new views::Label(item.title);
  title->SetHorizontalAlignment(gfx::ALIGN_LEFT);
  title->SetElideBehavior(views::Label::ELIDE_AT_END);
  title->SetEnabledColor(kTitleColor);
  title->SetBackgroundColor(kTitleBackgroundColor);
  AddChildViewAt(title, 0);

  views::Label* details = new views::Label(item.message);
  details->SetHorizontalAlignment(gfx::ALIGN_LEFT);
  details->SetElideBehavior(views::Label::ELIDE_AT_END);
  details->SetEnabledColor(kMessageColor);
  details->SetBackgroundColor(kMessageBackgroundColor);
  AddChildViewAt(details, 1);

  PreferredSizeChanged();
  SchedulePaint();
}

ItemView::~ItemView() {
}

}  // namespace

namespace message_center {

NotificationView::NotificationView(
    NotificationList::Delegate* list_delegate,
    const NotificationList::Notification& notification)
    : MessageView(list_delegate, notification) {
}

NotificationView::~NotificationView() {
}

void NotificationView::SetUpView() {
  set_background(views::Background::CreateSolidBackground(kBackgroundColor));

  views::GridLayout* layout = new views::GridLayout(this);
  SetLayoutManager(layout);

  // Three columns (icon, text, close button) surrounded by padding. The icon
  // and close button columns and the padding have fixed widths and the text
  // column fills up the remaining space. To minimize the number of columns and
  // simplify column spanning padding is applied to each view within columns
  // instead of through padding columns.
  views::ColumnSet* columns = layout->AddColumnSet(0);
  columns->AddColumn(views::GridLayout::LEADING, views::GridLayout::LEADING,
                     0, views::GridLayout::FIXED,
                     kIconLeftPadding + kIconColumnWidth + kIconToTextPadding,
                     kIconLeftPadding + kIconColumnWidth + kIconToTextPadding);
                     // Padding + icon + padding.
  columns->AddColumn(views::GridLayout::FILL, views::GridLayout::FILL,
                     100, views::GridLayout::USE_PREF,
                     0, 0);
                     // Text + padding (kTextToClosePadding).
  columns->AddColumn(views::GridLayout::LEADING, views::GridLayout::LEADING,
                     0, views::GridLayout::FIXED,
                     kCloseColumnWidth + kCloseRightPadding,
                     kCloseColumnWidth + kCloseRightPadding);
                     // Close button + padding.

  // First row: Icon. This vertically spans the close button padding row, the
  // close button row, and all item rows.
  layout->StartRow(0, 0);
  views::ImageView* icon = new views::ImageView();
  icon->SetImageSize(gfx::Size(message_center::kNotificationIconWidth,
                               message_center::kNotificationIconWidth));
  icon->SetImage(notification_.primary_icon);
  icon->SetHorizontalAlignment(views::ImageView::LEADING);
  icon->SetVerticalAlignment(views::ImageView::LEADING);
  icon->set_border(MakePadding(kIconTopPadding, kIconLeftPadding,
                               kIconBottomPadding, kIconToTextPadding));
  int displayed_item_count =
      std::min(notification_.items.size(), kNotificationMaxItems);
  layout->AddView(icon, 1, 2 + displayed_item_count);

  // First row: Title. This vertically spans the close button padding row and
  // the close button row.
  // TODO(dharcourt): Skip the title Label when there's no title text.
  views::Label* title = new views::Label(notification_.title);
  title->SetHorizontalAlignment(gfx::ALIGN_LEFT);
  title->SetElideBehavior(views::Label::ELIDE_AT_END);
  title->SetFont(title->font().DeriveFont(4));
  title->SetEnabledColor(kTitleColor);
  title->SetBackgroundColor(kTitleBackgroundColor);
  title->set_border(MakePadding(kTextTopPadding, 0, 3, kTextToClosePadding));
  layout->AddView(title, 1, 2);

  // First row: Close button padding.
  views::View* padding = new views::ImageView();
  padding->set_border(MakePadding(kCloseTopPadding, 1, 0, 0));
  layout->AddView(padding);

  // Second row: Close button, which has to be on a row of its own because its
  // top padding can't be set using empty borders (ImageButtons don't support
  // borders). The resize factor of this row (100) is much higher than that of
  // other rows (0) to ensure the first row's height stays at kCloseTopPadding.
  layout->StartRow(100, 0);
  layout->SkipColumns(2);
  DCHECK(close_button_);
  layout->AddView(close_button_);

  // One row for each notification item, including appropriate padding.
  for (int i = 0; i < displayed_item_count; ++i) {
    int bottom_padding =
        (i < displayed_item_count - 1) ? 4 : (kTextBottomPadding - 2);
    layout->StartRow(0, 0);
    layout->SkipColumns(1);
    ItemView* item = new ItemView(notification_.items[i]);
    item->set_border(MakePadding(0, 0, bottom_padding, kTextToClosePadding));
    layout->AddView(item);
    layout->SkipColumns(1);
  }
}

}  // namespace message_center
