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

#ifndef AUTO_TRADER_VIEW_FLOAT_VALIDATOR_H
#define AUTO_TRADER_VIEW_FLOAT_VALIDATOR_H

#include <QDoubleValidator>
#include <QObject>
#include <regex>

namespace auto_trader {
namespace view {
namespace validators {

class FloatValidator : public QDoubleValidator {
  Q_OBJECT

 public:
  explicit FloatValidator(QObject *parent = 0) : QDoubleValidator(parent) {}
  FloatValidator(double bottom, double top, int decimals, QObject *parent = nullptr)
      : QDoubleValidator(bottom, top, decimals, parent) {}
  virtual State validate(QString &input, int &pos) const {
    auto state = QDoubleValidator::validate(input, pos);
    if (state == Invalid) {
      return state;
    }
    if (input.isEmpty()) return Acceptable;

    const char *zeroSymbol = "0";
    const char *dot = ".";
    if (input[0] == *zeroSymbol) {
      if (input.size() == 1) {
        return Acceptable;
      }
      if (input[1] == *dot) {
        return Acceptable;
      }

      return Invalid;
    }

    const char *dotSymbol = ".";
    size_t dotPosition = input.toStdString().find(dotSymbol);
    if (input.toStdString().size() == 1 && dotPosition != -1) {
      return Invalid;
    }

    const char *comma = ",";
    size_t commaPosition = input.toStdString().find(comma);
    if (commaPosition != -1) {
      return Invalid;
    }

    const char *exponent = "e";
    size_t exponentPosition = input.toStdString().find(exponent);
    if (exponentPosition != -1) {
      return Invalid;
    }

    const char *dash = "-";
    size_t dashPosition = input.toStdString().find(dash);
    if (dashPosition != -1) {
      return Invalid;
    }

    int value = std::stod(input.toStdString());
    if (value > top()) {
      return Invalid;
    } else if (value < bottom()) {
      return Invalid;
    }

    return Acceptable;
  }
};

}  // namespace validators
}  // namespace view
}  // namespace auto_trader

#endif  // AUTO_TRADER_VIEW_FLOAT_VALIDATOR_H
