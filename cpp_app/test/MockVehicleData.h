/***********************************************
*
*   Mock implementation of VehicleDataSource
*
***********************************************/
#ifndef MOCK_VEHICLE_DATA_H
#define MOCK_VEHICLE_DATA_H

#include "VehicleDataSource.h"

class MockVehicleData : public VehicleDataSource
{
public:
    VehicleData getData() final 
    {
        VehicleData data;
        return data;
    }
};

#endif