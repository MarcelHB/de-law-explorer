/* SPDX-License-Identifier: GPL-3.0 */

#ifndef SEARCH_PERFORMER_H
#define SEARCH_PERFORMER_H

#include <vector>

#include "searcher/searcher.h"

class SearchPerformer {
  public:
    SearchPerformer(const QString&);
    SearchSuggestion run();
  private:
    const QString& query;

    void find_applicable_searchers(const std::vector<SearchStruct>&,
                                   std::vector<Searcher*>&);
};

#endif // SEARCH_PERFORMER_H
