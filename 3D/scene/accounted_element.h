/* SPDX-License-Identifier: GPL-3.0 */

#ifndef DE_LAW_EXPLORER_3D_ACCOUNTED_ELEMENT_H
#define DE_LAW_EXPLORER_3D_ACCOUNTED_ELEMENT_H

#include "scene_element.h"

class AccountedElement {
  public:
    AccountedElement(SceneElement&, bool);
    SceneElement* element();
    void visible(bool);
    bool visible() const;
  private:
    SceneElement *_element;
    bool on_screen;
};

#endif
