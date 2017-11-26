/* SPDX-License-Identifier: GPL-3.0 */

#include <cstring>
#include <cstdint>

#include "arrow_head_geometry_provider.h"

std::vector<float>& ArrowHeadGeometryProvider::bounding_data() const {
    return bbox_data;
}
GeometryBoundingType ArrowHeadGeometryProvider::bounding_type() const {
    return GP_BOX;
}
std::vector<float>& ArrowHeadGeometryProvider::normals() const {
    return normals_data;
}
std::vector<float>& ArrowHeadGeometryProvider::vertices() const {
    return data;
}
std::vector<uint16_t>& ArrowHeadGeometryProvider::indices() const {
    return _indices;
}

std::vector<float> ArrowHeadGeometryProvider::data = std::vector<float> {
    /* Top trig. */
    0.0f, 0.1f, 1.0f,
   -0.5f, 0.1f, 0.0f,
    0.5f, 0.1f, 0.0f,
    /* Bottom trig. */
    0.0f,-0.1f, 1.0f,
   -0.5f,-0.1f, 0.0f,
    0.5f,-0.1f, 0.0f,
    /* Left side */
    0.0f, 0.1f, 1.0f,
    0.0f,-0.1f, 1.0f,
   -0.5f, 0.1f, 0.0f,
   -0.5f,-0.1f, 0.0f,
    /* Right side. */
    0.0f, 0.1f, 1.0f,
    0.0f,-0.1f, 1.0f,
    0.5f, 0.1f, 0.0f,
    0.5f,-0.1f, 0.0f,
    /* Rear */
    -0.5f, 0.1f, 0.0f,
    -0.5f,-0.1f, 0.0f,
    0.5f, 0.1f, 0.0f,
    0.5f,-0.1f, 0.0f
};
std::vector<uint16_t> ArrowHeadGeometryProvider::_indices = std::vector<uint16_t> {
    0,  1,  2,
    3,  4,  5,
    6,  7,  8,
    7,  9,  8,
   10, 12, 11,
   13, 11, 12,
   14, 15, 17,
   14, 17, 16
};

std::vector<float> ArrowHeadGeometryProvider::normals_data = std::vector<float> {
    0.0f, 1.0f, 0.0f,
    0.0f, 1.0f, 0.0f,
    0.0f, 1.0f, 0.0f,
    0.0f,-1.0f, 0.0f,
    0.0f,-1.0f, 0.0f,
    0.0f,-1.0f, 0.0f,
    0.383f, 0.0f,0.924f,
    0.383f, 0.0f,0.924f,
    0.383f, 0.0f,0.924f,
    0.383f, 0.0f,0.924f,
   -0.383f, 0.0f,0.924f,
   -0.383f, 0.0f,0.924f,
   -0.383f, 0.0f,0.924f,
   -0.383f, 0.0f,0.924f,
    0.0f, 0.0f,-1.0f,
    0.0f, 0.0f,-1.0f,
    0.0f, 0.0f,-1.0f,
    0.0f, 0.0f,-1.0f
};


std::vector<float> ArrowHeadGeometryProvider::bbox_data = std::vector<float> {
    0.0f, 1.0f, 0.0f, 0.1f,
    0.0f,-1.0f, 0.0f, 0.1f,
    1.0f, 0.0f, 0.0f, 0.5f,
   -1.0f, 0.0f, 0.0f, 0.5f,
    0.0f, 0.0f, 1.0f, 1.0f,
    0.0f, 0.0f,-1.0f, 1.0f,
   -0.5f, 0.1f, 1.0f,
    0.5f, 0.1f, 1.0f,
   -0.5f, 0.1f, 0.0f,
    0.5f, 0.1f, 0.0f,
   -0.5f,-0.1f, 1.0f,
    0.5f,-0.1f, 1.0f,
   -0.5f,-0.1f, 0.0f,
    0.5f,-0.1f, 0.0f
};
