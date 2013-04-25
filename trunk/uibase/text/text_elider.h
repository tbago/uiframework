// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// This file defines utility functions for eliding and formatting UI text.

#ifndef UI_BASE_TEXT_TEXT_ELIDER_H_
#define UI_BASE_TEXT_TEXT_ELIDER_H_

#include <string>
#include <vector>

#include "base/basictypes.h"
#include "base/string16.h"
#include "uibase/ui_export.h"
#include "gfx/font.h"

class FilePath;
class GURL;

namespace ui {

UI_EXPORT extern const char kEllipsis[];

// Elides a well-formed email address (e.g. username@domain.com) to fit into
// |available_pixel_width| using the specified |font|.
// This function guarantees that the string returned will contain at least one
// character, other than the ellipses, on either side of the '@'. If it is
// impossible to achieve these requirements: only an ellipsis will be returned.
// If possible: this elides only the username portion of the |email|. Otherwise,
// the domain is elided in the middle so that it splits the available width
// equally with the elided username (should the username be short enough that it
// doesn't need half the available width: the elided domain will occupy that
// extra width).
UI_EXPORT string16 ElideEmail(const string16& email,
                              const gfx::Font& font,
                              int available_pixel_width);

enum ElideBehavior {
  // Add ellipsis at the end of the string.
  ELIDE_AT_END,
  // Add ellipsis in the middle of the string.
  ELIDE_IN_MIDDLE,
  // Truncate the end of the string.
  TRUNCATE_AT_END
};

// Elides |text| to fit in |available_pixel_width| according to the specified
// |elide_behavior|.
UI_EXPORT string16 ElideText(const string16& text,
                             const gfx::Font& font,
                             int available_pixel_width,
                             ElideBehavior elide_behavior);

// Elide a filename to fit a given pixel width, with an emphasis on not hiding
// the extension unless we have to. If filename contains a path, the path will
// be removed if filename doesn't fit into available_pixel_width. The elided
// filename is forced to have LTR directionality, which means that in RTL UI
// the elided filename is wrapped with LRE (Left-To-Right Embedding) mark and
// PDF (Pop Directional Formatting) mark.
UI_EXPORT string16 ElideFilename(const FilePath& filename,
                                 const gfx::Font& font,
                                 int available_pixel_width);

// Functions to elide strings when the font information is unknown.  As
// opposed to the above functions, the ElideString() and
// ElideRectangleString() functions operate in terms of character units,
// not pixels.

// If the size of |input| is more than |max_len|, this function returns
// true and |input| is shortened into |output| by removing chars in the
// middle (they are replaced with up to 3 dots, as size permits).
// Ex: ElideString(ASCIIToUTF16("Hello"), 10, &str) puts Hello in str and
// returns false.  ElideString(ASCIIToUTF16("Hello my name is Tom"), 10, &str)
// puts "Hell...Tom" in str and returns true.
// TODO(tsepez): Doesn't handle UTF-16 surrogate pairs properly.
// TODO(tsepez): Doesn't handle bidi properly.
UI_EXPORT bool ElideString(const string16& input, int max_len,
                           string16* output);

// Specifies the word wrapping behavior of |ElideRectangleText()| when a word
// would exceed the available width.
enum WordWrapBehavior {
  // Words that are too wide will be put on a new line, but will not be
  // truncated or elided.
  IGNORE_LONG_WORDS,

  // Words that are too wide will be put on a new line and will be truncated to
  // the available width.
  TRUNCATE_LONG_WORDS,

  // Words that are too wide will be put on a new line and will be elided to the
  // available width.
  ELIDE_LONG_WORDS,

  // Words that are too wide will be put on a new line and will be wrapped over
  // multiple lines.
  WRAP_LONG_WORDS,
};

// Indicates whether the |available_pixel_width| by |available_pixel_height|
// rectangle passed to |ElideRectangleText()| had insufficient space to
// accommodate the given |text|, leading to elision or truncation.
enum ReformattingResultFlags {
  INSUFFICIENT_SPACE_HORIZONTAL = 1 << 0,
  INSUFFICIENT_SPACE_VERTICAL = 1 << 1,
};

// Reformats |text| into output vector |lines| so that the resulting text fits
// into an |available_pixel_width| by |available_pixel_height| rectangle with
// the specified |font|. Input newlines are respected, but lines that are too
// long are broken into pieces. For words that are too wide to fit on a single
// line, the wrapping behavior can be specified with the |wrap_behavior| param.
// Returns a combination of |ReformattingResultFlags| that indicate whether the
// given rectangle had insufficient space to accommodate |texŧ|, leading to
// elision or truncation (and not just reformatting).
UI_EXPORT int ElideRectangleText(const string16& text,
                                 const gfx::Font& font,
                                 int available_pixel_width,
                                 int available_pixel_height,
                                 WordWrapBehavior wrap_behavior,
                                 std::vector<string16>* lines);
}  // namespace ui

#endif  // UI_BASE_TEXT_TEXT_ELIDER_H_
