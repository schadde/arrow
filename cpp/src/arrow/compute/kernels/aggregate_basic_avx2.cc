// Licensed to the Apache Software Foundation (ASF) under one
// or more contributor license agreements.  See the NOTICE file
// distributed with this work for additional information
// regarding copyright ownership.  The ASF licenses this file
// to you under the Apache License, Version 2.0 (the
// "License"); you may not use this file except in compliance
// with the License.  You may obtain a copy of the License at
//
//   http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing,
// software distributed under the License is distributed on an
// "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
// KIND, either express or implied.  See the License for the
// specific language governing permissions and limitations
// under the License.

#include "arrow/compute/kernels/aggregate_basic_internal.h"

namespace arrow {
namespace compute {
namespace aggregate {

// ----------------------------------------------------------------------
// Sum implementation

template <typename ArrowType>
struct SumImplAvx2 : public SumImpl<ArrowType, SimdLevel::AVX2> {
  using SumImpl<ArrowType, SimdLevel::AVX2>::SumImpl;
};

template <typename ArrowType>
struct MeanImplAvx2 : public MeanImpl<ArrowType, SimdLevel::AVX2> {
  using MeanImpl<ArrowType, SimdLevel::AVX2>::MeanImpl;
};

Result<std::unique_ptr<KernelState>> SumInitAvx2(KernelContext* ctx,
                                                 const KernelInitArgs& args) {
  SumLikeInit<SumImplAvx2> visitor(
      ctx, args.inputs[0].type,
      static_cast<const ScalarAggregateOptions&>(*args.options));
  return visitor.Create();
}

Result<std::unique_ptr<KernelState>> MeanInitAvx2(KernelContext* ctx,
                                                  const KernelInitArgs& args) {
  SumLikeInit<MeanImplAvx2> visitor(
      ctx, args.inputs[0].type,
      static_cast<const ScalarAggregateOptions&>(*args.options));
  return visitor.Create();
}

// ----------------------------------------------------------------------
// MinMax implementation

Result<std::unique_ptr<KernelState>> MinMaxInitAvx2(KernelContext* ctx,
                                                    const KernelInitArgs& args) {
  MinMaxInitState<SimdLevel::AVX2> visitor(
      ctx, *args.inputs[0].type, args.kernel->signature->out_type().type(),
      static_cast<const ScalarAggregateOptions&>(*args.options));
  return visitor.Create();
}

void AddSumAvx2AggKernels(ScalarAggregateFunction* func) {
  AddBasicAggKernels(SumInitAvx2, internal::SignedIntTypes(), int64(), func,
                     SimdLevel::AVX2);
  AddBasicAggKernels(SumInitAvx2, internal::UnsignedIntTypes(), uint64(), func,
                     SimdLevel::AVX2);
  AddBasicAggKernels(SumInitAvx2, internal::FloatingPointTypes(), float64(), func,
                     SimdLevel::AVX2);
}

void AddMeanAvx2AggKernels(ScalarAggregateFunction* func) {
  AddBasicAggKernels(MeanInitAvx2, internal::NumericTypes(), float64(), func,
                     SimdLevel::AVX2);
}

void AddMinMaxAvx2AggKernels(ScalarAggregateFunction* func) {
  // Enable int types for AVX2 variants.
  // No auto vectorize for float/double as it use fmin/fmax which has NaN handling.
  AddMinMaxKernels(MinMaxInitAvx2, internal::IntTypes(), func, SimdLevel::AVX2);
}

}  // namespace aggregate
}  // namespace compute
}  // namespace arrow
