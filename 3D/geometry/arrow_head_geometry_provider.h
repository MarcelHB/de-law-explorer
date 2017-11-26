/* SPDX-License-Identifier: GPL-3.0 */

#ifndef ARROW_HEAD_GEOMETRY_PROVIDER_H
#define ARROW_HEAD_GEOMETRY_PROVIDER_H

#include "geometry_provider.h"

class ArrowHeadGeometryProvider : public GeometryProvider {
  public:
    std::vector<float>& bounding_data() const;
    GeometryBoundingType bounding_type() const;
    std::vector<float>& normals() const;
    std::vector<float>& vertices() const;
    std::vector<uint16_t>& indices() const;

  private:
    static std::vector<float> data;
    static std::vector<uint16_t> _indices;
    static std::vector<float> normals_data;
    static std::vector<float> bbox_data;
};

#endif // ARROW_HEAD_GEOMETRY_PROVIDER_H
