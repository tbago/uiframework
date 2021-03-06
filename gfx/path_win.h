// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef UI_GFX_PATH_WIN_H_
#define UI_GFX_PATH_WIN_H_

#include <windows.h>

#include "uibase/ui_export.h"

class SkPath;

namespace gfx {

// Creates a new HRGN given |path|. The caller is responsible for destroying
// the returned region.
UI_EXPORT HRGN CreateHRGNFromSkPath(const SkPath& path);

}  // namespace gfx

#endif  // UI_GFX_PATH_WIN_H_
