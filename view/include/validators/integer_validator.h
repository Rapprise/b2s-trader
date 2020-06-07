/*
 * Copyright (c) 2020, Rapprise.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef AUTO_TRADER_VIEW_INTEGERVALIDATOR_H
#define AUTO_TRADER_VIEW_INTEGERVALIDATOR_H

#include <QObject>
#include <QValidator>

namespace auto_trader {
namespace view {
namespace validators {

class IntegerValidator : public QIntValidator {
  Q_OBJECT

 public:
  explicit IntegerValidator(QObject *parent = 0) : QIntValidator(parent) {}
  IntegerValidator(int bottom, int top, QObject *parent = nullptr)
      : QIntValidator(bottom, top, parent) {}
  virtual State validate(QString &input, int &pos) const {
    auto state = QIntValidator::validate(input, pos);
    if (state == Invalid) {
      return state;
    }
    if (input.isEmpty()) return Acceptable;

    const char *comma = ",";

    if (input.toStdString().find(comma) != -1) {
      return Invalid;
    }

    int value = std::stoi(input.toStdString());
    if (value > top()) {
      return Invalid;
    } else if (value < bottom()) {
      return Invalid;
    }

    if (input.size() > 1 && input.at(0) == '0') {
      return Invalid;
    }

    return Acceptable;
  }
};

}  // namespace validators
}  // namespace view
}  // namespace auto_trader

#endif  // AUTO_TRADER_VIEW_INTEGERVALIDATOR_H
