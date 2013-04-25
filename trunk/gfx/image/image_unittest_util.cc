// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Because the unit tests for gfx::Image are spread across multiple
// implementation files, this header contains the reusable components.

#include "ui/gfx/image/image_unittest_util.h"

#include <cmath>

#include "base/memory/scoped_ptr.h"
#include "testing/gtest/include/gtest/gtest.h"
#include "third_party/skia/include/core/SkBitmap.h"
#include "ui/gfx/codec/png_codec.h"
#include "ui/gfx/image/image_skia.h"

#if defined(TOOLKIT_GTK)
#include <gtk/gtk.h>
#include "ui/gfx/gtk_util.h"
#elif defined(OS_IOS)
#include "base/mac/foundation_util.h"
#include "base/mac/scoped_cftyperef.h"
#include "skia/ext/skia_utils_ios.h"
#elif defined(OS_MACOSX)
#include "base/mac/mac_util.h"
#include "skia/ext/skia_utils_mac.h"
#endif

namespace gfx {
namespace test {

namespace {

bool ColorComponentsClose(SkColor component1, SkColor component2) {
  int c1 = static_cast<int>(component1);
  int c2 = static_cast<int>(component2);
  return std::abs(c1 - c2) <= 40;
}

bool ColorsClose(SkColor color1, SkColor color2) {
  // Be tolerant of floating point rounding and lossy color space conversions.
  return ColorComponentsClose(SkColorGetR(color1), SkColorGetR(color2)) &&
         ColorComponentsClose(SkColorGetG(color1), SkColorGetG(color2)) &&
         ColorComponentsClose(SkColorGetB(color1), SkColorGetB(color2)) &&
         ColorComponentsClose(SkColorGetA(color1), SkColorGetA(color2));
}

}  // namespace

void SetSupportedScaleFactorsTo1xAnd2x() {
  std::vector<ui::ScaleFactor> supported_scale_factors;
  supported_scale_factors.push_back(ui::SCALE_FACTOR_100P);
  supported_scale_factors.push_back(ui::SCALE_FACTOR_200P);
  ui::test::SetSupportedScaleFactors(supported_scale_factors);
}

const SkBitmap CreateBitmap(int width, int height) {
  SkBitmap bitmap;
  bitmap.setConfig(SkBitmap::kARGB_8888_Config, width, height);
  bitmap.allocPixels();
  bitmap.eraseRGB(0, 255, 0);
  return bitmap;
}

scoped_refptr<base::RefCountedMemory> CreatePNGBytes(int edge_size) {
  SkBitmap bitmap = CreateBitmap(edge_size, edge_size);
  scoped_refptr<base::RefCountedBytes> bytes(new base::RefCountedBytes());
  PNGCodec::EncodeBGRASkBitmap(bitmap, false, &bytes->data());
  return bytes;
}

gfx::Image CreateImage() {
  return CreateImage(100, 50);
}

gfx::Image CreateImage(int width, int height) {
  return gfx::Image(CreateBitmap(width, height));
}

bool IsEqual(const gfx::Image& img1, const gfx::Image& img2) {
  std::vector<gfx::ImageSkiaRep> img1_reps = img1.AsImageSkia().image_reps();
  gfx::ImageSkia image_skia2 = img2.AsImageSkia();
  if (image_skia2.image_reps().size() != img1_reps.size())
    return false;

  for (size_t i = 0; i < img1_reps.size(); ++i) {
    ui::ScaleFactor scale_factor = img1_reps[i].scale_factor();
    const gfx::ImageSkiaRep& image_rep2 = image_skia2.GetRepresentation(
        scale_factor);
    if (image_rep2.scale_factor() != scale_factor ||
        !IsEqual(img1_reps[i].sk_bitmap(), image_rep2.sk_bitmap())) {
      return false;
    }
  }
  return true;
}

bool IsEqual(const SkBitmap& bmp1, const SkBitmap& bmp2) {
  if (bmp1.isNull() && bmp2.isNull())
    return true;

  if (bmp1.width() != bmp2.width() ||
      bmp1.height() != bmp2.height() ||
      bmp1.config() != SkBitmap::kARGB_8888_Config ||
      bmp2.config() != SkBitmap::kARGB_8888_Config) {
    return false;
  }

  SkAutoLockPixels lock1(bmp1);
  SkAutoLockPixels lock2(bmp2);
  if (!bmp1.getPixels() || !bmp2.getPixels())
    return false;

  for (int y = 0; y < bmp1.height(); ++y) {
    for (int x = 0; x < bmp1.width(); ++x) {
      if (!ColorsClose(bmp1.getColor(x,y), bmp2.getColor(x,y)))
        return false;
    }
  }

  return true;
}

bool IsEqual(const scoped_refptr<base::RefCountedMemory>& bytes,
             const SkBitmap& bitmap) {
  SkBitmap decoded;
  if (!bytes.get() ||
      !PNGCodec::Decode(bytes->front(), bytes->size(), &decoded)) {
    return bitmap.isNull();
  }

  return IsEqual(bitmap, decoded);
}

void CheckImageIndicatesPNGDecodeFailure(const gfx::Image& image) {
  SkBitmap bitmap = image.AsBitmap();
  EXPECT_FALSE(bitmap.isNull());
  EXPECT_LE(16, bitmap.width());
  EXPECT_LE(16, bitmap.height());
  SkAutoLockPixels auto_lock(bitmap);
  CheckColors(bitmap.getColor(10, 10), SK_ColorRED);
}

bool ImageSkiaStructureMatches(
    const gfx::ImageSkia& image_skia,
    int width,
    int height,
    const std::vector<ui::ScaleFactor>& scale_factors) {
  if (image_skia.isNull() ||
      image_skia.width() != width ||
      image_skia.height() != height ||
      image_skia.image_reps().size() != scale_factors.size()) {
    return false;
  }

  for (size_t i = 0; i < scale_factors.size(); ++i) {
    gfx::ImageSkiaRep image_rep =
        image_skia.GetRepresentation(scale_factors[i]);
    if (image_rep.is_null() ||
        image_rep.scale_factor() != scale_factors[i])
      return false;

    float scale = ui::GetScaleFactorScale(scale_factors[i]);
    if (image_rep.pixel_width() != static_cast<int>(width * scale) ||
        image_rep.pixel_height() != static_cast<int>(height * scale)) {
      return false;
    }
  }
  return true;
}

bool IsEmpty(const gfx::Image& image) {
  const SkBitmap& bmp = *image.ToSkBitmap();
  return bmp.isNull() ||
         (bmp.width() == 0 && bmp.height() == 0);
}

PlatformImage CreatePlatformImage() {
  const SkBitmap bitmap(CreateBitmap(25, 25));
#if defined(OS_IOS)
  ui::ScaleFactor scale_factor = ui::GetMaxScaleFactor();
  float scale = ui::GetScaleFactorScale(scale_factor);

  base::mac::ScopedCFTypeRef<CGColorSpaceRef> color_space(
      CGColorSpaceCreateDeviceRGB());
  UIImage* image =
      gfx::SkBitmapToUIImageWithColorSpace(bitmap, scale, color_space);
  base::mac::NSObjectRetain(image);
  return image;
#elif defined(OS_MACOSX)
  NSImage* image = gfx::SkBitmapToNSImage(bitmap);
  base::mac::NSObjectRetain(image);
  return image;
#elif defined(TOOLKIT_GTK)
  return gfx::GdkPixbufFromSkBitmap(bitmap);
#else
  return bitmap;
#endif
}

gfx::Image::RepresentationType GetPlatformRepresentationType() {
#if defined(OS_IOS)
  return gfx::Image::kImageRepCocoaTouch;
#elif defined(OS_MACOSX)
  return gfx::Image::kImageRepCocoa;
#elif defined(TOOLKIT_GTK)
  return gfx::Image::kImageRepGdk;
#else
  return gfx::Image::kImageRepSkia;
#endif
}

PlatformImage ToPlatformType(const gfx::Image& image) {
#if defined(OS_IOS)
  return image.ToUIImage();
#elif defined(OS_MACOSX)
  return image.ToNSImage();
#elif defined(TOOLKIT_GTK)
  return image.ToGdkPixbuf();
#else
  return *image.ToSkBitmap();
#endif
}

PlatformImage CopyPlatformType(const gfx::Image& image) {
#if defined(OS_IOS)
  return image.CopyUIImage();
#elif defined(OS_MACOSX)
  return image.CopyNSImage();
#elif defined(TOOLKIT_GTK)
  return image.CopyGdkPixbuf();
#else
  return *image.ToSkBitmap();
#endif
}

#if defined(OS_MACOSX)
// Defined in image_unittest_util_mac.mm.
#elif defined(TOOLKIT_GTK)
SkColor GetPlatformImageColor(PlatformImage image, int x, int y) {
  int n_channels = gdk_pixbuf_get_n_channels(image);
  int rowstride = gdk_pixbuf_get_rowstride(image);
  guchar* gdk_pixels = gdk_pixbuf_get_pixels(image);

  guchar* pixel = gdk_pixels + (y * rowstride) + (x * n_channels);
  guchar alpha = gdk_pixbuf_get_has_alpha(image) ? pixel[3] : 255;
  return SkColorSetARGB(alpha, pixel[0], pixel[1], pixel[2]);
}
#else
SkColor GetPlatformImageColor(PlatformImage image, int x, int y) {
  SkAutoLockPixels auto_lock(image);
  return image.getColor(x, y);
}
#endif

void CheckColors(SkColor color1, SkColor color2) {
  EXPECT_TRUE(ColorsClose(color1, color2));
}

void CheckIsTransparent(SkColor color) {
  EXPECT_LT(SkColorGetA(color) / 255.0, 0.05);
}

bool IsPlatformImageValid(PlatformImage image) {
#if defined(OS_MACOSX) || defined(TOOLKIT_GTK)
  return image != NULL;
#else
  return !image.isNull();
#endif
}

bool PlatformImagesEqual(PlatformImage image1, PlatformImage image2) {
#if defined(OS_MACOSX) || defined(TOOLKIT_GTK)
  return image1 == image2;
#else
  return image1.getPixels() == image2.getPixels();
#endif
}

}  // namespace test
}  // namespace gfx
