#pragma once

#include <optional>
#include <string>
#include <vector>

#include <netcdf.h>

#include "Graphics/OpenGL/Texture.h"


//constexpr auto MAP_REGION_SIZE = 8192;
constexpr auto MAP_REGION_SIZE = 16384 / 2;

struct HeightMap
{
    gl::Texture2D heightmap;
    size_t width = 0;
    size_t height = 0;

    float region_x;
    float region_y;

    float min_elevation;
    float max_elevation;
};

struct TerrainHeightMaps
{
    std::vector<HeightMap> maps;
    float min_elevation;
    float max_elevation;
};

std::optional<HeightMap> heightmap_from_netcdf(const std::string& nc_file_name);

TerrainHeightMaps split_heightmaps_from_netcdf(const std::string& nc_file_name,
                                                    size_t size_per_map = MAP_REGION_SIZE);
