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

#include "include/trading_selling_strategy_processor.h"

#include "features/include/stop_loss_announcer.h"
#include "features/include/telegram_announcer.h"
#include "model/include/settings/strategies_settings/bollinger_bands_advanced_settings.h"
#include "model/include/settings/strategies_settings/bollinger_bands_settings.h"
#include "model/include/settings/strategies_settings/custom_strategy_settings.h"
#include "model/include/settings/strategies_settings/ema_settings.h"
#include "model/include/settings/strategies_settings/ma_crossing_settings.h"
#include "model/include/settings/strategies_settings/rsi_settings.h"
#include "model/include/settings/strategies_settings/sma_settings.h"
#include "model/include/settings/strategies_settings/stochastic_oscillator_settings.h"
#include "stocks_exchange/include/query_processor.h"
#include "stocks_exchange/include/stock_exchange_utils.h"

namespace auto_trader {
namespace trader {

TradingSellStrategyProcessor::TradingSellStrategyProcessor(
    stock_exchange::QueryProcessor &queryProcessor, strategies::StrategyFacade &strategiesLibrary,
    database::Database &databaseProvider, common::AppListener &appListener,
    const model::StrategiesSettingsHolder &strategiesSettingsHolder,
    model::TradeOrdersHolder &tradeOrdersHolder, model::TradeConfigsHolder &tradeConfigsHolder,
    model::TradeSignaledStrategyMarketHolder &tradeSignaledStrategyMarketHolder,
    const model::TradeConfiguration &tradeConfiguration, TradingMessageSender &messageSender,
    const stock_exchange::CurrencyLotsHolder &lotsHolder, const TradingManager &tradingManager)
    : queryProcessor_(queryProcessor),
      strategiesLibrary_(strategiesLibrary),
      databaseProvider_(databaseProvider),
      appListener_(appListener),
      tradeConfiguration_(tradeConfiguration),
      strategiesSettingsHolder_(strategiesSettingsHolder),
      tradeOrdersHolder_(tradeOrdersHolder),
      tradeConfigsHolder_(tradeConfigsHolder),
      tradeSignaledStrategyMarketHolder_(tradeSignaledStrategyMarketHolder),
      messageSender_(messageSender),
      lotsHolder_(lotsHolder),
      tradingManager_(tradingManager),
      processingResult(true) {}

void TradingSellStrategyProcessor::runStrategyProcessor() {
  const auto &strategySettings =
      strategiesSettingsHolder_.getCustomStrategy(tradeConfiguration_.getStrategyName());
  const auto &coinSettings = tradeConfiguration_.getCoinSettings();
  const auto &stockExchangeSettings = tradeConfiguration_.getStockExchangeSettings();
  auto query = queryProcessor_.getQuery(stockExchangeSettings.stockExchangeType_);
  auto &orderMatching = tradeOrdersHolder_.takeOrderMatching();

  auto currentTick = query->getCurrencyTick(coinSettings.baseCurrency_, currentTradedCurrency_);

  for (int index = 0; index < coinSettings.tradedCurrencies_.size(); ++index) {
    auto tradedCurrency = coinSettings.tradedCurrencies_[index];
    if (!tradeOrdersHolder_.containOrdersProfit(tradedCurrency)) continue;

    if (!processingResult) continue;

    if (!tradingManager_.isRunning()) {
      return;
    }

    currentTradedCurrency_ = tradedCurrency;

    auto &ordersProfit = tradeOrdersHolder_.takeOrdersProfit(tradedCurrency);

    std::set<common::MarketOrder> orders;
    ordersProfit.forEachOrder([&](const common::MarketOrder &order) { orders.insert(order); });

    if (orders.empty()) {
      continue;
    }

    strategySettings.accept(*this);

    for (auto &order : orders) {
      double quantity = calculateOrderQuantity(order, ordersProfit);
      if (quantity == 0) {
        resetOrderProfit(ordersProfit);
        return;
      }

      openOrder(order, quantity, currentTick.bid_, "STRATEGY SIGNAL: Opened sell order");
    }
  }
}

void TradingSellStrategyProcessor::visit(const model::BollingerBandsSettings &bandsSettings) {
  const auto &stockExchangeSettings = tradeConfiguration_.getStockExchangeSettings();
  const auto &coinSettings = tradeConfiguration_.getCoinSettings();
  bool anyIndicatorTriggered =
      tradeConfiguration_.getBuySettings().openOrderWhenAnyIndicatorIsTriggered_;

  auto marketHistory = getMarketHistory(bandsSettings.tickInterval_);
  if (marketHistory->marketData_.empty()) {
    processingResult = (anyIndicatorTriggered) ? (processingResult | false) : (false);
    return;
  }

  auto currentStrategy = strategiesLibrary_.getBollingerBandStrategy();
  currentStrategy->createLines(marketHistory->marketData_, bandsSettings.period_,
                               bandsSettings.bbInputType_, bandsSettings.standardDeviations_,
                               bandsSettings.crossingInterval_, bandsSettings.lastBuyCrossingPoint_,
                               bandsSettings.lastSellCrossingPoint_);

  processingResult = (anyIndicatorTriggered) ? (processingResult | currentStrategy->isNeedToSell())
                                             : (processingResult & currentStrategy->isNeedToSell());
}

void TradingSellStrategyProcessor::visit(
    const model::BollingerBandsAdvancedSettings &bandsAdvancedSettings) {
  const auto &stockExchangeSettings = tradeConfiguration_.getStockExchangeSettings();
  const auto &coinSettings = tradeConfiguration_.getCoinSettings();
  bool anyIndicatorTriggered =
      tradeConfiguration_.getBuySettings().openOrderWhenAnyIndicatorIsTriggered_;

  auto marketHistory = getMarketHistory(bandsAdvancedSettings.tickInterval_);
  if (marketHistory->marketData_.empty()) {
    processingResult = (anyIndicatorTriggered) ? (processingResult | false) : (false);
    return;
  }

  auto currentStrategy = strategiesLibrary_.getBollingerBandAdvanceStrategy();
  currentStrategy->setPercentageForBottomLine(bandsAdvancedSettings.bottomLinePercentage_);
  currentStrategy->setPercentageForTopLine(bandsAdvancedSettings.topLinePercentage_);

  currentStrategy->createLines(
      marketHistory->marketData_, bandsAdvancedSettings.period_, bandsAdvancedSettings.bbInputType_,
      bandsAdvancedSettings.standardDeviations_, bandsAdvancedSettings.crossingInterval_,
      bandsAdvancedSettings.lastBuyCrossingPoint_, bandsAdvancedSettings.lastSellCrossingPoint_);

  processingResult = (anyIndicatorTriggered) ? (processingResult | currentStrategy->isNeedToSell())
                                             : (processingResult & currentStrategy->isNeedToSell());
}

void TradingSellStrategyProcessor::visit(const model::RsiSettings &rsiSettings) {
  const auto &stockExchangeSettings = tradeConfiguration_.getStockExchangeSettings();
  const auto &coinSettings = tradeConfiguration_.getCoinSettings();

  bool anyIndicatorTriggered =
      tradeConfiguration_.getBuySettings().openOrderWhenAnyIndicatorIsTriggered_;
  auto marketHistory = getMarketHistory(rsiSettings.tickInterval_);
  if (marketHistory->marketData_.empty()) {
    processingResult = (anyIndicatorTriggered) ? (processingResult | false) : (false);
    return;
  }

  auto currentStrategy = strategiesLibrary_.getRsiStrategy();
  currentStrategy->setTopRsiIndex(rsiSettings.topLevel_);
  currentStrategy->setBottomRsiIndex(rsiSettings.bottomLevel_);

  currentStrategy->createLine(marketHistory->marketData_, rsiSettings.period_,
                              rsiSettings.crossingInterval_, rsiSettings.lastBuyCrossingPoint_,
                              rsiSettings.lastSellCrossingPoint_);

  processingResult = (anyIndicatorTriggered) ? (processingResult | currentStrategy->isNeedToBuy())
                                             : (processingResult & currentStrategy->isNeedToBuy());
}

void TradingSellStrategyProcessor::visit(const model::EmaSettings &emaSettings) {
  const auto &stockExchangeSettings = tradeConfiguration_.getStockExchangeSettings();
  const auto &coinSettings = tradeConfiguration_.getCoinSettings();

  bool anyIndicatorTriggered =
      tradeConfiguration_.getBuySettings().openOrderWhenAnyIndicatorIsTriggered_;
  auto marketHistory = getMarketHistory(emaSettings.tickInterval_);
  if (marketHistory->marketData_.empty()) {
    processingResult = (anyIndicatorTriggered) ? (processingResult | false) : (false);
    return;
  }

  auto currentStrategy = strategiesLibrary_.getEmaStrategy();
  currentStrategy->createLine(marketHistory->marketData_, emaSettings.period_,
                              emaSettings.crossingInterval_, emaSettings.lastBuyCrossingPoint_,
                              emaSettings.lastSellCrossingPoint_);

  processingResult = (anyIndicatorTriggered) ? (processingResult | currentStrategy->isNeedToBuy())
                                             : (processingResult & currentStrategy->isNeedToBuy());
}

void TradingSellStrategyProcessor::visit(const model::SmaSettings &smaSettings) {
  const auto &stockExchangeSettings = tradeConfiguration_.getStockExchangeSettings();
  const auto &coinSettings = tradeConfiguration_.getCoinSettings();

  bool anyIndicatorTriggered =
      tradeConfiguration_.getBuySettings().openOrderWhenAnyIndicatorIsTriggered_;
  auto marketHistory = getMarketHistory(smaSettings.tickInterval_);
  if (marketHistory->marketData_.empty()) {
    processingResult = (anyIndicatorTriggered) ? (processingResult | false) : (false);
    return;
  }

  auto currentStrategy = strategiesLibrary_.getSmaStrategy();
  currentStrategy->createLine(marketHistory->marketData_, smaSettings.period_,
                              smaSettings.crossingInterval_, smaSettings.lastBuyCrossingPoint_,
                              smaSettings.lastSellCrossingPoint_);

  processingResult = (anyIndicatorTriggered) ? (processingResult | currentStrategy->isNeedToBuy())
                                             : (processingResult & currentStrategy->isNeedToBuy());
}

void TradingSellStrategyProcessor::visit(
    const model::MovingAveragesCrossingSettings &movingAveragesCrossingSettings) {
  const auto &stockExchangeSettings = tradeConfiguration_.getStockExchangeSettings();
  const auto &coinSettings = tradeConfiguration_.getCoinSettings();

  bool anyIndicatorTriggered =
      tradeConfiguration_.getBuySettings().openOrderWhenAnyIndicatorIsTriggered_;
  auto marketHistory = getMarketHistory(movingAveragesCrossingSettings.tickInterval_);
  if (marketHistory->marketData_.empty()) {
    processingResult = (anyIndicatorTriggered) ? (processingResult | false) : (false);
    return;
  }
  auto currentStrategy = strategiesLibrary_.getMACrossingStrategy();
  currentStrategy->setCrossingInterval(movingAveragesCrossingSettings.crossingInterval_);
  currentStrategy->createLines(marketHistory->marketData_,
                               movingAveragesCrossingSettings.smallerPeriod_,
                               movingAveragesCrossingSettings.biggerPeriod_,
                               movingAveragesCrossingSettings.lastBuyCrossingPoint_,
                               movingAveragesCrossingSettings.lastSellCrossingPoint_,
                               movingAveragesCrossingSettings.movingAverageType_);

  processingResult = (anyIndicatorTriggered) ? (processingResult | currentStrategy->isNeedToBuy())
                                             : (processingResult & currentStrategy->isNeedToBuy());
}

void TradingSellStrategyProcessor::visit(
    const model::StochasticOscillatorSettings &stochasticOscillatorSettings) {
  const auto &stockExchangeSettings = tradeConfiguration_.getStockExchangeSettings();
  const auto &coinSettings = tradeConfiguration_.getCoinSettings();

  bool anyIndicatorTriggered =
      tradeConfiguration_.getBuySettings().openOrderWhenAnyIndicatorIsTriggered_;
  auto marketHistory = getMarketHistory(stochasticOscillatorSettings.tickInterval_);
  if (marketHistory->marketData_.empty()) {
    processingResult = (anyIndicatorTriggered) ? (processingResult | false) : (false);
    return;
  }

  auto currentStrategy = strategiesLibrary_.getStochasticOscillatorStrategy();
  currentStrategy->setBottomLevel(stochasticOscillatorSettings.bottomLevel);
  currentStrategy->setTopLevel(stochasticOscillatorSettings.topLevel);

  currentStrategy->createLines(marketHistory->marketData_,
                               stochasticOscillatorSettings.stochasticType_,
                               stochasticOscillatorSettings.periodsForClassicLine_,
                               stochasticOscillatorSettings.smoothFastPeriod_,
                               stochasticOscillatorSettings.smoothSlowPeriod_,
                               stochasticOscillatorSettings.crossingInterval_,
                               stochasticOscillatorSettings.lastBuyCrossingPoint_,
                               stochasticOscillatorSettings.lastSellCrossingPoint_);

  processingResult = (anyIndicatorTriggered) ? (processingResult | currentStrategy->isNeedToBuy())
                                             : (processingResult & currentStrategy->isNeedToBuy());
}

void TradingSellStrategyProcessor::visit(
    const model::CustomStrategySettings &customStrategySettings) {
  size_t strategiesCount = customStrategySettings.getStrategiesCount();
  for (int index = 0; index < strategiesCount; ++index) {
    if (!tradingManager_.isRunning()) {
      return;
    }
    customStrategySettings.getStrategy(index)->accept(*this);
  }
}

common::MarketHistoryPtr TradingSellStrategyProcessor::getMarketHistory(
    common::TickInterval::Enum interval) const {
  auto &coinSettings = tradeConfiguration_.getCoinSettings();
  auto &stockExchangeSettings = tradeConfiguration_.getStockExchangeSettings();
  auto query = queryProcessor_.getQuery(stockExchangeSettings.stockExchangeType_);

  auto marketHistory =
      query->getMarketHistory(coinSettings.baseCurrency_, currentTradedCurrency_, interval);
  return marketHistory;
}

void TradingSellStrategyProcessor::runStopLossProcessor() {
  auto &coinSettings = tradeConfiguration_.getCoinSettings();
  auto &stockExchangeSettings = tradeConfiguration_.getStockExchangeSettings();
  auto query = queryProcessor_.getQuery(stockExchangeSettings.stockExchangeType_);

  for (int index = 0; index < coinSettings.tradedCurrencies_.size(); ++index) {
    auto tradedCurrency = coinSettings.tradedCurrencies_[index];
    if (!tradeOrdersHolder_.containOrdersProfit(tradedCurrency)) continue;

    auto &ordersProfit = tradeOrdersHolder_.takeOrdersProfit(tradedCurrency);

    std::set<common::MarketOrder> stopLossOrders;
    ordersProfit.forEachOrder(
        [&](const common::MarketOrder &order) { stopLossOrders.insert(order); });

    for (const auto &order : stopLossOrders) {
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

        openOrder(order, quantity, currentTick.bid_, "STOP LOSS SIGNAL: Opened sell order");
      }
    }
  }
}

void TradingSellStrategyProcessor::runTakeProfitProcessor() {
  auto &coinSettings = tradeConfiguration_.getCoinSettings();
  auto &stockExchangeSettings = tradeConfiguration_.getStockExchangeSettings();
  auto &sellSettings = tradeConfiguration_.getSellSettings();
  auto query = queryProcessor_.getQuery(stockExchangeSettings.stockExchangeType_);

  for (int index = 0; index < coinSettings.tradedCurrencies_.size(); ++index) {
    auto tradedCurrency = coinSettings.tradedCurrencies_.at(index);
    if (!tradeOrdersHolder_.containOrdersProfit(tradedCurrency)) continue;

    auto &ordersProfit = tradeOrdersHolder_.takeOrdersProfit(tradedCurrency);

    std::set<common::MarketOrder> sellOrders;
    ordersProfit.forEachOrder([&](const common::MarketOrder &order) { sellOrders.insert(order); });

    for (const auto &order : sellOrders) {
      double profitDelta = order.price_ * (sellSettings.profitPercentage_ / 100);
      double boughtPrice = order.price_ + profitDelta;
      auto currentTick = query->getCurrencyTick(order.fromCurrency_, order.toCurrency_);
      if (currentTick.bid_ >= boughtPrice) {
        double quantity = calculateOrderQuantity(order, ordersProfit);
        if (quantity == 0) {
          resetOrderProfit(ordersProfit);
          return;
        }

        openOrder(order, quantity, currentTick.bid_, "TAKE PROFIT SIGNAL: Opened sell order");
      }
    }
  }
}

common::MarketOrder TradingSellStrategyProcessor::openOrder(const common::MarketOrder &order,
                                                            double quantity, double price,
                                                            const std::string &message) {
  auto &stockExchangeSettings = tradeConfiguration_.getStockExchangeSettings();
  auto query = queryProcessor_.getQuery(stockExchangeSettings.stockExchangeType_);
  auto &orderMatching = tradeOrdersHolder_.takeOrderMatching();
  auto &coinSettings = tradeConfiguration_.getCoinSettings();
  auto &ordersProfit = tradeOrdersHolder_.takeOrdersProfit(currentTradedCurrency_);

  quantity = calculateLotSize(currentTradedCurrency_, quantity);
  if (quantity == 0) {
    return common::MarketOrder();
  }

  common::MarketOrder currentOrder =
      query->sellOrder(coinSettings.baseCurrency_, currentTradedCurrency_, quantity, price);

  const std::string fullMessage = message + " : [ " + currentOrder.toString() + " ]";
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

  databaseProvider_.insertOrderMatching(stockExchangeSettings.stockExchangeType_, currencyPair,
                                        common::OrderType::SELL, common::OrderType::BUY,
                                        currentOrder.databaseId_, order.databaseId_);

  ordersProfit.removeOrder(order);
  databaseProvider_.removeOrderProfit(stockExchangeSettings.stockExchangeType_, order.toCurrency_,
                                      order.databaseId_);

  return currentOrder;
}

double TradingSellStrategyProcessor::calculateLotSize(common::Currency::Enum tradedCurrency,
                                                      double quantity) const {
  auto &stockExchangeSettings = tradeConfiguration_.getStockExchangeSettings();
  auto &coinSettings = tradeConfiguration_.getCoinSettings();

  if (!lotsHolder_.empty()) {
    std::string marketPair = stock_exchange_utils::getMarketPair(
        stockExchangeSettings.stockExchangeType_, coinSettings.baseCurrency_, tradedCurrency);

    auto lotSize = lotsHolder_.getLot(marketPair);
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

double TradingSellStrategyProcessor::calculateOrderQuantity(const common::MarketOrder &order,
                                                            model::OrdersProfit &orderProfit) {
  auto &stockExchangeSettings = tradeConfiguration_.getStockExchangeSettings();
  auto &coinSettings = tradeConfiguration_.getCoinSettings();
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

void TradingSellStrategyProcessor::resetOrderProfit(model::OrdersProfit &orderProfit) {
  auto &currentTradeConfiguration = tradeConfigsHolder_.takeCurrentTradeConfiguration();
  auto &stockExchangeSettings = currentTradeConfiguration.getStockExchangeSettings();
  common::Currency::Enum tradedCurrency = orderProfit.getCurrency();

  databaseProvider_.removeCurrencyProfit(tradedCurrency, stockExchangeSettings.stockExchangeType_);
  orderProfit.forEachOrder([&](const common::MarketOrder &marketOrder) {
    databaseProvider_.removeMarketOrder(marketOrder);
  });

  orderProfit.clear();
}

}  // namespace trader
}  // namespace auto_trader