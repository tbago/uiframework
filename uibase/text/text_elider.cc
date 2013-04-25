// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// This file implements utility functions for eliding and formatting UI text.
//
// Note that several of the functions declared in text_elider.h are implemented
// in this file using helper classes in an unnamed namespace.

#include "uibase/text/text_elider.h"

#include <string>
#include <vector>

#include "base/file_path.h"
#include "base/i18n/rtl.h"
#include "base/memory/scoped_ptr.h"
#include "base/string_split.h"
#include "base/string_util.h"
#include "base/sys_string_conversions.h"
#include "base/utf_string_conversions.h"
#include "googleurl/src/gurl.h"
#include "gfx/font.h"


namespace ui {

// U+2026 in utf8
const char kEllipsis[] = "\xE2\x80\xA6";
const char16 kForwardSlash = '/';

namespace {

// Helper class to split + elide text, while respecting UTF16 surrogate pairs.
class StringSlicer {
 public:
  StringSlicer(const string16& text,
               const string16& ellipsis,
               bool elide_in_middle)
      : text_(text),
        ellipsis_(ellipsis),
        elide_in_middle_(elide_in_middle) {
  }

  // Cuts |text_| to be |length| characters long.  If |cut_in_middle_| is true,
  // the middle of the string is removed to leave equal-length pieces from the
  // beginning and end of the string; otherwise, the end of the string is
  // removed and only the beginning remains.  If |insert_ellipsis| is true,
  // then an ellipsis character will by inserted at the cut point.
  string16 CutString(size_t length, bool insert_ellipsis) {
    const string16 ellipsis_text = insert_ellipsis ? ellipsis_ : string16();

    if (!elide_in_middle_)
      return text_.substr(0, FindValidBoundaryBefore(length)) + ellipsis_text;

    // We put the extra character, if any, before the cut.
    const size_t half_length = length / 2;
    const size_t prefix_length = FindValidBoundaryBefore(length - half_length);
    const size_t suffix_start_guess = text_.length() - half_length;
    const size_t suffix_start = FindValidBoundaryAfter(suffix_start_guess);
    const size_t suffix_length =
        half_length - (suffix_start_guess - suffix_start);
    return text_.substr(0, prefix_length) + ellipsis_text +
           text_.substr(suffix_start, suffix_length);
  }

 private:
  // Returns a valid cut boundary at or before |index|.
  size_t FindValidBoundaryBefore(size_t index) const {
    return 0;
  }

  // Returns a valid cut boundary at or after |index|.
  size_t FindValidBoundaryAfter(size_t index) const {
      return 0;
  }

  // The text to be sliced.
  const string16& text_;

  // Ellipsis string to use.
  const string16& ellipsis_;

  // If true, the middle of the string will be elided.
  bool elide_in_middle_;

  DISALLOW_COPY_AND_ASSIGN(StringSlicer);
};

// Build a path from the first |num_components| elements in |path_elements|.
// Prepends |path_prefix|, appends |filename|, inserts ellipsis if appropriate.
string16 BuildPathFromComponents(const string16& path_prefix,
                                 const std::vector<string16>& path_elements,
                                 const string16& filename,
                                 size_t num_components) {
  const string16 kEllipsisAndSlash = UTF8ToUTF16(kEllipsis) + kForwardSlash;

  // Add the initial elements of the path.
  string16 path = path_prefix;

  // Build path from first |num_components| elements.
  for (size_t j = 0; j < num_components; ++j)
    path += path_elements[j] + kForwardSlash;

  // Add |filename|, ellipsis if necessary.
  if (num_components != (path_elements.size() - 1))
    path += kEllipsisAndSlash;
  path += filename;

  return path;
}

// Takes a prefix (Domain, or Domain+subdomain) and a collection of path
// components and elides if possible. Returns a string containing the longest
// possible elided path, or an empty string if elision is not possible.
string16 ElideComponentizedPath(const string16& url_path_prefix,
                                const std::vector<string16>& url_path_elements,
                                const string16& url_filename,
                                const string16& url_query,
                                const gfx::Font& font,
                                int available_pixel_width) {
  const size_t url_path_number_of_elements = url_path_elements.size();

  const string16 kEllipsisAndSlash = UTF8ToUTF16(kEllipsis) + kForwardSlash;

  CHECK(url_path_number_of_elements);
  for (size_t i = url_path_number_of_elements - 1; i > 0; --i) {
    string16 elided_path = BuildPathFromComponents(url_path_prefix,
        url_path_elements, url_filename, i);
    if (available_pixel_width >= font.GetStringWidth(elided_path))
      return ElideText(elided_path + url_query,
                       font, available_pixel_width, ELIDE_AT_END);
  }

  return string16();
}

}  // namespace

string16 ElideEmail(const string16& email,
                    const gfx::Font& font,
                    int available_pixel_width) {
  if (font.GetStringWidth(email) <= available_pixel_width)
    return email;

  // Split the email into its local-part (username) and domain-part. The email
  // spec technically allows for @ symbols in the local-part (username) of the
  // email under some special requirements. It is guaranteed that there is no @
  // symbol in the domain part of the email however so splitting at the last @
  // symbol is safe.
  const size_t split_index = email.find_last_of('@');
  DCHECK_NE(split_index, string16::npos);
  string16 username = email.substr(0, split_index);
  string16 domain = email.substr(split_index + 1);
  DCHECK(!username.empty());
  DCHECK(!domain.empty());

  const string16 kEllipsisUTF16 = UTF8ToUTF16(kEllipsis);

  // Subtract the @ symbol from the available width as it is mandatory.
  const string16 kAtSignUTF16 = ASCIIToUTF16("@");
  available_pixel_width -= font.GetStringWidth(kAtSignUTF16);

  // Check whether eliding the domain is necessary: if eliding the username
  // is sufficient, the domain will not be elided.
  const int full_username_width = font.GetStringWidth(username);
  const int available_domain_width =
      available_pixel_width -
      std::min(full_username_width,
               font.GetStringWidth(username.substr(0, 1) + kEllipsisUTF16));
  if (font.GetStringWidth(domain) > available_domain_width) {
    // Elide the domain so that it only takes half of the available width.
    // Should the username not need all the width available in its half, the
    // domain will occupy the leftover width.
    // If |desired_domain_width| is greater than |available_domain_width|: the
    // minimal username elision allowed by the specifications will not fit; thus
    // |desired_domain_width| must be <= |available_domain_width| at all cost.
    const int desired_domain_width =
        std::min(available_domain_width,
                 std::max(available_pixel_width - full_username_width,
                          available_pixel_width / 2));
    domain = ElideText(domain, font, desired_domain_width, ELIDE_IN_MIDDLE);
    // Failing to elide the domain such that at least one character remains
    // (other than the ellipsis itself) remains: return a single ellipsis.
    if (domain.length() <= 1U)
      return kEllipsisUTF16;
  }

  // Fit the username in the remaining width (at this point the elided username
  // is guaranteed to fit with at least one character remaining given all the
  // precautions taken earlier).
  username = ElideText(username,
                       font,
                       available_pixel_width - font.GetStringWidth(domain),
                       ELIDE_AT_END);

  return username + kAtSignUTF16 + domain;
}

string16 ElideFilename(const FilePath& filename,
                       const gfx::Font& font,
                       int available_pixel_width) {
#if defined(OS_WIN)
  string16 filename_utf16 = filename.value();
  string16 extension = filename.Extension();
  string16 rootname = filename.BaseName().RemoveExtension().value();
#elif defined(OS_POSIX)
  string16 filename_utf16 = WideToUTF16(base::SysNativeMBToWide(
      filename.value()));
  string16 extension = WideToUTF16(base::SysNativeMBToWide(
      filename.Extension()));
  string16 rootname = WideToUTF16(base::SysNativeMBToWide(
      filename.BaseName().RemoveExtension().value()));
#endif

  const int full_width = font.GetStringWidth(filename_utf16);
  if (full_width <= available_pixel_width)
    return base::i18n::GetDisplayStringInLTRDirectionality(filename_utf16);

  if (rootname.empty() || extension.empty()) {
    const string16 elided_name = ElideText(filename_utf16, font,
                                           available_pixel_width, ELIDE_AT_END);
    return base::i18n::GetDisplayStringInLTRDirectionality(elided_name);
  }

  const int ext_width = font.GetStringWidth(extension);
  const int root_width = font.GetStringWidth(rootname);

  // We may have trimmed the path.
  if (root_width + ext_width <= available_pixel_width) {
    const string16 elided_name = rootname + extension;
    return base::i18n::GetDisplayStringInLTRDirectionality(elided_name);
  }

  if (ext_width >= available_pixel_width) {
    const string16 elided_name = ElideText(rootname + extension, font,
                                           available_pixel_width,
                                           ELIDE_IN_MIDDLE);
    return base::i18n::GetDisplayStringInLTRDirectionality(elided_name);
  }

  int available_root_width = available_pixel_width - ext_width;
  string16 elided_name =
      ElideText(rootname, font, available_root_width, ELIDE_AT_END);
  elided_name += extension;
  return base::i18n::GetDisplayStringInLTRDirectionality(elided_name);
}

string16 ElideText(const string16& text,
                   const gfx::Font& font,
                   int available_pixel_width,
                   ElideBehavior elide_behavior) {
  if (text.empty())
    return text;

  const string16 kEllipsisUTF16 = UTF8ToUTF16(kEllipsis);

  const int current_text_pixel_width = font.GetStringWidth(text);
  const bool elide_in_middle = (elide_behavior == ELIDE_IN_MIDDLE);
  const bool insert_ellipsis = (elide_behavior != TRUNCATE_AT_END);

  StringSlicer slicer(text, kEllipsisUTF16, elide_in_middle);

  // Pango will return 0 width for absurdly long strings. Cut the string in
  // half and try again.
  // This is caused by an int overflow in Pango (specifically, in
  // pango_glyph_string_extents_range). It's actually more subtle than just
  // returning 0, since on super absurdly long strings, the int can wrap and
  // return positive numbers again. Detecting that is probably not worth it
  // (eliding way too much from a ridiculous string is probably still
  // ridiculous), but we should check other widths for bogus values as well.
  if (current_text_pixel_width <= 0 && !text.empty()) {
    const string16 cut = slicer.CutString(text.length() / 2, false);
    return ElideText(cut, font, available_pixel_width, elide_behavior);
  }

  if (current_text_pixel_width <= available_pixel_width)
    return text;

  if (font.GetStringWidth(kEllipsisUTF16) > available_pixel_width)
    return string16();

  // Use binary search to compute the elided text.
  size_t lo = 0;
  size_t hi = text.length() - 1;
  size_t guess;
  for (guess = (lo + hi) / 2; lo <= hi; guess = (lo + hi) / 2) {
    // We check the length of the whole desired string at once to ensure we
    // handle kerning/ligatures/etc. correctly.
    const string16 cut = slicer.CutString(guess, insert_ellipsis);
    const int guess_length = font.GetStringWidth(cut);
    // Check again that we didn't hit a Pango width overflow. If so, cut the
    // current string in half and start over.
    if (guess_length <= 0) {
      return ElideText(slicer.CutString(guess / 2, false),
                       font, available_pixel_width, elide_behavior);
    }
    if (guess_length > available_pixel_width)
      hi = guess - 1;
    else
      lo = guess + 1;
  }

  return slicer.CutString(guess, insert_ellipsis);
}

bool ElideString(const string16& input, int max_len, string16* output) {
  DCHECK_GE(max_len, 0);
  if (static_cast<int>(input.length()) <= max_len) {
    output->assign(input);
    return false;
  }

  switch (max_len) {
    case 0:
      output->clear();
      break;
    case 1:
      output->assign(input.substr(0, 1));
      break;
    case 2:
      output->assign(input.substr(0, 2));
      break;
    case 3:
      output->assign(input.substr(0, 1) + ASCIIToUTF16(".") +
                     input.substr(input.length() - 1));
      break;
    case 4:
      output->assign(input.substr(0, 1) + ASCIIToUTF16("..") +
                     input.substr(input.length() - 1));
      break;
    default: {
      int rstr_len = (max_len - 3) / 2;
      int lstr_len = rstr_len + ((max_len - 3) % 2);
      output->assign(input.substr(0, lstr_len) + ASCIIToUTF16("...") +
                     input.substr(input.length() - rstr_len));
      break;
    }
  }

  return true;
}

}  // namespace ui

namespace {

// Internal class used to track progress of a rectangular text elide
// operation.  Exists so the top-level ElideRectangleText() function
// can be broken into smaller methods sharing this state.
class RectangleText {
 public:
  RectangleText(const gfx::Font& font,
                int available_pixel_width,
                int available_pixel_height,
                ui::WordWrapBehavior wrap_behavior,
                std::vector<string16>* lines)
      : font_(font),
        line_height_(font.GetHeight()),
        available_pixel_width_(available_pixel_width),
        available_pixel_height_(available_pixel_height),
        wrap_behavior_(wrap_behavior),
        current_width_(0),
        current_height_(0),
        last_line_ended_in_lf_(false),
        lines_(lines),
        insufficient_width_(false),
        insufficient_height_(false) {}

  // Perform deferred initializions following creation.  Must be called
  // before any input can be added via AddString().
  void Init() { lines_->clear(); }
  // Perform any deferred output processing.  Must be called after the last
  // AddString() call has occured. Returns a combination of
  // |ReformattingResultFlags| indicating whether the given width or height was
  // insufficient, leading to elision or truncation.
  int Finalize();

 private:

  // Wrap the specified word across multiple lines.
  int WrapWord(const string16& word);

  // Add a long word - wrapping, eliding or truncating per the wrap behavior.
  int AddWordOverflow(const string16& word);

  // Add a word to the rectangluar region at the current position.
  int AddWord(const string16& word);

  // Append the specified |text| to the current output line, incrementing the
  // running width by the specified amount. This is an optimization over
  // |AddToCurrentLine()| when |text_width| is already known.
  void AddToCurrentLineWithWidth(const string16& text, int text_width);

  // Append the specified |text| to the current output line.
  void AddToCurrentLine(const string16& text);

  // Set the current position to the beginning of the next line.
  bool NewLine();

  // The font used for measuring text width.
  const gfx::Font& font_;

  // The height of each line of text.
  const int line_height_;

  // The number of pixels of available width in the rectangle.
  const int available_pixel_width_;

  // The number of pixels of available height in the rectangle.
  const int available_pixel_height_;

  // The wrap behavior for words that are too long to fit on a single line.
  const ui::WordWrapBehavior wrap_behavior_;

  // The current running width.
  int current_width_;

  // The current running height.
  int current_height_;

  // The current line of text.
  string16 current_line_;

  // Indicates whether the last line ended with \n.
  bool last_line_ended_in_lf_;

  // The output vector of lines.
  std::vector<string16>* lines_;

  // Indicates whether a word was so long that it had to be truncated or elided
  // to fit the available width.
  bool insufficient_width_;

  // Indicates whether there were too many lines for the available height.
  bool insufficient_height_;

  DISALLOW_COPY_AND_ASSIGN(RectangleText);
};

int RectangleText::Finalize() {
  // Remove trailing whitespace from the last line or remove the last line
  // completely, if it's just whitespace.
  if (!insufficient_height_ && !lines_->empty()) {
    TrimWhitespace(lines_->back(), TRIM_TRAILING, &lines_->back());
    if (lines_->back().empty() && !last_line_ended_in_lf_)
      lines_->pop_back();
  }
  if (last_line_ended_in_lf_)
    lines_->push_back(string16());
  return (insufficient_width_ ? ui::INSUFFICIENT_SPACE_HORIZONTAL : 0) |
         (insufficient_height_ ? ui::INSUFFICIENT_SPACE_VERTICAL : 0);
}

int RectangleText::WrapWord(const string16& word) {
  // Word is so wide that it must be fragmented.
  string16 text = word;
  int lines_added = 0;
  bool first_fragment = true;
  while (!insufficient_height_ && !text.empty()) {
    const string16 fragment =
        ui::ElideText(text, font_, available_pixel_width_, ui::TRUNCATE_AT_END);
    if (!first_fragment && NewLine())
      lines_added++;
    AddToCurrentLine(fragment);
    text = text.substr(fragment.length());
    first_fragment = false;
  }
  return lines_added;
}

int RectangleText::AddWordOverflow(const string16& word) {
  int lines_added = 0;

  // Unless this is the very first word, put it on a new line.
  if (!current_line_.empty()) {
    if (!NewLine())
      return 0;
    lines_added++;
  }

  if (wrap_behavior_ == ui::IGNORE_LONG_WORDS) {
    current_line_ = word;
    current_width_ = available_pixel_width_;
  } else if (wrap_behavior_ == ui::WRAP_LONG_WORDS) {
    lines_added += WrapWord(word);
  } else {
    const ui::ElideBehavior elide_behavior =
        (wrap_behavior_ == ui::ELIDE_LONG_WORDS ? ui::ELIDE_AT_END :
                                                  ui::TRUNCATE_AT_END);
    const string16 elided_word =
        ui::ElideText(word, font_, available_pixel_width_, elide_behavior);
    AddToCurrentLine(elided_word);
    insufficient_width_ = true;
  }

  return lines_added;
}

int RectangleText::AddWord(const string16& word) {
  int lines_added = 0;
  string16 trimmed;
  TrimWhitespace(word, TRIM_TRAILING, &trimmed);
  const int trimmed_width = font_.GetStringWidth(trimmed);
  if (trimmed_width <= available_pixel_width_) {
    // Word can be made to fit, no need to fragment it.
    if ((current_width_ + trimmed_width > available_pixel_width_) && NewLine())
      lines_added++;
    // Append the non-trimmed word, in case more words are added after.
    AddToCurrentLine(word);
  } else {
    lines_added = AddWordOverflow(wrap_behavior_ == ui::IGNORE_LONG_WORDS ?
                                  trimmed : word);
  }
  return lines_added;
}

void RectangleText::AddToCurrentLine(const string16& text) {
  AddToCurrentLineWithWidth(text, font_.GetStringWidth(text));
}

void RectangleText::AddToCurrentLineWithWidth(const string16& text,
                                              int text_width) {
  if (current_height_ >= available_pixel_height_) {
    insufficient_height_ = true;
    return;
  }
  current_line_.append(text);
  current_width_ += text_width;
}

bool RectangleText::NewLine() {
  bool line_added = false;
  if (current_height_ < available_pixel_height_) {
    lines_->push_back(current_line_);
    current_line_.clear();
    line_added = true;
  } else {
    insufficient_height_ = true;
  }
  current_height_ += line_height_;
  current_width_ = 0;
  return line_added;
}

}  // namespace

namespace ui {

int ElideRectangleText(const string16& input,
                        const gfx::Font& font,
                        int available_pixel_width,
                        int available_pixel_height,
                        WordWrapBehavior wrap_behavior,
                        std::vector<string16>* lines) {
  RectangleText rect(font,
                     available_pixel_width,
                     available_pixel_height,
                     wrap_behavior,
                     lines);
  rect.Init();
  return rect.Finalize();
}


}  // namespace ui
