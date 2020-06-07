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

#ifndef AUTO_TRADER_COMMON_GUI_LISTENER_H
#define AUTO_TRADER_COMMON_GUI_LISTENER_H

#include <memory>
#include <mutex>

#include "common/market_history.h"
#include "common/market_order.h"
#include "model/include/trade_configuration.h"

namespace auto_trader {
namespace common {

class GuiListener {
 public:
  virtual ~GuiListener() = default;

  virtual void showMainWindow() = 0;
  virtual bool checkLicense() = 0;

  virtual std::string getLicenseOwner() const = 0;
  virtual std::string getLicenseExpirationDate() const = 0;

  virtual void refreshTradeConfigurationView() = 0;
  virtual void refreshStrategiesView() = 0;

  virtual void refreshTradingCurrenciesView(
      const std::vector<common::CurrencyTick>& currencies) = 0;
  virtual void refreshAllCurrenciesView(const std::vector<common::CurrencyTick>& currencies) = 0;

  virtual void refreshChartViewStart() = 0;

  virtual void refreshChartViewFinish(common::MarketHistoryPtr marketHistory,
                                      common::StockExchangeType stockExchangeType) = 0;

  virtual void refreshStockExchangeChartInterval() = 0;
  virtual void refreshStockExchangeChartMarket() = 0;

  virtual void refreshLogging() = 0;
  virtual void refreshConfigurationStatusBar() = 0;

  typedef std::pair<common::Currency::Enum, double> AccountBalance;
  virtual void refreshAccountBalanceView(const std::vector<AccountBalance>& accountBalance) = 0;

  virtual void refreshAllOrdersView(const std::vector<common::MarketOrder>& allOrders) = 0;
  virtual void refreshOpenOrdersView(const std::vector<common::MarketOrder>& openOrders) = 0;

  virtual void refreshTradingStartButton(bool value) = 0;
  virtual void refreshTradingStopButton(bool value) = 0;
  virtual bool refreshTradingOutdatedData() = 0;

  virtual void createTradeConfiguration(
      std::unique_ptr<model::TradeConfiguration> configuration) = 0;
  virtual void editTradeConfiguration(std::unique_ptr<model::TradeConfiguration> configuration,
                                      const std::string& currentConfigName) = 0;

  virtual void createCustomStrategy(std::unique_ptr<model::CustomStrategySettings> settings) = 0;
  virtual void editCustomStrategy(std::unique_ptr<model::CustomStrategySettings> settings,
                                  const std::string& currentCustomStrategyName) = 0;

  virtual void dispatchTradingStartEvent() = 0;
  virtual void dispatchTradingFinishEvent() = 0;

  virtual void dispatchProgressBarStartEvent(int maximum) = 0;
  virtual void dispatchProgressBarFinishEvent() = 0;

  virtual void incrementProgressBarValue(int value) = 0;

  virtual std::unique_lock<std::mutex> acquireUILock() = 0;

  virtual bool isUIUpdating() = 0;
  virtual void setUIUpdating(bool value) = 0;

  virtual void printMessage(const std::string& message) = 0;

  virtual void disableChart() = 0;
  virtual void enableChart() = 0;
  virtual void resetChart() = 0;
};

}  // namespace common
}  // namespace auto_trader

#endif  // AUTO_TRADER_COMMON_GUI_LISTENER_H
