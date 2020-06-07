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

#ifndef AUTO_TRADER_MODEL_TRADING_ORDERS_HOLDER_H
#define AUTO_TRADER_MODEL_TRADING_ORDERS_HOLDER_H

#include <set>

#include "common/currency.h"
#include "common/enumerations/strategies_type.h"
#include "common/market_data.h"
#include "common/market_order.h"
#include "model/include/orders/orders_matching.h"
#include "model/include/orders/orders_profit.h"

namespace auto_trader {
namespace model {

class TradeOrdersHolder {
 public:
  void addBuyOrder(const common::MarketOrder &order);
  void removeBuyOrder(const common::MarketOrder &order);

  void addSellOrder(const common::MarketOrder &order);
  void removeSellOrder(const common::MarketOrder &order);

  bool containBuyOrder(const common::MarketOrder &order) const;
  bool containSellOrder(const common::MarketOrder &order) const;

  void forEachBuyingOrder(std::function<void(const common::MarketOrder &)> callback);
  void forEachSellingOrder(std::function<void(const common::MarketOrder &)> callback);

  size_t getBuyOrdersCount() const;
  size_t getSellOrdersCount() const;

  const std::set<common::MarketOrder> getBuyOrdersDiff(const std::set<common::MarketOrder> &orders);
  const std::set<common::MarketOrder> getSellOrdersDiff(
      const std::set<common::MarketOrder> &orders);

  int getBuyOrderDatabaseId(const common::MarketOrder &order) const;
  int getSellOrderDatabaseId(const common::MarketOrder &order) const;

 public:
  void addOrdersProfit(common::Currency::Enum currency, model::OrdersProfit &ordersProfit);
  bool containOrdersProfit(common::Currency::Enum currency) const;

  const model::OrderMatching &getOrderMatching() const;
  model::OrderMatching &takeOrderMatching();

  const model::OrdersProfit &getOrdersProfit(common::Currency::Enum currency) const;
  model::OrdersProfit &takeOrdersProfit(common::Currency::Enum currency);

 public:
  common::Date getLocalTimestampFromBuyOrder(const common::MarketOrder &order);
  common::Date getLocalTimestampFromSellOrder(const common::MarketOrder &order);

  int getBuyOpenPositionsForMarket(common::Currency::Enum fromCurrency,
                                   common::Currency::Enum toCurrency);
  int getSellOpenPositionsForMarket(common::Currency::Enum fromCurrency,
                                    common::Currency::Enum toCurrency);

 public:
  double getCoinInTradingCount();
  void clear();

 private:
  std::set<common::MarketOrder> buyingOrders_;
  std::set<common::MarketOrder> sellingOrders_;
  std::map<common::Currency::Enum, model::OrdersProfit> ordersProfit_;

  model::OrderMatching orderMatching_{common::OrderType::SELL, common::OrderType::BUY};
};

}  // namespace model
}  // namespace auto_trader

#endif  // AUTO_TRADER_MODEL_TRADING_ORDERS_HOLDER_H
