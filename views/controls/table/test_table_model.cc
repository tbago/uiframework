// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "views/controls/table/test_table_model.h"

#include "base/string_number_conversions.h"
#include "base/utf_string_conversions.h"
#include "third_party/skia/include/core/SkBitmap.h"
#include "uibase/models/table_model_observer.h"
#include "gfx/image/image_skia.h"

TestTableModel::TestTableModel(int row_count)
    : row_count_(row_count),
      observer_(NULL) {
}

TestTableModel::~TestTableModel() {
}

int TestTableModel::RowCount() {
  return row_count_;
}

string16 TestTableModel::GetText(int row, int column_id) {
  return ASCIIToUTF16(base::IntToString(row) + "x" +
                      base::IntToString(column_id));
}

gfx::ImageSkia TestTableModel::GetIcon(int row) {
  SkBitmap bitmap;
  bitmap.setConfig(SkBitmap::kARGB_8888_Config, 16, 16);
  return gfx::ImageSkia(bitmap);
}

void TestTableModel::SetObserver(ui::TableModelObserver* observer) {
  observer_ = observer;
}
