/* SPDX-License-Identifier: GPL-3.0 */

#include <cstring>
#include <algorithm>

#define USE_GLEW_NOT_QT
#include "default_chip_renderer.h"

#include "../font/ft_glyph_loader.h"
#include "../font/os_font_path.h"
#include "../texturerers/chip_top_texturerer.h"
#include "../shaders/gl_fragment_shader.h"
#include "../shaders/gl_vertex_shader.h"

#define DEFAULT_FT_PX_SIZE 72
#define DEFAULT_TEXT_DIM 256

static const char *vs_string =
    "#version 120\n"
    "attribute vec2 v_uv;\n"
    "attribute vec3 norm;\n"
    "varying vec2 uv;\n"
    "varying vec3 pos_ws;\n"
    "varying vec3 cdir_cs;\n"
    "varying vec3 ldir_cs;\n"
    "varying vec3 norm_cs;\n"
    "uniform mat4 mcp;\n"
    "uniform mat4 c;\n"
    "uniform mat4 m;\n"
    "uniform vec3 lpos_ws;\n"
    "void main() {\n"
    "   gl_Position =  mcp * gl_Vertex;\n"
    "   uv = v_uv;\n"
    "   pos_ws = (m * gl_Vertex).xyz;\n"
    "   vec3 pos_cs = (c * m * gl_Vertex).xyz;\n"
    "   cdir_cs = vec3(0.0f, 0.0f, 0.0f) - pos_cs;\n"
    "   cdir_cs = normalize(cdir_cs);\n"
    "   vec3 lpos_cs = (c * vec4(lpos_ws, 1.0f)).xyz;\n"
    "   ldir_cs = lpos_cs + cdir_cs;\n"
    "   ldir_cs = normalize(ldir_cs);\n"
    "   norm_cs = (c * m * vec4(norm, 0.0f)).xyz;\n"
    "   norm_cs = normalize(norm_cs);\n"
    "}\n";

static const char *fs_string =
    "#version 120\n"
    "varying vec2 uv;\n"
    "varying vec3 pos_ws;\n"
    "varying vec3 cdir_cs;\n"
    "varying vec3 ldir_cs;\n"
    "varying vec3 norm_cs;\n"
    "uniform sampler2D sampler;\n"
    "uniform vec3 lpos_ws;\n"
    "uniform vec3 color_mod;\n"
    "void main() {\n"
    "    vec3 lcolor = vec3(1.0f, 1.0f, 1.0f);\n"
    "    float lamplifier = 4.0f;\n"
    "    vec3 diffuse_color = texture2D(sampler, uv).rgb;\n"
    "    diffuse_color = diffuse_color * color_mod;\n"
    "    vec3 ambient_color = vec3(0.45f, 0.45f, 0.45f) * diffuse_color;\n"
    "    vec3 specular_color = vec3(0.4f, 0.4f, 0.4f);\n"
    "    float dist = length(lpos_ws - pos_ws);\n"
    "    float theta = clamp(dot(norm_cs, ldir_cs), 0.0f, 1.0f);\n"
    "    vec3 r_cs = reflect(-ldir_cs, norm_cs);\n"
    "    float alpha = clamp(dot(cdir_cs, r_cs), 0.0f, 1.0f);\n"
    "    float dist_sq = dist;\n" // dist square is too much?
    "    vec3 color = ambient_color +\n"
    "        diffuse_color * lcolor * lamplifier * theta / dist_sq +\n"
    "        specular_color * lcolor * lamplifier * pow(alpha, 5) / dist_sq;\n"
    "    gl_FragColor = vec4(color, 1.0f);\n"
    "}\n";

DefaultChipRenderer::DefaultChipRenderer() :
  fgl(NULL),
  fglc(NULL),
  sp(NULL),
  default_font_size(DEFAULT_FT_PX_SIZE),
  initialized(false)
{ }

void DefaultChipRenderer::gl_ready() {
    if(!this->initialized) {
        this->init_fonts();
        this->init_statics();
        this->init_shaders();
        this->initialized = true;
    }
}

DefaultChipRenderer::~DefaultChipRenderer() {
    this->delete_buffers();
    if(this->initialized) {
        delete this->sp;
        delete this->fglc;
        delete this->fgl;
    }
}

void DefaultChipRenderer::init_fonts() {
    std::string font_path = default_font_path();
    this->fgl = new FTGlyphLoader(font_path.c_str(), this->default_font_size);
    this->fglc = new FontGlyphCacher(*this->fgl);
}

void DefaultChipRenderer::init_statics() {
    /* Vertices. */
    std::vector<float>& vs = this->gp.vertices();
    glGenBuffers(1, &(this->vertex_buf_id));
    glBindBuffer(GL_ARRAY_BUFFER, this->vertex_buf_id);
    glBufferData(GL_ARRAY_BUFFER, vs.size() * sizeof(float), &vs[0], GL_STATIC_DRAW);

    /* Normals. */
    std::vector<float>& normals = this->gp.normals();
    glGenBuffers(1, &(this->normals_buf_id));
    glBindBuffer(GL_ARRAY_BUFFER, this->normals_buf_id);
    glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(float), &normals[0], GL_STATIC_DRAW);

    /* UV coordinates. */
    ChipTopTexturerer<CHIP_SEGMENTS> ctt(this->gp);
    ctt.write_texture_coords(this->uv_coords);
    glGenBuffers(1, &(this->uv_buf_id));
    glBindBuffer(GL_ARRAY_BUFFER, this->uv_buf_id);
    glBufferData(GL_ARRAY_BUFFER, sizeof(this->uv_coords), this->uv_coords, GL_STATIC_DRAW);

    /* Indices. */
    glGenBuffers(1, &(this->idcs_buf_id));
    std::vector<uint16_t>& idcs = this->gp.indices();
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->idcs_buf_id);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, idcs.size() * sizeof(uint16_t), &idcs[0], GL_STATIC_DRAW);
}

void DefaultChipRenderer::init_shaders() {
    this->sp = new GLShaderProgram;
    std::stringstream ss;

    ss.write(vs_string, strlen(vs_string));
    GLVertexShader vs(ss);
    this->sp->attach_shader(vs);

    ss.str("");
    ss.clear();
    ss.write(fs_string, strlen(fs_string));
    GLFragmentShader fs(ss);

    this->sp->attach_shader(fs);
    this->sp->bind_attrib_location(1, "v_uv");
    this->sp->bind_attrib_location(2, "norm");

    this->sp->finalize();

    this->uni_mcp_id = this->sp->uniform_location("mcp");
    this->uni_m_id = this->sp->uniform_location("m");
    this->uni_c_id = this->sp->uniform_location("c");
    this->uni_samp_id = this->sp->uniform_location("sampler");
    this->uni_lpos_id = this->sp->uniform_location("lpos_ws");
    this->uni_cmod_id = this->sp->uniform_location("color_mod");
}

void DefaultChipRenderer::delete_buffers() {
    glDeleteBuffers(1, &(this->idcs_buf_id));
    glDeleteBuffers(1, &(this->vertex_buf_id));
    glDeleteBuffers(1, &(this->uv_buf_id));
    glDeleteBuffers(1, &(this->normals_buf_id));
}

void DefaultChipRenderer::draw(const glm::mat4 &m,
                                  const glm::mat4 &c,
                                  const glm::mat4 &p,
                                  const glm::vec3 &cam_pos,
                                  GLTexture &texture) {
    this->sp->use();

    // TODO: selected object
    glm::vec3 color_mod = glm::vec3(1.0f);
    glm::mat4 mcp = p * c * m;

    glUniformMatrix4fv(this->uni_c_id, 1, GL_FALSE, &c[0][0]);
    glUniformMatrix4fv(this->uni_m_id, 1, GL_FALSE, &m[0][0]);
    glUniformMatrix4fv(this->uni_mcp_id, 1, GL_FALSE, &mcp[0][0]);
    glm::vec3 light_pos = cam_pos + glm::vec3(0, -0.1f, 0);
    glUniform3fv(this->uni_lpos_id, 1, &light_pos[0]);
    glUniform3fv(this->uni_cmod_id, 1, &color_mod[0]);

    glActiveTexture(GL_TEXTURE0);
    texture.bind();
    glUniform1i(this->uni_samp_id, 0);

    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, this->vertex_buf_id);
    glVertexAttribPointer(
        0,
        3,
        GL_FLOAT,
        GL_FALSE,
        0,
        (void*) 0
    );

    glEnableVertexAttribArray(1);
    glBindBuffer(GL_ARRAY_BUFFER, this->uv_buf_id);
    glVertexAttribPointer(
        1,
        2,
        GL_FLOAT,
        GL_FALSE,
        0,
        (void*) 0
    );

    glEnableVertexAttribArray(2);
    glBindBuffer(GL_ARRAY_BUFFER, this->normals_buf_id);
    glVertexAttribPointer(
        2,
        3,
        GL_FLOAT,
        GL_FALSE,
        0,
        (void*) 0
    );

    glDrawElements(GL_TRIANGLES, this->gp.indices().size(), GL_UNSIGNED_SHORT, (void*)0);

    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
    glDisableVertexAttribArray(2);
}

BGRATexture* DefaultChipRenderer::generate_texture(const std::vector<uint16_t> &chars) {
    BGRATexture *tex = new BGRATexture(DEFAULT_TEXT_DIM);
    std::vector<unsigned char>& data = tex->data();
    FontRecommendation fr = this->optimal_font_settings(chars, DEFAULT_FT_PX_SIZE, DEFAULT_TEXT_DIM);
    size_t n_off = 0;
    bool overflow = false;

    size_t prev_size = this->fglc->height();
    size_t max_h = 0, max_bd = 0;
    this->fglc->height(fr.fs);

    for(auto it = chars.cbegin(); it != chars.cend() && !overflow; ++it) {
        FontGlyph *fg = this->fglc->glyph(*it);
        const std::vector<unsigned char>& fg_data = fg->data();
        size_t h = fg_data.size() / fg->bytes_per_row();
        max_h = std::max(h, max_h);
        max_bd = std::max(fg->bearing_down(), max_bd);
        size_t w = fg->bytes_per_row() / fg->bytes_per_pixel();
        size_t c_off = fr.fs - h;

        for(size_t j = 0; j < h; ++j) {
            size_t g_off = j * fg->bytes_per_row();
            size_t off = fr.x_off + n_off +
                         (fr.y_off + j + fg->bearing_down() + c_off) * DEFAULT_TEXT_DIM;
            /* Just in case ... */
            if(off + fg->bytes_per_row() >= data.size()) {
                overflow = true;
                break;
            }
            memcpy(&data[off*4], &(fg->data()[g_off]), fg->bytes_per_row());
        }
        n_off += w + fg->bearing_left();
    }

    size_t fp = fr.y_off * DEFAULT_TEXT_DIM + fr.x_off;
    /* If the actual total width is too far away from the predicted one, we fix this here. */
    if(abs(static_cast<int32_t>(fr.width) - static_cast<int32_t>(n_off)) / 2 > 5) {
        int32_t diff = (static_cast<int32_t>(fr.width) - static_cast<int32_t>(n_off)) / 2;
        size_t pxs = DEFAULT_TEXT_DIM * (DEFAULT_FT_PX_SIZE + 1);

        if(diff > 0) {
            memmove(&data[(fp + diff) * 4], &data[fp*4], pxs*4);
            memset(&data[fp*4], 0xFF, diff*4);
        } else {
            size_t lp = fp + fr.width * DEFAULT_FT_PX_SIZE;
            memmove(&data[(fp + diff) * 4], &data[fp*4], pxs*4);
            memset(&data[lp*4], 0xFF, -diff*4);
        }
        fp += diff;
    }

    /* This should never be negative, so always pull up. Maybe this can be merged with the upper
       move operations. But this is less ugly. */
    size_t hdiff = (fr.fs - max_h) / 2;
    if(hdiff > 5) {
        size_t lp = fp + DEFAULT_TEXT_DIM * (fr.fs + max_bd) - hdiff * DEFAULT_TEXT_DIM;
        memmove(&data[(fp - hdiff * DEFAULT_TEXT_DIM)*4], &data[fp*4], (max_bd + fr.fs) * DEFAULT_TEXT_DIM * 4);
        memset(&data[lp*4], 0xFF, (hdiff + 1) * DEFAULT_TEXT_DIM * 4);
    }

    this->fglc->height(prev_size);

    return tex;
}

ChipGeometryProvider<CHIP_SEGMENTS>& DefaultChipRenderer::geometry() {
    return this->gp;
}


/* Does ONLY calculate things for `current_size' < `tex_dim' */
FontRecommendation DefaultChipRenderer::optimal_font_settings(const std::vector<uint16_t> &chars,
                                                              size_t current_size, size_t tex_dim) {
    FontRecommendation fr = { 0, 0, current_size, 0 };
    size_t total = 0;
    size_t eff_tex_dim = tex_dim * 0.9f;

    /* Except for spaces, expect 2/3 (codes mostly have uppercases!) */
    size_t s_px = current_size * 0.3;
    size_t c_px = current_size * 0.65;
    for(auto it = chars.cbegin(); it != chars.cend(); ++it) {
        total += (*it) == 32 ? s_px : c_px;
    }

    if(total > eff_tex_dim) {
        float scale_down = ((eff_tex_dim*1.0f)/(total*1.0f));
        fr.fs = current_size * scale_down;
        total *= scale_down;
    }

    fr.width = total;
    fr.x_off = (tex_dim - total) / 2;
    fr.y_off = (tex_dim - fr.fs) / 2;

    return fr;
}
