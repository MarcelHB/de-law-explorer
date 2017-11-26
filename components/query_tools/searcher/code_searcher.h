/* SPDX-License-Identifier: GPL-3.0 */

#ifndef DE_LAW_EXPLORER_CODE_SEARCHER_H
#define DE_LAW_EXPLORER_CODE_SEARCHER_H

#include <unordered_set>
#include <vector>

#include "searcher.h"

class CodeSearcher : public Searcher {
  public:
    CodeSearcher(const std::vector<SearchStruct>&);

    void search(std::vector<SearchSuggestion>&);
    static bool qualifies(const std::vector<SearchStruct>&);

  private:
    std::unordered_set<uint32_t> find_by_name(const QString&);
};

#endif
