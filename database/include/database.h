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

#ifndef AUTO_TRADER_DATABASE_DATABASE_H
#define AUTO_TRADER_DATABASE_DATABASE_H

#include <sqlite3.h>

#include "common/enumerations/strategies_type.h"
#include "common/market_data.h"
#include "common/market_order.h"
#include "model/include/orders/orders_matching.h"
#include "model/include/orders/orders_profit.h"

namespace auto_trader {
namespace database {

class Database {
 public:
  Database();
  ~Database();

  void insertMarketOrder(const common::MarketOrder &order);
  void removeMarketOrder(const common::MarketOrder &order);

  typedef std::vector<common::MarketOrder> MarketOrdersCollection;
  MarketOrdersCollection browseMarketOrders(common::StockExchangeType stockExchangeType);
  MarketOrdersCollection browseMarketOrders(common::Currency::Enum fromCurrency,
                                            common::Currency::Enum toCurrency,
                                            common::StockExchangeType stockExchangeType);

  typedef std::vector<model::OrdersProfit> OrdersProfitCollection;
  OrdersProfitCollection browseOrdersProfit(common::StockExchangeType stockExchangeType);

  model::OrderMatching browseOrdersMatching(common::OrderType fromType, common::OrderType toType,
                                            const std::string &currencyPair,
                                            common::StockExchangeType stockExchangeType);

  void insertOrderProfit(common::StockExchangeType stockExchangeType,
                         common::Currency::Enum currency, unsigned int orderId);
  void removeOrderProfit(common::StockExchangeType stockExchangeType,
                         common::Currency::Enum currency, unsigned int orderId);

  void insertOrderMatching(common::StockExchangeType stockExchangeType,
                           const std::string &currencyPair, common::OrderType fromType,
                           common::OrderType toType, unsigned int fromOrderId,
                           unsigned int toOrderId);

  void removeOrderMatching(common::StockExchangeType stockExchangeType, common::OrderType fromType,
                           common::OrderType toType, unsigned int fromOrderId,
                           unsigned int toOrderId);

  void insertMarketData(common::StockExchangeType stockExchangeType,
                        common::Currency::Enum fromCurrency, common::Currency::Enum toCurrency,
                        common::StrategiesType type, const common::MarketData &data);

  void removeMarketData(common::StockExchangeType stockExchangeType,
                        common::Currency::Enum fromCurrency, common::Currency::Enum toCurrency,
                        common::StrategiesType strategiesType);

  void removeMarketData(common::StockExchangeType stockExchangeType,
                        common::Currency::Enum fromCurrency, common::Currency::Enum toCurrency);

  typedef std::map<common::StrategiesType, common::MarketData> LastMarketDataCollection;

  LastMarketDataCollection browseLastMarketData(common::StockExchangeType stockExchangeType,
                                                common::Currency::Enum fromCurrency,
                                                common::Currency::Enum toCurrency);

  void removeCurrencyProfit(common::Currency::Enum currency,
                            common::StockExchangeType stockExchangeType);
  void removeCurrencyOrdersMatching(const std::string &currencyPair,
                                    common::StockExchangeType stockExchangeType);
  void removeMarketOrders(common::Currency::Enum baseCurrency,
                          common::Currency::Enum tradedCurrency,
                          common::StockExchangeType stockExchangeType);
  void removeMarketData(common::StockExchangeType stockExchangeType);

  int getLastInsertRowId() const;

 private:
  MarketOrdersCollection browseMarketOrdersWithId(int64_t orderId);

  typedef int (*SqlCallback)(void *, int, char **, char **);
  void executeStmt(const char *sqlStmt, SqlCallback callback, void *param);

  friend int browseMarketOrderCallback(void *data, int argc, char **argv, char **azColName);
  friend int browseOrdersProfitCallback(void *data, int argc, char **argv, char **azColName);
  friend int browseOrdersMatchingCallback(void *data, int argc, char **argv, char **azColName);

 private:
  sqlite3 *dbHandler_;
};

}  // namespace database
}  // namespace auto_trader

#endif  // AUTO_TRADER_DATABASE_DATABASE_H
