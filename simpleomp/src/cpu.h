// Tencent is pleased to support the open source community by making ncnn available.
//
// Copyright (C) 2017 THL A29 Limited, a Tencent company. All rights reserved.
//
// Licensed under the BSD 3-Clause License (the "License"); you may not use this file except
// in compliance with the License. You may obtain a copy of the License at
//
// https://opensource.org/licenses/BSD-3-Clause
//
// Unless required by applicable law or agreed to in writing, software distributed
// under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
// CONDITIONS OF ANY KIND, either express or implied. See the License for the
// specific language governing permissions and limitations under the License.

// I took what I needed for simpleomp from:
// https://raw.githubusercontent.com/Tencent/ncnn/9e11dac7d17aae3c600d30de471b57478459a624/src/cpu.h

#ifndef NCNN_CPU_H
#define NCNN_CPU_H

#ifndef __EMSCRIPTEN__
#error This is Emscripten specific!
#endif

#include <assert.h>
#include <emscripten/threading.h>

namespace ncnn { static int inline get_cpu_count() {
    int count;
    if (emscripten_has_threading_support())
        count = emscripten_num_logical_cores();
    else
        count = 1;
    assert(count >= 1);
    return count;
}}

#endif