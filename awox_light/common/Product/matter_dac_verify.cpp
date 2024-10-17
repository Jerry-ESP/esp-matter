// Copyright 2022 Espressif Systems (Shanghai) PTE LTD
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

/** TODO: Temporarily disabling it. Need to make this generalised and fix it. */
#if 1

#include <string.h>
#include <esp_mac.h>
#include <esp_log.h>
#include <crypto/CHIPCryptoPAL.h>
#include <credentials/attestation_verifier/DeviceAttestationVerifier.h>
#include <credentials/CHIPCert.h>
#include <lib/support/Span.h>
#include <lib/core/CHIPError.h>
#include <lib/support/logging/CHIPLogging.h>
#include <platform/ESP32/ESP32DeviceInfoProvider.h>
#include <platform/ESP32/ESP32FactoryDataProvider.h>
#include <platform/ESP32/ESP32SecureCertDACProvider.h>
#include <credentials/examples/DeviceAttestationCredsExample.h>

#include <esp_flash_encrypt.h>
#include <esp_secure_boot.h>
#include <esp_matter.h>
#include <esp_matter_providers.h>
#include <matter_dac_verify.h>

#include <mbedtls/pk.h>

using namespace chip;
using namespace chip::Crypto;
using namespace chip::DeviceLayer;
using namespace chip::Credentials;

const char *TAG = "dac-verify";

uint8_t s_dac_cert_buffer[kMaxDERCertLength];  // 600 bytes

MutableByteSpan dac_span;

DeviceAttestationCredentialsProvider * get_dac_provider(void)
{
#if CONFIG_SEC_CERT_DAC_PROVIDER
    static ESP32SecureCertDACProvider instance;
    return &instance;
#elif CONFIG_FACTORY_PARTITION_DAC_PROVIDER
    static ESP32FactoryDataProvider instance;
    return &instance;
#else // CONFIG_EXAMPLE_DAC_PROVIDER
    return Examples::GetExampleDACProvider();
#endif
}

CHIP_ERROR dump_dac_cert_details()
{
    char *type = "DAC";
    MutableByteSpan dac_span;

    // DAC Provider implementation
    DeviceAttestationCredentialsProvider * dac_provider = get_dac_provider();
    VerifyOrReturnError(dac_provider, CHIP_ERROR_INTERNAL, ESP_LOGE(TAG, "ERROR: Failed to get the DAC provider impl"));

    // Read DAC
    dac_span = MutableByteSpan(s_dac_cert_buffer);
    CHIP_ERROR err = dac_provider->GetDeviceAttestationCert(dac_span);
    VerifyOrReturnError(err == CHIP_NO_ERROR, err, ESP_LOGE(TAG, "ERROR: Failed to read the DAC, %" CHIP_ERROR_FORMAT, err.Format()));

    ESP_LOGI(TAG, "---------- %s ----------", type);

    // Get VID, PID from the certificate
    AttestationCertVidPid vidpid;
    err = ExtractVIDPIDFromX509Cert(dac_span, vidpid);
    VerifyOrReturnError(err == CHIP_NO_ERROR, err, ESP_LOGE(TAG, "ERROR: Failed to extract VID and PID, error: %" CHIP_ERROR_FORMAT, err.Format()));

    if (vidpid.mVendorId.HasValue()) {
        ESP_LOGI(TAG, "Vendor ID: 0x%04X", vidpid.mVendorId.Value());
    }

    if (vidpid.mProductId.HasValue()) {
        ESP_LOGI(TAG, "Product ID: 0x%04X", vidpid.mProductId.Value());
    }

    // Get Public key from the certificate
    P256PublicKey pubkey;
    err = ExtractPubkeyFromX509Cert(dac_span, pubkey);
    VerifyOrReturnError(err == CHIP_NO_ERROR, err, ESP_LOGE(TAG, "ERROR: Failed to extract public key, error: %" CHIP_ERROR_FORMAT, err.Format()));

    // Print public key
    ESP_LOGI(TAG, "Public Key encoded as hex string:");
    for (uint8_t i = 0; i < pubkey.Length(); i++) {
        printf("%02x", pubkey.ConstBytes()[i]);
    }
    printf("\n\n");

    // Get AKID from the certificate
    uint8_t akid_buffer[64];
    MutableByteSpan akid_span(akid_buffer);
    err = ExtractAKIDFromX509Cert(dac_span, akid_span);
    VerifyOrReturnError(err == CHIP_NO_ERROR, err, ESP_LOGE(TAG, "ERROR: Failed to extract AKID, error: %" CHIP_ERROR_FORMAT, err.Format()));

    // Print AKID
    ESP_LOGI(TAG, "X509v3 Authority Key Identifier:");
    printf("%02x", akid_span.data()[0]);
    for (uint8_t i = 1; i < akid_span.size(); i++) {
        printf(":%02X", akid_span.data()[i]);
    }
    printf("\n\n");

    // Get SKID from the certificate
    uint8_t skid_buffer[64];
    MutableByteSpan skid_span(skid_buffer);
    err = ExtractSKIDFromX509Cert(dac_span, skid_span);
    VerifyOrReturnError(err == CHIP_NO_ERROR, err, ESP_LOGE(TAG, "ERROR: Failed to extract SKID, error: %" CHIP_ERROR_FORMAT, err.Format()));

    // Print SKID
    ESP_LOGI(TAG, "X509v3 Subject Key Identifier:");
    printf("%02x", skid_span.data()[0]);
    for (uint8_t i = 1; i < skid_span.size(); i++) {
        printf(":%02X", skid_span.data()[i]);
    }
    printf("\n\n");

    ESP_LOGI(TAG, "------------------------------");
    return CHIP_NO_ERROR;
}

#endif