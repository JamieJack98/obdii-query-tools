#pragma once

#include "esp_lcd_types.h"
#include "misc/lv_types.h"
#include <cstdint>
#include <string>

class Dashboard
{
public:
    Dashboard();

    void initialise();

    void updateEngineLoad(const uint8_t& val);

    void updateIntakeTemp(const uint8_t& val);

    void updateCoolantTemp(const uint8_t& val);

private:
    // Screen/display handles.
    lv_obj_t* m_screen{nullptr};
    lv_display_t* m_display{nullptr};

    // Panel handles.
    esp_lcd_panel_handle_t m_panelHandle{nullptr};
    esp_lcd_panel_io_handle_t m_ioHandle{nullptr};

    // Titles.
    lv_obj_t* m_dashboardTitle{nullptr};
    lv_obj_t* m_engineLoadTitle{nullptr};
    lv_obj_t* m_intakeTitle{nullptr};
    lv_obj_t* m_coolantTitle{nullptr};

    // Values.
    std::string m_engineLoad{"0"};
    std::string m_intakeTemp{"0"};
    std::string m_coolantTemp{"0"};

    // Display element handles.
    lv_obj_t* m_engineLoadHandle{nullptr};
    lv_obj_t* m_engineLoadBarHandle{nullptr};
    lv_obj_t* m_intakeTempHandle{nullptr};
    lv_obj_t* m_coolantTempHandle{nullptr};
};