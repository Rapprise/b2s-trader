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

#ifndef AUTO_TRADER_UT_TRADER_FAKE_GUI_PROCESSOR_H
#define AUTO_TRADER_UT_TRADER_FAKE_GUI_PROCESSOR_H

#include <iostream>

#include "common/listeners/gui_listener.h"

namespace auto_trader {
namespace trader {
namespace unit_test {

class FakeGuiProcessor : public common::GuiListener {
 public:
  void showMainWindow() override {}
  bool checkLicense() override { return false; }

  std::string getLicenseOwner() const override { return ""; }
  std::string getLicenseExpirationDate() const override { return ""; }

  void refreshTradeConfigurationView() override {}
  void refreshStrategiesView() override {}

  void refreshTradingCurrenciesView(const std::vector<common::CurrencyTick>& currencies) override {}
  void refreshAllCurrenciesView(const std::vector<common::CurrencyTick>& currencies) override {}

  void refreshChartViewStart() override {}
  void refreshChartViewFinish(common::MarketHistoryPtr marketHistory,
                              common::StockExchangeType stockExchangeType) override {}

  typedef std::pair<common::Currency::Enum, double> AccountBalance;
  void refreshAccountBalanceView(const std::vector<AccountBalance>& accountBalance) override {}

  void refreshAllOrdersView(const std::vector<common::MarketOrder>& allOrders) override {}
  void refreshOpenOrdersView(const std::vector<common::MarketOrder>& openOrders) override {}

  void refreshLogging() override {}
  void refreshConfigurationStatusBar() override {}

  void refreshTradingStartButton(bool value) override {}
  void refreshTradingStopButton(bool value) override {}

  bool refreshTradingOutdatedData() override { return false; }

  void createTradeConfiguration(std::unique_ptr<model::TradeConfiguration> configuration) override {
  }
  void editTradeConfiguration(std::unique_ptr<model::TradeConfiguration> configuration,
                              const std::string& currentConfigName) override {}

  void createCustomStrategy(std::unique_ptr<model::CustomStrategySettings> settings) override {}
  void editCustomStrategy(std::unique_ptr<model::CustomStrategySettings> settings,
                          const std::string& currentCustomStrategyName) override {}

  void dispatchTradingStartEvent() override {}
  void dispatchTradingFinishEvent() override {}

  void dispatchProgressBarStartEvent(int maximum) override {}
  void dispatchProgressBarFinishEvent() override {}

  void incrementProgressBarValue(int value) override {}
  void refreshStockExchangeChartInterval() override {}
  void refreshStockExchangeChartMarket() override {}

  std::unique_lock<std::mutex> acquireUILock() override { throw std::exception(); }

  bool isUIUpdating() override { return false; }

  void setUIUpdating(bool) override {}

  void printMessage(const std::string& message) override { std::cout << message << std::endl; }

  void disableChart() override {}
  void enableChart() override {}
  void resetChart() override {}
};

}  // namespace unit_test
}  // namespace trader
}  // namespace auto_trader

#endif  // AUTO_TRADER_UT_TRADER_FAKE_GUI_PROCESSOR_H
