/* SPDX-License-Identifier: GPL-3.0 */

#include "accounted_element.h"

AccountedElement::AccountedElement(SceneElement& elem, bool visible) :
  _element(&elem),
  on_screen(visible)
{}

SceneElement* AccountedElement::element() {
    return this->_element;
}

bool AccountedElement::visible() const {
    return this->on_screen;
}

void AccountedElement::visible(bool state) {
    this->on_screen = state;
}
