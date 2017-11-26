/* SPDX-License-Identifier: GPL-3.0 */

#ifndef SCENE_WRITER_H
#define SCENE_WRITER_H

#include <string>
#include <vector>
#include <functional>

class SceneWriter {
  public:
    bool operator==(const SceneWriter&);
  protected:
    virtual size_t signature() const = 0;
    virtual size_t create_signature(std::vector<char32_t>&);
};

#endif // SCENE_WRITER_H
