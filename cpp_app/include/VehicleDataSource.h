/***********************************************
*
*   Class to represent a source of vehicle data
*
***********************************************/
#ifndef VEHICLE_DATA_SOURCE_H
#define VEHICLE_DATA_SOURCE_H
#include "VehicleData.h"

class VehicleDataSource
{
public:
    virtual VehicleData getData() = 0;
};

#endif