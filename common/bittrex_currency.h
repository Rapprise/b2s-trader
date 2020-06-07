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

#ifndef AUTO_TRADER_COMMON_BITTREX_CURRENCY_H
#define AUTO_TRADER_COMMON_BITTREX_CURRENCY_H

#include <map>
#include <vector>

#include "currency.h"

namespace auto_trader {
namespace common {

class BittrexCurrency {
 public:
  inline std::vector<Currency::Enum> getTradedCurrencies(Currency::Enum baseCurrency) {
    return bittrexCurrencies_.at(baseCurrency);
  }

  inline std::vector<Currency::Enum> getBaseCurrencies() {
    std::vector<Currency::Enum> baseCurencies;
    for (const auto& currency : bittrexCurrencies_) {
      baseCurencies.emplace_back(currency.first);
    }

    return baseCurencies;
  }

  inline std::string getBittrexPair(Currency::Enum fromCurrency, Currency::Enum toCurrency) {
    return Currency::toString(fromCurrency) + "-" + Currency::toString(toCurrency);
  }

 private:
  const std::map<const Currency::Enum, std::vector<Currency::Enum>> bittrexCurrencies_ = {
      {Currency::USD, {Currency::BTC, Currency::ETH,  Currency::LTC, Currency::XRP, Currency::USDT,
                       Currency::ADA, Currency::BCH,  Currency::BAT, Currency::ZEN, Currency::ETC,
                       Currency::BSV, Currency::ZEC,  Currency::TRX, Currency::DGB, Currency::TUSD,
                       Currency::ZRX, Currency::USDS, Currency::KMD, Currency::SC,  Currency::PAX}},
      {Currency::BTC,
       {Currency::ETH,   Currency::SOLVE, Currency::SPND,  Currency::ORBS, Currency::RVN,
        Currency::CMCT,  Currency::VBK,   Currency::XRP,   Currency::META, Currency::BCH,
        Currency::BAT,   Currency::ADA,   Currency::AERGO, Currency::NGC,  Currency::IHT,
        Currency::BSV,   Currency::XEM,   Currency::XVG,   Currency::ENJ,  Currency::MONA,
        Currency::LTC,   Currency::TUSD,  Currency::DGB,   Currency::ZEC,  Currency::TRX,
        Currency::WAVES, Currency::NEO,   Currency::DOGE}},
      {Currency::ETH,
       {Currency::BCH,   Currency::ADA, Currency::XRP,   Currency::BAT,  Currency::TRX,
        Currency::SOLVE, Currency::ANT, Currency::XEM,   Currency::LTC,  Currency::BSV,
        Currency::SC,    Currency::DGB, Currency::XLM,   Currency::TUSD, Currency::ETC,
        Currency::ZEC,   Currency::REP, Currency::STRAT, Currency::OMG,  Currency::ANKR,
        Currency::WAVES, Currency::SRN}},
      {Currency::USDT,
       {Currency::BTC, Currency::ETH, Currency::XRP, Currency::BCH, Currency::LTC, Currency::ADA,
        Currency::NEO, Currency::BAT, Currency::ZEC, Currency::ETC, Currency::XLM, Currency::TRX,
        Currency::OMG, Currency::DGB, Currency::XVG, Currency::DASH, Currency::DOGE, Currency::BSV,
        Currency::XMR}}};
};

}  // namespace common
}  // namespace auto_trader

#endif  // AUTO_TRADER_COMMON_BITTREX_CURRENCY_H
