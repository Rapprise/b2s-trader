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

#include "database_ut.h"

#include "common/market_order.h"

namespace auto_trader {
namespace database {
namespace unit_test {

/**
 *              Test Plan:
 *
 *  1. Browse market orders from empty database.
 *  2. Browse order matching from empty database.
 *  3. Browse orders profit from empty database.
 *  4. Add one market order to database and check result.
 *  5. Add orders matching to db and check result.
 *  6. Add orders profit to db and check result.
 *  7. Remove added order from database.
 *  8. Remove orders from matching and check result.
 *  9. Remove orders from profit and check result.
 *  10. Remove orders from MARKET_ORDERS table and check matching query.
 *  11. Remove orders from MARKET_ORDERS table and check orders profit query.
 *  12. Add last market data and check db.
 *  13. Remove last market data and check db.
 *  14. Remove market orders.
 *
 **/

TEST_F(DatabaseUTFixture, NoMarketOrders_1) {
  auto& db = GetDatabase();
  auto orders = db.browseMarketOrders(common::StockExchangeType::Bittrex);
  EXPECT_TRUE(orders.empty());
}

TEST_F(DatabaseUTFixture, NoOrdersProfit_2) {
  auto& db = GetDatabase();
  auto ordersProfit = db.browseOrdersProfit(common::StockExchangeType::Bittrex);
  EXPECT_TRUE(ordersProfit.empty());
}

TEST_F(DatabaseUTFixture, NoOrdersMatching_3) {
  auto& db = GetDatabase();
  auto ordersMatching = db.browseOrdersMatching(common::OrderType::SELL, common::OrderType::BUY,
                                                "BTCUSD", common::StockExchangeType::Bittrex);
  unsigned int ordersCount = 0;
  ordersMatching.forEachMatching(
      [&](const common::MarketOrder& from, const common::MarketOrder& to) { ++ordersCount; });

  EXPECT_TRUE(ordersCount == 0);
}

TEST_F(DatabaseUTFixture, Add_MarketOrder_4) {
  common::Date opened{1, 2, 3, 4, 5, 6};
  common::Date closed{2, 3, 5, 6, 2, 1};

  common::MarketOrder order{0,
                            "ff-01",
                            common::Currency::BTC,
                            common::Currency::USD,
                            common::OrderType::BUY,
                            common::StockExchangeType::Bittrex,
                            1.45,
                            2.21,
                            opened,
                            false};

  auto& db = GetDatabase();
  db.insertMarketOrder(order);
  auto orders = db.browseMarketOrders(common::StockExchangeType::Bittrex);

  EXPECT_EQ(orders.size(), 1);
  EXPECT_EQ(orders[0], order);
}

TEST_F(DatabaseUTFixture, Add_OrderMatching_5) {
  common::Date opened{1, 2, 3, 4, 5, 6};
  common::Date closed{2, 3, 5, 6, 2, 1};

  common::MarketOrder buyOrder{0,
                               "ff-01",
                               common::Currency::BTC,
                               common::Currency::USD,
                               common::OrderType::BUY,
                               common::StockExchangeType::Bittrex,
                               1.45,
                               2.21,
                               opened,
                               false};

  common::MarketOrder sellOrder{0,
                                "ff-02",
                                common::Currency::USD,
                                common::Currency::BTC,
                                common::OrderType::SELL,
                                common::StockExchangeType::Bittrex,
                                1.3252,
                                2.2351,
                                opened,
                                false};

  auto& db = GetDatabase();
  db.insertMarketOrder(buyOrder);
  buyOrder.databaseId_ = db.getLastInsertRowId();
  EXPECT_EQ(buyOrder.databaseId_, 1);

  db.insertMarketOrder(sellOrder);
  sellOrder.databaseId_ = db.getLastInsertRowId();
  EXPECT_EQ(sellOrder.databaseId_, 2);

  db.insertOrderMatching(common::StockExchangeType ::Bittrex, "BTCUSD", common::OrderType::SELL,
                         common::OrderType::BUY, sellOrder.databaseId_, buyOrder.databaseId_);

  bool matchingFound = false;
  auto ordersMatching = db.browseOrdersMatching(common::OrderType::SELL, common::OrderType::BUY,
                                                "BTCUSD", common::StockExchangeType::Bittrex);

  ordersMatching.forEachMatching(
      [&](const common::MarketOrder& fromOrder, const common::MarketOrder& toOrder) {
        EXPECT_EQ(fromOrder, sellOrder);
        EXPECT_EQ(toOrder, buyOrder);
        matchingFound = true;
      });

  EXPECT_TRUE(matchingFound);
}

TEST_F(DatabaseUTFixture, Add_OrdersProfit_6) {
  common::Date opened{1, 2, 3, 4, 5, 6};
  common::Date closed{2, 3, 5, 6, 2, 8};

  common::MarketOrder buyOrder1{0,
                                "ff-01",
                                common::Currency::BTC,
                                common::Currency::USD,
                                common::OrderType::BUY,
                                common::StockExchangeType::Bittrex,
                                1.45,
                                2.21,
                                opened,
                                false};

  common::MarketOrder buyOrder2{0,
                                "ff-02",
                                common::Currency::BTC,
                                common::Currency::USD,
                                common::OrderType::BUY,
                                common::StockExchangeType::Bittrex,
                                1.3252,
                                2.2351,
                                opened,
                                false};

  auto& db = GetDatabase();
  db.insertMarketOrder(buyOrder1);
  buyOrder1.databaseId_ = db.getLastInsertRowId();
  EXPECT_EQ(buyOrder1.databaseId_, 1);

  db.insertMarketOrder(buyOrder2);
  buyOrder2.databaseId_ = db.getLastInsertRowId();
  EXPECT_EQ(buyOrder2.databaseId_, 2);

  db.insertOrderProfit(common::StockExchangeType::Bittrex, common::Currency::USD,
                       buyOrder1.databaseId_);
  db.insertOrderProfit(common::StockExchangeType::Bittrex, common::Currency::USD,
                       buyOrder2.databaseId_);

  auto ordersProfit = db.browseOrdersProfit(common::StockExchangeType::Bittrex);
  EXPECT_EQ(ordersProfit.size(), 1);
  auto orderProfit = ordersProfit[0];

  unsigned ordersCount = 0;
  EXPECT_EQ(orderProfit.getCurrency(), common::Currency::USD);
  orderProfit.forEachOrder([&](const common::MarketOrder& order) {
    if (order.uuid_ == buyOrder1.uuid_) {
      EXPECT_EQ(order, buyOrder1);
    } else if (order.uuid_ == buyOrder2.uuid_) {
      EXPECT_EQ(order, buyOrder2);
    } else {
      FAIL();
    }

    ++ordersCount;
  });

  EXPECT_EQ(ordersCount, 2);
}

TEST_F(DatabaseUTFixture, Remove_Existed_MarketOrder_7) {
  common::Date opened{1, 2, 3, 4, 5, 6};
  common::Date closed{2, 3, 5, 6, 2, 1};

  common::MarketOrder order{0,
                            "ff-01",
                            common::Currency::BTC,
                            common::Currency::USD,
                            common::OrderType::BUY,
                            common::StockExchangeType::Bittrex,
                            1.45,
                            2.21,
                            opened,
                            false};

  auto& db = GetDatabase();
  db.insertMarketOrder(order);
  order.databaseId_ = db.getLastInsertRowId();
  db.removeMarketOrder(order);

  auto orders = db.browseMarketOrders(common::StockExchangeType::Bittrex);
  EXPECT_TRUE(orders.empty());
}

TEST_F(DatabaseUTFixture, Remove_Existed_OrdersMatching_8) {
  common::Date opened{1, 2, 3, 4, 5, 6};
  common::Date closed{2, 3, 5, 6, 2, 1};

  common::MarketOrder buyOrder{0,
                               "ff-01",
                               common::Currency::BTC,
                               common::Currency::USD,
                               common::OrderType::BUY,
                               common::StockExchangeType::Bittrex,
                               1.45,
                               2.21,
                               opened,
                               false};

  common::MarketOrder sellOrder{0,
                                "ff-02",
                                common::Currency::USD,
                                common::Currency::BTC,
                                common::OrderType::SELL,
                                common::StockExchangeType::Bittrex,
                                1.3252,
                                2.2351,
                                opened,
                                false};

  auto& db = GetDatabase();
  db.insertMarketOrder(buyOrder);
  buyOrder.databaseId_ = db.getLastInsertRowId();
  EXPECT_EQ(buyOrder.databaseId_, 1);

  db.insertMarketOrder(sellOrder);
  sellOrder.databaseId_ = db.getLastInsertRowId();
  EXPECT_EQ(sellOrder.databaseId_, 2);

  db.insertOrderMatching(common::StockExchangeType::Bittrex, "BTCUSD", common::OrderType::SELL,
                         common::OrderType::BUY, sellOrder.databaseId_, buyOrder.databaseId_);

  db.removeOrderMatching(common::StockExchangeType::Bittrex, common::OrderType::SELL,
                         common::OrderType::BUY, sellOrder.databaseId_, buyOrder.databaseId_);

  auto ordersMatching = db.browseOrdersMatching(common::OrderType::SELL, common::OrderType::BUY,
                                                "BTCUSD", common::StockExchangeType::Bittrex);

  unsigned int ordersCount = 0;

  ordersMatching.forEachMatching(
      [&](const common::MarketOrder& from, const common::MarketOrder& to) { ++ordersCount; });

  EXPECT_EQ(ordersCount, 0);

  auto orders = db.browseMarketOrders(common::StockExchangeType::Bittrex);
  EXPECT_EQ(orders.size(), 2);

  EXPECT_TRUE(std::find(orders.begin(), orders.end(), buyOrder) != orders.end());
  EXPECT_TRUE(std::find(orders.begin(), orders.end(), sellOrder) != orders.end());
}

TEST_F(DatabaseUTFixture, Remove_Existed_OrdersProfit_9) {
  common::Date opened{1, 2, 3, 4, 5, 6};
  common::Date closed{2, 3, 5, 6, 2, 8};

  common::MarketOrder buyOrder1{0,
                                "ff-01",
                                common::Currency::BTC,
                                common::Currency::USD,
                                common::OrderType::BUY,
                                common::StockExchangeType::Bittrex,
                                1.45,
                                2.21,
                                opened,
                                false};

  common::MarketOrder buyOrder2{0,
                                "ff-02",
                                common::Currency::BTC,
                                common::Currency::USD,
                                common::OrderType::BUY,
                                common::StockExchangeType::Bittrex,
                                1.3252,
                                2.2351,
                                opened,
                                false};

  auto& db = GetDatabase();
  db.insertMarketOrder(buyOrder1);
  buyOrder1.databaseId_ = db.getLastInsertRowId();
  EXPECT_EQ(buyOrder1.databaseId_, 1);

  db.insertMarketOrder(buyOrder2);
  buyOrder2.databaseId_ = db.getLastInsertRowId();
  EXPECT_EQ(buyOrder2.databaseId_, 2);

  db.insertOrderProfit(common::StockExchangeType::Bittrex, common::Currency::USD,
                       buyOrder1.databaseId_);
  db.insertOrderProfit(common::StockExchangeType::Bittrex, common::Currency::USD,
                       buyOrder2.databaseId_);

  db.removeOrderProfit(common::StockExchangeType::Bittrex, common::Currency::USD,
                       buyOrder1.databaseId_);
  db.removeOrderProfit(common::StockExchangeType::Bittrex, common::Currency::USD,
                       buyOrder2.databaseId_);

  auto ordersProfit = db.browseOrdersProfit(common::StockExchangeType::Bittrex);
  EXPECT_TRUE(ordersProfit.empty());

  auto orders = db.browseMarketOrders(common::StockExchangeType::Bittrex);
  EXPECT_EQ(orders.size(), 2);

  EXPECT_TRUE(std::find(orders.begin(), orders.end(), buyOrder1) != orders.end());
  EXPECT_TRUE(std::find(orders.begin(), orders.end(), buyOrder2) != orders.end());
}

TEST_F(DatabaseUTFixture, Remove_MarketOrder_Run_Matching_10) {
  common::Date opened{1, 2, 3, 4, 5, 6};
  common::Date closed{2, 3, 5, 6, 2, 1};

  common::MarketOrder buyOrder{0,
                               "ff-01",
                               common::Currency::BTC,
                               common::Currency::USD,
                               common::OrderType::BUY,
                               common::StockExchangeType::Bittrex,
                               1.45,
                               2.21,
                               opened,
                               false};

  common::MarketOrder sellOrder{0,
                                "ff-02",
                                common::Currency::USD,
                                common::Currency::BTC,
                                common::OrderType::SELL,
                                common::StockExchangeType::Bittrex,
                                1.3252,
                                2.2351,
                                opened,
                                false};

  auto& db = GetDatabase();
  db.insertMarketOrder(buyOrder);
  buyOrder.databaseId_ = db.getLastInsertRowId();
  EXPECT_EQ(buyOrder.databaseId_, 1);

  db.insertMarketOrder(sellOrder);
  sellOrder.databaseId_ = db.getLastInsertRowId();
  EXPECT_EQ(sellOrder.databaseId_, 2);

  db.removeMarketOrder(buyOrder);
  db.removeMarketOrder(sellOrder);

  unsigned int ordersCount = 0;
  auto matching = db.browseOrdersMatching(common::OrderType::SELL, common::OrderType::BUY, "USDBTC",
                                          common::StockExchangeType::Bittrex);
  matching.forEachMatching(
      [&](const common::MarketOrder& from, const common::MarketOrder& to) { ++ordersCount; });

  EXPECT_EQ(ordersCount, 0);
}

TEST_F(DatabaseUTFixture, Remove_MarketOrder_And_Browse_OrdersProfit_11) {
  common::Date opened{1, 2, 3, 4, 5, 6};

  common::MarketOrder buyOrder1{0,
                                "ff-01",
                                common::Currency::BTC,
                                common::Currency::USD,
                                common::OrderType::BUY,
                                common::StockExchangeType::Bittrex,
                                1.45,
                                2.21,
                                opened,
                                false};

  common::MarketOrder buyOrder2{0,
                                "ff-02",
                                common::Currency::BTC,
                                common::Currency::USD,
                                common::OrderType::BUY,
                                common::StockExchangeType::Bittrex,
                                1.3252,
                                2.2351,
                                opened,
                                false};

  auto& db = GetDatabase();
  db.insertMarketOrder(buyOrder1);
  buyOrder1.databaseId_ = db.getLastInsertRowId();
  EXPECT_EQ(buyOrder1.databaseId_, 1);

  db.insertMarketOrder(buyOrder2);
  buyOrder2.databaseId_ = db.getLastInsertRowId();
  EXPECT_EQ(buyOrder2.databaseId_, 2);

  db.removeMarketOrder(buyOrder1);
  db.removeMarketOrder(buyOrder2);

  auto ordersProfit = db.browseOrdersProfit(common::StockExchangeType::Bittrex);
  EXPECT_TRUE(ordersProfit.empty());
}

TEST_F(DatabaseUTFixture, Add_lastMarketData_12) {
  common::Date opened{1, 2, 3, 4, 5, 6};

  common::MarketData marketData{3899.354, 3910.847, 3899.354, 3910.847, 31771.19121};
  marketData.date_ = opened;

  auto& db = GetDatabase();
  db.insertMarketData(common::StockExchangeType::Bittrex, common::Currency::BTC,
                      common::Currency::USD, common::StrategiesType::SMA, marketData);

  auto lastMarketDataCollection = db.browseLastMarketData(
      common::StockExchangeType::Bittrex, common::Currency::BTC, common::Currency::USD);
  auto dataIterator = lastMarketDataCollection.find(common::StrategiesType::SMA);
  EXPECT_TRUE(dataIterator != lastMarketDataCollection.end());
  EXPECT_TRUE(dataIterator->second == marketData);
}

TEST_F(DatabaseUTFixture, Add_Remove_lastMarketData_13) {
  common::Date opened{1, 2, 3, 4, 5, 6};

  common::MarketData marketData{3899.354, 3910.847, 3899.354, 3910.847, 31771.19121};
  marketData.date_ = opened;

  auto& db = GetDatabase();
  db.insertMarketData(common::StockExchangeType::Bittrex, common::Currency::BTC,
                      common::Currency::USD, common::StrategiesType::SMA, marketData);

  db.removeMarketData(common::StockExchangeType::Bittrex, common::Currency::BTC,
                      common::Currency::USD, common::StrategiesType::SMA);

  auto lastMarketDataCollection = db.browseLastMarketData(
      common::StockExchangeType::Bittrex, common::Currency::BTC, common::Currency::USD);

  EXPECT_TRUE(lastMarketDataCollection.empty());
}

TEST_F(DatabaseUTFixture, Remove_Market_Orders_14) {
  common::Date opened{1, 2, 3, 4, 5, 6};

  common::MarketOrder order{0,
                            "ff-01",
                            common::Currency::BTC,
                            common::Currency::USD,
                            common::OrderType::BUY,
                            common::StockExchangeType::Bittrex,
                            1.45,
                            2.21,
                            opened,
                            false};

  common::MarketOrder order2{0,
                             "ff-02",
                             common::Currency::BTC,
                             common::Currency::USD,
                             common::OrderType::BUY,
                             common::StockExchangeType::Bittrex,
                             1.46,
                             2.22,
                             opened,
                             false};

  auto& db = GetDatabase();
  db.insertMarketOrder(order);
  db.insertMarketOrder(order2);
  auto orders = db.browseMarketOrders(common::StockExchangeType::Bittrex);

  EXPECT_EQ(orders.size(), 2);

  db.removeMarketOrders(common::Currency::USD, common::Currency::BTC,
                        common::StockExchangeType::Bittrex);

  orders = db.browseMarketOrders(common::StockExchangeType::Bittrex);

  EXPECT_EQ(orders.size(), 0);
}

}  // namespace unit_test
}  // namespace database
}  // namespace auto_trader
