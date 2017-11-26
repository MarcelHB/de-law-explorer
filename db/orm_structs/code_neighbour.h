/* SPDX-License-Identifier: GPL-3.0 */

#ifndef CODE_NEIGHBOUR_H
#define CODE_NEIGHBOUR_H

#include "code.h"

struct CodeNeighbour {
    Code code;
    size_t num_links;
    bool from;
};

#endif // CODE_NEIGHBOUR_H
