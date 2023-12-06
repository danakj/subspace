// Copyright 2022 Google LLC
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

#include <concepts>
#include <type_traits>

#include "sus/macros/builtin.h"
#include "sus/macros/compiler.h"
#include "sus/marker/unsafe.h"
#include "sus/mem/size_of.h"

namespace sus::mem {

namespace __private {

/// Detects the presence and value of the static const member
/// `T::SusUnsafeTrivialRelocate`.
///
/// The static member is created by `sus_class_trivially_relocatable()` and
/// similar macros.
template <class T>
struct relocatable_tag final {
  static constexpr bool value(...) { return false; }

  static constexpr bool value(int)
    requires requires {
      requires(std::same_as<decltype(T::SusUnsafeTrivialRelocate), const bool>);
    }
  {
    return T::SusUnsafeTrivialRelocate;
  };
};

// We use a separate concept to test each `T` so that it can short-circuit when
// `T` is a reference type to an undefined (forward-declared) type.
// clang-format off
template <class T>
concept TriviallyRelocatable_impl =
    std::is_reference_v<T>
    || (!std::is_volatile_v<std::remove_all_extents_t<T>>
       && sus::mem::data_size_of<std::remove_all_extents_t<std::remove_reference_t<T>>>() != size_t(-1)
       && (__private::relocatable_tag<std::remove_all_extents_t<T>>::value(0)
         || std::is_trivially_copyable_v<std::remove_all_extents_t<T>>
         || (std::is_trivially_move_constructible_v<std::remove_all_extents_t<T>> &&
             std::is_trivially_move_assignable_v<std::remove_all_extents_t<T>> &&
             std::is_trivially_destructible_v<std::remove_all_extents_t<T>>)
  #if __has_extension(trivially_relocatable)
         || __is_trivially_relocatable(std::remove_all_extents_t<T>)
  #endif
         )
       );
// clang-format on

}  // namespace __private

/// Tests if a variable of type `T` can be relocated with
/// [`ptr::copy`]($sus::ptr::copy).
///
/// IMPORTANT: If a class satisfies this trait, only
/// [`sus::mem::data_size_of<T>()`]($sus::mem::data_size_of)
/// bytes should be copied when relocating the type, or Undefined Behaviour can
/// result, due to the possibility of overwriting data stored in the tail
/// padding bytes of `T`. See the docs on
/// [`data_size_of`]($sus::mem::data_size_of) for more.
///
/// Volatile types are excluded, as they can not be safely `memcpy`'d
/// byte-by-byte without introducing tearing.
/// References are treated like pointers, and are always trivially relocatable,
/// as reference data members are relocatable in the same way pointers are.
///
/// # Marking a type as trivially relocatable
///
/// Use one of the provided macros inside a class to mark it as conditionally or
/// unconditionally trivially relocatable. They are unsafe because there are no
/// compiler checks that the claim is actually true, though macros are provided
/// to make this easier. The
/// [`sus_class_trivially_relocatable`]($sus_class_trivially_relocatable) and
/// [`sus_class_trivially_relocatable_if_types`]($sus_class_trivially_relocatable_if_types)
/// macros verify that all the types given to it are also trivially relocatable.
/// As long as every field type of the class is given as a parameter, and each
/// field type correctly advertises its trivial relocatability, it will ensure
/// correctness.
///
/// This concept also honors the `[[clang::trivial_abi]]` Clang attribute, as
/// types annotated with the attribute are now considered trivially relocatable
/// in https://reviews.llvm.org/D114732. However, when marking a class with this
/// attribute it is highly recommended to use the
/// [`sus_class_trivially_relocatable`]($sus_class_trivially_relocatable) macro
/// as well to help verify correctness and gain compiler independence.
///
/// | Macro | Style |
/// | ----- | ----- |
/// | [`sus_class_trivially_relocatable`]($sus_class_trivially_relocatable) | **asserts** all param types are trivially relocatable |
/// | [`sus_class_trivially_relocatable_if_types`]($sus_class_trivially_relocatable_if_types) | is **conditionally** trivially relocatable if all param types are |
/// | [`sus_class_trivially_relocatable_if`]($sus_class_trivially_relocatable_if) | is **conditionally** trivially relocatable if the condition is true |
/// | [`sus_class_trivially_relocatable_unchecked`]($sus_class_trivially_relocatable_unchecked) | is trivially relocatable without any condition or assertion |
///
/// # Implementation notes
/// The concept tests against
/// [`std::remove_all_extents_t<T>`](
/// https://en.cppreference.com/w/cpp/types/remove_all_extents)
/// so that the same answer is returned for arrays of `T`, such as for `T` or
/// `T[]` or `T[][][]`.
///
/// While the standard expects types to be `std::is_trivially_copyble` in order
/// to copy by `memcpy`, Clang also allows trivial relocation of move-only types
/// with non-trivial move operations and destructors through
/// `[[clang::trivial_abi]]` while also noting that attributes are not part of
/// and do not modify the type or how the compiler itself treats it outside of
/// argument passing. As such, we consider trivally moveable and destructible
/// types as automatically trivially relocatable, and allow other types to opt
/// in conditionally with the macros and without `[[clang::trivial_abi]]`.
template <class... T>
concept TriviallyRelocatable = (... && __private::TriviallyRelocatable_impl<T>);

}  // namespace sus::mem

/// An attribute to allow a class to be passed in registers.
///
/// This should only be used when the class is also marked as unconditionally
/// relocatable with `sus_class_trivially_relocatable()` or
/// `sus_class_trivially_relocatable_unchecked()`.
///
/// This also enables trivial relocation in libc++ if compiled with clang.
#define _sus_trivial_abi sus_if_clang(clang::trivial_abi)

/// Mark a class as unconditionally trivially relocatable while also asserting
/// that all of the types passed as arguments are also marked as such.
///
/// Typically all field types in the class should be passed to the macro as
/// its arguments.
///
/// To additionally allow the class to be passed in registers, the class can be
/// marked with the `[[clang::trivial_abi]]` attribute.
///
/// Use the [`TriviallyRelocatable`]($sus::mem::TriviallyRelocatable) concept to
/// determine if a type is trivially relocatable, and to test with static_assert
/// that types are matching what you are expecting. This allows containers to
/// optimize their implementations when relocating the type in memory.
///
/// | Macro | Style |
/// | ----- | ----- |
/// | [`sus_class_trivially_relocatable`]($sus_class_trivially_relocatable) | **asserts** all param types are trivially relocatable |
/// | [`sus_class_trivially_relocatable_if_types`]($sus_class_trivially_relocatable_if_types) | is **conditionally** trivially relocatable if all param types are |
/// | [`sus_class_trivially_relocatable_if`]($sus_class_trivially_relocatable_if) | is **conditionally** trivially relocatable if the condition is true |
/// | [`sus_class_trivially_relocatable_unchecked`]($sus_class_trivially_relocatable_unchecked) | is trivially relocatable without any condition or assertion |
///
/// # Example
/// ```
/// struct S {
///   Thing<i32> thing;
///   i32 more;
///
///   sus_class_trivially_relocatable(
///       unsafe_fn,
///       decltype(thing),
///       decltype(more));
/// };
/// ```
#define sus_class_trivially_relocatable(unsafe_fn, ...)               \
  static_assert(std::is_same_v<decltype(unsafe_fn),                   \
                               const ::sus::marker::UnsafeFnMarker>); \
  sus_class_trivially_relocatable_if_types(unsafe_fn, __VA_ARGS__);   \
  static_assert(SusUnsafeTrivialRelocate, "Type is not trivially relocatable")

/// Mark a class as trivially relocatable if the types passed as arguments are
/// all trivially relocatable.
///
/// This macro is most useful in templates where the template parameter types
/// are unknown and can be passed to the macro to determine if they are
/// trivially relocatable.
///
/// Avoid marking the class with the `[[clang::trivial_abi]]` attribute, as when
/// the class is not trivially relocatable (since a subtype is not trivially
/// relocatable), it can cause memory safety bugs and Undefined Behaviour.
///
/// Use the [`TriviallyRelocatable`]($sus::mem::TriviallyRelocatable) concept to
/// determine if a type is trivially relocatable, and to test with static_assert
/// that types are matching what you are expecting. This allows containers to
/// optimize their implementations when relocating the type in memory.
///
/// | Macro | Style |
/// | ----- | ----- |
/// | [`sus_class_trivially_relocatable`]($sus_class_trivially_relocatable) | **asserts** all param types are trivially relocatable |
/// | [`sus_class_trivially_relocatable_if_types`]($sus_class_trivially_relocatable_if_types) | is **conditionally** trivially relocatable if all param types are |
/// | [`sus_class_trivially_relocatable_if`]($sus_class_trivially_relocatable_if) | is **conditionally** trivially relocatable if the condition is true |
/// | [`sus_class_trivially_relocatable_unchecked`]($sus_class_trivially_relocatable_unchecked) | is trivially relocatable without any condition or assertion |
///
/// # Example
/// ```
/// template <class T>
/// struct S {
///   Thing<i32> thing;
///   i32 more;
///
///   sus_class_trivially_relocatable_if_types(
///       unsafe_fn,
///       decltype(thing),
///       decltype(more));
/// };
/// ```
#define sus_class_trivially_relocatable_if_types(unsafe_fn, ...)      \
  static_assert(std::is_same_v<decltype(unsafe_fn),                   \
                               const ::sus::marker::UnsafeFnMarker>); \
  template <class SusOuterClassTypeForTriviallyReloc>                 \
  friend struct ::sus::mem::__private::relocatable_tag;               \
  /** #[doc.hidden] */                                                \
  static constexpr bool SusUnsafeTrivialRelocate =                    \
      ::sus::mem::TriviallyRelocatable<__VA_ARGS__>

/// Mark a class as trivially relocatable based on a compile-time condition.
///
/// This macro is most useful in templates where the condition is based on the
/// template parameters.
///
/// Avoid marking the class with the `[[clang::trivial_abi]]` attribute, as when
/// the class is not trivially relocatable (the value is false), it can cause
/// memory safety bugs and Undefined Behaviour.
///
/// Use the [`TriviallyRelocatable`]($sus::mem::TriviallyRelocatable) concept to
/// determine if a type is trivially relocatable, and to test with static_assert
/// that types are matching what you are expecting. This allows containers to
/// optimize their implementations when relocating the type in memory.
///
/// | Macro | Style |
/// | ----- | ----- |
/// | [`sus_class_trivially_relocatable`]($sus_class_trivially_relocatable) | **asserts** all param types are trivially relocatable |
/// | [`sus_class_trivially_relocatable_if_types`]($sus_class_trivially_relocatable_if_types) | is **conditionally** trivially relocatable if all param types are |
/// | [`sus_class_trivially_relocatable_if`]($sus_class_trivially_relocatable_if) | is **conditionally** trivially relocatable if the condition is true |
/// | [`sus_class_trivially_relocatable_unchecked`]($sus_class_trivially_relocatable_unchecked) | is trivially relocatable without any condition or assertion |
///
/// # Example
/// ```
/// template <class T>
/// struct S {
///   Thing<i32> thing;
///   i32 more;
///
///   sus_class_trivially_relocatable_if(
///       unsafe_fn,
///       SomeCondition<Thing<T>>
///       && sus::mem::TriviallyRelocatable<decltype(thing)>
///       && sus::mem::TriviallyRelocatable<decltype(more)>);
/// };
/// ```
#define sus_class_trivially_relocatable_if(unsafe_fn, is_trivially_reloc)    \
  static_assert(std::is_same_v<decltype(unsafe_fn),                          \
                               const ::sus::marker::UnsafeFnMarker>);        \
  static_assert(                                                             \
      std::is_same_v<std::remove_cv_t<decltype(is_trivially_reloc)>, bool>); \
  template <class SusOuterClassTypeForTriviallyReloc>                        \
  friend struct ::sus::mem::__private::relocatable_tag;                      \
  static constexpr bool SusUnsafeTrivialRelocate = is_trivially_reloc

/// Mark a class as unconditionally trivially relocatable, without any
/// additional assertion to help verify correctness.
///
/// Generally, prefer to use sus_class_trivially_relocatable() with
/// all field types passed to the macro.
/// To additionally allow the class to be passed in registers, the class can be
/// marked with the `[[clang::trivial_abi]]` attribute.
///
/// Use the [`TriviallyRelocatable`]($sus::mem::TriviallyRelocatable) concept to
/// determine if a type is trivially relocatable, and to test with static_assert
/// that types are matching what you are expecting. This allows containers to
/// optimize their implementations when relocating the type in memory.
///
/// | Macro | Style |
/// | ----- | ----- |
/// | [`sus_class_trivially_relocatable`]($sus_class_trivially_relocatable) | **asserts** all param types are trivially relocatable |
/// | [`sus_class_trivially_relocatable_if_types`]($sus_class_trivially_relocatable_if_types) | is **conditionally** trivially relocatable if all param types are |
/// | [`sus_class_trivially_relocatable_if`]($sus_class_trivially_relocatable_if) | is **conditionally** trivially relocatable if the condition is true |
/// | [`sus_class_trivially_relocatable_unchecked`]($sus_class_trivially_relocatable_unchecked) | is trivially relocatable without any condition or assertion |
///
/// # Example
/// ```
/// struct S {
///   Thing<i32> thing;
///   i32 more;
///
///   sus_class_trivially_relocatable_unchecked(unsafe_fn);
/// };
/// ```
#define sus_class_trivially_relocatable_unchecked(unsafe_fn)          \
  static_assert(std::is_same_v<decltype(unsafe_fn),                   \
                               const ::sus::marker::UnsafeFnMarker>); \
  template <class SusOuterClassTypeForTriviallyReloc>                 \
  friend struct ::sus::mem::__private::relocatable_tag;               \
  static constexpr bool SusUnsafeTrivialRelocate = true
