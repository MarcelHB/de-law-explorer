/* SPDX-License-Identifier: GPL-3.0 */

#include "word_classifier.h"

WordClassifier::WordClassifier(const QStringList &list) :
  input(list)
{}

void WordClassifier::classify(std::vector<Classification> &results) {
    for(auto it = this->input.cbegin(); it != this->input.cend(); ++it) {
        const QString& str = *it;
        bool classified = false;

        classified = this->is_code_token(str, results);

        if(!classified) {
            classified = this->is_rel_token(str, results);
        } else { continue; }

        if(!classified) {
            classified = this->is_numericalalpha(str, results);
        } else { continue; }

        if(!classified) {
            classified = this->is_reserved(str, results);
        } else { continue; }

        /* Whatever */
        if(!classified) {
            results.push_back(NAME);
        }
    }
}

bool WordClassifier::is_code_token(const QString &str, std::vector<Classification> &results) {
    QChar paragraph(0xA7);
    QString article("Art");

    bool is = (str.length() == 1 && str[0] == paragraph);

    /* Maybe 'Art' */
    if(!is) {
        QStringRef first_three(&str, 0, 3);
        is = first_three.compare(article, Qt::CaseInsensitive) == 0;
    }

    if(is) {
        results.push_back(CODE_TOKEN);
    }

    return is;
}

bool WordClassifier::is_rel_token(const QString &str, std::vector<Classification> &results) {
    bool is = (str.length() == 1 && str[0] == ':');

    if(is) {
        results.push_back(REL_TOKEN);
    }

    return is;
}

bool WordClassifier::is_numericalalpha(const QString &str, std::vector<Classification> &results) {
    QChar first = str[0];
    bool is = first.isNumber();

    if(is) {
        results.push_back(NUMERICALPHA);
    }

    return is;
}

bool WordClassifier::is_reserved(const QString &str, std::vector<Classification> &results) {
    QString words[] = { QString("Abs"), QString("Nr"),
                        QString("Nummer"), QString("Satz") };
    bool is = false;

    for(size_t i = 0; i < sizeof(words)/sizeof(QString) && !is; ++i) {
        QString& reserved = words[i];
        QStringRef sub(&str, 0, str.length());
        is = sub.compare(reserved, Qt::CaseInsensitive) == 0;
    }

    if(is) {
        results.push_back(RESERVED_TOKEN);
    }

    return is;
}

bool WordClassifier::is_short_name(const QString &str, std::vector<Classification> &results) {
    if(str.length() > 1) {
        QChar first = str[0];
        QChar last = str[str.length()-1];

        bool is = first.isLetterOrNumber() && first.isUpper() &&
                last.isLetter() && last.isUpper();
        if(is) {
            results.push_back(RESERVED_TOKEN);
        }

        return is;
    }

    return false;
}
