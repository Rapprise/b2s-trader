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

#ifndef AUTO_TRADER_COMMON_MARKET_ORDER_H
#define AUTO_TRADER_COMMON_MARKET_ORDER_H

#include <functional>
#include <iomanip>
#include <string>
#include <unordered_set>

#include "currency.h"
#include "date.h"
#include "enumerations/order_type.h"
#include "enumerations/stock_exchange_type.h"

namespace auto_trader {
namespace common {

constexpr int COIN_PRECISION = 8;

struct MarketOrder {
  int databaseId_{0};
  std::string uuid_{""};
  Currency::Enum toCurrency_{Currency::UNKNOWN};
  Currency::Enum fromCurrency_{Currency::UNKNOWN};
  OrderType orderType_{OrderType::UNKNOWN};
  StockExchangeType stockExchangeType_{StockExchangeType::UNKNOWN};
  double quantity_{0.0};
  double price_{0.0};
  Date opened_;
  bool isCanceled_{false};

  bool operator<(const MarketOrder& order) const { return uuid_ < order.uuid_; }

  bool operator==(const MarketOrder& order) const {
    return uuid_ == order.uuid_ && toCurrency_ == order.toCurrency_ &&
           fromCurrency_ == order.fromCurrency_ && orderType_ == order.orderType_ &&
           stockExchangeType_ == order.stockExchangeType_ && quantity_ == order.quantity_ &&
           price_ == order.price_ && opened_ == order.opened_ && isCanceled_ == order.isCanceled_;
  }

  const std::string toString() const {
    const std::string message =
        "order type : " + common::convertOrderTypeToString(orderType_) + ", " +
        "stock exchange type : " + common::convertStockExchangeTypeToString(stockExchangeType_) +
        ", " + "from currency : " + common::Currency::toString(fromCurrency_) + ", " +
        "to currency : " + common::Currency::toString(toCurrency_) + ", " +
        "quantity : " + convertCoinToString(quantity_) + ", " +
        "price : " + convertCoinToString(price_) + ", " +
        "canceled : " + (isCanceled_ ? "yes" : "no");

    return message;
  }

  static std::string convertCoinToString(double value) {
    std::ostringstream stream;
    stream << std::fixed << std::setprecision(COIN_PRECISION);
    stream << value;
    return stream.str();
  }

  static std::string convertCoinWithPrecision(double value, int precision) {
    std::ostringstream stream;
    stream << std::fixed << std::setprecision(precision);
    stream << value;
    return stream.str();
  }
};

struct MarketOrderHasher {
  size_t operator()(const MarketOrder& order) const {
    return std::hash<std::string>()(order.uuid_);
  }
};

typedef std::unordered_set<common::MarketOrder, common::MarketOrderHasher> MarketOrders;

}  // namespace common
}  // namespace auto_trader

#endif  // AUTO_TRADER_DATA_MARKET_ORDER_H
