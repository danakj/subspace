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

#include <stdlib.h>

#include <source_location>

#include "sus/assertions/panic.h"
#include "sus/macros/inline.h"

namespace sus::assertions {

/// Verifies that `cond` is true, and will [`panic`]($sus::assertions::panic)
/// otherwise, terminating the program.
///
/// See [`check_with_message`]($sus::assertions::check_with_message) to add a
/// message to the display of the panic.
///
/// The displayed output can be controlled by overriding the behaviour of
/// [`panic`]($sus::assertions::panic) as described there.
constexpr sus_always_inline void check(
    bool cond,
    const std::source_location location = std::source_location::current()) {
  if (!cond) [[unlikely]]
    ::sus::panic(location);
}

/// Verifies that `cond` is true, and will
/// [`panic_with_message`]($sus::assertions::panic_with_message)
/// otherwise, terminating the program.
///
/// Use [`check`]($sus::assertions::check) when there's nothing useful to add
/// in the message.
///
/// The displayed output can be controlled by overriding the behaviour of
/// [`panic`]($sus::assertions::panic) as described there.
constexpr sus_always_inline void check_with_message(
    bool cond, const char* msg,
    const std::source_location location = std::source_location::current()) {
  if (!cond) [[unlikely]]
    ::sus::panic_with_message(msg, location);
}

constexpr sus_always_inline void check_with_message(
    bool cond, std::string_view msg,
    const std::source_location location = std::source_location::current()) {
  if (!cond) [[unlikely]]
    ::sus::panic_with_message(msg, location);
}

constexpr sus_always_inline void check_with_message(
    bool cond, const std::string& msg,
    const std::source_location location = std::source_location::current()) {
  if (!cond) [[unlikely]]
    ::sus::panic_with_message(msg, location);
}

}  // namespace sus::assertions

// Promote check() and check_with_message() into the `sus` namespace.
namespace sus {
using ::sus::assertions::check;
using ::sus::assertions::check_with_message;
}  // namespace sus
