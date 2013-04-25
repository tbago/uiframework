// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "uibase/ime/composition_text.h"

namespace ui {

CompositionText::CompositionText() {
}

CompositionText::~CompositionText() {
}

void CompositionText::Clear() {
  text.clear();
  underlines.clear();
  selection = Range();
}

}  // namespace ui
