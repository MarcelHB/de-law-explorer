/* SPDX-License-Identifier: GPL-3.0 */

#include "inter_links_searcher.h"
#include "code_searcher.h"
#include "../../../controllers/inter_links_controller.h"
#include "../../../db/loaders/code_finder.h"

InterLinksSearcher::InterLinksSearcher(const std::vector<SearchStruct> &tokens) :
  Searcher(tokens)
{}

/* Look for something like /(SHORT|NAME):(SHORT|NAME)/. */
bool InterLinksSearcher::qualifies(const std::vector<SearchStruct> &tokens) {
    enum { SCANNING_1ST_SHORT, SCANNING_REL, SCANNING_2ND_SHORT,
           HAS_REL, IS_OUT } state = SCANNING_1ST_SHORT;

    for(auto it = tokens.cbegin(); it != tokens.cend() && !(state == HAS_REL || state == IS_OUT); ++it) {
        const SearchStruct& ss = *it;

        switch(state) {
          case SCANNING_1ST_SHORT:
            if(ss.type == WordClassifier::SHORT_NAME || ss.type == WordClassifier::NAME) {
                state = SCANNING_REL;
            } else {
                state = IS_OUT;
            }
            break;
          case SCANNING_REL:
            if(ss.type == WordClassifier::REL_TOKEN) {
                state = SCANNING_2ND_SHORT;
            } else {
                state = IS_OUT;
            }
            break;
          case SCANNING_2ND_SHORT:
            if(ss.type == WordClassifier::SHORT_NAME || ss.type == WordClassifier::NAME) {
                state = HAS_REL;
            } else {
                state = IS_OUT;
            }
            break;
          default:
            break;
        }
    }

    return (state == HAS_REL);
}

/* Take /(SHORT|NAME):(SHORT|NAME)/ and do a lookup on both of them. */
void InterLinksSearcher::search(std::vector<SearchSuggestion> &suggestions) {
    QString first_short, second_short;
    enum { SCANNING_1ST_SHORT, SCANNING_REL, SCANNING_2ND_SHORT,
           HAS_REL } state = SCANNING_1ST_SHORT;

    for(auto it = this->tokens.cbegin(); it != this->tokens.cend(); ++it) {
        const SearchStruct& ss = *it;

        switch(state) {
          case SCANNING_1ST_SHORT:
            if(ss.type == WordClassifier::SHORT_NAME || ss.type == WordClassifier::NAME) {
                first_short = ss.value;
                state = SCANNING_REL;
            }
            break;
          case SCANNING_REL:
            if(ss.type == WordClassifier::REL_TOKEN) {
                state = SCANNING_2ND_SHORT;
            }
            break;
          case SCANNING_2ND_SHORT:
            if(ss.type == WordClassifier::SHORT_NAME || ss.type == WordClassifier::NAME) {
                second_short = ss.value;
                state = HAS_REL;
            }
            break;
          default:
            break;
        }
    }

    if(state == HAS_REL) {
        std::vector<uint32_t> first_results = this->find_by_name(first_short);
        std::vector<uint32_t> second_results = this->find_by_name(second_short);

        if(first_results.size() > 0 && second_results.size() > 0) {
            uint32_t first_id = first_results[0];
            uint32_t second_id = second_results[0];
            InterLinksControllerArgs *args = new InterLinksControllerArgs(first_id, second_id);
            SearchSuggestion ss = { args, CONTROLLER_INTER_CODE_LINKS };
            suggestions.push_back(ss);
        }
    }
}


std::vector<uint32_t> InterLinksSearcher::find_by_name(const QString &pattern) {
    std::vector<uint32_t> keys;

    Code c;
    CodeFinder cf(pattern);
    cf.load();

    while(cf.next_row(c)) {
        keys.push_back(c.id);
    }

    return keys;
}
