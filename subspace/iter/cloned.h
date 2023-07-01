// Copyright 2023 Google LLC
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     https://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#pragma once

#include "subspace/iter/iterator_defn.h"
#include "subspace/iter/sized_iterator.h"
#include "subspace/mem/clone.h"
#include "subspace/mem/move.h"
#include "subspace/mem/relocate.h"

namespace sus::iter {

/// An iterator that clones the elements of an underlying iterator.
///
/// This type is returned from `Iterator::cloned()`.
template <class InnerSizedIter>
class [[nodiscard]] [[sus_trivial_abi]] Cloned final
    : public IteratorBase<Cloned<InnerSizedIter>,
                          decltype(sus::clone(
                              std::declval<typename InnerSizedIter::Item>()))> {
 public:
  using Item =
      decltype(sus::clone(std::declval<typename InnerSizedIter::Item>()));

  // sus::iter::Iterator trait.
  Option<Item> next() noexcept {
    return next_iter_.next().map(
        [](const Item& item) { return ::sus::clone(item); });
  }

  /// sus::iter::Iterator trait.
  ::sus::iter::SizeHint size_hint() const noexcept {
    return next_iter_.size_hint();
  }

  // sus::iter::DoubleEndedIterator trait.
  Option<Item> next_back() noexcept
    requires(InnerSizedIter::DoubleEnded)
  {
    return next_iter_.next_back().map(
        [](const Item& item) { return ::sus::clone(item); });
  }

  // sus::iter::ExactSizeIterator trait.
  usize exact_size_hint() const noexcept
    requires(InnerSizedIter::ExactSize)
  {
    return next_iter_.exact_size_hint();
  }

 private:
  template <class U, class V>
  friend class IteratorBase;

  static Cloned with(InnerSizedIter&& next_iter) noexcept {
    return Cloned(::sus::move(next_iter));
  }

  Cloned(InnerSizedIter&& next_iter) : next_iter_(::sus::move(next_iter)) {}

  InnerSizedIter next_iter_;

  // The InnerSizedIter is trivially relocatable.
  sus_class_trivially_relocatable(::sus::marker::unsafe_fn,
                                  decltype(next_iter_));
};

}  // namespace sus::iter
