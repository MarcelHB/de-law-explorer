/* SPDX-License-Identifier: GPL-3.0 */

#include "code_atom_searcher.h"
#include "code_searcher.h"
#include "../../../controllers/code_atom_controller.h"
#include "../../../controllers/code_index_controller.h"
#include "../../../db/loaders/code_atom_finder.h"

CodeAtomSearcher::CodeAtomSearcher(const std::vector<SearchStruct> &tokens) :
  Searcher(tokens)
{}

/* Look for something like /(AN)((NAME)+|(SHORT))/. */
bool CodeAtomSearcher::qualifies(const std::vector<SearchStruct> &tokens) {
    enum { OPEN, HAS_ALPHANUM, SCANNING_CODE, HAS_CODE } state = OPEN;

    for(auto it = tokens.cbegin(); it != tokens.cend() && state != HAS_CODE; ++it) {
        const SearchStruct& ss = *it;

        switch(state) {
          case OPEN:
            if(ss.type == WordClassifier::NUMERICALPHA) {
                state = HAS_ALPHANUM;
            }
            break;
          case HAS_ALPHANUM:
            if(ss.type == WordClassifier::SHORT_NAME) {
                state = HAS_CODE;
            } else if(ss.type == WordClassifier::NAME) {
                state = SCANNING_CODE;
            }
            break;
          case SCANNING_CODE:
            if(ss.type == WordClassifier::SHORT_NAME) {
                state = HAS_CODE;
            } else if(ss.type == WordClassifier::NUMERICALPHA) {
                state = HAS_ALPHANUM;
            }
            break;
          default:
            break;
        }
    }

    return (state == HAS_CODE || state == SCANNING_CODE);
}

/* Take every tuple of (AN)((NAME)+|(SHORT)) to test. */
void CodeAtomSearcher::search(std::vector<SearchSuggestion> &suggestions) {
    enum { OPEN, COLLECTING } state = OPEN;
    std::vector<SearchStruct> sliced_names;
    QString last_alphanumeric;

    for(auto it = this->tokens.cbegin(); it != this->tokens.cend(); ++it) {
        const SearchStruct& ss = *it;
        bool test = false;

        switch(state) {
          case OPEN:
            if(ss.type == WordClassifier::NUMERICALPHA) {
                last_alphanumeric = ss.value;
                state = COLLECTING;
            }
            break;
          case COLLECTING:
            if(ss.type == WordClassifier::NAME || ss.type == WordClassifier::SHORT_NAME) {
                sliced_names.push_back(ss);
            } else {
                test = true;
                state = OPEN;
            }
        }

        if(test) {
            this->test_extracted_tuple(last_alphanumeric, sliced_names, suggestions);
        }
    }

    if(state == COLLECTING) {
        this->test_extracted_tuple(last_alphanumeric, sliced_names, suggestions);
    }
}

void CodeAtomSearcher::test_extracted_tuple(const QString &an, const std::vector<SearchStruct>& partials,
                                            std::vector<SearchSuggestion> &results) {
    /* First: collect all applicable codes from the group, use the searcher. */
    CodeSearcher cs(partials);
    std::vector<SearchSuggestion> code_results;
    cs.search(code_results);

    if(!code_results.empty()) {
        /* For each code, try to find the alphanumeric paragraph inside. */
        for(auto it = code_results.cbegin(); it != code_results.cend(); ++it) {
            const SearchSuggestion& ss = *it;
            const CodeIndexControllerArgs *args = dynamic_cast<CodeIndexControllerArgs*>(ss.args);
            uint32_t code_id = args->code_id;

            CodeAtom ca;
            CodeAtomFinder caf(an, code_id);
            caf.load();

            while(caf.next_row(ca)) {
                CodeAtomControllerArgs *ctrl_args = new CodeAtomControllerArgs(ca.id);
                SearchSuggestion ss = { ctrl_args, CONTROLLER_CODE_ATOM };
                results.push_back(ss);
            }

            /* Same for Artikel. Maybe merge these two somehow ... */
            CodeAtomFinder caf_art(an, code_id, CodeAtomType::TYPE_ARTIKEL);
            caf_art.load();

            while(caf_art.next_row(ca)) {
                CodeAtomControllerArgs *ctrl_args = new CodeAtomControllerArgs(ca.id);
                SearchSuggestion ss = { ctrl_args, CONTROLLER_CODE_ATOM };
                results.push_back(ss);
            }
        }

        /* We do not re-use them. */
        for(auto it = code_results.begin(); it != code_results.end(); ++it) {
            SearchSuggestion& ss = *it;
            delete ss.args;
        }
    }
}
