#include "misc/lv_types.h"
#include <cstdint>
#include <string>

class Dashboard
{
public:
    Dashboard();

    void initialise();

    void updateEngineLoad(uint8_t& val);

    void updateIntakeTemp(uint8_t& val);

    void updateCoolantTemp(uint8_t& val);

private:
    std::string m_engineLoad;
    std::string m_intakeTemp;
    std::string m_coolantTemp;

    lv_obj_t* m_engineLoadHandle;
    lv_obj_t* m_intakeTempHandle;
    lv_obj_t* m_coolantTempHandle;

    lv_obj_t* m_screen;
};