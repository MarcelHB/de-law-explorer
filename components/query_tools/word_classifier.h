/* SPDX-License-Identifier: GPL-3.0 */

#ifndef DE_LAW_EXPLORER_QUERY_WORD_CLASSIFIER_H
#define DE_LAW_EXPLORER_QUERY_WORD_CLASSIFIER_H

#include <vector>
#include <QStringList>

class WordClassifier {
  public:
    enum Classification { CODE_TOKEN, NUMERICALPHA, NAME,
           RESERVED_TOKEN, SHORT_NAME, REL_TOKEN };
    WordClassifier(const QStringList&);

    void classify(std::vector<Classification>&);
  private:
    const QStringList& input;

    bool is_code_token(const QString&, std::vector<Classification>&);
    bool is_rel_token(const QString&, std::vector<Classification>&);
    bool is_numericalalpha(const QString&, std::vector<Classification>&);
    bool is_reserved(const QString&, std::vector<Classification>&);
    bool is_short_name(const QString&, std::vector<Classification>&);
};

#endif
