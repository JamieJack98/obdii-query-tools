idf_component_register(
    SRCS 
        "main.cpp"
        "Dashboard.cpp"
    INCLUDE_DIRS "."
    PRIV_REQUIRES 
        spi_flash 
        esp_driver_twai esp_lcd
        esp_lcd_ili9341
        esp_lvgl_port
)