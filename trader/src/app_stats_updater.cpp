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

#include "include/app_stats_updater.h"

#include <algorithm>

#include "common/binance_currency.h"
#include "common/bittrex_currency.h"
#include "common/exceptions/model_exception/no_active_trading_config_exception.h"
#include "common/kraken_currency.h"
#include "common/loggers/file_logger.h"
#include "common/market_order.h"
#include "common/poloniex_currency.h"
#include "common/tick_interval_ratio.h"
#include "features/include/telegram_announcer.h"
#include "stocks_exchange/include/query_processor.h"

namespace auto_trader {
namespace trader {

constexpr int OPEN_ORDERS_FROM_MARKET_COUNT = 50;
constexpr int PROGRESS_BAR_MAX_VALUE = 100;
constexpr int PROGRESS_BAR_INCREMENT = 20;

static std::vector<common::MarketOrder> filtrateOrdersForType(
    const std::vector<common::MarketOrder>& allOrders, common::OrderType type) {
  std::vector<common::MarketOrder> ordersForType;
  for (const auto& order : allOrders) {
    if (order.orderType_ == type) ordersForType.emplace_back(order);
  }

  return ordersForType;
}

static std::vector<common::Currency::Enum> getTradedStockExchangeCurrencies(
    common::StockExchangeType type, common::Currency::Enum baseCurrency) {
  switch (type) {
    case common::StockExchangeType::Bittrex: {
      common::BittrexCurrency bittrexCurrency;
      return bittrexCurrency.getTradedCurrencies(baseCurrency);
    }
    case common::StockExchangeType::Binance: {
      common::BinanceCurrency binanceCurrency;
      return binanceCurrency.getTradedCurrencies(baseCurrency);
    }
    case common::StockExchangeType::Kraken: {
      common::KrakenCurrency krakenCurrency;
      return krakenCurrency.getTradedCurrencies(baseCurrency);
    }
    case common::StockExchangeType::Poloniex: {
      common::PoloniexCurrency poloniexCurrency;
      return poloniexCurrency.getTradedCurrencies(baseCurrency);
    }

    default:
      return std::vector<common::Currency::Enum>();
  }
}

AppStatsUpdater::AppStatsUpdater(common::AppListener& appListener, common::GuiListener& guiListener)
    : appListener_(appListener),
      guiListener_(guiListener),
      isRunning_(false),
      isProgressBarActive_(false),
      isStatsInterruptable_(false) {
  lastUpdatedTick_ = QDateTime::currentDateTime();
}

void AppStatsUpdater::initialize() {
  refreshAllCurrenciesUI();
  refreshTradingCurrenciesUI();
  refreshAccountBalanceUI();
  refreshOpenOrdersUI();
  refreshAllOrdersUI();
}

void AppStatsUpdater::start() {
  isRunning_ = true;
  while (isRunning_) {
    try {
      refreshUI();
      runMessageQueue();
      runTelegramQueue();

      if (messages_.empty()) {
        std::unique_lock<std::mutex> lock(uiLocker_);
        std::chrono::duration<int> seconds_type{20};
        condVar_.wait_for(lock, seconds_type);
      }
    } catch (std::exception& exception) {
      common::loggers::FileLogger::getLogger() << exception.what();
    }
  }
}

void AppStatsUpdater::stop() {
  std::lock_guard<std::mutex> lock(uiLocker_);
  isRunning_ = false;

  condVar_.notify_all();
}

void AppStatsUpdater::runMessageQueue() {
  std::lock_guard<std::mutex> lock(uiLocker_);

  if (messages_.empty()) {
    return;
  }

  common::RefreshUiType signal = messages_.front();
  switch (signal) {
    case common::RefreshUiType::TRADING_CURRENCIES: {
      refreshTradingCurrencies();
    } break;

    case common::RefreshUiType::ALL_CURRENCIES: {
      refreshAllCurrencies();
    } break;

    case common::RefreshUiType::ACCOUNT_BALANCE: {
      refreshAccountBalance();
    } break;

    case common::RefreshUiType::ALL_ORDERS: {
      refreshAllOrders();
    } break;

    case common::RefreshUiType::OPEN_ORDERS: {
      refreshOpenOrders();
    } break;

    default:
      break;
  }

  messages_.pop();
}

void AppStatsUpdater::runTelegramQueue() {
  auto& announcer = features::telegram_announcer::TelegramAnnouncer::instance();
  if (!announcer.isInitialized()) return;

  try {
    auto messages = announcer.getLastMessages();
    for (auto& message : messages) {
      if (message == "trading_on") {
        try {
          auto& currentConfiguration =
              appListener_.getTradeConfigsHolder().getCurrentTradeConfiguration();
          if (currentConfiguration.isRunning()) {
            announcer.sendMessage("Trading is running right now.");
            continue;
          }

          if (checkTradingInterval()) {
            announcer.sendMessage(
                "Trading tick interval is unsuitable. Please go to desktop app to get more "
                "information.");
            continue;
          }

          announcer.sendMessage("Start trading.");
          emit tradingStartedFromTelegram();

        } catch (common::exceptions::NoActiveTradingConfigException& exception) {
          announcer.sendMessage(exception.what());
        }

      } else if (message == "trading_off") {
        try {
          auto& currentConfiguration =
              appListener_.getTradeConfigsHolder().getCurrentTradeConfiguration();
          if (!currentConfiguration.isRunning()) {
            announcer.sendMessage("Trading is not running right now.");
            continue;
          }

          announcer.sendMessage("Stop trading.");

          emit tradingStoppedFromTelegram();

        } catch (common::exceptions::NoActiveTradingConfigException& exception) {
          announcer.sendMessage(exception.what());
        }
      } else if (message == "logging_on") {
        announcer.setLoggingEnabled(true);
        announcer.sendMessage("Start logging.");
      } else if (message == "logging_off") {
        announcer.setLoggingEnabled(false);
        announcer.sendMessage("Stop logging.");
      } else {
        announcer.sendMessage("The command : \"" + message + "\" is invalid.");
      }
    }
  } catch (std::exception& exception) {
    common::loggers::FileLogger::getLogger() << exception.what();
  }

  appListener_.saveFeaturesSettings();
}

void AppStatsUpdater::addMessage(common::RefreshUiType message) {
  std::lock_guard<std::mutex> lock(uiLocker_);
  messages_.push(message);

  condVar_.notify_all();
}

void AppStatsUpdater::refreshStockExchangeViewData() {
  std::lock_guard<std::mutex> lock(uiLocker_);

  isProgressBarActive_ = true;

  messages_.push(common::RefreshUiType::TRADING_CURRENCIES);
  messages_.push(common::RefreshUiType::ALL_CURRENCIES);
  messages_.push(common::RefreshUiType::ALL_ORDERS);
  messages_.push(common::RefreshUiType::OPEN_ORDERS);
  messages_.push(common::RefreshUiType::ACCOUNT_BALANCE);

  guiListener_.dispatchProgressBarStartEvent(PROGRESS_BAR_MAX_VALUE);

  condVar_.notify_all();
}

void AppStatsUpdater::refreshTradingViewData() {
  std::lock_guard<std::mutex> lock(uiLocker_);

  messages_.push(common::RefreshUiType::OPEN_ORDERS);
  messages_.push(common::RefreshUiType::ACCOUNT_BALANCE);
  messages_.push(common::RefreshUiType::TRADING_CURRENCIES);

  condVar_.notify_all();
}

void AppStatsUpdater::refreshUI() {
  std::lock_guard<std::mutex> lock(uiLocker_);

  if (!messages_.empty()) {
    return;
  }

  auto date = QDateTime::currentDateTime();
  auto difference = lastUpdatedTick_.secsTo(date);
  auto& appSettings = appListener_.getAppSettings();
  unsigned int appStatsTimeoutSec = appSettings.appStatsTimeout_ * 60;
  if (difference >= appStatsTimeoutSec) {
    auto uiLock = guiListener_.acquireUILock();
    uiLock.lock();
    guiListener_.setUIUpdating(true);
    isProgressBarActive_ = true;

    emit progressStarted(PROGRESS_BAR_MAX_VALUE);

    messages_.push(common::RefreshUiType::TRADING_CURRENCIES);
    messages_.push(common::RefreshUiType::ALL_CURRENCIES);
    messages_.push(common::RefreshUiType::ALL_ORDERS);
    messages_.push(common::RefreshUiType::OPEN_ORDERS);
    messages_.push(common::RefreshUiType::ACCOUNT_BALANCE);

    lastUpdatedTick_ = date;
  }
}

void AppStatsUpdater::refreshTradingCurrencies() {
  tradingCurrencies_.clear();
  auto& tradeConfigurationsHolder = appListener_.getTradeConfigsHolder();
  if (tradeConfigurationsHolder.isEmpty()) {
    emit tradingCurrenciesChanged();
    return;
  }

  try {
    const auto& currentTradeConfiguration =
        tradeConfigurationsHolder.getCurrentTradeConfiguration();
    const auto& stockExchangeSettings = currentTradeConfiguration.getStockExchangeSettings();
    const auto& coinSettings = currentTradeConfiguration.getCoinSettings();
    auto queryProcessor = appListener_.getQueryProcessor();
    auto query = queryProcessor.getQuery(stockExchangeSettings.stockExchangeType_);

    size_t tradedCurrenciesCount = coinSettings.tradedCurrencies_.size();
    for (int index = 0; index < tradedCurrenciesCount; ++index) {
      if (!isRunning_) {
        return;
      }
      if (isStatsInterruptable_) {
        emit statsUpdateInterrupted();
        return;
      }
      try {
        common::CurrencyTick currencyTick{0.0, 0.0, coinSettings.baseCurrency_,
                                          coinSettings.tradedCurrencies_[index]};
        auto tick = query->getCurrencyTick(coinSettings.baseCurrency_,
                                           coinSettings.tradedCurrencies_[index]);
        currencyTick.bid_ = tick.bid_;
        currencyTick.ask_ = tick.ask_;

        tradingCurrencies_.push_back(currencyTick);
      } catch (std::exception& exception) {
        common::loggers::FileLogger::getLogger() << exception.what();
      }
    }
  } catch (std::exception& exception) {
    common::loggers::FileLogger::getLogger() << exception.what();
  }

  emit tradingCurrenciesChanged();
}

void AppStatsUpdater::refreshAllCurrencies() {
  allCurrencies_.clear();
  auto& tradeConfigurationsHolder = appListener_.getTradeConfigsHolder();
  if (tradeConfigurationsHolder.isEmpty()) {
    emit allCurrenciesChanged();
    return;
  }

  try {
    const auto& currentTradeConfiguration =
        tradeConfigurationsHolder.getCurrentTradeConfiguration();
    const auto& coinSettings = currentTradeConfiguration.getCoinSettings();
    const auto& stockExchangeSettings = currentTradeConfiguration.getStockExchangeSettings();
    auto& queryProcessor = appListener_.getQueryProcessor();
    auto query = queryProcessor.getQuery(stockExchangeSettings.stockExchangeType_);
    auto allTradedCurrencies = getTradedStockExchangeCurrencies(
        stockExchangeSettings.stockExchangeType_, coinSettings.baseCurrency_);

    for (int index = 0; index < allTradedCurrencies.size(); ++index) {
      if (!isRunning_) {
        return;
      }
      if (isStatsInterruptable_) {
        emit statsUpdateInterrupted();
        return;
      }
      try {
        auto toCurrency = allTradedCurrencies.at(index);
        auto tick = query->getCurrencyTick(coinSettings.baseCurrency_, toCurrency);
        common::CurrencyTick currencyTick{tick.ask_, tick.bid_, coinSettings.baseCurrency_,
                                          toCurrency};
        allCurrencies_.push_back(currencyTick);
      } catch (std::exception& exception) {
        common::loggers::FileLogger::getLogger() << exception.what();
      }
    }
  } catch (std::exception& exception) {
    common::loggers::FileLogger::getLogger() << exception.what();
  }
  emit allCurrenciesChanged();
}

void AppStatsUpdater::refreshAccountBalance() {
  accountBalance_.clear();
  auto& tradeConfigurationsHolder = appListener_.getTradeConfigsHolder();
  if (tradeConfigurationsHolder.isEmpty()) {
    emit accountBalanceChanged();
    return;
  }

  const auto& currentTradeConfiguration = tradeConfigurationsHolder.getCurrentTradeConfiguration();
  const auto& stockExchangeSettings = currentTradeConfiguration.getStockExchangeSettings();
  const auto& coinSettings = currentTradeConfiguration.getCoinSettings();
  auto queryProcessor = appListener_.getQueryProcessor();
  auto query = queryProcessor.getQuery(stockExchangeSettings.stockExchangeType_);

  try {
    double baseCurrencyBalance = query->getBalance(coinSettings.baseCurrency_);
    accountBalance_.emplace_back(std::make_pair(coinSettings.baseCurrency_, baseCurrencyBalance));

    size_t tradedCurrenciesCount = coinSettings.tradedCurrencies_.size();
    for (int index = 0; index < tradedCurrenciesCount; ++index) {
      if (!isRunning_) {
        return;
      }
      if (isStatsInterruptable_) {
        emit statsUpdateInterrupted();
        return;
      }
      double balance = query->getBalance(coinSettings.tradedCurrencies_[index]);
      accountBalance_.emplace_back(std::make_pair(coinSettings.tradedCurrencies_[index], balance));
    }
  } catch (std::exception& exception) {
    common::loggers::FileLogger::getLogger() << exception.what();
  }

  emit accountBalanceChanged();
}

void AppStatsUpdater::refreshAllOrders() {
  allOrders_.clear();
  auto& tradeConfigurationsHolder = appListener_.getTradeConfigsHolder();
  if (tradeConfigurationsHolder.isEmpty()) {
    emit allOrdersChanged();
    return;
  }

  try {
    const auto& currentTradeConfiguration =
        tradeConfigurationsHolder.getCurrentTradeConfiguration();
    const auto& stockExchangeSettings = currentTradeConfiguration.getStockExchangeSettings();
    auto queryProcessor = appListener_.getQueryProcessor();
    auto query = queryProcessor.getQuery(stockExchangeSettings.stockExchangeType_);

    auto coinSettings = currentTradeConfiguration.getCoinSettings();
    for (const auto& tradedCoin : coinSettings.tradedCurrencies_) {
      if (!isRunning_) {
        return;
      }
      if (isStatsInterruptable_) {
        emit statsUpdateInterrupted();
        return;
      }
      try {
        auto openMarketOrdersForPair =
            query->getMarketOpenOrders(coinSettings.baseCurrency_, tradedCoin);
        std::random_shuffle(openMarketOrdersForPair.begin(), openMarketOrdersForPair.end());
        std::copy(openMarketOrdersForPair.begin(),
                  openMarketOrdersForPair.begin() + OPEN_ORDERS_FROM_MARKET_COUNT,
                  std::back_inserter(allOrders_));

      } catch (std::exception& exception) {
        common::loggers::FileLogger::getLogger() << exception.what();
      }
    }
  } catch (std::exception& exception) {
    common::loggers::FileLogger::getLogger() << exception.what();
  }

  emit allOrdersChanged();
}

void AppStatsUpdater::refreshOpenOrders() {
  openOrders_.clear();
  auto& tradeConfigurationsHolder = appListener_.getTradeConfigsHolder();
  if (tradeConfigurationsHolder.isEmpty()) {
    emit openOrdersChanged();
    return;
  }

  try {
    const auto& currentTradeConfiguration =
        tradeConfigurationsHolder.getCurrentTradeConfiguration();
    const auto& stockExchangeSettings = currentTradeConfiguration.getStockExchangeSettings();
    auto queryProcessor = appListener_.getQueryProcessor();
    auto query = queryProcessor.getQuery(stockExchangeSettings.stockExchangeType_);
    auto coinSettings = currentTradeConfiguration.getCoinSettings();
    try {
      if (isStatsInterruptable_) {
        emit statsUpdateInterrupted();
        return;
      }
      std::vector<common::MarketOrder> marketOrders;
      for (const auto& tradedCoin : coinSettings.tradedCurrencies_) {
        std::vector<common::MarketOrder> allOrders =
            query->getAccountOpenOrders(coinSettings.baseCurrency_, tradedCoin);
        std::copy(allOrders.begin(), allOrders.end(), std::back_inserter(marketOrders));
      }
      auto buyOrders = filtrateOrdersForType(marketOrders, common::OrderType::BUY);
      auto sellOrders = filtrateOrdersForType(marketOrders, common::OrderType::SELL);

      openOrders_.reserve(buyOrders.size() + sellOrders.size());

      openOrders_.insert(openOrders_.end(), buyOrders.begin(), buyOrders.end());
      openOrders_.insert(openOrders_.end(), sellOrders.begin(), sellOrders.end());
    } catch (std::exception& exception) {
      common::loggers::FileLogger::getLogger() << exception.what();
    }
  } catch (std::exception& exception) {
    common::loggers::FileLogger::getLogger() << exception.what();
  }

  emit openOrdersChanged();
}

void AppStatsUpdater::refreshTradingCurrenciesUI() {
  guiListener_.refreshTradingCurrenciesView(tradingCurrencies_);
  if (!isProgressBarActive_) {
    return;
  }
  guiListener_.incrementProgressBarValue(PROGRESS_BAR_INCREMENT);
  currentProgress_ += PROGRESS_BAR_INCREMENT;
  if (currentProgress_ >= PROGRESS_BAR_MAX_VALUE) {
    isProgressBarActive_ = false;
    currentProgress_ = 0;
  }
}

void AppStatsUpdater::refreshAllCurrenciesUI() {
  guiListener_.refreshAllCurrenciesView(allCurrencies_);
  if (!isProgressBarActive_) {
    return;
  }

  guiListener_.incrementProgressBarValue(PROGRESS_BAR_INCREMENT);
  currentProgress_ += PROGRESS_BAR_INCREMENT;
  if (currentProgress_ >= PROGRESS_BAR_MAX_VALUE) {
    isProgressBarActive_ = false;
    currentProgress_ = 0;
  }
}

void AppStatsUpdater::refreshAccountBalanceUI() {
  guiListener_.refreshAccountBalanceView(accountBalance_);
  if (!isProgressBarActive_) {
    return;
  }

  guiListener_.incrementProgressBarValue(PROGRESS_BAR_INCREMENT);
  currentProgress_ += PROGRESS_BAR_INCREMENT;
  if (currentProgress_ >= PROGRESS_BAR_MAX_VALUE) {
    isProgressBarActive_ = false;
    currentProgress_ = 0;
  }
}

void AppStatsUpdater::refreshAllOrdersUI() {
  guiListener_.refreshAllOrdersView(allOrders_);
  if (!isProgressBarActive_) {
    return;
  }
  guiListener_.incrementProgressBarValue(PROGRESS_BAR_INCREMENT);
  currentProgress_ += PROGRESS_BAR_INCREMENT;
  if (currentProgress_ >= PROGRESS_BAR_MAX_VALUE) {
    isProgressBarActive_ = false;
    currentProgress_ = 0;
  }
}

void AppStatsUpdater::refreshOpenOrdersUI() {
  guiListener_.refreshOpenOrdersView(openOrders_);
  if (!isProgressBarActive_) {
    return;
  }
  guiListener_.incrementProgressBarValue(PROGRESS_BAR_INCREMENT);
  currentProgress_ += PROGRESS_BAR_INCREMENT;
  if (currentProgress_ >= PROGRESS_BAR_MAX_VALUE) {
    isProgressBarActive_ = false;
    currentProgress_ = 0;
  }
}

void AppStatsUpdater::refreshStatsInterruptableUI() {
  std::lock_guard<std::mutex> lock(uiLocker_);

  isProgressBarActive_ = false;
  currentProgress_ = 0;
  guiListener_.dispatchProgressBarFinishEvent();
  isStatsInterruptable_ = false;

  std::queue<common::RefreshUiType> emptyMessages;
  std::swap(messages_, emptyMessages);
}

void AppStatsUpdater::setUpdateInterruptable() { isStatsInterruptable_ = true; }

bool AppStatsUpdater::checkTradingInterval() {
  std::map<std::string, std::string> strategiesToRevise;
  auto& currentConfiguration = appListener_.getTradeConfigsHolder().getCurrentTradeConfiguration();
  const std::string strategyName = currentConfiguration.getStrategyName();

  auto stockExchangeType = currentConfiguration.getStockExchangeSettings().stockExchangeType_;
  auto stockExchangeIntervals = common::getStockExchangeIntervals(stockExchangeType);
  auto& customStrategy = appListener_.getStrategySettingsHolder().getCustomStrategy(strategyName);
  for (size_t index = 0; index < customStrategy.getStrategiesCount(); ++index) {
    auto& strategy = customStrategy.getStrategy(index);
    if (stockExchangeIntervals.find(strategy->tickInterval_) == stockExchangeIntervals.end()) {
      return true;
    }
  }

  return false;
}

}  // namespace trader
}  // namespace auto_trader