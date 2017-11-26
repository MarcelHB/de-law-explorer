/* SPDX-License-Identifier: GPL-3.0 */

#include "code_searcher.h"

#include "../../../controllers/code_index_controller.h"
#include "../../../db/loaders/code_finder.h"

CodeSearcher::CodeSearcher(const std::vector<SearchStruct> &tokens) :
  Searcher(tokens)
{}

/* Look for something like /((NAME)+|(SHORT))/. */
bool CodeSearcher::qualifies(const std::vector<SearchStruct> &tokens) {
    enum { OPEN, SCANNING_CODE, HAS_CODE } state = OPEN;

    for(auto it = tokens.cbegin(); it != tokens.cend() && state != HAS_CODE; ++it) {
        const SearchStruct& ss = *it;

        switch(state) {
          case OPEN:
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
                state = OPEN;
            }
            break;
          default:
            break;
        }
    }

    return (state == HAS_CODE || state == SCANNING_CODE);
}

/* Basically try to rip off all tokens and try each group. NOT supporting
 * numeric things right now ... */
void CodeSearcher::search(std::vector<SearchSuggestion> &suggestions) {
    std::unordered_set<uint32_t> code_ids;
    QStringList list_buffer;
    enum { OPEN, COLLECTING } state = OPEN;

    for(auto it = this->tokens.cbegin(); it != this->tokens.cend(); ++it) {
        const SearchStruct& ss = *it;
        bool test = false;

        switch(state) {
          case OPEN:
            if(ss.type == WordClassifier::NAME) {
                list_buffer.append(ss.value);
                state = COLLECTING;
            } else if(ss.type == WordClassifier::SHORT_NAME) {
                list_buffer.append(ss.value);
                test = true;
                state = OPEN;
            }
            break;
          case COLLECTING:
            if(ss.type == WordClassifier::NAME || ss.type == WordClassifier::SHORT_NAME) {
                list_buffer.append(ss.value);
            } else {
                test = true;
                state = OPEN;
            }
            break;
        }

        if(test) {
            QString str = list_buffer.join(QChar(' '));
            /* Collect all IDs. */
            std::unordered_set<uint32_t> results = this->find_by_name(str);
            code_ids.insert(results.begin(), results.end());
            list_buffer.clear();
        }
    }

    if(state == COLLECTING && !list_buffer.empty()) {
        QString str = list_buffer.join(QChar(' '));
        std::unordered_set<uint32_t> results = this->find_by_name(str);
        code_ids.insert(results.begin(), results.end());
    }

    for(auto it = code_ids.cbegin(); it != code_ids.cend(); ++it) {
        uint32_t code_id = *it;
        CodeIndexControllerArgs *args = new CodeIndexControllerArgs(code_id);
        SearchSuggestion suggestion = { args, CONTROLLER_CODE_INDEX };

        suggestions.push_back(suggestion);
    }
}

std::unordered_set<uint32_t> CodeSearcher::find_by_name(const QString &pattern) {
    std::unordered_set<uint32_t> keys;

    Code c;
    CodeFinder cf(pattern);
    cf.load();

    while(cf.next_row(c)) {
        keys.insert(c.id);
    }

    return keys;
}
