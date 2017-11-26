/* SPDX-License-Identifier: GPL-3.0 */

#include "../../glm/glm.hpp"
#include "chip_top_texturerer.h"

ChipTopTexturerer::ChipTopTexturerer(ChipGeometryProvider &_gp) :
  gp(&_gp)
{}

void ChipTopTexturerer::write_texture_coords(float *buf) {
    float uv_consts[4] = {
        0.5f, 0.5f,
        0.0f, 0.0f
    };
    memcpy(&buf, &uv_consts, sizeof(uv_consts));

    float *vs = this->gp->vertices();
    uint16_t* vidcs = this->gp->indices();

    for(size_t i = 0; i < this->gp->segments(); ++i) {
        float *vd = &vs[vidcs[idcs_per_seg * i + 1]*3];
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
