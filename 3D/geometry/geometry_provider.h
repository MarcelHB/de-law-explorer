/* SPDX-License-Identifier: GPL-3.0 */

#ifndef GEOMETRY_PROVIDER_H
#define GEOMETRY_PROVIDER_H

#include <cstdint>
#include <vector>

#define GEOM_IDX_TYPE(x) static_cast<uint16_t>((x))

#ifndef M_PI
#define M_PI 3.14159265358979323846264338327
#endif

template <uint16_t N>
struct pi_nth {
    static const float v;
};

template <uint16_t N>
const float pi_nth<N>::v = 2.0f * M_PI/N;

/* Just convenience shortcuts. */
#define SIN_PI_N(k) static_cast<float>(sin((k)*pi_nth<N>::v))
#define COS_PI_N(k) static_cast<float>(cos((k)*pi_nth<N>::v))

/* Interface for exchangable vertex data sources. */

enum GeometryBoundingType { GP_BOX };

class GeometryProvider {
  public:
    virtual std::vector<float>& bounding_data() const = 0;
    virtual GeometryBoundingType bounding_type() const = 0;
    virtual std::vector<float>& normals() const = 0;
    virtual std::vector<float>& vertices() const = 0;
    virtual std::vector<uint16_t>& indices() const = 0;
    virtual ~GeometryProvider() {}
};

#endif // GEOMETRY_PROVIDER_H
