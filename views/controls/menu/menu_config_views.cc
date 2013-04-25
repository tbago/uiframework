// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "views/controls/menu/menu_config.h"

#include "grit/ui_resources.h"
#include "uibase/layout.h"
#include "uibase/resource/resource_bundle.h"
#include "gfx/image/image.h"
#include "gfx/image/image_skia.h"
#include "native_theme/native_theme_aura.h"
#include "views/controls/menu/menu_image_util.h"

namespace views {

#if !defined(OS_WIN)
void MenuConfig::Init(const ui::NativeTheme* theme) {
  InitAura();
}
#endif

#if !defined(OS_WIN)
// static
const MenuConfig& MenuConfig::instance(const ui::NativeTheme* theme) {
  static MenuConfig* views_instance = NULL;
  if (!views_instance)
    views_instance = new MenuConfig(ui::NativeTheme::instance());
  return *views_instance;
}
#endif

}  // namespace views
