// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "views/controls/tree/tree_view_controller.h"

#include "uibase/models/tree_model.h"
#include "views/controls/tree/tree_view.h"

namespace views {

bool TreeViewController::CanEdit(TreeView* tree_view, ui::TreeModelNode* node) {
  return true;
}

TreeViewController::~TreeViewController() {
}

}  // namespace views
