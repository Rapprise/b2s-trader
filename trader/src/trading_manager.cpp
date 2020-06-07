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

#include "include/trading_manager.h"

#include <QDateTime>
#include <exception>
#include <thread>

#include "common/loggers/file_logger.h"
#include "features/include/stop_loss_announcer.h"
#include "features/include/telegram_announcer.h"
#include "include/trading_processor.h"
#include "model/include/orders/orders_profit.h"
#include "stocks_exchange/include/query_processor.h"
#include "stocks_exchange/include/stock_exchange_utils.h"

namespace auto_trader {
namespace trader {

static std::set<common::MarketOrder> getOrdersForType(
    const std::vector<common::MarketOrder> &allOrders, common::OrderType type) {
  std::set<common::MarketOrder> ordersForType;
  for (const auto &order : allOrders) {
    if (order.orderType_ == type) ordersForType.insert(order);
  }

  return ordersForType;
}

TradingManager::TradingManager(
    stock_exchange::QueryProcessor &queryProcessor, strategies::StrategyFacade &strategyFacade,
    database::Database &databaseProvider, common::AppListener &appListener,
    common::GuiListener &guiListener, model::AppSettings &appSettings,
    TradingMessageSender &messageSender, model::StrategiesSettingsHolder &strategiesSettingsHolder,
    model::TradeOrdersHolder &tradeOrdersHolder, model::TradeConfigsHolder &tradeConfigsHolder,
    model::TradeSignaledStrategyMarketHolder &tradeSignaledStrategyMarketHolder)
    : strategyFacade_(strategyFacade),
      queryProcessor_(queryProcessor),
      databaseProvider_(databaseProvider),
      appListener_(appListener),
      guiListener_(guiListener),
      appSettings_(appSettings),
      strategiesSettingsHolder_(strategiesSettingsHolder),
      tradeOrdersHolder_(tradeOrdersHolder),
      tradeConfigsHolder_(tradeConfigsHolder),
      tradeSignaledStrategyMarketHolder_(tradeSignaledStrategyMarketHolder),
      messageSender_(messageSender),
      isRunning_(false),
      isReset_(false) {}

void TradingManager::startTradingSlot() {
  isRunning_ = true;
  auto &currentTradeConfiguration = tradeConfigsHolder_.takeCurrentTradeConfiguration();
  currentTradeConfiguration.start();

  messageSender_.setDefaultPrefix();

  if (QCoreApplication::instance()) {
    messageSender_.sendMessage("TRADING STARTED.");

    if (isDataExists()) {
      emit tradingDataOutdated();
    }

    emit tradingStarted();
  }

  if (isReset_) {
    resetData();
  }

  currencyLotsHolder_.clear();

  auto stockExchangeType = currentTradeConfiguration.getStockExchangeSettings().stockExchangeType_;
  auto query = queryProcessor_.getQuery(stockExchangeType);
  currencyLotsHolder_ = query->getCurrencyLotsHolder();

  loadOrders();

  while (currentTradeConfiguration.isRunning() && isRunning_) {
    prepareBuying();
    prepareSelling();

    uploadBuyingOrders();

    if (!isRunning()) {
      break;
    }

    uploadSellOrders();

    if (!isRunning()) {
      break;
    }

    runStopLoss();

    appListener_.refreshTradingView();

    std::unique_lock<std::mutex> lock(locker_);
    std::chrono::duration<int, std::milli> milli_seconds_type{appSettings_.tradingTimeout_ * 60 *
                                                              1000};
    condVar_.wait_for(lock, milli_seconds_type, [&]() { return !isRunning_; });
  }

  if (QCoreApplication::instance()) {
    messageSender_.setDefaultPrefix();
    messageSender_.sendMessage("TRADING STOPPED.");
    emit tradingStopped();
  }

  currentTradeConfiguration.stop();
}

void TradingManager::stopTradingSlot() {
  if (!isRunning_) return;

  std::unique_lock<std::mutex> lock(locker_);
  isRunning_ = false;
  isReset_ = false;

  currencyLotsHolder_.clear();

  condVar_.notify_all();
}

bool TradingManager::isRunning() const { return isRunning_; }

void TradingManager::reset(bool value) { isReset_ = value; }

void TradingManager::prepareBuying() {
  try {
    messageSender_.setBuyingPrefix();
    auto &currentTradeConfiguration = tradeConfigsHolder_.takeCurrentTradeConfiguration();
    auto stockExchangeType =
        currentTradeConfiguration.getStockExchangeSettings().stockExchangeType_;
    auto query = queryProcessor_.getQuery(stockExchangeType);
    auto coinSettings = currentTradeConfiguration.getCoinSettings();

    std::vector<common::MarketOrder> allOrders;
    for (const auto &tradedCoin : coinSettings.tradedCurrencies_) {
      std::vector<common::MarketOrder> orders =
          query->getAccountOpenOrders(coinSettings.baseCurrency_, tradedCoin);
      std::copy(orders.begin(), orders.end(), std::back_inserter(allOrders));
    }

    auto ordersToBuy = getOrdersForType(allOrders, common::OrderType::BUY);
    auto updatedOrders = updateManuallyOpenedBuyingOrders(ordersToBuy);

    updateClosedBuyingOrders(updatedOrders);
    cancelOutdatedBuyingOrders(updatedOrders);

  } catch (std::exception &exception) {
    messageSender_.sendMessage(exception.what());
  }
}

void TradingManager::cancelOutdatedBuyingOrders(
    const std::set<auto_trader::common::MarketOrder> &openOrders) {
  auto &currentTradeConfiguration = tradeConfigsHolder_.takeCurrentTradeConfiguration();
  auto stockExchangeType = currentTradeConfiguration.getStockExchangeSettings().stockExchangeType_;
  auto query = queryProcessor_.getQuery(stockExchangeType);

  for (auto &order : openOrders) {
    auto maxOpenTime = currentTradeConfiguration.getBuySettings().maxOpenTime_;
    auto opened = tradeOrdersHolder_.getLocalTimestampFromBuyOrder(order);
    QDateTime openedDateTime(QDate(opened.year_, opened.month_, opened.day_),
                             QTime(opened.hour_, opened.minute_, opened.second_));
    auto outdatedOrderDate = openedDateTime.addSecs(maxOpenTime * 60);
    auto currentDate = QDateTime::currentDateTime();

    if (currentDate > outdatedOrderDate) {
      bool orderCanceled = query->cancelOrder(order.fromCurrency_, order.toCurrency_, order.uuid_);
      if (orderCanceled) {
        const std::string message = "Cancel outdated order : [ " + order.toString() + " ]";
        messageSender_.sendMessage(message);
        auto &telegramAnnouncer = features::telegram_announcer::TelegramAnnouncer::instance();
        if (telegramAnnouncer.isLoggingEnabled()) {
          telegramAnnouncer.sendMessage(message);
        }

        tradeOrdersHolder_.removeBuyOrder(order);
        databaseProvider_.removeMarketOrder(order);
      }
    }
  }
}

void TradingManager::updateClosedBuyingOrders(const std::set<common::MarketOrder> &openOrders) {
  auto &currentTradeConfiguration = tradeConfigsHolder_.takeCurrentTradeConfiguration();
  auto stockExchangeType = currentTradeConfiguration.getStockExchangeSettings().stockExchangeType_;

  std::set<common::MarketOrder> difference = tradeOrdersHolder_.getBuyOrdersDiff(openOrders);
  std::lock_guard<std::mutex> lock(locker_);

  for (auto &order : difference) {
    if (isOrderManuallyCanceled(order)) {
      tradeOrdersHolder_.removeBuyOrder(order);
      databaseProvider_.removeMarketOrder(order);
      messageSender_.sendMessage("Order : [ " + order.toString() +
                                 " ] was manually canceled and removed from trading.");
      continue;
    }

    if (tradeOrdersHolder_.containOrdersProfit(order.toCurrency_)) {
      auto &orderProfit = tradeOrdersHolder_.takeOrdersProfit(order.toCurrency_);
      orderProfit.addOrder(order);
    } else {
      model::OrdersProfit ordersProfit(order.toCurrency_);
      ordersProfit.addOrder(order);
      tradeOrdersHolder_.addOrdersProfit(order.toCurrency_, ordersProfit);
    }

    messageSender_.sendMessage("Order : [ " + order.toString() + " ] was closed.");

    tradeOrdersHolder_.removeBuyOrder(order);
    databaseProvider_.insertOrderProfit(stockExchangeType, order.toCurrency_, order.databaseId_);
  }
}

const std::set<common::MarketOrder> TradingManager::updateManuallyOpenedBuyingOrders(
    const std::set<common::MarketOrder> &openOrders) {
  auto &currentTradeConfiguration = tradeConfigsHolder_.takeCurrentTradeConfiguration();
  auto &coinSettings = currentTradeConfiguration.getCoinSettings();
  std::set<common::MarketOrder> updatedOrders;

  auto marketCallback = [&](const common::MarketOrder &order) {
    for (int index = 0; index < coinSettings.tradedCurrencies_.size(); ++index) {
      auto tradedCurrency = coinSettings.tradedCurrencies_.at(index);
      if ((order.fromCurrency_ == coinSettings.baseCurrency_) &&
          (tradedCurrency == order.toCurrency_)) {
        auto marketOrder = order;
        marketOrder.opened_ = common::Date::getCurrentTime();
        databaseProvider_.insertMarketOrder(marketOrder);
        marketOrder.databaseId_ = databaseProvider_.getLastInsertRowId();
        tradeOrdersHolder_.addBuyOrder(marketOrder);
        updatedOrders.insert(marketOrder);
        messageSender_.sendMessage("Manually opened order : [ " + marketOrder.toString() +
                                   " ] is added to trading.");
      }
    }
  };

  for (auto &order : openOrders) {
    if (!tradeOrdersHolder_.containBuyOrder(order)) {
      marketCallback(order);
    } else {
      auto marketOrder = order;
      marketOrder.databaseId_ = tradeOrdersHolder_.getBuyOrderDatabaseId(marketOrder);
      updatedOrders.insert(marketOrder);
    }
  }

  return updatedOrders;
}

void TradingManager::uploadBuyingOrders() {
  try {
    messageSender_.setBuyingPrefix();

    auto &currentTradeConfiguration = tradeConfigsHolder_.takeCurrentTradeConfiguration();
    TradingProcessor processor(queryProcessor_, strategyFacade_, databaseProvider_, appListener_,
                               strategiesSettingsHolder_, tradeOrdersHolder_,
                               tradeSignaledStrategyMarketHolder_, currentTradeConfiguration,
                               messageSender_, currencyLotsHolder_, *this);
    processor.run();
  } catch (std::exception &exception) {
    messageSender_.sendMessage(exception.what());
  }
}

void TradingManager::prepareSelling() {
  try {
    messageSender_.setSellingPrefix();
    auto &currentTradeConfiguration = tradeConfigsHolder_.takeCurrentTradeConfiguration();
    auto stockExchangeType =
        currentTradeConfiguration.getStockExchangeSettings().stockExchangeType_;
    auto query = queryProcessor_.getQuery(stockExchangeType);
    auto coinSettings = currentTradeConfiguration.getCoinSettings();
    std::vector<common::MarketOrder> allOrders;
    for (const auto &tradedCoin : coinSettings.tradedCurrencies_) {
      std::vector<common::MarketOrder> orders =
          query->getAccountOpenOrders(coinSettings.baseCurrency_, tradedCoin);
      std::copy(orders.begin(), orders.end(), std::back_inserter(allOrders));
    }
    auto ordersToSell = getOrdersForType(allOrders, common::OrderType::SELL);
    auto updatedSellOrders = updateManuallyOpenedSellingOrders(ordersToSell);

    cancelOutdatedSellingOrders(updatedSellOrders);
    updateClosedSellingOrders(updatedSellOrders);

  } catch (std::exception &exception) {
    messageSender_.sendMessage(exception.what());
  }
}

void TradingManager::updateClosedSellingOrders(
    const std::set<auto_trader::common::MarketOrder> &orders) {
  auto difference = tradeOrdersHolder_.getSellOrdersDiff(orders);
  auto &currentTradeConfiguration = tradeConfigsHolder_.takeCurrentTradeConfiguration();
  auto stockExchangeType = currentTradeConfiguration.getStockExchangeSettings().stockExchangeType_;
  auto &orderMatching = tradeOrdersHolder_.takeOrderMatching();

  for (auto &sellOrder : difference) {
    if (isOrderManuallyCanceled(sellOrder)) {
      tradeOrdersHolder_.removeSellOrder(sellOrder);
      auto buyingOrder = orderMatching.getMatchedOrder(sellOrder);
      if (tradeOrdersHolder_.containOrdersProfit(sellOrder.toCurrency_)) {
        auto &ordersProfit = tradeOrdersHolder_.takeOrdersProfit(sellOrder.toCurrency_);
        ordersProfit.addOrder(buyingOrder);
      } else {
        model::OrdersProfit ordersProfit(sellOrder.toCurrency_);
        ordersProfit.addOrder(buyingOrder);
        tradeOrdersHolder_.addOrdersProfit(sellOrder.toCurrency_, ordersProfit);
      }
      orderMatching.removeMatching(sellOrder);

      databaseProvider_.removeOrderMatching(stockExchangeType, common::OrderType::SELL,
                                            common::OrderType::BUY, sellOrder.databaseId_,
                                            buyingOrder.databaseId_);

      databaseProvider_.removeMarketOrder(sellOrder);
      databaseProvider_.insertOrderProfit(stockExchangeType, buyingOrder.toCurrency_,
                                          buyingOrder.databaseId_);

      messageSender_.sendMessage("Order : [ " + sellOrder.toString() + " ] was manually canceled.");
      continue;
    }

    tradeOrdersHolder_.removeSellOrder(sellOrder);
    auto matchedBuyOrder = orderMatching.getMatchedOrder(sellOrder);
    orderMatching.removeMatching(sellOrder);
    databaseProvider_.removeOrderMatching(stockExchangeType, common::OrderType::SELL,
                                          common::OrderType::BUY, sellOrder.databaseId_,
                                          matchedBuyOrder.databaseId_);

    databaseProvider_.removeMarketOrder(matchedBuyOrder);
    databaseProvider_.removeMarketOrder(sellOrder);

    messageSender_.sendMessage("Order : [ " + sellOrder.toString() + " ] was closed.");
  }
}

void TradingManager::cancelOutdatedSellingOrders(const std::set<common::MarketOrder> &orders) {
  std::lock_guard<std::mutex> lock(locker_);

  auto &currentTradeConfiguration = tradeConfigsHolder_.takeCurrentTradeConfiguration();
  auto stockExchangeType = currentTradeConfiguration.getStockExchangeSettings().stockExchangeType_;
  auto query = queryProcessor_.getQuery(stockExchangeType);
  auto &orderMatching = tradeOrdersHolder_.takeOrderMatching();

  for (const auto &order : orders) {
    auto maxOpenTime = currentTradeConfiguration.getSellSettings().openOrderTime_;
    auto opened = tradeOrdersHolder_.getLocalTimestampFromSellOrder(order);
    QDateTime openedDateTime(QDate(opened.year_, opened.month_, opened.day_),
                             QTime(opened.hour_, opened.minute_, opened.second_));

    auto outdatedOrderDate = openedDateTime.addSecs(maxOpenTime * 60);
    auto currentDate = QDateTime::currentDateTime();
    if (currentDate > outdatedOrderDate) {
      bool canceledOrder = query->cancelOrder(order.fromCurrency_, order.toCurrency_, order.uuid_);
      if (canceledOrder) {
        const std::string message = "Cancel outdated order : [ " + order.toString() + " ]";
        messageSender_.sendMessage(message);

        auto &telegramAnnouncer = features::telegram_announcer::TelegramAnnouncer::instance();
        if (telegramAnnouncer.isLoggingEnabled()) {
          telegramAnnouncer.sendMessage(message);
        }

        tradeOrdersHolder_.removeSellOrder(order);
        auto buyingOrder = orderMatching.getMatchedOrder(order);
        if (tradeOrdersHolder_.containOrdersProfit(order.toCurrency_)) {
          auto &ordersProfit = tradeOrdersHolder_.takeOrdersProfit(order.toCurrency_);
          ordersProfit.addOrder(buyingOrder);
        } else {
          model::OrdersProfit ordersProfit(order.toCurrency_);
          ordersProfit.addOrder(buyingOrder);
          tradeOrdersHolder_.addOrdersProfit(order.toCurrency_, ordersProfit);
        }

        orderMatching.removeMatching(order);

        databaseProvider_.removeOrderMatching(stockExchangeType, common::OrderType::SELL,
                                              common::OrderType::BUY, order.databaseId_,
                                              buyingOrder.databaseId_);

        databaseProvider_.removeMarketOrder(order);
        databaseProvider_.insertOrderProfit(stockExchangeType, buyingOrder.toCurrency_,
                                            buyingOrder.databaseId_);
      }
    }
  }
}

void TradingManager::uploadSellOrders() {
  try {
    messageSender_.setSellingPrefix();
    auto &currentTradeConfiguration = tradeConfigsHolder_.takeCurrentTradeConfiguration();
    auto &stockExchangeSettings = currentTradeConfiguration.getStockExchangeSettings();
    auto &sellSettings = currentTradeConfiguration.getSellSettings();
    auto &coinSettings = currentTradeConfiguration.getCoinSettings();
    auto query = queryProcessor_.getQuery(stockExchangeSettings.stockExchangeType_);
    auto &orderMatching = tradeOrdersHolder_.takeOrderMatching();

    std::lock_guard<std::mutex> lock(locker_);

    for (int index = 0; index < coinSettings.tradedCurrencies_.size(); ++index) {
      auto tradedCurrency = coinSettings.tradedCurrencies_.at(index);
      if (!tradeOrdersHolder_.containOrdersProfit(tradedCurrency)) continue;

      auto &ordersProfit = tradeOrdersHolder_.takeOrdersProfit(tradedCurrency);

      std::set<common::MarketOrder> sellOrders;
      ordersProfit.forEachOrder(
          [&](const common::MarketOrder &order) { sellOrders.insert(order); });

      for (auto order : sellOrders) {
        double profitDelta = order.price_ * (sellSettings.profitPercentage_ / 100);
        double boughtPrice = order.price_ + profitDelta;
        auto currentTick = query->getCurrencyTick(order.fromCurrency_, order.toCurrency_);
        if (currentTick.bid_ >= boughtPrice) {
          double quantity = calculateOrderQuantity(order, ordersProfit);
          if (quantity == 0) {
            resetOrderProfit(ordersProfit);
            return;
          }

          quantity = calculateLotSize(tradedCurrency, quantity);
          if (quantity == 0) {
            continue;
          }

          common::MarketOrder currentOrder = query->sellOrder(
              coinSettings.baseCurrency_, tradedCurrency, quantity, currentTick.bid_);

          const std::string message = "Opened sell order : [ " + currentOrder.toString() + " ]";
          messageSender_.sendMessage(message);

          auto &telegramAnnouncer = features::telegram_announcer::TelegramAnnouncer::instance();
          if (telegramAnnouncer.isLoggingEnabled()) {
            telegramAnnouncer.sendMessage(message);
          }

          databaseProvider_.insertMarketOrder(currentOrder);
          currentOrder.databaseId_ = databaseProvider_.getLastInsertRowId();

          auto currencyPair = common::Currency::toString(order.fromCurrency_) +
                              common::Currency::toString(order.toCurrency_);

          tradeOrdersHolder_.addSellOrder(currentOrder);
          orderMatching.addOrderMatching(currentOrder, order);

          databaseProvider_.insertOrderMatching(
              stockExchangeSettings.stockExchangeType_, currencyPair, common::OrderType::SELL,
              common::OrderType::BUY, currentOrder.databaseId_, order.databaseId_);

          ordersProfit.removeOrder(order);
          databaseProvider_.removeOrderProfit(stockExchangeSettings.stockExchangeType_,
                                              order.toCurrency_, order.databaseId_);
        }
      }
    }
  } catch (std::exception &exception) {
    messageSender_.sendMessage(exception.what());
  }
}

void TradingManager::runStopLoss() {
  try {
    messageSender_.setSellingPrefix();
    auto &currentTradeConfiguration = tradeConfigsHolder_.takeCurrentTradeConfiguration();
    auto &stockExchangeSettings = currentTradeConfiguration.getStockExchangeSettings();
    auto &sellSettings = currentTradeConfiguration.getSellSettings();
    auto &coinSettings = currentTradeConfiguration.getCoinSettings();
    auto query = queryProcessor_.getQuery(stockExchangeSettings.stockExchangeType_);
    auto &orderMatching = tradeOrdersHolder_.takeOrderMatching();

    std::lock_guard<std::mutex> lock(locker_);

    for (int index = 0; index < coinSettings.tradedCurrencies_.size(); ++index) {
      auto tradedCurrency = coinSettings.tradedCurrencies_[index];
      if (!tradeOrdersHolder_.containOrdersProfit(tradedCurrency)) continue;

      auto &ordersProfit = tradeOrdersHolder_.takeOrdersProfit(tradedCurrency);

      std::set<common::MarketOrder> stopLossOrders;
      ordersProfit.forEachOrder(
          [&](const common::MarketOrder &order) { stopLossOrders.insert(order); });

      for (auto order : stopLossOrders) {
        auto currentTick = query->getCurrencyTick(order.fromCurrency_, order.toCurrency_);
        auto &stopLossAnnounces = features::stop_loss_announcer::StopLossAnnouncer::instance();
        double stopLossPercentage = stopLossAnnounces.getValue() / 100;
        double stopLossEdge = order.price_ * (1 - stopLossPercentage);
        if (currentTick.ask_ <= stopLossEdge) {
          double quantity = calculateOrderQuantity(order, ordersProfit);
          if (quantity == 0) {
            resetOrderProfit(ordersProfit);
            return;
          }

          quantity = calculateLotSize(tradedCurrency, quantity);
          if (quantity == 0) {
            continue;
          }

          common::MarketOrder currentOrder = query->sellOrder(
              coinSettings.baseCurrency_, tradedCurrency, quantity, currentTick.bid_);

          const std::string message =
              "STOP LOSS SIGNAL: Open order : [ " + currentOrder.toString() + " ]";
          messageSender_.sendMessage(message);

          auto &telegramAnnouncer = features::telegram_announcer::TelegramAnnouncer::instance();
          if (telegramAnnouncer.isLoggingEnabled()) {
            telegramAnnouncer.sendMessage(message);
          }

          databaseProvider_.insertMarketOrder(currentOrder);
          currentOrder.databaseId_ = databaseProvider_.getLastInsertRowId();

          auto currencyPair = common::Currency::toString(order.fromCurrency_) +
                              common::Currency::toString(order.toCurrency_);

          tradeOrdersHolder_.addSellOrder(currentOrder);
          orderMatching.addOrderMatching(currentOrder, order);

          databaseProvider_.insertOrderMatching(
              stockExchangeSettings.stockExchangeType_, currencyPair, common::OrderType::SELL,
              common::OrderType::BUY, currentOrder.databaseId_, order.databaseId_);

          ordersProfit.removeOrder(order);
          databaseProvider_.removeOrderProfit(stockExchangeSettings.stockExchangeType_,
                                              order.toCurrency_, order.databaseId_);
        }
      }
    }
  } catch (std::exception &exception) {
    messageSender_.sendMessage(exception.what());
  }
}

const std::set<common::MarketOrder> TradingManager::updateManuallyOpenedSellingOrders(
    const std::set<common::MarketOrder> &orders) {
  std::set<common::MarketOrder> updatedOrders;
  for (auto &order : orders) {
    // User manually open sell order. Lets skip this one.
    if (!tradeOrdersHolder_.containSellOrder(order)) {
      messageSender_.sendMessage("Manually opened order : [ " + order.toString() +
                                 " ] is ignored and will not be included in trading.");
    } else {
      auto marketOrder = order;
      marketOrder.databaseId_ = tradeOrdersHolder_.getSellOrderDatabaseId(marketOrder);
      updatedOrders.insert(marketOrder);
    }
  }

  return updatedOrders;
}

bool TradingManager::isOrderManuallyCanceled(const common::MarketOrder &order) const {
  auto &currentTradeConfiguration = tradeConfigsHolder_.takeCurrentTradeConfiguration();
  auto stockExchangeType = currentTradeConfiguration.getStockExchangeSettings().stockExchangeType_;
  auto query = queryProcessor_.getQuery(stockExchangeType);
  auto marketOrder = query->getAccountOrder(order.fromCurrency_, order.toCurrency_, order.uuid_);
  return marketOrder.isCanceled_;
}

void TradingManager::loadOrders() {
  tradeOrdersHolder_.clear();
  tradeSignaledStrategyMarketHolder_.clear();
  auto &currentTradeConfiguration = tradeConfigsHolder_.takeCurrentTradeConfiguration();
  auto &stockExchangeSettings = currentTradeConfiguration.getStockExchangeSettings();
  auto query = queryProcessor_.getQuery(stockExchangeSettings.stockExchangeType_);
  auto &coinSettings = currentTradeConfiguration.getCoinSettings();
  auto &orderMatching = tradeOrdersHolder_.takeOrderMatching();

  auto ordersProfitCollection =
      databaseProvider_.browseOrdersProfit(stockExchangeSettings.stockExchangeType_);
  for (size_t tradingCurrenciesIndex = 0; tradingCurrenciesIndex < ordersProfitCollection.size();
       tradingCurrenciesIndex++) {
    auto &currentOrderProfit = ordersProfitCollection.at(tradingCurrenciesIndex);
    for (size_t index = 0; index < coinSettings.tradedCurrencies_.size(); index++) {
      if (currentOrderProfit.getCurrency() == coinSettings.tradedCurrencies_.at(index)) {
        tradeOrdersHolder_.addOrdersProfit(currentOrderProfit.getCurrency(), currentOrderProfit);
        break;
      }
    }
  }

  for (size_t index = 0; index < coinSettings.tradedCurrencies_.size(); index++) {
    auto marketOrders = databaseProvider_.browseMarketOrders(
        coinSettings.baseCurrency_, coinSettings.tradedCurrencies_.at(index),
        currentTradeConfiguration.getStockExchangeSettings().stockExchangeType_);

    auto buyingDbOrders = getOrdersForType(marketOrders, common::OrderType::BUY);
    for (auto &order : buyingDbOrders) {
      if (tradeOrdersHolder_.containOrdersProfit(order.toCurrency_)) {
        auto &ordersProfit = tradeOrdersHolder_.getOrdersProfit(order.toCurrency_);
        if (ordersProfit.containOrder(order)) {
          continue;
        }
      }
      tradeOrdersHolder_.addBuyOrder(order);
    }
  }

  for (size_t index = 0; index < coinSettings.tradedCurrencies_.size(); index++) {
    auto marketOrders = databaseProvider_.browseMarketOrders(
        coinSettings.baseCurrency_, coinSettings.tradedCurrencies_.at(index),
        currentTradeConfiguration.getStockExchangeSettings().stockExchangeType_);

    auto sellingOrders = getOrdersForType(marketOrders, common::OrderType::SELL);
    for (auto &order : sellingOrders) {
      tradeOrdersHolder_.addSellOrder(order);
    }
  }

  for (size_t index = 0; index < coinSettings.tradedCurrencies_.size(); index++) {
    const std::string currencyPair =
        common::Currency::toString(coinSettings.baseCurrency_) +
        common::Currency::toString(coinSettings.tradedCurrencies_.at(index));

    auto orderMatchingCollection = databaseProvider_.browseOrdersMatching(
        common::OrderType::SELL, common::OrderType::BUY, currencyPair,
        stockExchangeSettings.stockExchangeType_);

    orderMatchingCollection.forEachMatching(
        [&](const common::MarketOrder &fromOrder, const common::MarketOrder &toOrder) {
          orderMatching.addOrderMatching(fromOrder, toOrder);
        });
  }

  orderMatching.forEachMatching(
      [&](const common::MarketOrder &fromOrder, const common::MarketOrder &toOrder) {
        if (tradeOrdersHolder_.containBuyOrder(toOrder)) {
          tradeOrdersHolder_.removeBuyOrder(toOrder);
        }
      });

  for (size_t index = 0; index < coinSettings.tradedCurrencies_.size(); index++) {
    auto tradedCurrency = coinSettings.tradedCurrencies_.at(index);
    auto marketDataCollection = databaseProvider_.browseLastMarketData(
        stockExchangeSettings.stockExchangeType_, coinSettings.baseCurrency_, tradedCurrency);

    for (auto marketDataIterator : marketDataCollection) {
      tradeSignaledStrategyMarketHolder_.addMarket(coinSettings.baseCurrency_, tradedCurrency,
                                                   marketDataIterator.first,
                                                   marketDataIterator.second);
    }
  }
}

bool TradingManager::isDataExists() const {
  auto &currentTradeConfiguration = tradeConfigsHolder_.takeCurrentTradeConfiguration();
  auto &stockExchangeSettings = currentTradeConfiguration.getStockExchangeSettings();
  auto &coinSettings = currentTradeConfiguration.getCoinSettings();

  for (size_t index = 0; index < coinSettings.tradedCurrencies_.size(); index++) {
    auto marketOrders = databaseProvider_.browseMarketOrders(
        coinSettings.baseCurrency_, coinSettings.tradedCurrencies_.at(index),
        stockExchangeSettings.stockExchangeType_);

    if (!marketOrders.empty()) return true;
  }

  for (size_t index = 0; index < coinSettings.tradedCurrencies_.size(); index++) {
    auto marketDataCollection = databaseProvider_.browseLastMarketData(
        stockExchangeSettings.stockExchangeType_, coinSettings.baseCurrency_,
        coinSettings.tradedCurrencies_.at(index));

    if (!marketDataCollection.empty()) {
      return true;
    }
  }

  return false;
}

void TradingManager::resetData() {
  auto &currentTradeConfiguration = tradeConfigsHolder_.takeCurrentTradeConfiguration();
  auto &stockExchangeSettings = currentTradeConfiguration.getStockExchangeSettings();
  auto &coinSettings = currentTradeConfiguration.getCoinSettings();

  for (size_t index = 0; index < coinSettings.tradedCurrencies_.size(); index++) {
    const std::string currencyPair =
        common::Currency::toString(coinSettings.baseCurrency_) +
        common::Currency::toString(coinSettings.tradedCurrencies_.at(index));

    auto orderMatchingCollection = databaseProvider_.browseOrdersMatching(
        common::OrderType::SELL, common::OrderType::BUY, currencyPair,
        stockExchangeSettings.stockExchangeType_);

    orderMatchingCollection.forEachMatching(
        [&](const common::MarketOrder &fromOrder, const common::MarketOrder &toOrder) {
          databaseProvider_.removeOrderMatching(stockExchangeSettings.stockExchangeType_,
                                                common::OrderType::SELL, common::OrderType::BUY,
                                                fromOrder.databaseId_, toOrder.databaseId_);
        });
  }

  auto ordersProfitCollection =
      databaseProvider_.browseOrdersProfit(stockExchangeSettings.stockExchangeType_);
  for (size_t tradingCurrenciesIndex = 0; tradingCurrenciesIndex < ordersProfitCollection.size();
       tradingCurrenciesIndex++) {
    auto &currentOrderProfit = ordersProfitCollection.at(tradingCurrenciesIndex);
    for (size_t index = 0; index < coinSettings.tradedCurrencies_.size(); index++) {
      auto tradedCurrency = coinSettings.tradedCurrencies_.at(index);
      if (currentOrderProfit.getCurrency() == tradedCurrency) {
        currentOrderProfit.forEachOrder([&](const common::MarketOrder &order) {
          databaseProvider_.removeOrderProfit(stockExchangeSettings.stockExchangeType_,
                                              tradedCurrency, order.databaseId_);
        });
      }
    }
  }

  for (size_t index = 0; index < coinSettings.tradedCurrencies_.size(); index++) {
    auto tradedCurrency = coinSettings.tradedCurrencies_.at(index);
    databaseProvider_.removeMarketOrders(coinSettings.baseCurrency_, tradedCurrency,
                                         stockExchangeSettings.stockExchangeType_);
    databaseProvider_.removeMarketData(stockExchangeSettings.stockExchangeType_,
                                       coinSettings.baseCurrency_, tradedCurrency);
  }
}

void TradingManager::resetOrderProfit(model::OrdersProfit &orderProfit) {
  auto &currentTradeConfiguration = tradeConfigsHolder_.takeCurrentTradeConfiguration();
  auto &stockExchangeSettings = currentTradeConfiguration.getStockExchangeSettings();
  common::Currency::Enum tradedCurrency = orderProfit.getCurrency();

  databaseProvider_.removeCurrencyProfit(tradedCurrency, stockExchangeSettings.stockExchangeType_);
  orderProfit.forEachOrder([&](const common::MarketOrder &marketOrder) {
    databaseProvider_.removeMarketOrder(marketOrder);
  });

  orderProfit.clear();
}

double TradingManager::calculateLotSize(common::Currency::Enum tradedCurrency,
                                        double quantity) const {
  auto &currentTradeConfiguration = tradeConfigsHolder_.takeCurrentTradeConfiguration();
  auto &stockExchangeSettings = currentTradeConfiguration.getStockExchangeSettings();
  auto &coinSettings = currentTradeConfiguration.getCoinSettings();

  if (!currencyLotsHolder_.empty()) {
    std::string marketPair = stock_exchange_utils::getMarketPair(
        stockExchangeSettings.stockExchangeType_, coinSettings.baseCurrency_, tradedCurrency);

    auto lotSize = currencyLotsHolder_.getLot(marketPair);
    if (quantity < lotSize.minQty_) {
      const std::string message = "Coins quantity is smaller than allowed minimum for currency " +
                                  common::Currency::toString(tradedCurrency) +
                                  "Order cannot be opened.";
      messageSender_.sendMessage(message);
      return 0;
    }
    double reminder = fmod(quantity, lotSize.stepSize_);
    if (reminder > 0) {
      quantity = quantity - reminder;
    }
  }

  return quantity;
}

double TradingManager::calculateOrderQuantity(const common::MarketOrder &order,
                                              model::OrdersProfit &orderProfit) {
  auto &currentTradeConfiguration = tradeConfigsHolder_.takeCurrentTradeConfiguration();
  auto &stockExchangeSettings = currentTradeConfiguration.getStockExchangeSettings();
  auto &coinSettings = currentTradeConfiguration.getCoinSettings();
  auto query = queryProcessor_.getQuery(stockExchangeSettings.stockExchangeType_);
  auto tradedCurrency = orderProfit.getCurrency();
  double balance = query->getBalance(tradedCurrency);

  if (balance >= order.quantity_) {
    return order.quantity_;
  }

  if (orderProfit.getOrdersCount() > 1) {
    std::string accountBalanceStr = common::MarketOrder::convertCoinToString(balance);
    std::string orderProfitBalanceStr =
        common::MarketOrder::convertCoinToString(orderProfit.getBalance());
    messageSender_.sendMessage("You have more that 1 closed buying orders. Account Balance '" +
                               accountBalanceStr + "' is lower than profit '" +
                               orderProfitBalanceStr +
                               "' from buying orders."
                               "Trading state for currency " +
                               common::Currency::toString(tradedCurrency) + " will be reset.");

    return 0;
  }

  return balance;
}

}  // namespace trader
}  // namespace auto_trader
