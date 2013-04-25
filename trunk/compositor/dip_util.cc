// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "compositor/dip_util.h"

#include "base/command_line.h"
#include "uibase/ui_base_switches.h"
#include "compositor/compositor.h"
#include "compositor/compositor_switches.h"
#include "compositor/layer.h"
#include "gfx/display.h"
#include "gfx/point.h"
#include "gfx/point_conversions.h"
#include "gfx/rect.h"
#include "gfx/rect_conversions.h"
#include "gfx/size.h"
#include "gfx/size_conversions.h"

namespace ui {

float GetDeviceScaleFactor(const Layer* layer) {
  return layer->device_scale_factor();
}

gfx::Point ConvertPointToDIP(const Layer* layer,
                             const gfx::Point& point_in_pixel) {
  return gfx::ToFlooredPoint(
      gfx::ScalePoint(point_in_pixel, 1.0f / GetDeviceScaleFactor(layer)));
}

gfx::PointF ConvertPointToDIP(const Layer* layer,
                              const gfx::PointF& point_in_pixel) {
  return gfx::ScalePoint(point_in_pixel, 1.0f / GetDeviceScaleFactor(layer));
}

gfx::Size ConvertSizeToDIP(const Layer* layer,
                           const gfx::Size& size_in_pixel) {
  return gfx::ToFlooredSize(
      gfx::ScaleSize(size_in_pixel, 1.0f / GetDeviceScaleFactor(layer)));
}

gfx::Rect ConvertRectToDIP(const Layer* layer,
                           const gfx::Rect& rect_in_pixel) {
  float scale = 1.0f / GetDeviceScaleFactor(layer);
  return gfx::ToFlooredRectDeprecated(gfx::ScaleRect(rect_in_pixel, scale));
}

gfx::Point ConvertPointToPixel(const Layer* layer,
                               const gfx::Point& point_in_dip) {
  return gfx::ToFlooredPoint(
      gfx::ScalePoint(point_in_dip, GetDeviceScaleFactor(layer)));
}

gfx::Size ConvertSizeToPixel(const Layer* layer,
                             const gfx::Size& size_in_dip) {
  return gfx::ToFlooredSize(
      gfx::ScaleSize(size_in_dip, GetDeviceScaleFactor(layer)));
}

gfx::Rect ConvertRectToPixel(const Layer* layer,
                             const gfx::Rect& rect_in_dip) {
  float scale = GetDeviceScaleFactor(layer);
  return gfx::ToFlooredRectDeprecated(gfx::ScaleRect(rect_in_dip, scale));
}
}  // namespace ui
