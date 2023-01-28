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

#include "subdoc/llvm.h"
#include "subspace/choice/choice.h"
#include "subspace/containers/vec.h"
#include "subspace/option/option.h"
#include "subspace/prelude.h"

namespace subdoc {

enum InheritPathElementType {
  InheritPathNamespace,
  InheritPathRecord,
  InheritPathFunction,
};

using InheritPathElement =
    sus::Choice<sus_choice_types((InheritPathNamespace, std::string),  //
                                 (InheritPathRecord, std::string),     //
                                 (InheritPathFunction, std::string))>;

struct DocAttributes {
  sus::Option<u64> overload_set;
  clang::SourceLocation location;
  sus::Option<sus::Vec<InheritPathElement>> inherit;

  DocAttributes clone() const {
    return DocAttributes(sus::clone(overload_set),  //
                         sus::clone(location),      //
                         sus::clone(inherit));
  }
};

}  // namespace subdoc