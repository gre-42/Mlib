#
# Copyright (C) The Android Open Source Project
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

# now build app's shared lib
add_library(AGameHelper
  STATIC
    AContext.cpp
    AWindow.cpp
    ARenderLoop.cpp
    AEngine.cpp
    AssetUtil.cpp
)
set_target_properties(AGameHelper
  PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED YES
    CXX_EXTENSIONS NO
)

target_include_directories(AGameHelper PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../../../)

# add lib dependencies
target_link_libraries(AGameHelper PUBLIC NdkHelper)
