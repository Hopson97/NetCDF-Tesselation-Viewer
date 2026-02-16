#include "HeightMapLoaders.h"

#include <format>
#include <iostream>

#include <SFML/System/Clock.hpp>
#include <SFML/System/Time.hpp>
#include <glad/glad.h>
#include <netcdf.h>

#define NC_CHECK(status)                                                                           \
    if (status != NC_NOERR)                                                                        \
    {                                                                                              \
        std::println(std::cerr, "NetCDF error: {}", nc_strerror(status));                          \
        return {};                                                                                 \
    }

namespace
{
    void flip_heightmap(const std::vector<int16_t> source, std::vector<int16_t>& dest, size_t width,
                        size_t height)
    {
        // As the OpenGL coordinate system (0, 0 bottom left) is different to NetCDF (0, 0 top
        // left), the data must be flipped to be correct
        for (size_t y = 0; y < height; ++y)
        {
            size_t source_row = (height - 1 - y) * width;
            size_t dest_row = y * width;

            std::copy(source.begin() + source_row, source.begin() + source_row + width,
                      dest.begin() + dest_row);
        }
    }

    /// Helper for loading NetCDF file from GEBCO
    struct GebcoNCFile
    {
        int netcdf_id = 0;
        int elevation_id = 0;
        size_t width = 0;
        size_t height = 0;

        /// Open the NetCDF and loads the metadata
        bool open(const std::string& nc_file_name)
        {
            // Open the file itself
            auto path = std::format("./assets/heightmaps/{}", nc_file_name);
            NC_CHECK(nc_open(path.c_str(), NC_NOWRITE, &netcdf_id));

            // Get the variable IDs
            int latitude_id = 0;
            int longitude_id = 0;
            NC_CHECK(nc_inq_dimid(netcdf_id, "lat", &latitude_id));
            NC_CHECK(nc_inq_dimid(netcdf_id, "lon", &longitude_id));
            NC_CHECK(nc_inq_varid(netcdf_id, "elevation", &elevation_id));

            // Query the width and height of the netcdf
            NC_CHECK(nc_inq_dimlen(netcdf_id, longitude_id, &width));
            NC_CHECK(nc_inq_dimlen(netcdf_id, latitude_id, &height));

            return true;
        }

        void close()
        {
            ncclose(netcdf_id);
        }
    };

    struct NetCDFMetadata
    {
        int elevation_id = 0;
        size_t width = 0;
        size_t height = 0;
    };
} // namespace

std::optional<HeightMap> heightmap_from_netcdf(const std::string& nc_file_name)
{
    GebcoNCFile netcdf;
    netcdf.open(nc_file_name);

    // Create a buffer to store the heightmap from the netcdf file
    std::vector<int16_t> elevation(netcdf.width * netcdf.height);

    // Read the heightmap into a buffer
    NC_CHECK(nc_get_var_short(netcdf.netcdf_id, netcdf.elevation_id, elevation.data()));

    HeightMap heightmap{
        .width = netcdf.width,
        .height = netcdf.height,
        .region_x = 0.0f,
        .region_y = 0.0f,
        .min_elevation = static_cast<float>(*std::min_element(elevation.begin(), elevation.end())),
        .max_elevation = static_cast<float>(*std::max_element(elevation.begin(), elevation.end())),
    };

    std::println(
        "Loading texture for {}\n Width {}\n Height {}\n Min elevation: {}\n Max elevation: {}",
        nc_file_name, heightmap.width, heightmap.height, heightmap.min_elevation,
        heightmap.max_elevation);

    std::vector<int16_t> flipped_elevation(elevation.size());
    flip_heightmap(elevation, flipped_elevation, heightmap.width, heightmap.height);

    if (!heightmap.heightmap.load_from_memory(flipped_elevation.data(), heightmap.width,
                                              heightmap.height, 1, {}, gl::TextureFormat::Red,
                                              gl::InternalTextureFormat::R16I, GL_SHORT))
    {
        return {};
    }

    netcdf.close();
    return heightmap;
}

TerrainHeightMaps split_heightmaps_from_netcdf(const std::string& nc_file_name, size_t size_per_map)
{
    TerrainHeightMaps terrain_heightmaps;
    GebcoNCFile netcdf;
    netcdf.open(nc_file_name);

    std::vector<int16_t> elevation(size_per_map * size_per_map);
    std::vector<int16_t> flipped_elevation(elevation.size());

    sf::Clock clock;

    size_t max_height = netcdf.height;
    size_t max_width = netcdf.width - MAP_REGION_SIZE;

    for (size_t y = 0; y < max_height; y += size_per_map)
    {
        for (size_t x = 0; x < max_width; x += size_per_map)
        {
            std::fill(elevation.begin(), elevation.end(), 0);

            size_t current_width = std::min(size_per_map, max_width - x);
            size_t current_height = std::min(size_per_map, max_height - y);

            std::println("Loading sub region ({}, {}) with size ({} {})", x, y, current_width,
                         current_height);

            // Prevents smaller regions from overlapping
            if (current_height != MAP_REGION_SIZE || current_width != MAP_REGION_SIZE)
            {
                continue;
            }

            // Loads a sub-region of the netcdf data
            size_t start[] = {y, x};
            size_t count[] = {current_height, current_width};

            NC_CHECK(nc_get_vara_short(netcdf.netcdf_id, netcdf.elevation_id, start, count,
                                       elevation.data()));

            auto min_elevation = *std::min_element(elevation.begin(), elevation.end());
            auto max_elevation = *std::max_element(elevation.begin(), elevation.end());
            auto& heightmap = terrain_heightmaps.maps.emplace_back(HeightMap{
                .width = current_width,
                .height = current_height,
                .region_x = static_cast<float>(x),
                .region_y = static_cast<float>(max_height - y - current_height),
                .min_elevation = static_cast<float>(min_elevation),
                .max_elevation = static_cast<float>(max_elevation)
            });


            terrain_heightmaps.min_elevation =
                std::min(heightmap.min_elevation, terrain_heightmaps.min_elevation);
            terrain_heightmaps.max_elevation =
                std::max(heightmap.max_elevation, terrain_heightmaps.max_elevation);

            flip_heightmap(elevation, flipped_elevation, heightmap.width, heightmap.height);

            if (!heightmap.heightmap.load_from_memory(
                    flipped_elevation.data(), heightmap.width, heightmap.height, 1, {},
                    gl::TextureFormat::Red, gl::InternalTextureFormat::R16I, GL_SHORT))
            {
                return {};
            }
        }
    }
    std::println("Loaded {} heightmaps in {} seconds", terrain_heightmaps.maps.size(),
                 clock.getElapsedTime().asSeconds());
    netcdf.close();
    return terrain_heightmaps;
}