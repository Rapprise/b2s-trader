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

#include "include/trading_processor.h"

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

TradingProcessor::TradingProcessor(
    stock_exchange::QueryProcessor &queryProcessor, strategies::StrategyFacade &strategiesLibrary,
    database::Database &databaseProvider, common::AppListener &appListener,
    const model::StrategiesSettingsHolder &strategiesSettingsHolder,
    model::TradeOrdersHolder &tradeOrdersHolder,
    model::TradeSignaledStrategyMarketHolder &tradeSignaledStrategyMarketHolder,
    const model::TradeConfiguration &tradeConfiguration, TradingMessageSender &messageSender,
    const stock_exchange::CurrencyLotsHolder &lotsHolder, const TradingManager &tradingManager

    )
    : queryProcessor_(queryProcessor),
      strategiesLibrary_(strategiesLibrary),
      databaseProvider_(databaseProvider),
      appListener_(appListener),
      tradeConfiguration_(tradeConfiguration),
      strategiesSettingsHolder_(strategiesSettingsHolder),
      tradeOrdersHolder_(tradeOrdersHolder),
      tradeSignaledStrategyMarketHolder_(tradeSignaledStrategyMarketHolder),
      messageSender_(messageSender),
      lotsHolder_(lotsHolder),
      tradingManager_(tradingManager),
      currentTradedCurrency(common::Currency::UNKNOWN),
      processingResult(true) {}

void TradingProcessor::run() {
  const auto &strategySettings =
      strategiesSettingsHolder_.getCustomStrategy(tradeConfiguration_.getStrategyName());
  const auto &coinSettings = tradeConfiguration_.getCoinSettings();
  const auto &stockExchangeSettings = tradeConfiguration_.getStockExchangeSettings();
  const auto &buySettings = tradeConfiguration_.getBuySettings();
  auto query = queryProcessor_.getQuery(stockExchangeSettings.stockExchangeType_);

  bool anyIndicatorTriggered =
      tradeConfiguration_.getBuySettings().openOrderWhenAnyIndicatorIsTriggered_;

  size_t tradedCurrenciesSize = coinSettings.tradedCurrencies_.size();
  for (int index = 0; index < tradedCurrenciesSize; ++index) {
    try {
      processingResult = !anyIndicatorTriggered;
      strategyMarkets_.clear();
      strategyCrossingPoints_.clear();
      currentTradedCurrency = coinSettings.tradedCurrencies_[index];

      strategySettings.accept(*this);

      if (!tradingManager_.isRunning()) {
        return;
      }

      if (!isNeedToOpenBuyOrder()) {
        continue;
      }

      auto coinInTradingCount = tradeOrdersHolder_.getCoinInTradingCount();
      auto baseAmountPerEachOrder = buySettings.getBaseCurrencyAmountPerEachOrder();
      double potentialCoinInTradingAmount = coinInTradingCount + baseAmountPerEachOrder;
      const std::string marketStr = common::Currency::toString(coinSettings.baseCurrency_) + "-" +
                                    common::Currency::toString(currentTradedCurrency);

      if (potentialCoinInTradingAmount > buySettings.maxCoinAmount_) {
        double potentialCoinInTradingWithMinOrderPrice =
            coinInTradingCount + buySettings.minOrderPrice_;
        if (potentialCoinInTradingWithMinOrderPrice > buySettings.maxCoinAmount_) {
          messageSender_.sendMessage(
              "The maximum coin amount in trading is reached. Buy order cannot be opened for "
              "market " +
              marketStr);
          continue;
        } else {
          messageSender_.sendMessage(
              "The total amount of orders exceeds \"Funded amount\" when establishing a new order. "
              "The \"Min order price\" will be used for order forming for market " +
              marketStr);
          baseAmountPerEachOrder = buySettings.minOrderPrice_;
        }
      }

      auto currentTick = query->getCurrencyTick(coinSettings.baseCurrency_, currentTradedCurrency);
      auto balance = query->getBalance(coinSettings.baseCurrency_);
      if (balance < baseAmountPerEachOrder) {
        if (balance > buySettings.minOrderPrice_) {
          messageSender_.sendMessage(
              "The account balance is smaller than defined order price. The minimum order price "
              "will be used for market " +
              marketStr);
          baseAmountPerEachOrder = buySettings.minOrderPrice_;
        } else {
          messageSender_.sendMessage("There is not enough money to open buy order for market " +
                                     marketStr);
          continue;
        }
      }

      double quantity = baseAmountPerEachOrder / currentTick.ask_;
      if (quantity <= 0) {
        const std::string message = "Coins quantity is too small. Order cannot be opened.";
        messageSender_.sendMessage(message);
        continue;
      }

      if (!lotsHolder_.empty()) {
        std::string marketPair =
            stock_exchange_utils::getMarketPair(stockExchangeSettings.stockExchangeType_,
                                                coinSettings.baseCurrency_, currentTradedCurrency);

        auto lotSize = lotsHolder_.getLot(marketPair);
        if (quantity < lotSize.minQty_) {
          const std::string message =
              "Coins quantity is smaller that allowed minimum for current market. Buy order cannot "
              "be opened.";
          messageSender_.sendMessage(message);
          continue;
        }
        double reminder = fmod(quantity, lotSize.stepSize_);
        if (reminder > 0) {
          quantity = quantity - reminder;
        }
      }
      auto currentOrder = query->buyOrder(coinSettings.baseCurrency_, currentTradedCurrency,
                                          quantity, currentTick.ask_);
      const std::string message = "Opened buy order : " + currentOrder.toString();
      messageSender_.sendMessage(message);
      databaseProvider_.insertMarketOrder(currentOrder);
      currentOrder.databaseId_ = databaseProvider_.getLastInsertRowId();
      tradeOrdersHolder_.addBuyOrder(currentOrder);
      for (auto strategyMarket : strategyMarkets_) {
        if (tradeSignaledStrategyMarketHolder_.containMarket(
                coinSettings.baseCurrency_, currentTradedCurrency, strategyMarket.first)) {
          databaseProvider_.removeMarketData(stockExchangeSettings.stockExchangeType_,
                                             coinSettings.baseCurrency_, currentTradedCurrency,
                                             strategyMarket.first);
        }
        tradeSignaledStrategyMarketHolder_.addMarket(coinSettings.baseCurrency_,
                                                     currentTradedCurrency, strategyMarket.first,
                                                     strategyMarket.second);

        databaseProvider_.insertMarketData(stockExchangeSettings.stockExchangeType_,
                                           coinSettings.baseCurrency_, currentTradedCurrency,
                                           strategyMarket.first, strategyMarket.second);
      }

      for (auto strategyCrossingPoint : strategyCrossingPoints_) {
        updateCrossingPoint(*strategyCrossingPoint.first, strategyCrossingPoint.second);
      }

    } catch (std::exception &exception) {
      messageSender_.sendMessage(exception.what());
    }
  }
}

void TradingProcessor::visit(const model::BollingerBandsSettings &bandsSettings) {
  const auto &stockExchangeSettings = tradeConfiguration_.getStockExchangeSettings();
  const auto &coinSettings = tradeConfiguration_.getCoinSettings();
  bool anyIndicatorTriggered =
      tradeConfiguration_.getBuySettings().openOrderWhenAnyIndicatorIsTriggered_;

  auto marketHistory = getMarketHistory(bandsSettings.tickInterval_);
  if (marketHistory->marketData_.empty()) {
    processingResult = (anyIndicatorTriggered) ? (processingResult | false) : (false);
    return;
  }

  if (tradeSignaledStrategyMarketHolder_.containMarket(coinSettings.baseCurrency_,
                                                       currentTradedCurrency,
                                                       common::StrategiesType::BOLLINGER_BANDS)) {
    auto lastSavedMarketData = tradeSignaledStrategyMarketHolder_.getMarket(
        coinSettings.baseCurrency_, currentTradedCurrency, common::StrategiesType::BOLLINGER_BANDS);

    auto marketHistoryMarketData = marketHistory->marketData_.back();
    if (lastSavedMarketData == marketHistoryMarketData) {
      processingResult = (anyIndicatorTriggered) ? (processingResult | false) : (false);
      return;
    }
  }

  auto currentStrategy = strategiesLibrary_.getBollingerBandStrategy();
  currentStrategy->createLines(marketHistory->marketData_, bandsSettings.period_,
                               bandsSettings.bbInputType_, bandsSettings.standardDeviations_,
                               bandsSettings.crossingInterval_, bandsSettings.lastBuyCrossingPoint_,
                               bandsSettings.lastSellCrossingPoint_);

  processingResult = (anyIndicatorTriggered) ? (processingResult | currentStrategy->isNeedToBuy())
                                             : (processingResult & currentStrategy->isNeedToBuy());

  auto marketData = marketHistory->marketData_.back();
  strategyMarkets_[common::StrategiesType::BOLLINGER_BANDS] = marketData;
  strategyCrossingPoints_[&bandsSettings] = currentStrategy->getLastBuyCrossingPoint();
}

void TradingProcessor::visit(
    const model::BollingerBandsAdvancedSettings &bollingerBandsAdvancedSettings) {
  const auto &stockExchangeSettings = tradeConfiguration_.getStockExchangeSettings();
  const auto &coinSettings = tradeConfiguration_.getCoinSettings();
  bool anyIndicatorTriggered =
      tradeConfiguration_.getBuySettings().openOrderWhenAnyIndicatorIsTriggered_;

  auto marketHistory = getMarketHistory(bollingerBandsAdvancedSettings.tickInterval_);
  if (marketHistory->marketData_.empty()) {
    processingResult = (anyIndicatorTriggered) ? (processingResult | false) : (false);
    return;
  }

  auto currentStrategy = strategiesLibrary_.getBollingerBandAdvanceStrategy();
  currentStrategy->setPercentageForBottomLine(bollingerBandsAdvancedSettings.bottomLinePercentage_);
  currentStrategy->setPercentageForTopLine(bollingerBandsAdvancedSettings.topLinePercentage_);

  if (tradeSignaledStrategyMarketHolder_.containMarket(
          coinSettings.baseCurrency_, currentTradedCurrency,
          common::StrategiesType::BOLLINGER_BANDS_ADVANCED)) {
    auto lastSavedMarketData = tradeSignaledStrategyMarketHolder_.getMarket(
        coinSettings.baseCurrency_, currentTradedCurrency,
        common::StrategiesType::BOLLINGER_BANDS_ADVANCED);
    auto marketHistoryMarketData = marketHistory->marketData_.back();
    if (lastSavedMarketData == marketHistoryMarketData) {
      processingResult = (anyIndicatorTriggered) ? (processingResult | false) : (false);
      return;
    }
  }
  currentStrategy->createLines(marketHistory->marketData_, bollingerBandsAdvancedSettings.period_,
                               bollingerBandsAdvancedSettings.bbInputType_,
                               bollingerBandsAdvancedSettings.standardDeviations_,
                               bollingerBandsAdvancedSettings.crossingInterval_,
                               bollingerBandsAdvancedSettings.lastBuyCrossingPoint_,
                               bollingerBandsAdvancedSettings.lastSellCrossingPoint_);

  processingResult = (anyIndicatorTriggered) ? (processingResult | currentStrategy->isNeedToBuy())
                                             : (processingResult & currentStrategy->isNeedToBuy());

  auto marketData = marketHistory->marketData_.back();
  strategyMarkets_[common::StrategiesType::BOLLINGER_BANDS_ADVANCED] = marketData;
  strategyCrossingPoints_[&bollingerBandsAdvancedSettings] =
      currentStrategy->getLastBuyCrossingPoint();
}

void TradingProcessor::visit(const model::RsiSettings &rsiSettings) {
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

  if (tradeSignaledStrategyMarketHolder_.containMarket(
          coinSettings.baseCurrency_, currentTradedCurrency, common::StrategiesType::RSI)) {
    auto lastSavedMarketData = tradeSignaledStrategyMarketHolder_.getMarket(
        coinSettings.baseCurrency_, currentTradedCurrency, common::StrategiesType::RSI);
    auto marketHistoryMarketData = marketHistory->marketData_.back();
    if (lastSavedMarketData == marketHistoryMarketData) {
      processingResult = (anyIndicatorTriggered) ? (processingResult | false) : (false);
      return;
    }
  }

  currentStrategy->setTopRsiIndex(rsiSettings.topLevel_);
  currentStrategy->setBottomRsiIndex(rsiSettings.bottomLevel_);

  currentStrategy->createLine(marketHistory->marketData_, rsiSettings.period_,
                              rsiSettings.crossingInterval_, rsiSettings.lastBuyCrossingPoint_,
                              rsiSettings.lastSellCrossingPoint_);

  processingResult = (anyIndicatorTriggered) ? (processingResult | currentStrategy->isNeedToBuy())
                                             : (processingResult & currentStrategy->isNeedToBuy());
  auto marketData = marketHistory->marketData_.back();
  strategyMarkets_[common::StrategiesType::RSI] = marketData;
  strategyCrossingPoints_[&rsiSettings] = currentStrategy->getLastBuyCrossingPoint();
}

void TradingProcessor::visit(const model::EmaSettings &emaSettings) {
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
  if (tradeSignaledStrategyMarketHolder_.containMarket(
          coinSettings.baseCurrency_, currentTradedCurrency, common::StrategiesType::EMA)) {
    auto lastSavedMarketData = tradeSignaledStrategyMarketHolder_.getMarket(
        coinSettings.baseCurrency_, currentTradedCurrency, common::StrategiesType::EMA);
    auto marketHistoryMarketData = marketHistory->marketData_.back();
    if (lastSavedMarketData == marketHistoryMarketData) {
      processingResult = (anyIndicatorTriggered) ? (processingResult | false) : (false);
      return;
    }
  }

  currentStrategy->createLine(marketHistory->marketData_, emaSettings.period_,
                              emaSettings.crossingInterval_, emaSettings.lastBuyCrossingPoint_,
                              emaSettings.lastSellCrossingPoint_);

  processingResult = (anyIndicatorTriggered) ? (processingResult | currentStrategy->isNeedToBuy())
                                             : (processingResult & currentStrategy->isNeedToBuy());

  common::MarketData marketData = marketHistory->marketData_.back();

  strategyMarkets_[common::StrategiesType::EMA] = marketData;
  strategyCrossingPoints_[&emaSettings] = currentStrategy->getLastBuyCrossingPoint();
}

void TradingProcessor::visit(const model::SmaSettings &smaSettings) {
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
  if (tradeSignaledStrategyMarketHolder_.containMarket(
          coinSettings.baseCurrency_, currentTradedCurrency, common::StrategiesType::SMA)) {
    auto lastSavedMarketData = tradeSignaledStrategyMarketHolder_.getMarket(
        coinSettings.baseCurrency_, currentTradedCurrency, common::StrategiesType::SMA);
    auto marketHistoryMarketData = marketHistory->marketData_.back();
    if (lastSavedMarketData == marketHistoryMarketData) {
      processingResult = (anyIndicatorTriggered) ? (processingResult | false) : (false);
      return;
    }
  }

  currentStrategy->createLine(marketHistory->marketData_, smaSettings.period_,
                              smaSettings.crossingInterval_, smaSettings.lastBuyCrossingPoint_,
                              smaSettings.lastSellCrossingPoint_);

  processingResult = (anyIndicatorTriggered) ? (processingResult | currentStrategy->isNeedToBuy())
                                             : (processingResult & currentStrategy->isNeedToBuy());

  auto marketData = marketHistory->marketData_.back();
  strategyMarkets_[common::StrategiesType::SMA] = marketData;
  strategyCrossingPoints_[&smaSettings] = currentStrategy->getLastBuyCrossingPoint();
}

void TradingProcessor::visit(
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

  if (tradeSignaledStrategyMarketHolder_.containMarket(
          coinSettings.baseCurrency_, currentTradedCurrency, common::StrategiesType::MA_CROSSING)) {
    auto lastSavedMarketData = tradeSignaledStrategyMarketHolder_.getMarket(
        coinSettings.baseCurrency_, currentTradedCurrency, common::StrategiesType::MA_CROSSING);
    auto marketHistoryMarketData = marketHistory->marketData_.back();
    if (lastSavedMarketData == marketHistoryMarketData) {
      processingResult = (anyIndicatorTriggered) ? (processingResult | false) : (false);
      return;
    }
  }

  currentStrategy->createLines(marketHistory->marketData_,
                               movingAveragesCrossingSettings.smallerPeriod_,
                               movingAveragesCrossingSettings.biggerPeriod_,
                               movingAveragesCrossingSettings.lastBuyCrossingPoint_,
                               movingAveragesCrossingSettings.lastSellCrossingPoint_,
                               movingAveragesCrossingSettings.movingAverageType_);

  processingResult = (anyIndicatorTriggered) ? (processingResult | currentStrategy->isNeedToBuy())
                                             : (processingResult & currentStrategy->isNeedToBuy());

  auto marketData = marketHistory->marketData_.back();
  strategyMarkets_[common::StrategiesType::MA_CROSSING] = marketData;
  strategyCrossingPoints_[&movingAveragesCrossingSettings] =
      currentStrategy->getLastBuyCrossingPoint();
}

void TradingProcessor::visit(
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

  if (tradeSignaledStrategyMarketHolder_.containMarket(
          coinSettings.baseCurrency_, currentTradedCurrency,
          common::StrategiesType::STOCHASTIC_OSCILLATOR)) {
    auto lastSavedMarketData = tradeSignaledStrategyMarketHolder_.getMarket(
        coinSettings.baseCurrency_, currentTradedCurrency,
        common::StrategiesType::STOCHASTIC_OSCILLATOR);
    auto marketHistoryMarketData = marketHistory->marketData_.back();
    if (lastSavedMarketData == marketHistoryMarketData) {
      processingResult = (anyIndicatorTriggered) ? (processingResult | false) : (false);
      return;
    }
  }

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

  auto marketData = marketHistory->marketData_.back();
  strategyMarkets_[common::StrategiesType::STOCHASTIC_OSCILLATOR] = marketData;
  strategyCrossingPoints_[&stochasticOscillatorSettings] =
      currentStrategy->getLastBuyCrossingPoint();
}

void TradingProcessor::visit(const model::CustomStrategySettings &customStrategySettings) {
  size_t strategiesCount = customStrategySettings.getStrategiesCount();
  for (int index = 0; index < strategiesCount; ++index) {
    if (!tradingManager_.isRunning()) {
      return;
    }
    customStrategySettings.getStrategy(index)->accept(*this);
  }
}

common::MarketHistoryPtr TradingProcessor::getMarketHistory(
    common::TickInterval::Enum interval) const {
  auto &coinSettings = tradeConfiguration_.getCoinSettings();
  auto &stockExchangeSettings = tradeConfiguration_.getStockExchangeSettings();
  auto query = queryProcessor_.getQuery(stockExchangeSettings.stockExchangeType_);

  auto marketHistory =
      query->getMarketHistory(coinSettings.baseCurrency_, currentTradedCurrency, interval);
  return marketHistory;
}

bool TradingProcessor::isNeedToOpenBuyOrder() const {
  if (!processingResult) return false;

  const auto &coinSettings = tradeConfiguration_.getCoinSettings();
  const auto &stockExchangeSettings = tradeConfiguration_.getStockExchangeSettings();
  const auto &buySettings = tradeConfiguration_.getBuySettings();
  auto query = queryProcessor_.getQuery(stockExchangeSettings.stockExchangeType_);
  auto currentTick = query->getCurrencyTick(coinSettings.baseCurrency_, currentTradedCurrency);
  const std::string marketStr = common::Currency::toString(coinSettings.baseCurrency_) + "-" +
                                common::Currency::toString(currentTradedCurrency);
  int openPositions = tradeOrdersHolder_.getBuyOpenPositionsForMarket(coinSettings.baseCurrency_,
                                                                      currentTradedCurrency);

  if ((buySettings.openPositionAmountPerCoins_ <= openPositions)) {
    const std::string tradedCurrencyStr = common::Currency::toString(currentTradedCurrency);
    const std::string message = "The open positions for traded coin : " + tradedCurrencyStr +
                                "has reached maximum. Buy order cannot be opened";
    messageSender_.sendMessage(message);
    return false;
  }

  if (buySettings.maxOpenOrders_ <= tradeOrdersHolder_.getBuyOrdersCount()) {
    messageSender_.sendMessage(
        "The maximum coin orders has been reached. Buy order cannot be opened for market " +
        marketStr);
    return false;
  }

  return true;
}

void TradingProcessor::updateCrossingPoint(const model::StrategySettings &strategySettings,
                                           double lastCrossingPoint) {
  // TODO: Revise this logic after release.
  auto &settings = const_cast<model::StrategySettings &>(strategySettings);
  settings.lastBuyCrossingPoint_ = lastCrossingPoint;

  appListener_.saveStrategiesSettingsFiles();
}

}  // namespace trader
}  // namespace auto_trader
