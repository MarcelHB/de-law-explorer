/* SPDX-License-Identifier: GPL-3.0 */

#ifndef INTER_LINKS_SEARCHER_H
#define INTER_LINKS_SEARCHER_H

#include <vector>

#include "searcher.h"

class InterLinksSearcher : public Searcher {
  public:
    InterLinksSearcher(const std::vector<SearchStruct>&);

    void search(std::vector<SearchSuggestion>&);
    static bool qualifies(const std::vector<SearchStruct>&);

  private:
    std::vector<uint32_t> find_by_name(const QString&);
};

#endif // INTER_LINKS_SEARCHER_H
