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

#ifndef B2S_TRADER_COMMON_POLONIEX_CURRENCY_H
#define B2S_TRADER_COMMON_POLONIEX_CURRENCY_H

#include <map>
#include <vector>

#include "currency.h"

namespace auto_trader {
namespace common {

class PoloniexCurrency {
 public:
  inline std::vector<Currency::Enum> getTradedCurrencies(Currency::Enum baseCurrency) {
    return poloniexCurrencies_.at(baseCurrency);
  }

  inline std::vector<Currency::Enum> getBaseCurrencies() {
    std::vector<Currency::Enum> baseCurencies;
    for (const auto& currency : poloniexCurrencies_) {
      baseCurencies.emplace_back(currency.first);
    }

    return baseCurencies;
  }

  inline std::string getPoloniexPair(Currency::Enum fromCurrency, Currency::Enum toCurrency) {
    return Currency::toString(fromCurrency) + "_" + Currency::toString(toCurrency);
  }

 private:
  const std::map<const Currency::Enum, std::vector<Currency::Enum>> poloniexCurrencies_ = {
      {Currency::USDC,
       {Currency::BTC, Currency::ETH, Currency::BCHSV, Currency::BCHABC, Currency::DASH,
        Currency::USDT, Currency::XRP, Currency::ATOM, Currency::LTC, Currency::XMR, Currency::STR,
        Currency::GRIN, Currency::ZEC, Currency::ETC, Currency::DOGE}},
      {Currency::BTC,
       {Currency::ETH, Currency::BCHSV, Currency::XRP,  Currency::BCHABC, Currency::LTC,
        Currency::XMR, Currency::ATOM,  Currency::DASH, Currency::STR,    Currency::ZEC,
        Currency::ETC, Currency::BTS,   Currency::GRIN, Currency::EOS,    Currency::KNC,
        Currency::REP, Currency::NXT,   Currency::BAT,  Currency::SC,     Currency::DOGE,
        Currency::XEM, Currency::ZRX,   Currency::QTUM}},
      {Currency::ETH,
       {Currency::ETC, Currency::EOS, Currency::ZEC, Currency::BAT, Currency::REP, Currency::ZRX}},
      {Currency::USDT,
       {Currency::BTC,  Currency::ETH,  Currency::LTC, Currency::XRP,  Currency::ETC,
        Currency::QTUM, Currency::ATOM, Currency::ZEC, Currency::XMR,  Currency::BCHSV,
        Currency::STR,  Currency::NXT,  Currency::EOS, Currency::DOGE, Currency::BCHABC,
        Currency::REP,  Currency::DASH, Currency::SC,  Currency::GRIN, Currency::BAT,
        Currency::ZRX}}};
};

}  // namespace common
}  // namespace auto_trader

#endif  // B2S_TRADER_COMMON_POLONIEX_CURRENCY_H
