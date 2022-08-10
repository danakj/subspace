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

#include <math.h>

#include "num/float.h"
#include "third_party/googletest/googletest/include/gtest/gtest.h"

TEST(f32, Consts) {
  {
    constexpr auto min = f32::MIN();
    static_assert(std::same_as<decltype(min), const f32>);
    EXPECT_EQ(min.primitive_value, -FLT_MAX);
    constexpr auto max = f32::MAX();
    static_assert(std::same_as<decltype(max), const f32>);
    EXPECT_EQ(max.primitive_value, FLT_MAX);
  }
  {
    constexpr auto min = f32::MIN_PRIMITIVE;
    static_assert(std::same_as<decltype(min), const float>);
    EXPECT_EQ(min, -FLT_MAX);
    constexpr auto max = f32::MAX_PRIMITIVE;
    static_assert(std::same_as<decltype(max), const float>);
    EXPECT_EQ(max, FLT_MAX);
  }

  EXPECT_EQ(f32::RADIX(), 2_u32);
  EXPECT_EQ(f32::MANTISSA_DIGITS(), 24_u32);
  EXPECT_EQ(f32::DIGITS(), 6_u32);
  EXPECT_EQ(f32::EPSILON(), f32(FLT_EPSILON));
  EXPECT_EQ(f32::MIN(), f32(-FLT_MAX));
  EXPECT_EQ(f32::MAX(), f32(FLT_MAX));
  EXPECT_EQ(f32::MIN_POSITIVE(), f32(FLT_MIN));
  EXPECT_EQ(f32::MIN_EXP(), -125_i32);
  EXPECT_EQ(f32::MAX_EXP(), 128_i32);
  EXPECT_EQ(f32::MIN_10_EXP(), -37_i32);
  EXPECT_EQ(f32::MAX_10_EXP(), 38_i32);
  EXPECT_TRUE(isnan(f32::TODO_NAN().primitive_value));
  EXPECT_TRUE(isinf(f32::TODO_INFINITY().primitive_value));
  EXPECT_GT(f32::TODO_INFINITY(), 0_f32);
  EXPECT_TRUE(isinf(f32::NEG_INFINITY().primitive_value));
  EXPECT_LT(f32::NEG_INFINITY(), 0_f32);
}

TEST(f32, Literals) {
  static_assert((0._f32).primitive_value == 0.f);
  static_assert((0.0_f32).primitive_value == 0.f);
  static_assert((1.234_f32).primitive_value == 1.234f);
  static_assert((-1.234_f32).primitive_value == -1.234f);

  // Whole numbers.
  static_assert((0_f32).primitive_value == 0.f);
  static_assert((1_f32).primitive_value == 1.f);
  static_assert((-5_f32).primitive_value == -5.f);
}

TEST(f32, ConstructPrimitive) {
  auto a = f32();
  EXPECT_EQ(a.primitive_value, 0.f);

  f32 b;
  EXPECT_EQ(b.primitive_value, 0.f);

  auto c = f32(1.2f);
  EXPECT_EQ(c.primitive_value, 1.2f);
}

TEST(f32, AssignPrimitive) {
  auto a = f32();
  EXPECT_EQ(a.primitive_value, 0.f);
  a = 1.2f;
  EXPECT_EQ(a.primitive_value, 1.2f);
}
