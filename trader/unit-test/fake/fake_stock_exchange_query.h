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

#ifndef AUTO_TRADER_TRADER_FAKE_STOCK_EXCHANGE_H
#define AUTO_TRADER_TRADER_FAKE_STOCK_EXCHANGE_H

#include <memory>
#include <mutex>

#include "stocks_exchange/include/query.h"

namespace auto_trader {
namespace trader {
namespace unit_test {

class FakeStockExchangeQuery : public stock_exchange::Query {
 public:
  FakeStockExchangeQuery();

  common::MarketOrder sellOrder(common::Currency::Enum fromCurrency,
                                common::Currency::Enum toCurrency, double quantity,
                                double rate) override;

  common::MarketOrder buyOrder(common::Currency::Enum fromCurrency,
                               common::Currency::Enum toCurrency, double quantity,
                               double rate) override;

  bool cancelOrder(common::Currency::Enum fromCurrency, common::Currency::Enum toCurrency,
                   const std::string& uuid) override;

  void updateApiKey(const std::string& api_key) override;
  void updateSecretKey(const std::string& secret_key) override;

  common::MarketHistoryPtr getMarketHistory(common::Currency::Enum fromCurrency,
                                            common::Currency::Enum toCurrency,
                                            common::TickInterval::Enum interval) override;

  common::MarketOrder getAccountOrder(common::Currency::Enum fromCurrency,
                                      common::Currency::Enum toCurrency,
                                      const std::string& uuid) override;

  std::vector<common::MarketOrder> getMarketOpenOrders(common::Currency::Enum fromCurrency,
                                                       common::Currency::Enum toCurrency) override;

  std::vector<common::MarketOrder> getAccountOpenOrders(common::Currency::Enum fromCurrency,
                                                        common::Currency::Enum toCurrency) override;

  common::CurrencyTick getCurrencyTick(common::Currency::Enum fromCurrency,
                                       common::Currency::Enum toCurrency) override;

  double getBalance(common::Currency::Enum currency) override;

  stock_exchange::CurrencyLotsHolder getCurrencyLotsHolder() override;
  stock_exchange::CurrencyLotsHolder& takeCurrencyLotsHolder();

 public:
  void addMarketHistory(common::Currency::Enum fromCurrency, common::Currency::Enum toCurrency,
                        std::shared_ptr<common::MarketHistory> marketHistory);

  void clearMarketHistory(common::Currency::Enum fromCurrency, common::Currency::Enum toCurrency);

  void setBalance(common::Currency::Enum currency, double balance);

  void setCurrencyTick(common::Currency::Enum fromCurrency, common::Currency::Enum toCurrency,
                       common::CurrencyTick currencyTick);

  void closeOrder(const std::string& uuid);

 private:
  void calculateBalanceAfterClosingOrder(const common::MarketOrder& order);
  void calculateBalanceAfterCancelingOrder(common::MarketOrder& order);

 private:
  std::map<common::Currency::Enum, double> currenciesBalance_;
  std::map<std::string, common::CurrencyTick> currenciesTick_;
  std::map<std::string, common::MarketOrder> accountOpenOrders_;
  std::map<std::string, common::MarketOrder> allOrders_;

  typedef std::shared_ptr<common::MarketHistory> MarketHistorySharedPtr;
  std::map<std::string, std::vector<MarketHistorySharedPtr>> currenciesMarketHistory_;

  stock_exchange::CurrencyLotsHolder lotsHolder_;

  unsigned int orderId{0};
  unsigned int currentMarketHistoryIndex_{0};
};

}  // namespace unit_test
}  // namespace trader
}  // namespace auto_trader

#endif  // AUTO_TRADER_TRADER_FAKE_STOCK_EXCHANGE_H
