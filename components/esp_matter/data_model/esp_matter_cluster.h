// Copyright 2021 Espressif Systems (Shanghai) PTE LTD
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#pragma once
#include <sdkconfig.h>
#if CONFIG_ESP_MATTER_ENABLE_GENERATED_DATA_MODEL
#include "generated/clusters/all_clusters.h"
#else
#include "legacy/esp_matter_cluster_impl.h"
#if defined(CONFIG_SUPPORT_HUMIDISTAT_CLUSTER)
#include "generated/clusters/humidistat/humidistat.h"
#endif
#endif
