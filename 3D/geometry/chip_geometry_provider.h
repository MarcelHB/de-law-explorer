/* SPDX-License-Identifier: GPL-3.0 */

#ifndef CHIP_GEOMETRY_PROVIDER_H
#define CHIP_GEOMETRY_PROVIDER_H

#define _USE_MATH_DEFINES
#include <cstring>
#include <cstdint>

#include "geometry_provider.h"

/*
 * Let's create a chip. Something like a coin, just a little higher.
 *
 * Initially, we wanted to have template-generated primitive meshes
 * but some compilers refused working on this. This is possibly no
 * longer the case.
 */

template <uint16_t N>
class ChipGeometryProvider : public GeometryProvider {
  public:
    ChipGeometryProvider() {
        if(!has_data) {
            this->calculate_data();
        }
    }
    std::vector<float>& bounding_data() const {
        return bbox_data;
    }
    GeometryBoundingType bounding_type() const {
        return GP_BOX;
    }
    std::vector<float>& normals() const {
        return normals_data;
    }
    std::vector<float>& vertices() const {
        return data;
    }
    std::vector<uint16_t>& indices() const {
        return _indices;
    }
  private:
    static std::vector<float> data;
    static std::vector<uint16_t> _indices;
    static std::vector<float> normals_data;
    static std::vector<float> bbox_data;
    static bool has_data;

    void calculate_data() {
        float cores[6] = {
            0.0f, 0.1f, 0.0f,
            0.0f,-0.1f, 0.0f
        };
        memcpy(&data[0], &cores, sizeof(cores));

        float verticals[6] = {
            0.0f, 1.0f, 0.0f,
            0.0f,-1.0f, 0.0f
        };

        memcpy(&normals_data[0], &verticals, sizeof(verticals));

        for(uint16_t i = 0; i < N; ++i) {
            uint16_t t = 2 + i * 4;
            uint16_t n = 2 + (i+1) * 4;
            if(i == N-1) {
                n = 2;
            }

            float coords[12] = {
                SIN_PI_N(i), 0.1f, COS_PI_N(i),
                SIN_PI_N(i),-0.1f, COS_PI_N(i),
                SIN_PI_N(i), 0.1f, COS_PI_N(i),
                SIN_PI_N(i),-0.1f, COS_PI_N(i),
            };
            memcpy(&data[6 + i * 12], &coords, sizeof(coords));

            uint16_t li[12] = {
                GEOM_IDX_TYPE(0),     GEOM_IDX_TYPE(t), GEOM_IDX_TYPE(n),
                GEOM_IDX_TYPE(1),   GEOM_IDX_TYPE(n+1), GEOM_IDX_TYPE(t+1),
                GEOM_IDX_TYPE(n+2), GEOM_IDX_TYPE(t+2), GEOM_IDX_TYPE(n+3),
                GEOM_IDX_TYPE(t+3), GEOM_IDX_TYPE(n+3), GEOM_IDX_TYPE(t+2)
            };
            memcpy(&_indices[i * 12], &li, sizeof(li));

            float norms[12] = {
                0.0f,     1.0f,   0.0f,
                0.0f,    -1.0f,   0.0f,
                coords[0], 0.0f, coords[2],
                coords[0], 0.0f, coords[2]
            };
            memcpy(&(normals_data[6 + i * 12]), &norms, sizeof(norms));
        }

        has_data = true;
    }
};

template <uint16_t N>
std::vector<float> ChipGeometryProvider<N>::data = std::vector<float>(6+N*12, 0);
template <uint16_t N>
std::vector<uint16_t> ChipGeometryProvider<N>::_indices = std::vector<uint16_t>(N*12, 0);
template <uint16_t N>
std::vector<float> ChipGeometryProvider<N>::normals_data = std::vector<float>(6+N*12, 0);
template <uint16_t N>
bool ChipGeometryProvider<N>::has_data = false;
 /* Normal vector + scaling offset. Next 24 vertices. */
template <uint16_t N>
std::vector<float> ChipGeometryProvider<N>::bbox_data = std::vector<float> {
    0.0f,  1.0f, 0.0f, 0.1f,
    0.0f, -1.0f, 0.0f, 0.1f,
    1.0f,  0.0f, 0.0f, 1.0f,
   -1.0f,  0.0f, 0.0f, 1.0f,
    0.0f,  0.0f, 1.0f, 1.0f,
    0.0f,  0.0f,-1.0f, 1.0f,
   -1.0f,  0.1f,-1.0f,
    1.0f,  0.1f,-1.0f,
    1.0f,  0.1f, 1.0f,
   -1.0f,  0.1f, 1.0f,
   -1.0f, -0.1f,-1.0f,
    1.0f, -0.1f,-1.0f,
    1.0f, -0.1f, 1.0f,
   -1.0f, -0.1f, 1.0f,
};


#endif // CHIP_GEOMETRY_PROVIDER_H
