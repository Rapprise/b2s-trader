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

#ifndef AUTO_TRADER_COMMON_BINANCE_CURRENCY_H
#define AUTO_TRADER_COMMON_BINANCE_CURRENCY_H

#include <map>
#include <vector>

#include "currency.h"

namespace auto_trader {
namespace common {

class BinanceCurrency {
 public:
  inline std::vector<Currency::Enum> getTradedCurrencies(Currency::Enum baseCurrency) {
    return binanceCurrencies_.at(baseCurrency);
  }

  inline std::vector<Currency::Enum> getBaseCurrencies() {
    std::vector<Currency::Enum> baseCurencies;
    for (const auto& currency : binanceCurrencies_) {
      baseCurencies.emplace_back(currency.first);
    }

    return baseCurencies;
  }

  inline std::string getBinancePair(Currency::Enum fromCurrency, Currency::Enum toCurrency) {
    if (binanceCurrencies_.find(fromCurrency) != binanceCurrencies_.end()) {
      return Currency::toString(toCurrency) + Currency::toString(fromCurrency);
    } else {
      return Currency::toString(fromCurrency) + Currency::toString(toCurrency);
    }
  }

 private:
  const std::map<const Currency::Enum, std::vector<Currency::Enum>> binanceCurrencies_ = {
      {Currency::BNB, {Currency::XRP, Currency::EOS, Currency::ADA, Currency::LTC,  Currency::BAT,
                       Currency::FET, Currency::TRX, Currency::BTT, Currency::ICX,  Currency::ENJ,
                       Currency::XLM, Currency::NEO, Currency::RVN, Currency::VET,  Currency::NANO,
                       Currency::ONT, Currency::ZIL, Currency::WTC, Currency::MITH, Currency::HOT}},
      {Currency::BTC,
       {Currency::BNB,  Currency::ETH, Currency::BCHABC, Currency::DGD, Currency::BAT,
        Currency::XRP,  Currency::ADA, Currency::LTC,    Currency::EOS, Currency::ICX,
        Currency::CELR, Currency::FET, Currency::ENJ,    Currency::QKC, Currency::TRX,
        Currency::NANO, Currency::NAS, Currency::IOTX,   Currency::AST, Currency::RVN,
        Currency::WTC,  Currency::TNB}},
      {Currency::ETH,
       {Currency::BNB, Currency::DGD, Currency::FUEL, Currency::BAT, Currency::XRP, Currency::ADA,
        Currency::ENJ, Currency::TRX, Currency::EOS, Currency::ICX, Currency::HOT, Currency::QKC,
        Currency::VET, Currency::DASH, Currency::LTC, Currency::ONT, Currency::ZEC, Currency::IOTX,
        Currency::NAS}},
      {Currency::USDT,
       {Currency::BTC, Currency::BNB, Currency::ETH, Currency::BTT, Currency::BCHABC, Currency::LTC,
        Currency::EOS, Currency::XRP, Currency::ADA, Currency::FET, Currency::TRX, Currency::ICX,
        Currency::NEO, Currency::BAT, Currency::TUSD, Currency::ONT, Currency::PAX, Currency::ETC,
        Currency::XLM}},
      {Currency::TUSD,
       {Currency::BTC, Currency::BNB, Currency::ETH, Currency::BCHABC, Currency::LTC, Currency::PAX,
        Currency::ADA, Currency::EOS, Currency::USDS, Currency::XRP, Currency::TRX, Currency::NEO,
        Currency::WAVES, Currency::ZEC, Currency::LINK, Currency::BTT}}};
};

}  // namespace common
}  // namespace auto_trader

#endif  // AUTO_TRADER_COMMON_BINANCE_CURRENCY_H
