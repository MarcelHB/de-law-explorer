/* SPDX-License-Identifier: GPL-3.0 */

#ifndef DE_LAW_EXPLORER_CODE_ATOM_SEARCHER_H
#define DE_LAW_EXPLORER_CODE_ATOM_SEARCHER_H

#include <vector>

#include "searcher.h"

class CodeAtomSearcher : public Searcher {
  public:
    CodeAtomSearcher(const std::vector<SearchStruct>&);

    void search(std::vector<SearchSuggestion>&);
    static bool qualifies(const std::vector<SearchStruct>&);

  private:
    void test_extracted_tuple(const QString&, const std::vector<SearchStruct>&,
                              std::vector<SearchSuggestion>&);
};

#endif
