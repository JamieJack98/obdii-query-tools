#include "Dashboard.h"

extern "C" void app_main(void)
{
    static Dashboard dashboard;

    dashboard.initialise();

    dashboard.updateCoolantTemp(45);
    dashboard.updateIntakeTemp(23);
    dashboard.updateEngineLoad(80);
}
    
