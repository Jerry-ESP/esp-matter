// Copyright 2025 Espressif Systems (Shanghai) PTE LTD
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

#include <esp_check.h>
#include <esp_err.h>
#include <mbedtls/base64.h>
#include <string.h>

#include <attestation_verification_utils.h>

constexpr char *TAG = "attestation_verification";

namespace esp_matter {
namespace controller {
namespace attestation_verification {

void remove_backslash_n(char *str)
{
    char *src = str, *dst = str;
    while (*src) {
        if (*src == '\\' && *(src + 1) == 'n' && *(src + 1) != '\0') {
            src += 2;
        } else {
            *dst++ = *src++;
        }
    }
    *dst = '\0';
}

esp_err_t convert_pem_to_der(const char *pem, uint8_t *der_buf, size_t *der_len)
{
    ESP_RETURN_ON_FALSE(pem && strlen(pem) > 0, ESP_ERR_INVALID_ARG, TAG, "pem cannot be NULL");
    ESP_RETURN_ON_FALSE(der_buf && der_len, ESP_ERR_INVALID_ARG, TAG, "der_buf cannot be NULL");
    size_t pem_len = strlen(pem);
    size_t len = 0;
    const char *s1, *s2, *end = pem + pem_len;
    constexpr char *begin_mark = "-----BEGIN";
    constexpr char *end_mark = "-----END";
    s1 = (char *)strstr(pem, begin_mark);
    if (s1 == NULL) {
        return ESP_FAIL;
    }
    s2 = (char *)strstr(pem, end_mark);
    if (s2 == NULL) {
        return ESP_FAIL;
    }
    s1 += strlen(begin_mark);
    while (s1 < end && *s1 != '-') {
        s1++;
    }
    while (s1 < end && *s1 == '-') {
        s1++;
    }
    if (*s1 == '\r') {
        s1++;
    }
    if (*s1 == '\n') {
        s1++;
    }
    int ret = mbedtls_base64_decode(NULL, 0, &len, (const unsigned char *)s1, s2 - s1);
    if (ret == MBEDTLS_ERR_BASE64_INVALID_CHARACTER) {
        return ESP_FAIL;
    }
    if (len > *der_len) {
        return ESP_FAIL;
    }
    if ((ret = mbedtls_base64_decode(der_buf, len, &len, (const unsigned char *)s1, s2 - s1)) != 0) {
        return ESP_FAIL;
    }
    *der_len = len;
    return ESP_OK;
}

} // namespace attestation_verification
} // namespace controller
} // namespace esp_matter
