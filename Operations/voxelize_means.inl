#pragma once

#include "voxelize_operation.h"

namespace DCM
{
namespace Operations
{

template <> struct Operation<OperationType::VoxelizeMeans> : public VoxelizeOperation
{
    Operation(const std::wstring& inputFolder, const std::wstring& outputFile, unsigned xInMillimeters, unsigned yInMillimeters, unsigned zInMillimeters) :
        VoxelizeOperation(L"Shaders\\voxelize_mean.hlsl", "CSMain", inputFolder, outputFile, xInMillimeters, yInMillimeters, zInMillimeters)
    {}
};

template <> void inline LogOperation<OperationType::VoxelizeMeans>() { Log(L"[OperationType::VoxelizeMeans]"); }

} // Operations
} // DCM