#include "input.h"
//#include "driver/adc.h"
#include "esp_adc/adc_oneshot.h"
#include "esp_adc/adc_cali.h"
#include "esp_adc/adc_cali_scheme.h"
#include "driver/gpio.h"
#include "esp_log.h"

#define SW_1 9
// #define X_1 0 // #define Y_1 1 // #define X_2 3 // #define Y_2 2
//GPIO0...GPIO3 => ADC1_0...ADC1_3

#define ADC_ATTEN           ADC_ATTEN_DB_12

const static char *TAG0 = "Read";
const static char *TAG1 = "Calib";
// static int adc_raw[4][10];
static int voltage[4][10];
static bool adc_calibration_init(adc_unit_t unit, adc_channel_t channel, adc_atten_t atten, adc_cali_handle_t *out_handle);
// static void adc_calibration_deinit(adc_cali_handle_t handle);

adc_oneshot_unit_handle_t adc1_handle;

adc_cali_handle_t adc1_cali_chan0_handle = NULL;
adc_cali_handle_t adc1_cali_chan1_handle = NULL;
adc_cali_handle_t adc1_cali_chan2_handle = NULL;
adc_cali_handle_t adc1_cali_chan3_handle = NULL;
bool do_calibration1_chan0;
bool do_calibration1_chan1;
bool do_calibration1_chan2;
bool do_calibration1_chan3;

void input_init(void) {
    //-------------ADC1 Init---------------//
    // adc_oneshot_unit_handle_t adc1_handle;
    adc_oneshot_unit_init_cfg_t init_config1 = {
        .unit_id = ADC_UNIT_1,
        .clk_src = ADC_DIGI_CLK_SRC_DEFAULT,
    };
    ESP_ERROR_CHECK(adc_oneshot_new_unit(&init_config1, &adc1_handle));

    //-------------ADC1 Config---------------//
    adc_oneshot_chan_cfg_t config = {
        .atten = ADC_ATTEN,
        .bitwidth = ADC_BITWIDTH_DEFAULT,
    };
    ESP_ERROR_CHECK(adc_oneshot_config_channel(adc1_handle, ADC_CHANNEL_0, &config));
    ESP_ERROR_CHECK(adc_oneshot_config_channel(adc1_handle, ADC_CHANNEL_1, &config));
    ESP_ERROR_CHECK(adc_oneshot_config_channel(adc1_handle, ADC_CHANNEL_2, &config));
    ESP_ERROR_CHECK(adc_oneshot_config_channel(adc1_handle, ADC_CHANNEL_3, &config));

    //-------------ADC1 Calibration Init---------------//
    do_calibration1_chan0 = adc_calibration_init(ADC_UNIT_1, ADC_CHANNEL_0, ADC_ATTEN, &adc1_cali_chan0_handle);
    do_calibration1_chan1 = adc_calibration_init(ADC_UNIT_1, ADC_CHANNEL_1, ADC_ATTEN, &adc1_cali_chan1_handle);
    do_calibration1_chan2 = adc_calibration_init(ADC_UNIT_1, ADC_CHANNEL_2, ADC_ATTEN, &adc1_cali_chan2_handle);
    do_calibration1_chan3 = adc_calibration_init(ADC_UNIT_1, ADC_CHANNEL_3, ADC_ATTEN, &adc1_cali_chan3_handle);

//    adc1_config_width(ADC_WIDTH_BIT_12);
//    adc1_config_channel_atten(X_1, ADC_ATTEN_DB_11);
//    adc1_config_channel_atten(Y_1, ADC_ATTEN_DB_11);
//    adc1_config_channel_atten(X_2, ADC_ATTEN_DB_11);
//    adc1_config_channel_atten(Y_2, ADC_ATTEN_DB_11);

   gpio_config_t io_conf = {
       .pin_bit_mask = (1ULL << SW_1),
       .mode = GPIO_MODE_INPUT,
   };
   gpio_config(&io_conf);
   gpio_pullup_en(SW_1);
}

JoystickData input_read(void) {

// Read Raw Data
    int raw_val0, raw_val1, raw_val2, raw_val3;
    ESP_ERROR_CHECK(adc_oneshot_read(adc1_handle, ADC_CHANNEL_0, &raw_val0));
    ESP_ERROR_CHECK(adc_oneshot_read(adc1_handle, ADC_CHANNEL_1, &raw_val1));
    ESP_ERROR_CHECK(adc_oneshot_read(adc1_handle, ADC_CHANNEL_2, &raw_val2));
    ESP_ERROR_CHECK(adc_oneshot_read(adc1_handle, ADC_CHANNEL_3, &raw_val3));

    ESP_LOGI(TAG0, "ADC%d Channel[%d] Raw Data: %d", ADC_UNIT_1 + 1, 0, raw_val0);
    ESP_LOGI(TAG0, "ADC%d Channel[%d] Raw Data: %d", ADC_UNIT_1 + 1, 1, raw_val1);
    ESP_LOGI(TAG0, "ADC%d Channel[%d] Raw Data: %d", ADC_UNIT_1 + 1, 2, raw_val2);
    ESP_LOGI(TAG0, "ADC%d Channel[%d] Raw Data: %d", ADC_UNIT_1 + 1, 3, raw_val3);

        if (do_calibration1_chan0) {
            ESP_ERROR_CHECK(adc_cali_raw_to_voltage(adc1_cali_chan0_handle, raw_val0, &voltage[0][0]));
            ESP_LOGI(TAG0, "ADC%d Channel[%d] Cali Voltage: %d mV", ADC_UNIT_1 + 1, 0, voltage[0][0]);
        }
        if (do_calibration1_chan1) {
            ESP_ERROR_CHECK(adc_cali_raw_to_voltage(adc1_cali_chan1_handle, raw_val1, &voltage[1][0]));
            ESP_LOGI(TAG0, "ADC%d Channel[%d] Cali Voltage: %d mV", ADC_UNIT_1 + 1, 1, voltage[1][0]);
        }
        if (do_calibration1_chan2) {
            ESP_ERROR_CHECK(adc_cali_raw_to_voltage(adc1_cali_chan2_handle, raw_val2, &voltage[2][0]));
            ESP_LOGI(TAG0, "ADC%d Channel[%d] Cali Voltage: %d mV", ADC_UNIT_1 + 1, 2, voltage[2][0]);
        }
        if (do_calibration1_chan3) {
            ESP_ERROR_CHECK(adc_cali_raw_to_voltage(adc1_cali_chan3_handle, raw_val3, &voltage[3][0]));
            ESP_LOGI(TAG0, "ADC%d Channel[%d] Cali Voltage: %d mV", ADC_UNIT_1 + 1, 3, voltage[3][0]);
        }

   return (JoystickData){
       .x1 = raw_val0,
       .y1 = raw_val1,
       .x2 = raw_val2,
       .y2 = raw_val3,
       .sw1 = gpio_get_level(SW_1)
   };
}

/*---------------------------------------------------------------
        ADC Calibration
---------------------------------------------------------------*/
static bool adc_calibration_init(adc_unit_t unit, adc_channel_t channel, adc_atten_t atten, adc_cali_handle_t *out_handle)
{
    adc_cali_handle_t handle = NULL;
    esp_err_t ret = ESP_FAIL;
    bool calibrated = false;

#if ADC_CALI_SCHEME_CURVE_FITTING_SUPPORTED
    if (!calibrated) {
        ESP_LOGI(TAG1, "calibration scheme version is %s", "Curve Fitting");
        adc_cali_curve_fitting_config_t cali_config = {
            .unit_id = unit,
            .chan = channel,
            .atten = atten,
            .bitwidth = ADC_BITWIDTH_DEFAULT,
        };
        ret = adc_cali_create_scheme_curve_fitting(&cali_config, &handle);
        if (ret == ESP_OK) {
            calibrated = true;
        }
    }
#endif

#if ADC_CALI_SCHEME_LINE_FITTING_SUPPORTED
    if (!calibrated) {
        ESP_LOGI(TAG1, "calibration scheme version is %s", "Line Fitting");
        adc_cali_line_fitting_config_t cali_config = {
            .unit_id = unit,
            .atten = atten,
            .bitwidth = ADC_BITWIDTH_DEFAULT,
        };
        ret = adc_cali_create_scheme_line_fitting(&cali_config, &handle);
        if (ret == ESP_OK) {
            calibrated = true;
        }
    }
#endif

    *out_handle = handle;
    if (ret == ESP_OK) {
        ESP_LOGI(TAG1, "Calibration Success");
    } else if (ret == ESP_ERR_NOT_SUPPORTED || !calibrated) {
        ESP_LOGW(TAG1, "eFuse not burnt, skip software calibration");
    } else {
        ESP_LOGE(TAG1, "Invalid arg or no memory");
    }

    return calibrated;
}

// static void adc_calibration_deinit(adc_cali_handle_t handle)
// {
// #if ADC_CALI_SCHEME_CURVE_FITTING_SUPPORTED
//     ESP_LOGI(TAG1, "deregister %s calibration scheme", "Curve Fitting");
//     ESP_ERROR_CHECK(adc_cali_delete_scheme_curve_fitting(handle));

// #elif ADC_CALI_SCHEME_LINE_FITTING_SUPPORTED
//     ESP_LOGI(TAG1, "deregister %s calibration scheme", "Line Fitting");
//     ESP_ERROR_CHECK(adc_cali_delete_scheme_line_fitting(handle));
// #endif
// }