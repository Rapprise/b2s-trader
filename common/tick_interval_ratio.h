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

#ifndef AUTO_TRADER_TICK_INTERVAL_RATIO_H
#define AUTO_TRADER_TICK_INTERVAL_RATIO_H

#include <map>

#include "common/exceptions/undefined_type_exception.h"
#include "enumerations/stock_exchange_type.h"
#include "enumerations/tick_interval.h"

namespace auto_trader {
namespace common {

const std::map<common::TickInterval::Enum, const char *> BinanceTickInterval{
    {common::TickInterval::Enum::ONE_MIN, "1m"},
    {common::TickInterval::Enum::THREE_MIN, "3m"},
    {common::TickInterval::Enum::FIVE_MIN, "5m"},
    {common::TickInterval::Enum::FIFTEEN_MIN, "15m"},
    {common::TickInterval::Enum::THIRTY_MIN, "30m"},
    {common::TickInterval::Enum::ONE_HOUR, "1h"},
    {common::TickInterval::Enum::TWO_HOURS, "2h"},
    {common::TickInterval::Enum::FOUR_HOURS, "4h"},
    {common::TickInterval::Enum::SIX_HOURS, "6h"},
    {common::TickInterval::Enum::EIGHT_HOURS, "8h"},
    {common::TickInterval::Enum::TWELVE_HOURS, "12h"},
    {common::TickInterval::Enum::ONE_DAY, "1d"},
    {common::TickInterval::Enum::THREE_DAYS, "3d"},
    {common::TickInterval::Enum::ONE_WEEK, "1w"},
    {common::TickInterval::Enum::ONE_MONTH, "1M"}};

const std::map<common::TickInterval::Enum, const char *> BittrexTickInterval{
    {common::TickInterval::Enum::ONE_MIN, "oneMin"},
    {common::TickInterval::Enum::FIVE_MIN, "fiveMin"},
    {common::TickInterval::Enum::THIRTY_MIN, "thirtyMin"},
    {common::TickInterval::Enum::ONE_HOUR, "hour"},
    {common::TickInterval::Enum::ONE_DAY, "day"}};

const std::map<common::TickInterval::Enum, const char *> KrakenickInterval{
    {common::TickInterval::Enum::ONE_MIN, "1"},
    {common::TickInterval::Enum::FIVE_MIN, "5"},
    {common::TickInterval::Enum::FIFTEEN_MIN, "15"},
    {common::TickInterval::Enum::THIRTY_MIN, "30"},
    {common::TickInterval::Enum::ONE_HOUR, "60"},
    {common::TickInterval::Enum::FOUR_HOURS, "240"},
    {common::TickInterval::Enum::ONE_DAY, "1440"},
    {common::TickInterval::Enum::ONE_WEEK, "10080"},
    {common::TickInterval::Enum::TWO_WEEKS, "21600"},
};

const std::map<common::TickInterval::Enum, const char *> PoloniexInterval{
    {common::TickInterval::Enum::FIVE_MIN, "300"},
    {common::TickInterval::Enum::FIFTEEN_MIN, "900"},
    {common::TickInterval::Enum::THIRTY_MIN, "1800"},
    {common::TickInterval::Enum::TWO_HOURS, "7200"},
    {common::TickInterval::Enum::FOUR_HOURS, "14400"},
    {common::TickInterval::Enum::ONE_DAY, "86400"}};

const std::map<common::TickInterval::Enum, const char *> HuobiInterval{
    {common::TickInterval::Enum::ONE_MIN, "1min"},
    {common::TickInterval::Enum::FIVE_MIN, "5min"},
    {common::TickInterval::Enum::FIFTEEN_MIN, "15min"},
    {common::TickInterval::Enum::THIRTY_MIN, "30min"},
    {common::TickInterval::Enum::ONE_HOUR, "60min"},
    {common::TickInterval::Enum::ONE_DAY, "1day"},
    {common::TickInterval::Enum::ONE_WEEK, "1week"},
    {common::TickInterval::Enum::ONE_MONTH, "1mon"},
    {common::TickInterval::Enum::ONE_YEAR, "1year"}};

inline std::map<common::TickInterval::Enum, const char *> getStockExchangeIntervals(
    common::StockExchangeType type) {
  switch (type) {
    case common::StockExchangeType::Bittrex:
      return BittrexTickInterval;
    case common::StockExchangeType::Binance:
      return BinanceTickInterval;
    case common::StockExchangeType::Kraken:
      return KrakenickInterval;
    case common::StockExchangeType::Poloniex:
      return PoloniexInterval;
    case common::StockExchangeType::Huobi:
      return HuobiInterval;

    default:
      throw exceptions::UndefinedTypeException("Undefined stock exchange.");
  }
}

}  // namespace common
}  // namespace auto_trader

#endif  // AUTO_TRADER_TICK_INTERVAL_RATIO_H
