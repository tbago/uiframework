// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef UI_BASE_L10N_L10N_UTIL_COLLATOR_H_
#define UI_BASE_L10N_L10N_UTIL_COLLATOR_H_

#include <algorithm>
#include <functional>
#include <string>
#include <vector>

#include "base/memory/scoped_ptr.h"
#include "base/utf_string_conversions.h"
#include "uibase/ui_export.h"

namespace l10n_util {

// Used by SortStringsUsingMethod. Invokes a method on the objects passed to
// operator (), comparing the string results using <.
template <class T, class Method>
class StringMethodComparator : public std::binary_function<const string16&,
                                                           const string16&,
                                                           bool> {
 public:
  explicit StringMethodComparator(Method method) : method_(method) { }

  // Returns true if lhs preceeds rhs.
  bool operator() (T* lhs_t, T* rhs_t) {
    return (lhs_t->*method_)() < (rhs_t->*method_)();
  }

 private:
  Method method_;
};

// Sorts the objects in |elements| using the method |method|, which must return
// a string. Sorting is done using a collator, unless a collator can not be
// found in which case the strings are sorted using the operator <.
template <class T, class Method>
void SortStringsUsingMethod(const std::string& locale,
                            std::vector<T*>* elements,
                            Method method) {
  UErrorCode error = U_ZERO_ERROR;
  icu::Locale loc(locale.c_str());
  scoped_ptr<icu::Collator> collator(icu::Collator::createInstance(loc, error));
  if (U_FAILURE(error)) {
    sort(elements->begin(), elements->end(),
         StringMethodComparator<T, Method>(method));
    return;
  }
}

template <class Element>
void SortVectorWithStringKey(const std::string& locale,
                             std::vector<Element>* elements,
                             bool needs_stable_sort) {
  SortVectorWithStringKey<Element>(locale, elements, 0, elements->size(),
                                   needs_stable_sort);
}

}  // namespace l10n_util

#endif  // UI_BASE_L10N_L10N_UTIL_COLLATOR_H_
