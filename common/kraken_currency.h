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

#ifndef AUTO_TRADER_COMMON_KRAKEN_CURRENCY_H
#define AUTO_TRADER_COMMON_KRAKEN_CURRENCY_H

#include <map>
#include <vector>

#include "currency.h"

namespace auto_trader {
namespace common {

class KrakenCurrency {
 public:
  inline std::vector<Currency::Enum> getTradedCurrencies(Currency::Enum baseCurrency) {
    return krakenCurrencies.at(baseCurrency);
  }

  inline std::vector<Currency::Enum> getBaseCurrencies() {
    std::vector<Currency::Enum> baseCurencies;
    for (const auto& currency : krakenCurrencies) {
      baseCurencies.emplace_back(currency.first);
    }

    return baseCurencies;
  }

  static std::string getKrakenPair(Currency::Enum fromCurrency, Currency::Enum toCurrency) {
    Currency::Enum fromCurrency_correct =
        fromCurrency == Currency::BTC ? Currency::XBT : fromCurrency;
    Currency::Enum toCurrency_correct = toCurrency == Currency::BTC ? Currency::XBT : toCurrency;

    if (fromCurrency_correct == Currency::ETH) {
      if ((toCurrency_correct == Currency::XBT) || (toCurrency_correct == Currency::CAD) ||
          (toCurrency_correct == Currency::EUR) || (toCurrency_correct == Currency::GBP) ||
          (toCurrency_correct == Currency::JPY) || (toCurrency_correct == Currency::USD)) {
        return Currency::toString(fromCurrency_correct) + Currency::toString(toCurrency_correct);
      }

      return Currency::toString(toCurrency_correct) + Currency::toString(fromCurrency_correct);
    }

    if (fromCurrency_correct == Currency::USD) {
      return Currency::toString(toCurrency_correct) + Currency::toString(fromCurrency_correct);
    }

    if (fromCurrency_correct == Currency::EUR) {
      return Currency::toString(toCurrency_correct) + Currency::toString(fromCurrency_correct);
    }

    if (fromCurrency_correct == Currency::XBT) {
      if ((toCurrency_correct == Currency::EUR) || (toCurrency_correct == Currency::USD)) {
        return Currency::toString(fromCurrency_correct) + Currency::toString(toCurrency_correct);
      } else {
        return Currency::toString(toCurrency_correct) + Currency::toString(fromCurrency_correct);
      }
    }

    return Currency::toString(fromCurrency_correct) + Currency::toString(toCurrency_correct);
  }

 private:
  const std::map<const Currency::Enum, std::vector<Currency::Enum>> krakenCurrencies = {
      {Currency::ETH,
       {Currency::ADA, Currency::ATOM, Currency::EOS, Currency::ETC, Currency::CAD, Currency::EUR,
        Currency::GBP, Currency::JPY, Currency::USD, Currency::BTC, Currency::GNO, Currency::MLN,
        Currency::QTUM, Currency::REP, Currency::XTZ}},
      {Currency::USD,
       {Currency::ADA, Currency::ATOM, Currency::BCH, Currency::DASH, Currency::EOS, Currency::ETC,
        Currency::ETH, Currency::GNO, Currency::LTC, Currency::QTUM, Currency::REP, Currency::BTC,
        Currency::XLM, Currency::XMR, Currency::XRP, Currency::XTZ, Currency::ZEC}},
      {Currency::EUR,
       {Currency::ADA, Currency::ATOM, Currency::BCH, Currency::DASH, Currency::EOS, Currency::ETC,
        Currency::ETH, Currency::GNO, Currency::LTC, Currency::QTUM, Currency::REP, Currency::BTC,
        Currency::XLM, Currency::XMR, Currency::XRP, Currency::XTZ, Currency::ZEC}},
      {Currency::BTC,
       {Currency::ADA,  Currency::ATOM, Currency::BCH, Currency::DASH, Currency::EOS,
        Currency::ETC,  Currency::ETH,  Currency::GNO, Currency::LTC,  Currency::MLN,
        Currency::QTUM, Currency::REP,  Currency::EUR, Currency::USD,  Currency::XDG,
        Currency::XLM,  Currency::XMR,  Currency::XRP, Currency::XTZ,  Currency::ZEC}}};
};

}  // namespace common
}  // namespace auto_trader

#endif  // AUTO_TRADER_COMMON_KRAKEN_CURRENCY_H
