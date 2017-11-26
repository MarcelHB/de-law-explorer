/* SPDX-License-Identifier: GPL-3.0 */

#include "scene_writer.h"

bool SceneWriter::operator==(const SceneWriter &other) {
    return this->signature() == other.signature();
}

size_t SceneWriter::create_signature(std::vector<char32_t> &dwords) {
    size_t lh = 0;
    size_t h = 0;
    std::hash<char32_t> hash_fn;

    for(auto it = dwords.cbegin(); it != dwords.cend(); ++it) {
        size_t th = hash_fn(*it);
        h = th ^ (lh << 1);
        lh = th;
    }

    return h;
}
