/* SPDX-License-Identifier: GPL-3.0 */

#include "wordifier.h"

Wordifier::Wordifier(const QString &str) :
  input(str),
  state(EMPTY)
{}

void Wordifier::wordify(QStringList &list) {
    QString buffer;

    for(auto it = input.cbegin(); it != input.cend(); ++it) {
        QChar c = *it;

        if(this->state == EMPTY) {
            if(!c.isSpace()) {
                if(c == 0xA7) {
                    this->state = CODE_TOKEN;
                    buffer.append(0xA7);
                } else {
                    this->state = FILLING;
                    buffer.append(c);
                }
            }
        } else if(this->state == FILLING) {
            if(c.isSpace()) {
                this->state = EMPTY;
                list.append(buffer);
                buffer.clear();
            } else if(c == ':') {
                this->state = REL_TOKEN;
                list.append(buffer);
                buffer.clear();
                buffer.append(c);
            } else {
                buffer.append(c);
            }
        } else if(this->state == CODE_TOKEN) {
            if(c.isSpace()) {
                this->state = EMPTY;
                list.append(buffer);
                buffer.clear();
            } else {
                if(c != 0xA7) {
                    this->state = FILLING;
                    list.append(buffer);
                    buffer.clear();
                    buffer.append(c);
                }
            }
        } else if(this->state == REL_TOKEN) {
            if(c.isSpace()) {
                this->state = EMPTY;
                list.append(buffer);
                buffer.clear();
            } else {
                if(c != ':') {
                    this->state = FILLING;
                    list.append(buffer);
                    buffer.clear();
                    buffer.append(c);
                }
            }
        }
    }

    if(this->state == FILLING) {
        list.append(buffer);
    }
}
