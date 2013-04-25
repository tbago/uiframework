// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "views/controls/table/table_view_views.h"

#include <map>

#include "base/i18n/rtl.h"
#include "uibase/events/event.h"
#include "gfx/canvas.h"
#include "gfx/image/image_skia.h"
#include "gfx/rect_conversions.h"
#include "gfx/skia_util.h"
#include "views/controls/scroll_view.h"
#include "views/controls/table/group_table_model.h"
#include "views/controls/table/table_grouper.h"
#include "views/controls/table/table_header.h"
#include "views/controls/table/table_utils.h"
#include "views/controls/table/table_view_observer.h"
#include "views/controls/table/table_view_row_background_painter.h"

// Padding around the text (on each side).
static const int kTextVerticalPadding = 3;
static const int kTextHorizontalPadding = 2;

// TODO: these should come from native theme or something.
static const SkColor kSelectedBackgroundColor = SkColorSetRGB(0xEE, 0xEE, 0xEE);
static const SkColor kTextColor = SK_ColorBLACK;

// Size of images.
static const int kImageSize = 16;

// Padding between the image and text.
static const int kImageToTextPadding = 4;

namespace views {

namespace {

// Returns result, unless ascending is false in which case -result is returned.
int SwapCompareResult(int result, bool ascending) {
  return ascending ? result : -result;
}

// Populates |model_index_to_range_start| based on the |grouper|.
void GetModelIndexToRangeStart(TableGrouper* grouper,
                               int row_count,
                               std::map<int, int>* model_index_to_range_start) {
  for (int model_index = 0; model_index < row_count;) {
    GroupRange range;
    grouper->GetGroupRange(model_index, &range);
    DCHECK_GT(range.length, 0);
    for (int range_counter = 0; range_counter < range.length; range_counter++)
      (*model_index_to_range_start)[range_counter + model_index] = model_index;
    model_index += range.length;
  }
}

} // namespace

// Used as the comparator to sort the contents of the table.
struct TableView::SortHelper {
  explicit SortHelper(TableView* table) : table(table) {}

  bool operator()(int model_index1, int model_index2) {
    return table->CompareRows( model_index1, model_index2) < 0;
  }

  TableView* table;
};

// Used as the comparator to sort the contents of the table when a TableGrouper
// is present. When groups are present we sort the groups based on the first row
// in the group and within the groups we keep the same order as the model.
struct TableView::GroupSortHelper {
  explicit GroupSortHelper(TableView* table) : table(table) {}

  bool operator()(int model_index1, int model_index2) {
    const int range1 = model_index_to_range_start[model_index1];
    const int range2 = model_index_to_range_start[model_index2];
    if (range1 == range2) {
      // The two rows are in the same group, sort so that items in the same
      // group always appear in the same order.
      return model_index1 < model_index2;
    }
    return table->CompareRows(range1, range2) < 0;
  }

  TableView* table;
  std::map<int, int> model_index_to_range_start;
};

TableView::VisibleColumn::VisibleColumn() : x(0), width(0) {}

TableView::VisibleColumn::~VisibleColumn() {}

TableView::PaintRegion::PaintRegion()
    : min_row(0),
      max_row(0),
      min_column(0),
      max_column(0) {
}

TableView::PaintRegion::~PaintRegion() {}

TableView::TableView(ui::TableModel* model,
                     const std::vector<ui::TableColumn>& columns,
                     TableTypes table_type,
                     bool single_selection,
                     bool resizable_columns,
                     // TODO(sky); remove autosize_columns as it's not needed.
                     bool autosize_columns)
    : model_(NULL),
      columns_(columns),
      header_(NULL),
      table_type_(table_type),
      table_view_observer_(NULL),
      row_height_(font_.GetHeight() + kTextVerticalPadding * 2),
      last_parent_width_(0),
      grouper_(NULL) {
  for (size_t i = 0; i < columns.size(); ++i) {
    VisibleColumn visible_column;
    visible_column.column = columns[i];
    visible_columns_.push_back(visible_column);
  }
  set_focusable(true);
  set_background(Background::CreateSolidBackground(SK_ColorWHITE));
  SetModel(model);
}

TableView::~TableView() {
  if (model_)
    model_->SetObserver(NULL);
}

// TODO: this doesn't support arbitrarily changing the model, rename this to
// ClearModel() or something.
void TableView::SetModel(ui::TableModel* model) {
  if (model == model_)
    return;

  if (model_)
    model_->SetObserver(NULL);
  model_ = model;
  selection_model_.Clear();
  if (model_)
    model_->SetObserver(this);
}

View* TableView::CreateParentIfNecessary() {
  ScrollView* scroll_view = ScrollView::CreateScrollViewWithBorder();
  scroll_view->SetContents(this);
  CreateHeaderIfNecessary();
  if (header_)
    scroll_view->SetHeader(header_);
  return scroll_view;
}

void TableView::SetRowBackgroundPainter(
    scoped_ptr<TableViewRowBackgroundPainter> painter) {
  row_background_painter_ = painter.Pass();
}

void TableView::SetGrouper(TableGrouper* grouper) {
  grouper_ = grouper;
  SortItemsAndUpdateMapping();
}

int TableView::RowCount() const {
  return model_ ? model_->RowCount() : 0;
}

int TableView::SelectedRowCount() {
  return static_cast<int>(selection_model_.size());
}

void TableView::Select(int model_row) {
  if (!model_)
    return;

  SelectByViewIndex(model_row == -1 ? -1 : ModelToView(model_row));
}

int TableView::FirstSelectedRow() {
  return SelectedRowCount() == 0 ? -1 : selection_model_.selected_indices()[0];
}

void TableView::SetColumnVisibility(int id, bool is_visible) {
  if (is_visible == IsColumnVisible(id))
    return;

  if (is_visible) {
    VisibleColumn visible_column;
    visible_column.column = FindColumnByID(id);
    visible_columns_.push_back(visible_column);
  } else {
    for (size_t i = 0; i < visible_columns_.size(); ++i) {
      if (visible_columns_[i].column.id == id) {
        visible_columns_.erase(visible_columns_.begin() + i);
        break;
      }
    }
  }
  UpdateVisibleColumnSizes();
  Layout();
  SchedulePaint();
  if (header_) {
    header_->Layout();
    header_->SchedulePaint();
  }
}

void TableView::ToggleSortOrder(int visible_column_index) {
  DCHECK(visible_column_index >= 0 &&
         visible_column_index < static_cast<int>(visible_columns_.size()));
  if (!visible_columns_[visible_column_index].column.sortable)
    return;
  const int column_id = visible_columns_[visible_column_index].column.id;
  SortDescriptors sort(sort_descriptors_);
  if (!sort.empty() && sort[0].column_id == column_id) {
    sort[0].ascending = !sort[0].ascending;
  } else {
    SortDescriptor descriptor(column_id, true);
    sort.insert(sort.begin(), descriptor);
    // Only persist two sort descriptors.
    if (sort.size() > 2)
      sort.resize(2);
  }
  SetSortDescriptors(sort);
}

bool TableView::IsColumnVisible(int id) const {
  for (size_t i = 0; i < visible_columns_.size(); ++i) {
    if (visible_columns_[i].column.id == id)
      return true;
  }
  return false;
}

void TableView::SetVisibleColumnWidth(int index, int width) {
  DCHECK(index >= 0 && index < static_cast<int>(visible_columns_.size()));
  if (visible_columns_[index].width == width)
    return;
  visible_columns_[index].width = width;
  for (size_t i = index + 1; i < visible_columns_.size(); ++i) {
    visible_columns_[i].x =
        visible_columns_[i - 1].x + visible_columns_[i - 1].width;
  }
  PreferredSizeChanged();
  SchedulePaint();
}

int TableView::ModelToView(int model_index) const {
  if (!is_sorted())
    return model_index;
  DCHECK_GE(model_index, 0) << " negative model_index " << model_index;
  DCHECK_LT(model_index, RowCount()) << " out of bounds model_index " <<
      model_index;
  return model_to_view_[model_index];
}

int TableView::ViewToModel(int view_index) const {
  if (!is_sorted())
    return view_index;
  DCHECK_GE(view_index, 0) << " negative view_index " << view_index;
  DCHECK_LT(view_index, RowCount()) << " out of bounds view_index " <<
      view_index;
  return view_to_model_[view_index];
}

void TableView::Layout() {
  // parent()->parent() is the scrollview. When its width changes we force
  // recalculating column sizes.
  View* scroll_view = parent() ? parent()->parent() : NULL;
  if (scroll_view && scroll_view->width() != last_parent_width_) {
    last_parent_width_ = scroll_view->width();
    UpdateVisibleColumnSizes();
  }
  // We have to override Layout like this since we're contained in a ScrollView.
  gfx::Size pref = GetPreferredSize();
  int width = pref.width();
  int height = pref.height();
  if (parent()) {
    width = std::max(parent()->width(), width);
    height = std::max(parent()->height(), height);
  }
  SetBounds(x(), y(), width, height);
}

gfx::Size TableView::GetPreferredSize() {
  int width = 50;
  if (header_ && !visible_columns_.empty())
    width = visible_columns_.back().x + visible_columns_.back().width;
  return gfx::Size(width, RowCount() * row_height_);
}

bool TableView::OnKeyPressed(const ui::KeyEvent& event) {
  if (!HasFocus())
    return false;

  switch (event.key_code()) {
    case ui::VKEY_HOME:
      if (RowCount())
        SelectByViewIndex(0);
      return true;

    case ui::VKEY_END:
      if (RowCount())
        SelectByViewIndex(RowCount() - 1);
      return true;

    case ui::VKEY_UP:
      AdvanceSelection(ADVANCE_DECREMENT);
      return true;

    case ui::VKEY_DOWN:
      AdvanceSelection(ADVANCE_INCREMENT);
      return true;

    default:
      break;
  }
  if (table_view_observer_)
    table_view_observer_->OnKeyDown(event.key_code());
  return false;
}

bool TableView::OnMousePressed(const ui::MouseEvent& event) {
  RequestFocus();
  int row = event.y() / row_height_;
  if (row >= 0 && row < RowCount()) {
    SelectByViewIndex(row);
    if (table_view_observer_ && event.flags() & ui::EF_IS_DOUBLE_CLICK)
      table_view_observer_->OnDoubleClick();
  }
  return true;
}

bool TableView::OnMouseDragged(const ui::MouseEvent& event) {
  int row = event.y() / row_height_;
  if (row >= 0 && row < RowCount())
    SelectByViewIndex(row);
  return true;
}

void TableView::OnModelChanged() {
  selection_model_.Clear();
  NumRowsChanged();
}

void TableView::OnItemsChanged(int start, int length) {
  SortItemsAndUpdateMapping();
}

void TableView::OnItemsAdded(int start, int length) {
  for (int i = 0; i < length; ++i)
    selection_model_.IncrementFrom(start);
  NumRowsChanged();
}

void TableView::OnItemsRemoved(int start, int length) {
  // Determine the currently selected index in terms of the view. We inline the
  // implementation here since ViewToModel() has DCHECKs that fail since the
  // model has changed but |model_to_view_| has not been updated yet.
  const int previously_selected_model_index = FirstSelectedRow();
  int previously_selected_view_index = previously_selected_model_index;
  if (previously_selected_model_index != -1 && is_sorted())
    previously_selected_view_index =
        model_to_view_[previously_selected_model_index];
  for (int i = 0; i < length; ++i)
    selection_model_.DecrementFrom(start);
  NumRowsChanged();
  // If the selection was empty and is no longer empty select the same visual
  // index.
  if (selection_model_.empty() && previously_selected_view_index != -1 &&
      RowCount()) {
    selection_model_.SetSelectedIndex(
        ViewToModel(std::min(RowCount() - 1, previously_selected_view_index)));
  }
  if (table_view_observer_)
    table_view_observer_->OnSelectionChanged();
}

gfx::Point TableView::GetKeyboardContextMenuLocation() {
  int first_selected = FirstSelectedRow();
  gfx::Rect vis_bounds(GetVisibleBounds());
  int y = vis_bounds.height() / 2;
  if (first_selected != -1) {
    gfx::Rect cell_bounds(GetRowBounds(first_selected));
    if (cell_bounds.bottom() >= vis_bounds.y() &&
        cell_bounds.bottom() < vis_bounds.bottom()) {
      y = cell_bounds.bottom();
    }
  }
  gfx::Point screen_loc(0, y);
  if (base::i18n::IsRTL())
    screen_loc.set_x(width());
  ConvertPointToScreen(this, &screen_loc);
  return screen_loc;
}

void TableView::OnPaint(gfx::Canvas* canvas) {
  // Don't invoke View::OnPaint so that we can render our own focus border.
  OnPaintBackground(canvas);

  if (!RowCount() || visible_columns_.empty())
    return;

  const PaintRegion region(GetPaintRegion(GetPaintBounds(canvas)));
  if (region.min_column == -1)
    return;  // No need to paint anything.

  const int icon_index = GetIconIndex();
  for (int i = region.min_row; i < region.max_row; ++i) {
    const int model_index = ViewToModel(i);
    if (selection_model_.IsSelected(model_index)) {
      const gfx::Rect row_bounds(GetRowBounds(i));
      canvas->FillRect(row_bounds, kSelectedBackgroundColor);
      if (HasFocus() && !header_)
        canvas->DrawFocusRect(row_bounds);
    } else if (row_background_painter_) {
      row_background_painter_->PaintRowBackground(model_index,
                                                  GetRowBounds(i),
                                                  canvas);
    }
    for (int j = region.min_column; j < region.max_column; ++j) {
      const gfx::Rect cell_bounds(GetCellBounds(i, j));
      int text_x = kTextHorizontalPadding + cell_bounds.x();
      if (j == icon_index) {
        gfx::ImageSkia image = model_->GetIcon(model_index);
        if (!image.isNull()) {
          int image_x = GetMirroredXWithWidthInView(text_x, image.width());
          canvas->DrawImageInt(
              image, 0, 0, image.width(), image.height(),
              image_x,
              cell_bounds.y() + (cell_bounds.height() - kImageSize) / 2,
              kImageSize, kImageSize, true);
        }
        text_x += kImageSize + kImageToTextPadding;
      }
      canvas->DrawStringInt(
          model_->GetText(model_index, visible_columns_[j].column.id), font_,
          kTextColor,
          GetMirroredXWithWidthInView(text_x, cell_bounds.right() - text_x),
          cell_bounds.y() + kTextVerticalPadding,
          cell_bounds.right() - text_x,
          cell_bounds.height() - kTextVerticalPadding * 2,
          TableColumnAlignmentToCanvasAlignment(
              visible_columns_[j].column.alignment));
    }
  }
}

void TableView::OnFocus() {
  SchedulePaintForSelection();
}

void TableView::OnBlur() {
  SchedulePaintForSelection();
}

void TableView::NumRowsChanged() {
  SortItemsAndUpdateMapping();
  PreferredSizeChanged();
  SchedulePaint();
}

void TableView::SetSortDescriptors(const SortDescriptors& sort_descriptors) {
  sort_descriptors_ = sort_descriptors;
  SortItemsAndUpdateMapping();
}

void TableView::SortItemsAndUpdateMapping() {
  if (!is_sorted()) {
    view_to_model_.clear();
    model_to_view_.clear();
  } else {
    const int row_count = RowCount();
    view_to_model_.resize(row_count);
    model_to_view_.resize(row_count);
    for (int i = 0; i < row_count; ++i)
      view_to_model_[i] = i;
    if (grouper_) {
      GroupSortHelper sort_helper(this);
      GetModelIndexToRangeStart(grouper_, RowCount(),
                                &sort_helper.model_index_to_range_start);
      std::sort(view_to_model_.begin(), view_to_model_.end(), sort_helper);
    } else {
      std::sort(view_to_model_.begin(), view_to_model_.end(), SortHelper(this));
    }
    for (int i = 0; i < row_count; ++i)
      model_to_view_[view_to_model_[i]] = i;
    model_->ClearCollator();
  }
  SchedulePaint();
}

int TableView::CompareRows(int model_row1, int model_row2) {
  const int sort_result = model_->CompareValues(
      model_row1, model_row2, sort_descriptors_[0].column_id);
  if (sort_result == 0 && sort_descriptors_.size() > 1) {
    // Try the secondary sort.
    return SwapCompareResult(
        model_->CompareValues(model_row1, model_row2,
                              sort_descriptors_[1].column_id),
        sort_descriptors_[1].ascending);
  }
  return SwapCompareResult(sort_result, sort_descriptors_[0].ascending);
}

gfx::Rect TableView::GetRowBounds(int row) {
  return gfx::Rect(0, row * row_height_, width(), row_height_);
}

gfx::Rect TableView::GetCellBounds(int row, int visible_column_index) {
  if (!header_)
    return GetRowBounds(row);
  const VisibleColumn& vis_col(visible_columns_[visible_column_index]);
  return gfx::Rect(vis_col.x, row * row_height_, vis_col.width, row_height_);
}

void TableView::CreateHeaderIfNecessary() {
  // Only create a header if there is more than one column or the title of the
  // only column is not empty.
  if (header_ || (columns_.size() == 1 && columns_[0].title.empty()))
    return;

  header_ = new TableHeader(this);
}

void TableView::UpdateVisibleColumnSizes() {
  if (!header_)
    return;

  std::vector<ui::TableColumn> columns;
  for (size_t i = 0; i < visible_columns_.size(); ++i)
    columns.push_back(visible_columns_[i].column);
  std::vector<int> sizes =
      views::CalculateTableColumnSizes(last_parent_width_, header_->font(),
                                       font_, 0,  // TODO: fix this
                                       columns, model_);
  DCHECK_EQ(visible_columns_.size(), sizes.size());
  int x = 0;
  for (size_t i = 0; i < visible_columns_.size(); ++i) {
    visible_columns_[i].x = x;
    visible_columns_[i].width = sizes[i];
    x += sizes[i];
  }
}

TableView::PaintRegion TableView::GetPaintRegion(
    const gfx::Rect& bounds) const {
  DCHECK(!visible_columns_.empty());
  DCHECK(RowCount());

  PaintRegion region;
  region.min_row = std::min(RowCount() - 1,
                            std::max(0, bounds.y() / row_height_));
  region.max_row = bounds.bottom() / row_height_;
  if (bounds.bottom() % row_height_ != 0)
    region.max_row++;
  region.max_row = std::min(region.max_row, RowCount());

  if (!header_) {
    region.max_column = 1;
    return region;
  }

  region.min_column = -1;
  region.max_column = visible_columns_.size();
  for (size_t i = 0; i < visible_columns_.size(); ++i) {
    int max_x = visible_columns_[i].x + visible_columns_[i].width;
    if (region.min_column == -1 && max_x >= bounds.x())
      region.min_column = static_cast<int>(i);
    if (region.min_column != -1 &&
        visible_columns_[i].x >= bounds.right()) {
      region.max_column = i;
      break;
    }
  }
  return region;
}

gfx::Rect TableView::GetPaintBounds(gfx::Canvas* canvas) const {
  SkRect sk_clip_rect;
  if (canvas->sk_canvas()->getClipBounds(&sk_clip_rect))
    return gfx::ToEnclosingRect(gfx::SkRectToRectF(sk_clip_rect));
  return GetVisibleBounds();
}

void TableView::SchedulePaintForSelection() {
  if (selection_model_.size() == 1)
    SchedulePaintInRect(GetRowBounds(ModelToView(FirstSelectedRow())));
  else if (selection_model_.size() > 1)
    SchedulePaint();
}

int TableView::GetIconIndex() {
  if (table_type_ != ICON_AND_TEXT || columns_.empty())
    return -1;
  if (!header_)
    return 0;

  for (size_t i = 0; i < visible_columns_.size(); ++i) {
    if (visible_columns_[i].column.id == columns_[0].id)
      return static_cast<int>(i);
  }
  return -1;
}

ui::TableColumn TableView::FindColumnByID(int id) const {
  for (size_t i = 0; i < columns_.size(); ++i) {
    if (columns_[i].id == id)
      return columns_[i];
  }
  NOTREACHED();
  return ui::TableColumn();
}

void TableView::SelectByViewIndex(int view_index) {
  ui::ListSelectionModel new_selection;
  if (view_index != -1) {
    const GroupRange range(GetGroupRange(ViewToModel(view_index)));
    new_selection.SetSelectedIndex(range.start);
    for (int i = 1; i < range.length; ++i)
      new_selection.AddIndexToSelection(range.start + i);
  }

  SetSelectionModel(new_selection);
}

void TableView::SetSelectionModel(const ui::ListSelectionModel& new_selection) {
  if (new_selection.Equals(selection_model_))
    return;

  SchedulePaintForSelection();
  selection_model_.Copy(new_selection);
  SchedulePaintForSelection();

  // Scroll the group for the active item to visible.
  if (selection_model_.active() != -1) {
    gfx::Rect vis_rect(GetVisibleBounds());
    const GroupRange range(GetGroupRange(selection_model_.active()));
    const int start_y = GetRowBounds(ModelToView(range.start)).y();
    const int end_y =
        GetRowBounds(ModelToView(range.start + range.length - 1)).bottom();
    vis_rect.set_y(start_y);
    vis_rect.set_height(end_y - start_y);
    ScrollRectToVisible(vis_rect);
  }

  if (table_view_observer_)
    table_view_observer_->OnSelectionChanged();
}

void TableView::AdvanceSelection(AdvanceDirection direction) {
  if (selection_model_.active() == -1) {
    SelectByViewIndex(0);
    return;
  }
  const GroupRange range(GetGroupRange(selection_model_.active()));
  int view_index = ModelToView(selection_model_.active());
  if (direction == ADVANCE_DECREMENT) {
    view_index = std::max(0, view_index -
                          (selection_model_.active() - range.start + 1));
  } else {
    view_index =
        std::min(RowCount() - 1,
                 view_index + (selection_model_.active() - range.start +
                               range.length));
  }
  SelectByViewIndex(view_index);
}

GroupRange TableView::GetGroupRange(int model_index) const {
  GroupRange range;
  if (grouper_) {
    grouper_->GetGroupRange(model_index, &range);
  } else {
    range.start = model_index;
    range.length = 1;
  }
  return range;
}

}  // namespace views
