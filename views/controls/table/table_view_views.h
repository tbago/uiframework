// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef UI_VIEWS_CONTROLS_TABLE_TABLE_VIEW_VIEWS_H_
#define UI_VIEWS_CONTROLS_TABLE_TABLE_VIEW_VIEWS_H_

#include <vector>

#include "base/memory/scoped_ptr.h"
#include "uibase/models/list_selection_model.h"
#include "uibase/models/table_model.h"
#include "uibase/models/table_model_observer.h"
#include "gfx/font.h"
#include "views/view.h"
#include "views/views_export.h"

// A TableView is a view that displays multiple rows with any number of columns.
// TableView is driven by a TableModel. The model returns the contents
// to display. TableModel also has an Observer which is used to notify
// TableView of changes to the model so that the display may be updated
// appropriately.
//
// TableView itself has an observer that is notified when the selection
// changes.
//
// When a table is sorted the model coordinates do not necessarily match the
// view coordinates. All table methods are in terms of the model. If you need to
// convert to view coordinates use ModelToView().
//
// Sorting is done by a locale sensitive string sort. You can customize the
// sort by way of overriding TableModel::CompareValues().
namespace views {

struct GroupRange;
class TableGrouper;
class TableHeader;
class TableViewObserver;
class TableViewRowBackgroundPainter;
class TableViewTestHelper;

// The cells in the first column of a table can contain:
// - only text
// - a small icon (16x16) and some text
// - a check box and some text
enum TableTypes {
  TEXT_ONLY = 0,
  ICON_AND_TEXT,
};

class VIEWS_EXPORT TableView
    : public views::View,
      public ui::TableModelObserver {
 public:
  // Used to track a visible column. Useful only for the header.
  struct VIEWS_EXPORT VisibleColumn {
    VisibleColumn();
    ~VisibleColumn();

    // The column.
    ui::TableColumn column;

    // Starting x-coordinate of the column.
    int x;

    // Width of the column.
    int width;
  };

  // Describes a sorted column.
  struct VIEWS_EXPORT SortDescriptor {
    SortDescriptor() : column_id(-1), ascending(true) {}
    SortDescriptor(int column_id, bool ascending)
        : column_id(column_id),
          ascending(ascending) {}

    // ID of the sorted column.
    int column_id;

    // Is the sort ascending?
    bool ascending;
  };

  typedef std::vector<SortDescriptor> SortDescriptors;

  // Creates a new table using the model and columns specified.
  // The table type applies to the content of the first column (text, icon and
  // text, checkbox and text).
  // When autosize_columns is true, columns always fill the available width. If
  // false, columns are not resized when the table is resized. An extra empty
  // column at the right fills the remaining space.
  // When resizable_columns is true, users can resize columns by dragging the
  // separator on the column header.  NOTE: Right now this is always true.  The
  // code to set it false is still in place to be a base for future, better
  // resizing behavior (see http://b/issue?id=874646 ), but no one uses or
  // tests the case where this flag is false.
  // Note that setting both resizable_columns and autosize_columns to false is
  // probably not a good idea, as there is no way for the user to increase a
  // column's size in that case.
  TableView(ui::TableModel* model,
            const std::vector<ui::TableColumn>& columns,
            TableTypes table_type,
            bool single_selection,
            bool resizable_columns,
            bool autosize_columns);
  virtual ~TableView();

  // Assigns a new model to the table view, detaching the old one if present.
  // If |model| is NULL, the table view cannot be used after this call. This
  // should be called in the containing view's destructor to avoid destruction
  // issues when the model needs to be deleted before the table.
  void SetModel(ui::TableModel* model);
  ui::TableModel* model() const { return model_; }

  // Returns a new ScrollView that contains the receiver.
  View* CreateParentIfNecessary();

  void SetRowBackgroundPainter(
      scoped_ptr<TableViewRowBackgroundPainter> painter);

  // Sets the TableGrouper. TableView does not own |grouper| (common use case is
  // to have TableModel implement TableGrouper).
  void SetGrouper(TableGrouper* grouper);

  // Returns the number of rows in the TableView.
  int RowCount() const;

  // Returns the number of selected rows.
  // TODO(sky): remove this and force callers to use selection_model().
  int SelectedRowCount();

  // Selects the specified item, making sure it's visible.
  void Select(int model_row);

  // Returns the first selected row in terms of the model.
  int FirstSelectedRow();

  const ui::ListSelectionModel& selection_model() const {
    return selection_model_;
  }

  // Changes the visibility of the specified column (by id).
  void SetColumnVisibility(int id, bool is_visible);
  bool IsColumnVisible(int id) const;

  // TODO(sky): rename to set_observer().
  void SetObserver(TableViewObserver* observer) {
    table_view_observer_ = observer;
  }
  TableViewObserver* observer() const { return table_view_observer_; }

  const std::vector<VisibleColumn>& visible_columns() const {
    return visible_columns_;
  }

  // Sets the width of the column. |index| is in terms of |visible_columns_|.
  void SetVisibleColumnWidth(int index, int width);

  // Toggles the sort order of the specified visible column index.
  void ToggleSortOrder(int visible_column_index);

  const SortDescriptors& sort_descriptors() const { return sort_descriptors_; }
  bool is_sorted() const { return !sort_descriptors_.empty(); }

  // Maps from the index in terms of the model to that of the view.
  int ModelToView(int model_index) const;

  // Maps from the index in terms of the view to that of the model.
  int ViewToModel(int view_index) const;

  // View overrides:
  virtual void Layout() OVERRIDE;
  virtual gfx::Size GetPreferredSize() OVERRIDE;
  virtual bool OnKeyPressed(const ui::KeyEvent& event) OVERRIDE;
  virtual bool OnMousePressed(const ui::MouseEvent& event) OVERRIDE;
  virtual bool OnMouseDragged(const ui::MouseEvent& event) OVERRIDE;

  // ui::TableModelObserver overrides:
  virtual void OnModelChanged() OVERRIDE;
  virtual void OnItemsChanged(int start, int length) OVERRIDE;
  virtual void OnItemsAdded(int start, int length) OVERRIDE;
  virtual void OnItemsRemoved(int start, int length) OVERRIDE;

 protected:
  // View overrides:
  virtual gfx::Point GetKeyboardContextMenuLocation() OVERRIDE;
  virtual void OnPaint(gfx::Canvas* canvas) OVERRIDE;
  virtual void OnFocus() OVERRIDE;
  virtual void OnBlur() OVERRIDE;

 private:
  friend class TableViewTestHelper;
  struct GroupSortHelper;
  struct SortHelper;

  // Used during painting to determine the range of cells that need to be
  // painted.
  // NOTE: the row indices returned by this are in terms of the view and column
  // indices in terms of |visible_columns_|.
  struct VIEWS_EXPORT PaintRegion {
    PaintRegion();
    ~PaintRegion();

    int min_row;
    int max_row;
    int min_column;
    int max_column;
  };

  // Used by AdvanceSelection() to determine the direction to change the
  // selection.
  enum AdvanceDirection {
    ADVANCE_DECREMENT,
    ADVANCE_INCREMENT,
  };

  // Invoked when the number of rows changes in some way.
  void NumRowsChanged();

  // Resets the sort descriptions.
  void SetSortDescriptors(const SortDescriptors& sort_descriptors);

  // Does the actual sort and updates the mappings (|view_to_model_| and
  // |model_to_view_|) appropriately.
  void SortItemsAndUpdateMapping();

  // Used to sort the two rows. Returns a value < 0, == 0 or > 0 indicating
  // whether the row2 comes before row1, row2 is the same as row1 or row1 comes
  // after row2. This invokes CompareValues on the model with the sorted column.
  int CompareRows(int model_row1, int model_row2);

  // Returns the bounds of the specified row.
  gfx::Rect GetRowBounds(int row);

  // Returns the bounds of the specified cell. |visible_column_index| indexes
  // into |visible_columns_|.
  gfx::Rect GetCellBounds(int row, int visible_column_index);

  // Creates |header_| if necessary.
  void CreateHeaderIfNecessary();

  // Updates the |x| and |width| of each of the columns in |visible_columns_|.
  void UpdateVisibleColumnSizes();

  // Returns the cells that need to be painted for the specified region.
  // |bounds| is in terms of |this|.
  PaintRegion GetPaintRegion(const gfx::Rect& bounds) const;

  // Returns the bounds that need to be painted based on the clip set on
  // |canvas|.
  gfx::Rect GetPaintBounds(gfx::Canvas* canvas) const;

  // Invokes SchedulePaint() for the selected rows.
  void SchedulePaintForSelection();

  // Returns the index into |visible_columns_| to draw the icon at, or -1 if no
  // icon is to be drawn.
  int GetIconIndex();

  // Returns the TableColumn matching the specified id.
  ui::TableColumn FindColumnByID(int id) const;

  // Sets the selection to the specified index (in terms of the view).
  void SelectByViewIndex(int view_index);

  // Sets the selection model to |new_selection|.
  void SetSelectionModel(const ui::ListSelectionModel& new_selection);

  // Advances the selection (from the active index) in the specified direction.
  void AdvanceSelection(AdvanceDirection direction);

  // Returns the range of the specified model index. If a TableGrouper has not
  // been set this returns a group with a start of |model_index| and length of
  // 1.
  GroupRange GetGroupRange(int model_index) const;

  ui::TableModel* model_;

  std::vector<ui::TableColumn> columns_;

  // The set of visible columns. The values of these point to |columns_|. This
  // may contain a subset of |columns_|.
  std::vector<VisibleColumn> visible_columns_;

  // The header. This is only created if more than one column is specified or
  // the first column has a non-empty title.
  TableHeader* header_;

  const TableTypes table_type_;

  // TODO(sky): rename to observer_.
  TableViewObserver* table_view_observer_;

  // The selection, in terms of the model.
  ui::ListSelectionModel selection_model_;

  gfx::Font font_;

  int row_height_;

  // Width of the ScrollView last time Layout() was invoked. Used to determine
  // when we should invoke UpdateVisibleColumnSizes().
  int last_parent_width_;

  // Current sort.
  SortDescriptors sort_descriptors_;

  // Mappings used when sorted.
  std::vector<int> view_to_model_;
  std::vector<int> model_to_view_;

  scoped_ptr<TableViewRowBackgroundPainter> row_background_painter_;

  TableGrouper* grouper_;

  DISALLOW_COPY_AND_ASSIGN(TableView);
};

}  // namespace views

#endif  // UI_VIEWS_CONTROLS_TABLE_TABLE_VIEW_VIEWS_H_
