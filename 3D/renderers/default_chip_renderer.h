/* SPDX-License-Identifier: GPL-3.0 */

#ifndef DE_LAW_EXPLORER_3D_DEFAULT_CHIP_RENDERER_H
#define DE_LAW_EXPLORER_3D_DEFAULT_CHIP_RENDERER_H

#include <string>
#include <vector>
#include <cstdint>
#include <sstream>
#include <cstdlib>

#ifdef USE_GLEW_NOT_QT
#include "../GL/glew.h"
#else
#include <QtOpenGL>
#endif

#include "../glm/glm.hpp"
#include "../font/font_glyph_cacher.h"
#include "../geometry/chip_geometry_provider.h"
#include "../textures/gl_texture.h"
#include "../textures/bgra_texture.h"
#include "../shaders/gl_shader_program.h"

#define CHIP_SEGMENTS 30

struct FontRecommendation {
    size_t x_off;
    size_t y_off;
    size_t fs;
    size_t width;
};

class DefaultChipRenderer {
  public:
    DefaultChipRenderer();
    ~DefaultChipRenderer();

    void draw(const glm::mat4&, const glm::mat4&, const glm::mat4&,
              const glm::vec3&, GLTexture&);
    void gl_ready();
    BGRATexture* generate_texture(const std::vector<uint16_t>&);
    ChipGeometryProvider<CHIP_SEGMENTS>& geometry();
  private:
    FontGlyphLoader *fgl;
    FontGlyphCacher *fglc;
    ChipGeometryProvider<CHIP_SEGMENTS> gp;
    GLShaderProgram *sp;
    size_t default_font_size;
    float uv_coords[4+CHIP_SEGMENTS*8];
    bool initialized;

    GLuint idcs_buf_id;
    GLuint vertex_buf_id;
    GLuint normals_buf_id;
    GLuint uv_buf_id;

    GLuint uni_mcp_id;
    GLuint uni_c_id;
    GLuint uni_m_id;
    GLuint uni_samp_id;
    GLuint uni_lpos_id;
    GLuint uni_cmod_id;

    void delete_buffers();
    void init_fonts();
    void init_shaders();
    void init_statics();
    FontRecommendation optimal_font_settings(const std::vector<uint16_t>&, size_t, size_t);
};

#endif
