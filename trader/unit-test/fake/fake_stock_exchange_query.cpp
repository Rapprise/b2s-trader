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

#include "fake_stock_exchange_query.h"

#include <common/exceptions/no_data_found_exception.h>

#include "common/exceptions/undefined_type_exception.h"
#include "common/market_history.h"
#include "common/market_order.h"

namespace auto_trader {
namespace trader {
namespace unit_test {

FakeStockExchangeQuery::FakeStockExchangeQuery() {
  currenciesBalance_.clear();
  currenciesTick_.clear();
  accountOpenOrders_.clear();
  allOrders_.clear();
  currenciesMarketHistory_.clear();
}

common::MarketOrder FakeStockExchangeQuery::sellOrder(common::Currency::Enum fromCurrency,
                                                      common::Currency::Enum toCurrency,
                                                      double quantity, double rate) {
  ++orderId;
  common::MarketOrder marketOrder;
  marketOrder.price_ = rate;
  marketOrder.quantity_ = quantity;
  marketOrder.fromCurrency_ = fromCurrency;
  marketOrder.toCurrency_ = toCurrency;
  marketOrder.stockExchangeType_ = common::StockExchangeType::Bittrex;
  marketOrder.opened_ = common::Date::getCurrentTime();
  marketOrder.orderType_ = common::OrderType::SELL;
  marketOrder.uuid_ = std::to_string(orderId);

  accountOpenOrders_.insert(std::make_pair<>(marketOrder.uuid_, marketOrder));
  allOrders_.insert(std::make_pair<>(marketOrder.uuid_, marketOrder));

  auto toCurrencyBalance = getBalance(marketOrder.toCurrency_);
  auto newToCurrencyBalance = toCurrencyBalance - quantity;
  setBalance(marketOrder.toCurrency_, newToCurrencyBalance);

  return marketOrder;
}

common::MarketOrder FakeStockExchangeQuery::buyOrder(common::Currency::Enum fromCurrency,
                                                     common::Currency::Enum toCurrency,
                                                     double quantity, double rate) {
  ++orderId;
  common::MarketOrder marketOrder;
  marketOrder.price_ = rate;
  marketOrder.quantity_ = quantity;
  marketOrder.fromCurrency_ = fromCurrency;
  marketOrder.toCurrency_ = toCurrency;
  marketOrder.stockExchangeType_ = common::StockExchangeType::Bittrex;
  marketOrder.opened_ = common::Date::getCurrentTime();
  marketOrder.orderType_ = common::OrderType::BUY;
  marketOrder.uuid_ = std::to_string(orderId);

  accountOpenOrders_.insert(std::make_pair<>(marketOrder.uuid_, marketOrder));
  allOrders_.insert(std::make_pair<>(marketOrder.uuid_, marketOrder));

  auto fromCurrencyBalance = getBalance(marketOrder.fromCurrency_);
  auto newFromCurrencyBalance = fromCurrencyBalance - marketOrder.price_ * quantity;
  setBalance(marketOrder.fromCurrency_, newFromCurrencyBalance);

  return marketOrder;
}

bool FakeStockExchangeQuery::cancelOrder(common::Currency::Enum fromCurrency,
                                         common::Currency::Enum toCurrency,
                                         const std::string& uuid) {
  accountOpenOrders_.erase(uuid);
  auto it = allOrders_.find(uuid);
  if (it != allOrders_.end()) {
    calculateBalanceAfterCancelingOrder(it->second);
    it->second.isCanceled_ = true;
    return it->second.isCanceled_;
  }

  return false;
}

void FakeStockExchangeQuery::updateApiKey(const std::string& api_key) {
  // nothing to do.
}

void FakeStockExchangeQuery::updateSecretKey(const std::string& secret_key) {
  // nothing to do.
}

common::MarketHistoryPtr FakeStockExchangeQuery::getMarketHistory(
    common::Currency::Enum fromCurrency, common::Currency::Enum toCurrency,
    common::TickInterval::Enum interval) {
  std::string currencyPair =
      common::Currency::toString(fromCurrency) + common::Currency::toString(toCurrency);
  auto it = currenciesMarketHistory_.find(currencyPair);
  if (it != currenciesMarketHistory_.end()) {
    if (currentMarketHistoryIndex_ == it->second.size()) currentMarketHistoryIndex_ = 0;

    if (it->second.empty()) {
      throw common::exceptions::NoDataFoundException("Market history");
    }

    auto marketHistoryPtr = std::make_unique<common::MarketHistory>();
    marketHistoryPtr->toBuy_ = it->second.at(currentMarketHistoryIndex_)->toBuy_;
    marketHistoryPtr->toSell_ = it->second.at(currentMarketHistoryIndex_)->toSell_;
    marketHistoryPtr->marketData_ = it->second.at(currentMarketHistoryIndex_)->marketData_;
    ++currentMarketHistoryIndex_;
    return marketHistoryPtr;
  }

  throw common::exceptions::NoDataFoundException("Market history");
}

void FakeStockExchangeQuery::clearMarketHistory(common::Currency::Enum fromCurrency,
                                                common::Currency::Enum toCurrency) {
  std::string currencyPair =
      common::Currency::toString(fromCurrency) + common::Currency::toString(toCurrency);
  currenciesMarketHistory_.erase(currencyPair);
  currentMarketHistoryIndex_ = 0;
}

std::vector<common::MarketOrder> FakeStockExchangeQuery::getMarketOpenOrders(
    common::Currency::Enum fromCurrency, common::Currency::Enum toCurrency) {
  return std::vector<common::MarketOrder>();
}

std::vector<common::MarketOrder> FakeStockExchangeQuery::getAccountOpenOrders(
    common::Currency::Enum fromCurrency, common::Currency::Enum toCurrency) {
  std::vector<common::MarketOrder> orders;
  for (auto accountOrderPair : accountOpenOrders_) {
    orders.push_back(accountOrderPair.second);
  }

  return orders;
}

common::CurrencyTick FakeStockExchangeQuery::getCurrencyTick(common::Currency::Enum fromCurrency,
                                                             common::Currency::Enum toCurrency) {
  std::string currencyPair =
      common::Currency::toString(fromCurrency) + common::Currency::toString(toCurrency);
  auto it = currenciesTick_.find(currencyPair);
  if (it != currenciesTick_.end()) {
    return it->second;
  }

  throw common::exceptions::NoDataFoundException("No currency tick.");
}

double FakeStockExchangeQuery::getBalance(common::Currency::Enum currency) {
  auto it = currenciesBalance_.find(currency);
  if (it != currenciesBalance_.end()) {
    return it->second;
  }

  throw common::exceptions::NoDataFoundException("Account balance.");
}

stock_exchange::CurrencyLotsHolder FakeStockExchangeQuery::getCurrencyLotsHolder() {
  return lotsHolder_;
}

stock_exchange::CurrencyLotsHolder& FakeStockExchangeQuery::takeCurrencyLotsHolder() {
  return lotsHolder_;
}

void FakeStockExchangeQuery::addMarketHistory(
    common::Currency::Enum fromCurrency, common::Currency::Enum toCurrency,
    std::shared_ptr<common::MarketHistory> marketHistory) {
  std::string currencyPair =
      common::Currency::toString(fromCurrency) + common::Currency::toString(toCurrency);
  currenciesMarketHistory_[currencyPair].push_back(marketHistory);
}

void FakeStockExchangeQuery::setCurrencyTick(common::Currency::Enum fromCurrency,
                                             common::Currency::Enum toCurrency,
                                             common::CurrencyTick currencyTick) {
  std::string currencyPair =
      common::Currency::toString(fromCurrency) + common::Currency::toString(toCurrency);
  currenciesTick_[currencyPair] = currencyTick;
}

void FakeStockExchangeQuery::setBalance(common::Currency::Enum currency, double balance) {
  currenciesBalance_[currency] = balance;
}

common::MarketOrder FakeStockExchangeQuery::getAccountOrder(common::Currency::Enum fromCurrency,
                                                            common::Currency::Enum toCurrency,
                                                            const std::string& uuid) {
  auto it = allOrders_.find(uuid);
  if (it != allOrders_.end()) {
    return it->second;
  }

  throw common::exceptions::NoDataFoundException("Account Order.");
}

void FakeStockExchangeQuery::closeOrder(const std::string& uuid) {
  auto orderIt = accountOpenOrders_.find(uuid);
  if (orderIt != accountOpenOrders_.end()) {
    calculateBalanceAfterClosingOrder(orderIt->second);
  }
  accountOpenOrders_.erase(uuid);
}

void FakeStockExchangeQuery::calculateBalanceAfterClosingOrder(const common::MarketOrder& order) {
  if (order.orderType_ == common::OrderType::BUY) {
    auto toBalance = currenciesBalance_.find(order.toCurrency_);
    if (toBalance != currenciesBalance_.end()) {
      auto balance = currenciesBalance_[order.toCurrency_];
      auto updatedBalance = balance + order.quantity_;
      currenciesBalance_[order.toCurrency_] = updatedBalance;
    } else {
      setBalance(order.toCurrency_, order.quantity_);
    }
  } else if (order.orderType_ == common::OrderType::SELL) {
    auto currentMainCurrencyBalance = currenciesBalance_[order.fromCurrency_];
    auto updatedMainCurrencyBalance = currentMainCurrencyBalance + (order.price_ * order.quantity_);
    currenciesBalance_[order.fromCurrency_] = updatedMainCurrencyBalance;
  }
}

void FakeStockExchangeQuery::calculateBalanceAfterCancelingOrder(common::MarketOrder& order) {
  if (order.orderType_ == common::OrderType::BUY) {
    auto fromCurrencyBalance = getBalance(order.fromCurrency_);
    auto newFromCurrencyBalance = fromCurrencyBalance + order.price_ * order.quantity_;
    setBalance(order.fromCurrency_, newFromCurrencyBalance);
  } else if (order.orderType_ == common::OrderType::SELL) {
    auto balance = getBalance(order.toCurrency_);
    auto newFromCurrencyBalance = balance + order.quantity_;
    setBalance(order.toCurrency_, newFromCurrencyBalance);
  }
}

}  // namespace unit_test
}  // namespace trader
}  // namespace auto_trader