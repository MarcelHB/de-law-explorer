/* SPDX-License-Identifier: GPL-3.0 */

#ifndef DE_LAW_EXPLORER_3D_TEXTURERERS_CHIP_TOP_H
#define DE_LAW_EXPLORER_3D_TEXTURERERS_CHIP_TOP_H

#include "../geometry/chip_geometry_provider.h"
#include "../glm/gtx/matrix_transform_2d.hpp"

template <uint16_t N>
class ChipTopTexturerer {
  public:
    ChipTopTexturerer(ChipGeometryProvider<N>&);
    void write_texture_coords(float (&)[4+N*8]);
  private:
    ChipGeometryProvider<N> *gp;
};

template <uint16_t N>
ChipTopTexturerer<N>::ChipTopTexturerer(ChipGeometryProvider<N> &_gp) :
  gp(&_gp)
{}

template <uint16_t N>
void ChipTopTexturerer<N>::write_texture_coords(float (&buf)[4+N*8]) {
    float uv_consts[4] = {
        0.5f, 0.5f,
        0.0f, 0.0f
    };
    memcpy(&buf, &uv_consts, sizeof(uv_consts));

    glm::mat3 tm_t(1.0f);
    tm_t = glm::translate(tm_t, glm::vec2(0.5f, 0.5f));
    glm::mat3 tm_s(1.0f);
    tm_s = glm::scale(tm_s, glm::vec2(0.5f, 0.5f));
    glm::mat3 tm = tm_t * tm_s;

    std::vector<float>& vs = this->gp->vertices();
    std::vector<uint16_t>& vidcs = this->gp->indices();

    for(size_t i = 0; i < N; ++i) {
        float *vd = &vs[vidcs[12 * i + 1]*3];
        glm::vec3 v = glm::vec3(vd[0], vd[2], 1);
        v = tm * v;
        float uv_seg_coords[8] = {
            v.x, v.y,
            0.0f, 0.0f,
            0.0f, 0.0f,
            0.0f, 0.0f,
        };
        memcpy(&buf[4 + i * 8], &uv_seg_coords, sizeof(uv_seg_coords));
    }
}

#endif
