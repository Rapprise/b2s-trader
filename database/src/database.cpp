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

#include "include/database.h"

#include <assert.h>

#include <iostream>

#include "common/loggers/file_logger.h"

namespace auto_trader {
namespace database {
namespace statement {

constexpr char CREATE_MARKET_ORDER_TABLE[] =
    "CREATE TABLE IF NOT EXISTS MARKET_ORDER("
    "DB_ID INTEGER PRIMARY KEY  AUTOINCREMENT,"
    "UUID TEXT NOT NULL,"
    "TO_CURRENCY INTEGER NOT NULL,"
    "FROM_CURRENCY INTEGER NOT NULL,"
    "ORDER_TYPE INTEGER NOT NULL,"
    "STOCK_EXCHANGE INTEGER NOT NULL,"
    "QUANTITY REAL NOT NULL,"
    "PRICE REAL NOT NULL,"
    "OPENED TEXT,"
    "CANCELED INTEGER NOT NULL);";

constexpr char CREATE_ORDERS_PROFIT_TABLE[] =
    "CREATE TABLE IF NOT EXISTS ORDERS_PROFIT("
    "ID INTEGER PRIMARY KEY AUTOINCREMENT,"
    "ORDER_ID INTEGER NOT NULL,"
    "CURRENCY INTEGER,"
    "STOCK_EXCHANGE INTEGER NOT NULL,"
    "FOREIGN KEY(ORDER_ID) REFERENCES MARKET_ORDER(ID));";

constexpr char CREATE_ORDERS_MATCHING_TABLE[] =
    "CREATE TABLE IF NOT EXISTS ORDERS_MATCHING("
    "ID INTEGER PRIMARY KEY AUTOINCREMENT,"
    "FROM_ORDER_ID INTEGER NOT NULL,"
    "TO_ORDER_ID INTEGER NOT NULL,"
    "FROM_ORDER_TYPE INTEGER NOT NULL,"
    "TO_ORDER_TYPE INTEGER NOT NULL,"
    "STOCK_EXCHANGE INTEGER NOT NULL,"
    "CURRENCY_PAIR TEXT NOT NULL,"
    "FOREIGN KEY(FROM_ORDER_ID) REFERENCES MARKET_ORDER(ID),"
    "FOREIGN KEY(TO_ORDER_ID) REFERENCES MARKET_ORDER(ID));";

constexpr char CREATE_LAST_MARKET_DATA_TABLE[] =
    "CREATE TABLE IF NOT EXISTS LAST_MARKET_DATA("
    "ID INTEGER PRIMARY KEY AUTOINCREMENT, "
    "STOCK_EXCHANGE INTEGER NOT NULL, "
    "TO_CURRENCY INTEGER NOT NULL,"
    "FROM_CURRENCY INTEGER NOT NULL,"
    "STRATEGY_TYPE INTEGER NOT NULL, "
    "DATE TEXT NOT NULL, "
    "OPEN_PRICE REAL NOT NULL, "
    "CLOSE_PRICE REAL NOT NULL, "
    "LOW_PRICE REAL NOT NULL, "
    "HIGH_PRICE REAL NOT NULL, "
    "VOLUME REAL NOT NULL);";

constexpr char MARKET_ORDER_TABLE[] = "MARKET_ORDER";
constexpr char ORDERS_PROFIT_TABLE[] = "ORDERS_PROFIT";
constexpr char ORDERS_MATCHING_TABLE[] = "ORDERS_MATCHING";
constexpr char LAST_MARKET_DATA_TABLE[] = "LAST_MARKET_DATA";

}  // namespace statement

int browseMarketOrderCallback(void *data, int argumentsCount, char **argument, char **columnName) {
  auto orders = (std::vector<common::MarketOrder> *)data;
  assert(orders);

  common::MarketOrder order;
  order.databaseId_ = atoi(argument[0]);
  order.uuid_ = argument[1];
  order.toCurrency_ = static_cast<common::Currency::Enum>(atoi(argument[2]));
  order.fromCurrency_ = static_cast<common::Currency::Enum>(atoi(argument[3]));
  order.orderType_ = static_cast<common::OrderType>(atoi(argument[4]));
  order.stockExchangeType_ = static_cast<common::StockExchangeType>(atoi(argument[5]));
  order.quantity_ = atof(argument[6]);
  order.price_ = atof(argument[7]);
  order.opened_ = argument[8] ? common::Date::parseDate(argument[8]) : common::Date();
  order.isCanceled_ = (bool)atoi(argument[9]);

  orders->push_back(order);

  common::loggers::FileLogger::getLogger()
      << "Browse market orders callback invoked."
      << columnName[0] + std::string(" : ") + std::to_string(order.databaseId_)
      << columnName[1] + std::string(" : ") + order.uuid_
      << columnName[2] + std::string(" : ") + std::to_string(order.toCurrency_)
      << columnName[3] + std::string(" : ") + std::to_string(order.fromCurrency_)
      << columnName[4] + std::string(" : ") + std::to_string((int)order.orderType_)
      << columnName[5] + std::string(" : ") + std::to_string((int)order.stockExchangeType_)
      << columnName[6] + std::string(" : ") +
             common::MarketOrder::convertCoinToString(order.quantity_)
      << columnName[7] + std::string(" : ") + common::MarketOrder::convertCoinToString(order.price_)
      << columnName[8] + std::string(" : ") + common::Date::toString(order.opened_)
      << columnName[9] + std::string(" : ") + std::to_string(order.isCanceled_);

  return 0;
}

int browseOrdersMatchingCallback(void *data, int argumentsCount, char **arguments,
                                 char **columnName) {
  int fromOrderId = atoi(arguments[1]);
  int toOrderId = atoi(arguments[2]);

  auto matching = static_cast<std::vector<std::pair<int, int>> *>(data);
  matching->push_back(std::make_pair(fromOrderId, toOrderId));

  common::loggers::FileLogger::getLogger()
      << "Browse Order matching callback invoked."
      << columnName[1] + std::string(" : ") + std::to_string(fromOrderId)
      << columnName[2] + std::string(" : ") + std::to_string(toOrderId);

  return 0;
}

int browseOrdersProfitCallback(void *data, int argumentsCount, char **arguments,
                               char **columnName) {
  int id = atoi(arguments[0]);
  int orderId = atoi(arguments[1]);
  auto currency = static_cast<common::Currency::Enum>(atoi(arguments[2]));

  auto collection = static_cast<std::map<common::Currency::Enum, std::vector<int>> *>(data);
  (*collection)[currency].push_back(orderId);

  common::loggers::FileLogger::getLogger()
      << "Browse Orders Profit callback invoked."
      << columnName[0] + std::string(" : ") + std::to_string(id)
      << columnName[1] + std::string(" : ") + std::to_string(orderId)
      << columnName[2] + std::string(" : ") + std::to_string(currency);

  return 0;
}

int browseLastMarketDataCallback(void *data, int argumentsCount, char **arguments,
                                 char **columnName) {
  int id = atoi(arguments[0]);
  auto stockExchangeType = static_cast<common::StockExchangeType>(atoi(arguments[1]));
  auto toCurrency_ = static_cast<common::Currency::Enum>(atoi(arguments[2]));
  auto fromCurrency_ = static_cast<common::Currency::Enum>(atoi(arguments[3]));
  auto strategyType = static_cast<common::StrategiesType>(atoi(arguments[4]));
  auto date = common::Date::parseDate(arguments[5]);
  auto opened = atof(arguments[6]);
  auto closed = atof(arguments[7]);
  auto low = atof(arguments[8]);
  auto high = atof(arguments[9]);
  auto volume = atof(arguments[10]);

  common::MarketData currentMarketData(opened, closed, low, high, volume);
  currentMarketData.date_ = date;

  auto marketDataCollection = static_cast<Database::LastMarketDataCollection *>(data);
  (*marketDataCollection)[strategyType] = currentMarketData;

  common::loggers::FileLogger::getLogger()
      << "Browse Last Market Data callback invoked."
      << columnName[0] + std::string(" : ") + std::to_string(id)
      << columnName[1] + std::string(" : ") + std::to_string((int)stockExchangeType)
      << columnName[2] + std::string(" : ") + std::to_string((int)strategyType)
      << columnName[3] + std::string(" : ") + common::Date::toString(date)
      << columnName[4] + std::string(" : ") + std::to_string((int)strategyType)
      << columnName[5] + std::string(" : ") + common::Date::toString(date)
      << columnName[6] + std::string(" : ") + std::to_string(opened)
      << columnName[7] + std::string(" : ") + std::to_string(closed)
      << columnName[8] + std::string(" : ") + std::to_string(low)
      << columnName[9] + std::string(" : ") + std::to_string(high)
      << columnName[10] + std::string(" : ") + std::to_string(volume);
  return 0;
}

Database::Database() {
  std::string filePath;
  if (QCoreApplication::instance()) {
    auto applicationDir = QCoreApplication::applicationDirPath();
    filePath = applicationDir.toStdString() + std::string("/") + "b2s_trader.db";
  } else {
    filePath = "b2s_trader.db";
  }

  int result = sqlite3_open(filePath.c_str(), &dbHandler_);
  if (result) {
    common::loggers::FileLogger::getLogger()
        << "Can't open database: " << sqlite3_errmsg(dbHandler_);
  } else {
    common::loggers::FileLogger::getLogger() << "Database opened successfully";
  }

  executeStmt(statement::CREATE_MARKET_ORDER_TABLE, NULL, 0);
  executeStmt(statement::CREATE_ORDERS_PROFIT_TABLE, NULL, 0);
  executeStmt(statement::CREATE_ORDERS_MATCHING_TABLE, NULL, 0);
  executeStmt(statement::CREATE_LAST_MARKET_DATA_TABLE, NULL, 0);
}

Database::~Database() { sqlite3_close(dbHandler_); }

void Database::insertMarketOrder(const common::MarketOrder &order) {
  std::stringstream stream;
  stream << "INSERT INTO " << statement::MARKET_ORDER_TABLE
         << " (UUID, TO_CURRENCY, FROM_CURRENCY, ORDER_TYPE, STOCK_EXCHANGE, QUANTITY, PRICE, "
            "OPENED, CANCELED) VALUES"
         << "("
         << "\"" << order.uuid_.c_str() << "\""
         << ", "
         << "\"" << order.toCurrency_ << "\""
         << ", "
         << "\"" << order.fromCurrency_ << "\""
         << ", " << int(order.orderType_) << ", " << int(order.stockExchangeType_) << ", "
         << order.quantity_ << ", " << order.price_ << ", "
         << "\"" << common::Date::toString(order.opened_) << "\""
         << ", " << order.isCanceled_ << ");";

  const std::string sqlStmt = stream.str();
  executeStmt(sqlStmt.c_str(), NULL, 0);
}

void Database::removeMarketOrder(const common::MarketOrder &order) {
  std::stringstream stream;
  stream << "DELETE FROM " << statement::MARKET_ORDER_TABLE
         << " WHERE DB_ID = " << order.databaseId_ << " AND UUID = "
         << "\"" << order.uuid_ << "\""
         << " AND STOCK_EXCHANGE = " << int(order.stockExchangeType_) << ";";

  const std::string sqlStmt = stream.str();
  executeStmt(sqlStmt.c_str(), NULL, 0);
}

Database::MarketOrdersCollection Database::browseMarketOrders(
    common::StockExchangeType stockExchangeType) {
  std::stringstream stream;
  stream << "SELECT * FROM " << statement::MARKET_ORDER_TABLE << ";";

  std::vector<common::MarketOrder> orders;

  const std::string sqlStmt = stream.str();
  executeStmt(sqlStmt.c_str(), browseMarketOrderCallback, (void *)&orders);

  return orders;
}

Database::MarketOrdersCollection Database::browseMarketOrders(
    common::Currency::Enum fromCurrency, common::Currency::Enum toCurrency,
    common::StockExchangeType stockExchangeType) {
  std::stringstream stream;
  stream << "SELECT * FROM " << statement::MARKET_ORDER_TABLE
         << " WHERE FROM_CURRENCY = " << int(fromCurrency)
         << " AND TO_CURRENCY = " << int(toCurrency) << ";";

  std::vector<common::MarketOrder> orders;

  const std::string sqlStmt = stream.str();
  executeStmt(sqlStmt.c_str(), browseMarketOrderCallback, (void *)&orders);

  return orders;
}

Database::MarketOrdersCollection Database::browseMarketOrdersWithId(int64_t orderId) {
  std::stringstream stream;
  stream << "SELECT * FROM " << statement::MARKET_ORDER_TABLE << " "
         << "WHERE DB_ID = " << orderId << ";";

  std::vector<common::MarketOrder> orders;

  const std::string sqlStmt = stream.str();
  executeStmt(sqlStmt.c_str(), browseMarketOrderCallback, (void *)&orders);

  return orders;
}

void Database::insertOrderProfit(common::StockExchangeType stockExchangeType,
                                 common::Currency::Enum currency, unsigned int orderId) {
  std::stringstream stream;
  stream << "INSERT INTO " << statement::ORDERS_PROFIT_TABLE
         << "(ORDER_ID, CURRENCY, STOCK_EXCHANGE) VALUES "
         << "(" << orderId << ", " << currency << ", " << int(stockExchangeType) << ");";

  const std::string sqlStmt = stream.str();
  executeStmt(sqlStmt.c_str(), NULL, 0);
}

void Database::removeOrderProfit(common::StockExchangeType stockExchangeType,
                                 common::Currency::Enum currency, unsigned int orderId) {
  std::stringstream stream;
  stream << "DELETE FROM " << statement::ORDERS_PROFIT_TABLE << " "
         << "WHERE ORDER_ID = " << orderId << " AND CURRENCY = " << currency
         << " AND STOCK_EXCHANGE = " << int(stockExchangeType) << ";";

  const std::string sqlStmt = stream.str();
  executeStmt(sqlStmt.c_str(), NULL, 0);
}

Database::OrdersProfitCollection Database::browseOrdersProfit(
    common::StockExchangeType stockExchangeType) {
  std::stringstream stream;
  stream << "SELECT * FROM " << statement::ORDERS_PROFIT_TABLE << " "
         << "WHERE STOCK_EXCHANGE = " << int(stockExchangeType) << ";";

  const std::string sqlStmt = stream.str();
  std::map<common::Currency::Enum, std::vector<int>> ordersProfit;
  executeStmt(sqlStmt.c_str(), browseOrdersProfitCallback, &ordersProfit);

  std::vector<model::OrdersProfit> orders;
  for (auto it : ordersProfit) {
    model::OrdersProfit orderProfit{it.first};
    for (auto orderId : it.second) {
      auto orders = browseMarketOrdersWithId(orderId);
      assert(orders.size() == 1);
      orderProfit.addOrder(orders.back());
    }
    orders.push_back(orderProfit);
  }

  return orders;
}

void Database::insertOrderMatching(common::StockExchangeType stockExchangeType,
                                   const std::string &currencyPair, common::OrderType fromType,
                                   common::OrderType toType, unsigned int fromOrderId,
                                   unsigned int toOrderId) {
  std::stringstream stream;
  stream
      << "INSERT INTO " << statement::ORDERS_MATCHING_TABLE << "("
      << "FROM_ORDER_ID, TO_ORDER_ID, FROM_ORDER_TYPE, TO_ORDER_TYPE, STOCK_EXCHANGE, CURRENCY_PAIR"
      << ")"
      << "VALUES "
      << "(" << fromOrderId << ", " << toOrderId << ", " << int(fromType) << ", " << int(toType)
      << ", " << int(stockExchangeType) << ", "
      << "\"" << currencyPair << "\""
      << ");";

  const std::string sqlStmt = stream.str();
  executeStmt(sqlStmt.c_str(), NULL, 0);
}

void Database::removeOrderMatching(common::StockExchangeType stockExchangeType,
                                   common::OrderType fromType, common::OrderType toType,
                                   unsigned int fromOrderId, unsigned int toOrderId) {
  std::stringstream stream;
  stream << "DELETE FROM " << statement::ORDERS_MATCHING_TABLE << " "
         << " WHERE FROM_ORDER_ID = " << fromOrderId << " AND TO_ORDER_ID = " << toOrderId
         << " AND FROM_ORDER_TYPE = " << int(fromType) << " AND TO_ORDER_TYPE = " << int(toType)
         << " AND STOCK_EXCHANGE = " << int(stockExchangeType) << ";";

  const std::string sqlStmt = stream.str();
  executeStmt(sqlStmt.c_str(), NULL, 0);
}

model::OrderMatching Database::browseOrdersMatching(common::OrderType fromType,
                                                    common::OrderType toType,
                                                    const std::string &currencyPair,
                                                    common::StockExchangeType stockExchangeType) {
  std::stringstream stream;
  stream << "SELECT * FROM " << statement::ORDERS_MATCHING_TABLE
         << " WHERE STOCK_EXCHANGE = " << int(stockExchangeType) << " AND CURRENCY_PAIR = "
         << "\"" << currencyPair << "\""
         << ";";

  const std::string sqlStmt = stream.str();
  std::vector<std::pair<int, int>> orderIdMatching;
  executeStmt(sqlStmt.c_str(), browseOrdersMatchingCallback, &orderIdMatching);

  model::OrderMatching orderMatching{common::OrderType::SELL, common::OrderType::BUY};
  for (auto pair : orderIdMatching) {
    auto fromOrder = browseMarketOrdersWithId(pair.first);
    auto toOrder = browseMarketOrdersWithId(pair.second);

    assert(fromOrder.size() == 1);
    assert(toOrder.size() == 1);

    orderMatching.addOrderMatching(fromOrder.back(), toOrder.back());
  }

  return orderMatching;
}

void Database::removeCurrencyProfit(common::Currency::Enum currency,
                                    common::StockExchangeType stockExchangeType) {
  std::stringstream stream;
  stream << "DELETE FROM " << statement::ORDERS_PROFIT_TABLE << " "
         << "WHERE CURRENCY = " << currency << " AND STOCK_EXCHANGE = " << int(stockExchangeType)
         << ";";

  const std::string sqlStmt = stream.str();
  executeStmt(sqlStmt.c_str(), NULL, 0);
}

void Database::removeCurrencyOrdersMatching(const std::string &currencyPair,
                                            common::StockExchangeType stockExchangeType) {
  std::stringstream stream;
  stream << "DELETE FROM " << statement::ORDERS_MATCHING_TABLE << " "
         << " WHERE STOCK_EXCHANGE = " << int(stockExchangeType)
         << " AND CURRENCY_PAIR = " << currencyPair << ";";

  const std::string sqlStmt = stream.str();
  executeStmt(sqlStmt.c_str(), NULL, 0);
}

void Database::removeMarketOrders(common::Currency::Enum baseCurrency,
                                  common::Currency::Enum tradedCurrency,
                                  common::StockExchangeType stockExchangeType) {
  std::stringstream stream;
  stream << "DELETE FROM " << statement::MARKET_ORDER_TABLE
         << " WHERE FROM_CURRENCY = " << int(baseCurrency)
         << " AND TO_CURRENCY = " << int(tradedCurrency)
         << " AND STOCK_EXCHANGE = " << int(stockExchangeType) << ";";

  const std::string sqlStmt = stream.str();
  executeStmt(sqlStmt.c_str(), NULL, 0);
}

void Database::insertMarketData(common::StockExchangeType stockExchangeType,
                                common::Currency::Enum fromCurrency,
                                common::Currency::Enum toCurrency, common::StrategiesType type,
                                const common::MarketData &data) {
  std::stringstream stream;
  stream << "INSERT INTO " << statement::LAST_MARKET_DATA_TABLE << "("
         << "STOCK_EXCHANGE, TO_CURRENCY, FROM_CURRENCY, STRATEGY_TYPE, DATE, OPEN_PRICE, "
            "CLOSE_PRICE, LOW_PRICE, HIGH_PRICE, VOLUME"
         << ")"
         << "VALUES "
         << "(" << static_cast<int>(stockExchangeType) << ", " << static_cast<int>(toCurrency)
         << ", " << static_cast<int>(fromCurrency) << ", " << static_cast<int>(type) << ", "
         << "\"" << common::Date::toString(data.date_) << "\""
         << ", " << std::to_string(data.openPrice_) << ", " << std::to_string(data.closePrice_)
         << ", " << std::to_string(data.lowPrice_) << ", " << std::to_string(data.highPrice_)
         << ", " << std::to_string(data.volume_) << "); ";

  const std::string sqlStmt = stream.str();
  executeStmt(sqlStmt.c_str(), NULL, 0);
}

void Database::removeMarketData(common::StockExchangeType stockExchangeType,
                                common::Currency::Enum fromCurrency,
                                common::Currency::Enum toCurrency,
                                common::StrategiesType strategiesType) {
  std::stringstream stream;
  stream << "DELETE FROM " << statement::LAST_MARKET_DATA_TABLE << " "
         << " WHERE STOCK_EXCHANGE = " << int(stockExchangeType)
         << " AND TO_CURRENCY = " << int(toCurrency) << " AND FROM_CURRENCY = " << int(fromCurrency)
         << " AND STRATEGY_TYPE = " << int(strategiesType) << ";";

  const std::string sqlStmt = stream.str();
  executeStmt(sqlStmt.c_str(), NULL, 0);
}

void Database::removeMarketData(common::StockExchangeType stockExchangeType,
                                common::Currency::Enum fromCurrency,
                                common::Currency::Enum toCurrency) {
  std::stringstream stream;
  stream << "DELETE FROM " << statement::LAST_MARKET_DATA_TABLE << " "
         << " WHERE STOCK_EXCHANGE = " << int(stockExchangeType)
         << " AND TO_CURRENCY = " << int(toCurrency) << " AND FROM_CURRENCY = " << int(fromCurrency)
         << ";";

  const std::string sqlStmt = stream.str();
  executeStmt(sqlStmt.c_str(), NULL, 0);
}

Database::LastMarketDataCollection Database::browseLastMarketData(
    common::StockExchangeType stockExchangeType, common::Currency::Enum fromCurrency,
    common::Currency::Enum toCurrency) {
  LastMarketDataCollection collection;
  std::stringstream stream;
  stream << "SELECT * FROM " << statement::LAST_MARKET_DATA_TABLE
         << " WHERE STOCK_EXCHANGE = " << int(stockExchangeType)
         << " AND FROM_CURRENCY = " << int(fromCurrency) << " AND TO_CURRENCY = " << int(toCurrency)
         << ";";

  const std::string sqlStmt = stream.str();
  executeStmt(sqlStmt.c_str(), browseLastMarketDataCallback, &collection);

  return collection;
}

int Database::getLastInsertRowId() const { return sqlite3_last_insert_rowid(dbHandler_); }

void Database::executeStmt(const char *sqlStmt, SqlCallback callback, void *param) {
  char *errMsg = nullptr;
  int result = sqlite3_exec(dbHandler_, sqlStmt, callback, param, &errMsg);
  if (result != SQLITE_OK) {
    common::loggers::FileLogger::getLogger() << "SQL ERROR " << errMsg;
    sqlite3_free(errMsg);
  }
}

}  // namespace database
}  // namespace auto_trader