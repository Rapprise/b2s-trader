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

#include "model/include/holders/trade_orders_holder.h"

#include <algorithm>
#include <iterator>

#include "common/exceptions/no_data_found_exception.h"

namespace auto_trader {
namespace model {

void TradeOrdersHolder::addBuyOrder(const common::MarketOrder &order) {
  buyingOrders_.insert(order);
}

void TradeOrdersHolder::removeBuyOrder(const common::MarketOrder &order) {
  buyingOrders_.erase(order);
}

void TradeOrdersHolder::addSellOrder(const common::MarketOrder &order) {
  sellingOrders_.insert(order);
}

void TradeOrdersHolder::removeSellOrder(const common::MarketOrder &order) {
  sellingOrders_.erase(order);
}

bool TradeOrdersHolder::containBuyOrder(const common::MarketOrder &order) const {
  return (buyingOrders_.find(order) != buyingOrders_.end());
}

bool TradeOrdersHolder::containSellOrder(const common::MarketOrder &order) const {
  return (sellingOrders_.find(order) != sellingOrders_.end());
}

void TradeOrdersHolder::forEachBuyingOrder(
    std::function<void(const common::MarketOrder &)> callback) {
  for (auto &order : buyingOrders_) callback(order);
}

void TradeOrdersHolder::forEachSellingOrder(
    std::function<void(const common::MarketOrder &)> callback) {
  for (auto &order : sellingOrders_) callback(order);
}

size_t TradeOrdersHolder::getBuyOrdersCount() const { return buyingOrders_.size(); }

size_t TradeOrdersHolder::getSellOrdersCount() const { return sellingOrders_.size(); }

const std::set<common::MarketOrder> TradeOrdersHolder::getBuyOrdersDiff(
    const std::set<common::MarketOrder> &orders) {
  std::set<common::MarketOrder> difference;
  std::set_difference(buyingOrders_.begin(), buyingOrders_.end(), orders.begin(), orders.end(),
                      std::inserter(difference, difference.end()));

  return difference;
}

const std::set<common::MarketOrder> TradeOrdersHolder::getSellOrdersDiff(
    const std::set<common::MarketOrder> &orders) {
  std::set<common::MarketOrder> difference;
  std::set_difference(sellingOrders_.begin(), sellingOrders_.end(), orders.begin(), orders.end(),
                      std::inserter(difference, difference.end()));

  return difference;
}

void TradeOrdersHolder::addOrdersProfit(common::Currency::Enum currency,
                                        model::OrdersProfit &ordersProfit) {
  ordersProfit_.insert(std::make_pair<>(currency, ordersProfit));
}

bool TradeOrdersHolder::containOrdersProfit(common::Currency::Enum currency) const {
  return (ordersProfit_.find(currency) != ordersProfit_.end());
}

const model::OrdersProfit &TradeOrdersHolder::getOrdersProfit(
    common::Currency::Enum currency) const {
  auto it = ordersProfit_.find(currency);
  if (it != ordersProfit_.end()) {
    return it->second;
  }

  throw common::exceptions::NoDataFoundException("Orders Profit");
}

const model::OrderMatching &TradeOrdersHolder::getOrderMatching() const { return orderMatching_; }

model::OrdersProfit &TradeOrdersHolder::takeOrdersProfit(common::Currency::Enum currency) {
  auto it = ordersProfit_.find(currency);
  if (it != ordersProfit_.end()) {
    return it->second;
  }

  throw common::exceptions::NoDataFoundException("Orders Profit");
}

model::OrderMatching &TradeOrdersHolder::takeOrderMatching() { return orderMatching_; }

common::Date TradeOrdersHolder::getLocalTimestampFromBuyOrder(const common::MarketOrder &order) {
  auto iterator = buyingOrders_.find(order);
  if (iterator != buyingOrders_.end()) {
    return iterator->opened_;
  }

  throw common::exceptions::NoDataFoundException("Buy Order.");
}

common::Date TradeOrdersHolder::getLocalTimestampFromSellOrder(const common::MarketOrder &order) {
  auto iterator = sellingOrders_.find(order);
  if (iterator != sellingOrders_.end()) {
    return iterator->opened_;
  }

  throw common::exceptions::NoDataFoundException("Sell Order.");
}

int TradeOrdersHolder::getBuyOpenPositionsForMarket(common::Currency::Enum fromCurrency,
                                                    common::Currency::Enum toCurrency) {
  int openPositions = 0;
  for (auto &marketOrder : buyingOrders_) {
    if (marketOrder.fromCurrency_ == fromCurrency && marketOrder.toCurrency_ == toCurrency) {
      ++openPositions;
    }
  }

  return openPositions;
}

int TradeOrdersHolder::getSellOpenPositionsForMarket(common::Currency::Enum fromCurrency,
                                                     common::Currency::Enum toCurrency) {
  int openPositions = 0;
  for (auto &marketOrder : sellingOrders_) {
    if (marketOrder.fromCurrency_ == fromCurrency && marketOrder.toCurrency_ == toCurrency) {
      ++openPositions;
    }
  }

  return openPositions;
}

int TradeOrdersHolder::getBuyOrderDatabaseId(const common::MarketOrder &order) const {
  auto iterator = buyingOrders_.find(order);
  if (iterator != buyingOrders_.end()) {
    return iterator->databaseId_;
  }

  throw common::exceptions::NoDataFoundException("Buy Order.");
}

int TradeOrdersHolder::getSellOrderDatabaseId(const common::MarketOrder &order) const {
  auto iterator = sellingOrders_.find(order);
  if (iterator != sellingOrders_.end()) {
    return iterator->databaseId_;
  }

  throw common::exceptions::NoDataFoundException("Sell Order.");
}

double TradeOrdersHolder::getCoinInTradingCount() {
  double coinInTrading = 0.0;
  for (const auto &order : buyingOrders_) {
    coinInTrading += order.price_ * order.quantity_;
  }

  for (const auto &order : sellingOrders_) {
    auto buyingOrder = orderMatching_.getMatchedOrder(order);
    coinInTrading += buyingOrder.price_ * buyingOrder.quantity_;
  }

  for (auto &iterator : ordersProfit_) {
    auto &orderProfit = iterator.second;
    orderProfit.forEachOrder(
        [&](common::MarketOrder const &order) { coinInTrading += order.price_ * order.quantity_; });
  }

  return coinInTrading;
}

void TradeOrdersHolder::clear() {
  buyingOrders_.clear();
  sellingOrders_.clear();
  ordersProfit_.clear();
  orderMatching_.clear();
}

}  // namespace model
}  // namespace auto_trader