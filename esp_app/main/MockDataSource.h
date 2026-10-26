#include "IVehicleDataSource.h"

class MockDataSource : public IVehicleDataSource
{
    VehicleData getData() override final
    {
        VehicleData data;
        return data;
    }
};