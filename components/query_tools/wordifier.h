/* SPDX-License-Identifier: GPL-3.0 */

#ifndef WORDIFIER_H
#define WORDIFIER_H

#include <QString>
#include <QStringList>

class Wordifier {
  public:
    Wordifier(const QString&);

    void wordify(QStringList&);
  private:
    const QString& input;
    enum { EMPTY, FILLING, CODE_TOKEN, REL_TOKEN } state;
};

#endif // WORDIFIER_H
