#include "Dashboard.h"
#include "driver/spi_common.h"
#include "esp_lcd_ili9341.h"
#include "esp_lcd_io_spi.h"
#include "esp_lcd_panel_dev.h"
#include "esp_lcd_panel_ops.h"
#include "esp_log.h"
#include "esp_lvgl_port.h"
#include "hal/spi_types.h"
#include "soc/gpio_num.h"
#include "widgets/bar/lv_bar.h"
#include <cstddef>
#include <cstdint>
#include <string>

namespace
{
    constexpr spi_host_device_t SPI_HOST = SPI2_HOST;

    constexpr gpio_num_t PIN_SCLK = GPIO_NUM_18;
    constexpr gpio_num_t PIN_MOSI = GPIO_NUM_23;
    constexpr gpio_num_t PIN_CS   = GPIO_NUM_5;
    constexpr gpio_num_t PIN_DC   = GPIO_NUM_2;
    constexpr gpio_num_t PIN_RST  = GPIO_NUM_4;

    constexpr int SCREEN_WIDTH = 240;
    constexpr int SCREEN_HEIGHT = 320;
    constexpr int BUFFER_LINES = 20;

    const char* TAG = "mini_dashboard";
}

Dashboard::Dashboard()
    : m_engineLoad("0"),
      m_intakeTemp("0"),
      m_coolantTemp("0")
{}

void Dashboard::initialise()
{
    ESP_LOGI(TAG, "Initialising Mini Dashboard");

    // Initialise the SPI bus.
    spi_bus_config_t busConfig{};
    busConfig.sclk_io_num = PIN_SCLK;
    busConfig.mosi_io_num = PIN_MOSI;
    busConfig.miso_io_num = -1;
    busConfig.quadwp_io_num = -1;
    busConfig.quadhd_io_num = -1;
    busConfig.max_transfer_sz = SCREEN_WIDTH * BUFFER_LINES * sizeof(uint16_t);

    ESP_ERROR_CHECK(
        spi_bus_initialize(SPI_HOST, &busConfig, SPI_DMA_CH_AUTO)
    );

    // Configure communication with the LCD.
    esp_lcd_panel_io_spi_config_t ioConfig{};
    ioConfig.cs_gpio_num = PIN_CS;
    ioConfig.dc_gpio_num = PIN_DC;
    ioConfig.spi_mode = 0;
    ioConfig.pclk_hz = 10 * 1000 * 1000;
    ioConfig.trans_queue_depth = 4;
    ioConfig.lcd_cmd_bits = 8;
    ioConfig.lcd_param_bits = 8;

    ESP_ERROR_CHECK(
        esp_lcd_new_panel_io_spi(
            static_cast<esp_lcd_spi_bus_handle_t>(SPI_HOST),
            &ioConfig,
            &m_ioHandle
        )
    );

     // Initialise the ILI9341 panel.
    esp_lcd_panel_dev_config_t panelConfig{};
    panelConfig.reset_gpio_num = PIN_RST;
    panelConfig.rgb_ele_order = LCD_RGB_ELEMENT_ORDER_RGB;
    panelConfig.bits_per_pixel = 16;

    ESP_ERROR_CHECK(
        esp_lcd_new_panel_ili9341(
            m_ioHandle,
            &panelConfig,
            &m_panelHandle
        )
    );

    ESP_ERROR_CHECK(esp_lcd_panel_reset(m_panelHandle));
    ESP_ERROR_CHECK(esp_lcd_panel_init(m_panelHandle));

    // Diagnostic: no mirroring on either axis
    ESP_ERROR_CHECK(
        esp_lcd_panel_mirror(m_panelHandle, true, false)
    );

    ESP_ERROR_CHECK(esp_lcd_panel_disp_on_off(m_panelHandle, true));

    // 4. Initialise LVGL.
    const lvgl_port_cfg_t lvglConfig =
        ESP_LVGL_PORT_INIT_CONFIG();

    ESP_ERROR_CHECK(lvgl_port_init(&lvglConfig));

    // 5. Connect LVGL to the existing LCD panel.
    lvgl_port_display_cfg_t displayConfig{};

    displayConfig.io_handle = m_ioHandle;
    displayConfig.panel_handle = m_panelHandle;
    displayConfig.buffer_size = SCREEN_WIDTH * BUFFER_LINES;
    displayConfig.double_buffer = true;
    displayConfig.hres = SCREEN_WIDTH;
    displayConfig.vres = SCREEN_HEIGHT;
    displayConfig.monochrome = false;
    displayConfig.color_format = LV_COLOR_FORMAT_RGB565;

    displayConfig.rotation.swap_xy = false;
    displayConfig.rotation.mirror_x = true;
    displayConfig.rotation.mirror_y = false;

    displayConfig.flags.buff_dma = true;
    displayConfig.flags.swap_bytes = false;

    m_display = lvgl_port_add_disp(&displayConfig);

    if (m_display == nullptr)
    {
        ESP_LOGE(TAG, "Failed to register LVGL display");
        abort();
    }

    // 6. Create the dashboard UI.
    if (lvgl_port_lock(0))
    {
        m_screen = lv_screen_active();

        if (m_screen == nullptr)
        {
            ESP_LOGE(TAG, "Failed to get active LVGL screen");
            lvgl_port_unlock();
            abort();
        }

        // Create Labels;
        m_engineLoadHandle = lv_label_create(m_screen);
        m_engineLoadBarHandle = lv_bar_create(m_screen);
        m_intakeTempHandle = lv_label_create(m_screen);
        m_coolantTempHandle = lv_label_create(m_screen);
        
        // Black background.
        lv_obj_set_style_bg_color(
            m_screen,
            lv_color_hex(0x000000),
            LV_PART_MAIN
        );
        lv_obj_set_style_bg_opa(
            m_screen,
            LV_OPA_COVER,
            LV_PART_MAIN
        );

        // Dashboard title.
        m_dashboardTitle = lv_label_create(m_screen);
        lv_label_set_text(m_dashboardTitle, "MINI DASHBOARD");

        lv_obj_set_style_text_color(
            m_dashboardTitle,
            lv_color_hex(0xFFFFFF),
            LV_PART_MAIN
        );

        lv_obj_set_style_text_font(
            m_dashboardTitle,
            &lv_font_montserrat_24,
            LV_PART_MAIN
        );

        lv_obj_align(m_dashboardTitle, LV_ALIGN_TOP_MID, 0, 15);

        ESP_LOGI(TAG, "Dashboard title created");

        // Engine Load Label
        m_engineLoadTitle = lv_label_create(m_screen);
        lv_label_set_text(m_engineLoadTitle, "ENGINE LOAD");

        lv_obj_set_style_text_color(
            m_engineLoadTitle,
            lv_color_hex(0xFFFFFF),
            LV_PART_MAIN
        );

        lv_obj_set_style_text_font(
            m_engineLoadTitle,
            &lv_font_montserrat_18,
            LV_PART_MAIN
        );

        lv_obj_align(m_engineLoadTitle, LV_ALIGN_TOP_MID, 0, 70);

        // Engine Load Value
        lv_label_set_text(m_engineLoadHandle, "0%");
        lv_obj_align(m_engineLoadHandle, LV_ALIGN_TOP_MID, 0, 95);

        // Engine Load bar
        lv_obj_set_size(m_engineLoadBarHandle, 200, 10);
        lv_obj_align(m_engineLoadBarHandle, LV_ALIGN_TOP_MID, 0, 110);
        lv_bar_set_range(m_engineLoadBarHandle, 0, 100);
        lv_bar_set_value(m_engineLoadBarHandle, 70, LV_ANIM_OFF);

        lv_obj_set_style_bg_color(
            m_engineLoadBarHandle, lv_color_hex(0x253141), LV_PART_MAIN);

        // Filled section.
        lv_obj_set_style_bg_color(
        m_engineLoadBarHandle, lv_color_hex(0x38D996), LV_PART_INDICATOR);

        // Intake Temp
        m_intakeTitle = lv_label_create(m_screen);
        lv_label_set_text(m_intakeTitle, "INTAKE TEMPERATURE");

        lv_obj_set_style_text_color(
            m_intakeTitle,
            lv_color_hex(0xFFFFFF),
            LV_PART_MAIN
        );

        lv_obj_set_style_text_font(
            m_intakeTitle,
            &lv_font_montserrat_18,
            LV_PART_MAIN
        );

        lv_obj_align(m_intakeTitle, LV_ALIGN_TOP_MID, 0, 140);

        // Intake Value
        lv_label_set_text(m_intakeTempHandle, "62°");
        lv_obj_align(m_intakeTempHandle, LV_ALIGN_TOP_MID, 0, 170);

        // Coolant Temp
        m_coolantTitle = lv_label_create(m_screen);
        lv_label_set_text(m_coolantTitle, "COOLANT TEMPERATURE");

        lv_obj_set_style_text_color(
            m_coolantTitle,
            lv_color_hex(0xFFFFFF),
            LV_PART_MAIN
        );

        lv_obj_set_style_text_font(
            m_coolantTitle,
            &lv_font_montserrat_18,
            LV_PART_MAIN
        );

        lv_obj_align(m_coolantTitle, LV_ALIGN_TOP_MID, 0, 210);

        // Coolant Value
        lv_label_set_text(m_coolantTempHandle, "0°");
        lv_obj_align(m_coolantTempHandle, LV_ALIGN_TOP_MID, 0, 240);

        lvgl_port_unlock();
    }
    else
    {
        ESP_LOGE(TAG, "Could not lock LVGL");
        abort();
    }
}

void Dashboard::updateEngineLoad(const uint8_t& val)
{
    const int percent = (val > 100) ? 100 : val;

    m_engineLoad = std::to_string(percent) + "%";

    if (lvgl_port_lock(0))
    {
        lv_label_set_text(
            m_engineLoadHandle,
            m_engineLoad.c_str()
        );

        lv_bar_set_value(
            m_engineLoadBarHandle,
            percent,
            LV_ANIM_OFF
        );

        lvgl_port_unlock();
    }
    else
    {
        ESP_LOGE(TAG, "Could not lock LVGL");
        abort();
    }
}

void Dashboard::updateIntakeTemp(const uint8_t& val)
{
    m_intakeTemp = std::to_string(val) + "°";
    if (lvgl_port_lock(0))
    {
        lv_label_set_text(m_intakeTempHandle, m_intakeTemp.c_str());
        lvgl_port_unlock();
    }
    else
    {
        ESP_LOGE(TAG, "Could not lock LVGL");
        abort();
    }
}

void Dashboard::updateCoolantTemp(const uint8_t& val)
{
    m_coolantTemp = std::to_string(val) + "°";
    if (lvgl_port_lock(0))
    {
        lv_label_set_text(m_coolantTempHandle, m_coolantTemp.c_str());
        lvgl_port_unlock();
    }
    else
    {
        ESP_LOGE(TAG, "Could not lock LVGL");
        abort();
    }
}