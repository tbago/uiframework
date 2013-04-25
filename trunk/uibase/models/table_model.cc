// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "uibase/models/table_model.h"

#include "base/logging.h"
#include "uibase/l10n/l10n_util.h"
#include "uibase/l10n/l10n_util_collator.h"
#include "gfx/image/image_skia.h"

namespace ui {

// TableColumn -----------------------------------------------------------------

TableColumn::TableColumn()
    : id(0),
      title(),
      alignment(LEFT),
      width(-1),
      percent(),
      min_visible_width(0),
      sortable(false) {
}

TableColumn::TableColumn(int id, Alignment alignment, int width, float percent)
    : id(id),
      title(l10n_util::GetStringUTF16(id)),
      alignment(alignment),
      width(width),
      percent(percent),
      min_visible_width(0),
      sortable(false) {
}

// TableModel -----------------------------------------------------------------

gfx::ImageSkia TableModel::GetIcon(int row) {
  return gfx::ImageSkia();
}

string16 TableModel::GetTooltip(int row) {
  return string16();
}

bool TableModel::ShouldIndent(int row) {
  return false;
}

bool TableModel::HasGroups() {
  return false;
}

TableModel::Groups TableModel::GetGroups() {
  // If you override HasGroups to return true, you must override this as
  // well.
  NOTREACHED();
  return std::vector<Group>();
}

int TableModel::GetGroupID(int row) {
  // If you override HasGroups to return true, you must override this as
  // well.
  NOTREACHED();
  return 0;
}

int TableModel::CompareValues(int row1, int row2, int column_id) {
  DCHECK(row1 >= 0 && row1 < RowCount() &&
         row2 >= 0 && row2 < RowCount());
  string16 value1 = GetText(row1, column_id);
  string16 value2 = GetText(row2, column_id);

  NOTREACHED();
  return 0;
}

}  // namespace ui
