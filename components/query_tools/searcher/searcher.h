/* SPDX-License-Identifier: GPL-3.0 */

#ifndef DE_LAW_EXPLORER_SEARCH_STRUCT_H
#define DE_LAW_EXPLORER_SEARCH_STRUCT_H

#include <vector>
#include <QString>

#include "../word_classifier.h"
#include "../../../controllers/controller.h"

struct SearchStruct {
    WordClassifier::Classification type;
    QString value;
};

struct SearchSuggestion {
    ControllerArgs *args;
    int ctrl_id;
};

class Searcher {
  public:
    Searcher(const std::vector<SearchStruct>& _tokens) : tokens(_tokens) {}
    virtual ~Searcher() {}

    virtual void search(std::vector<SearchSuggestion>&) = 0;
  protected:
    const std::vector<SearchStruct>& tokens;
};

#endif
