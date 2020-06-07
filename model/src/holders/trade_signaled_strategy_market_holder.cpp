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

#include "model/include/holders/trade_signaled_strategy_market_holder.h"

#include "common/exceptions/no_data_found_exception.h"

namespace auto_trader {
namespace model {

void TradeSignaledStrategyMarketHolder::addMarket(common::Currency::Enum baseCurrency,
                                                  common::Currency::Enum tradedCurrency,
                                                  common::StrategiesType strategyType,
                                                  common::MarketData& marketData) {
  const std::string market =
      common::Currency::toString(baseCurrency) + common::Currency::toString(tradedCurrency);
  signaledMarket_[market][strategyType] = marketData;
}

common::MarketData& TradeSignaledStrategyMarketHolder::getMarket(
    common::Currency::Enum baseCurrency, common::Currency::Enum tradedCurrency,
    common::StrategiesType strategyType) {
  const std::string market =
      common::Currency::toString(baseCurrency) + common::Currency::toString(tradedCurrency);
  auto signaledMarketIt = signaledMarket_.find(market);
  if (signaledMarketIt != signaledMarket_.end()) {
    auto strategyMarketIt = signaledMarketIt->second.find(strategyType);
    if (strategyMarketIt != signaledMarketIt->second.end()) {
      return strategyMarketIt->second;
    }
  }
  throw common::exceptions::NoDataFoundException("Signaled market for strategy " +
                                                 common::convertStrategyTypeToString(strategyType));
}

bool TradeSignaledStrategyMarketHolder::containMarket(common::Currency::Enum baseCurrency,
                                                      common::Currency::Enum tradedCurrency,
                                                      common::StrategiesType strategyType) {
  const std::string market =
      common::Currency::toString(baseCurrency) + common::Currency::toString(tradedCurrency);
  auto signaledMarketIt = signaledMarket_.find(market);
  if (signaledMarketIt != signaledMarket_.end()) {
    auto strategyMarketIt = signaledMarketIt->second.find(strategyType);
    if (strategyMarketIt != signaledMarketIt->second.end()) {
      return true;
    }
  }
  return false;
}

void TradeSignaledStrategyMarketHolder::clear() { signaledMarket_.clear(); }

}  // namespace model
}  // namespace auto_trader