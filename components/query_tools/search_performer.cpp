/* SPDX-License-Identifier: GPL-3.0 */

#include <QStringList>

#include "search_performer.h"
#include "wordifier.h"
#include "word_classifier.h"

#include "searcher/code_searcher.h"
#include "searcher/code_atom_searcher.h"
#include "searcher/inter_links_searcher.h"
#include "../../../controllers/no_search_results_controller.h"

SearchPerformer::SearchPerformer(const QString &q) :
  query(q)
{}

SearchSuggestion SearchPerformer::run() {
    QStringList words;

    Wordifier wordifier(this->query);
    wordifier.wordify(words);

    std::vector<WordClassifier::Classification> classifications;
    WordClassifier wc(words);
    wc.classify(classifications);

    std::vector<SearchStruct> search_elems;
    for(int i = 0; i < words.length(); ++i) {
        SearchStruct ss = { classifications[i], words[i] };
        search_elems.push_back(ss);
    }

    std::vector<Searcher*> searchers;
    this->find_applicable_searchers(search_elems, searchers);

    std::vector<SearchSuggestion> suggestions;
    for(auto it = searchers.cbegin(); it != searchers.cend(); ++it) {
        Searcher *s = *it;
        s->search(suggestions);
    }

    if(!suggestions.empty()) {
        /* Whatever is on top, it should be the best match. */
        SearchSuggestion ss = suggestions[0];

        for(size_t i = 1; i < suggestions.size(); ++i) {
            SearchSuggestion &sstd = suggestions[i];
            delete sstd.args;
        }

        return ss;
    } else {
        NoSearchResultsControllerArgs *ctrl_args = new NoSearchResultsControllerArgs(this->query);
        SearchSuggestion ss = { ctrl_args, CONTROLLER_NO_SEARCH_RESULTS };
        return ss;
    }
}

void SearchPerformer::find_applicable_searchers(const std::vector<SearchStruct> &search_terms,
                                                std::vector<Searcher*> &searchers) {
    Searcher *searcher = nullptr;

    if(InterLinksSearcher::qualifies(search_terms)) {
        searcher = new InterLinksSearcher(search_terms);
        searchers.push_back(searcher);
    }
    if(CodeAtomSearcher::qualifies(search_terms)) {
        searcher = new CodeAtomSearcher(search_terms);
        searchers.push_back(searcher);
    }
    if(CodeSearcher::qualifies(search_terms)) {
        searcher = new CodeSearcher(search_terms);
        searchers.push_back(searcher);
    }
}
