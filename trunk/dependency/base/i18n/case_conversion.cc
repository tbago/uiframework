// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/i18n/case_conversion.h"
#include "base/string16.h"

#ifdef OS_WIN
#include <Windows.h>
#endif

namespace base {
namespace i18n {

string16 ToLower(const StringPiece16& string) {
#ifdef OS_WIN
    //TODO (threedollers):remove icu dependency
    string16 lower(string.data());
    ::CharLowerW(const_cast<LPWSTR>(lower.c_str()));
    return lower;
#else
    DCHECK(false);
#endif
}

string16 ToUpper(const StringPiece16& string) {
#ifdef OS_WIN
    //TODO (threedollers):remove icu dependency
    string16 upper(string.data());
    ::CharUpperW(const_cast<LPWSTR>(upper.c_str()));
    return upper;
#else
    DCHECK(false);
#endif
}

}  // namespace i18n
}  // namespace base
