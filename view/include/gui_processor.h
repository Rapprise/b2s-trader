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

#ifndef AUTO_TRADER_MAIN_WINDOW_H
#define AUTO_TRADER_MAIN_WINDOW_H

#include <QSplashScreen>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QTableWidget>
#include <QtWidgets/QTextEdit>
#include <QtWidgets/QTreeWidget>
#include <mutex>

#include "common/listeners/gui_listener.h"
#include "context_menu/strategy_context_menu_handler.h"
#include "context_menu/trade_config_context_menu_handler.h"
#include "view_model/balance_view_model.h"
#include "view_model/currencies_view_model.h"
#include "view_model/strategies_view_model.h"
#include "view_model/trading_account_orders_view_model.h"
#include "view_model/trading_market_orders_view_model.h"
#include "view_model/trading_view_model.h"

namespace auto_trader {
namespace common {
class AppListener;
}
namespace view {

class CustomMenu;
class CustomChartWidget;

class GuiProcessor : public QMainWindow, public common::GuiListener {
  Q_OBJECT

 public:
  explicit GuiProcessor(common::AppListener &appListener);
  ~GuiProcessor() override;

  void showStrategiesView();
  void showConfigurationsView();
  void showTradingCurrenciesView();
  void showAllCurrenciesView();
  void showLoggingConsoleView();
  void showOpenOrdersView();
  void showAllOrdersView();
  void showAccountBalanceView();

  void showMainWindow() override;
  bool checkLicense() override;

  std::string getLicenseOwner() const override;
  std::string getLicenseExpirationDate() const override;

  void refreshTradeConfigurationView() override;
  void refreshStrategiesView() override;

  void refreshTradingCurrenciesView(const std::vector<common::CurrencyTick> &currencies) override;
  void refreshAllCurrenciesView(const std::vector<common::CurrencyTick> &currencies) override;
  void refreshAllOrdersView(const std::vector<common::MarketOrder> &allOrders) override;
  void refreshOpenOrdersView(const std::vector<common::MarketOrder> &openOrders) override;
  void refreshAccountBalanceView(const std::vector<AccountBalance> &accountBalance) override;
  void refreshLogging() override;
  void refreshConfigurationStatusBar() override;

  void refreshTradingStartButton(bool value) override;
  void refreshTradingStopButton(bool value) override;
  bool refreshTradingOutdatedData() override;

  void refreshChartViewStart() override;

  void refreshChartViewFinish(common::MarketHistoryPtr marketHistory,
                              common::StockExchangeType stockExchangeType) override;

  void refreshStockExchangeChartInterval() override;
  void refreshStockExchangeChartMarket() override;

  void setDarkTheme();
  void setWhiteTheme();

  void runTrading();
  void stopTrading();

  void createTradeConfiguration(std::unique_ptr<model::TradeConfiguration> configuration) override;
  void editTradeConfiguration(std::unique_ptr<model::TradeConfiguration> configuration,
                              const std::string &currentConfigName) override;

  void createCustomStrategy(std::unique_ptr<model::CustomStrategySettings> settings) override;
  void editCustomStrategy(std::unique_ptr<model::CustomStrategySettings> settings,
                          const std::string &customStrategyName) override;

  void dispatchTradingStartEvent() override;
  void dispatchTradingFinishEvent() override;

  void printMessage(const std::string &message) override;

  void closeEvent(QCloseEvent *event) override;
  void readSettings();

  void dispatchProgressBarStartEvent(int maximum) override;
  void dispatchProgressBarFinishEvent() override;

  void incrementProgressBarValue(int value) override;

  std::unique_lock<std::mutex> acquireUILock() override;

  bool isUIUpdating() override;
  void setUIUpdating(bool value) override;

  void disableChart() override;
  void enableChart() override;
  void resetChart() override;

  void clearLogging();

 private:
  void initDockWindows();
  bool checkTradingInterval();

 private:
  common::AppListener &appListener_;

  CustomMenu *customMenu_;

  QDockWidget *consoleDock_;
  CustomChartWidget *customChartWidget_;
  QTextEdit *customConsole_;

  QDockWidget *tradingConfigDock_;
  QTreeView *tradingExplorerView_;

  QDockWidget *strategiesDock_;
  QTreeView *strategiesView_;

  QDockWidget *currenciesDock_;
  QTableView *currenciesView_;

  QDockWidget *tradingCurrenciesDock_;
  QTableView *tradingCurrenciesView_;

  QDockWidget *accountBalanceDock_;
  QTableView *accountBalanceView_;

  QDockWidget *openOrdersDock_;
  QTableView *openOrdersView_;

  QDockWidget *allOrdersDock_;
  QTableView *allOrdersView_;

  QSplashScreen *splashScreen;

  std::unique_ptr<StrategiesViewModel> strategiesViewModel_;
  std::unique_ptr<TradingViewModel> tradingViewModel_;
  std::unique_ptr<CurrenciesViewModel> allCurrenciesViewModel_;
  std::unique_ptr<CurrenciesViewModel> tradingCurrenciesViewModel_;
  std::unique_ptr<BalanceViewModel> balanceViewModel_;
  std::unique_ptr<TradingAccountOrdersViewModel> openOrdersViewModel_;
  std::unique_ptr<TradingMarketOrdersViewModel> allOrdersViewModel_;

  std::unique_ptr<StrategyContextMenuHandler> strategyContextMenuHandler_;
  std::unique_ptr<TradeConfigContextMenuHandler> tradeConfigContextMenuHanler_;

  std::string licenseOwner_;
  std::string licenseExpirationDate_;

  std::mutex uiLocker_;
  std::atomic_bool uiUpdating_;
};

}  // namespace view
}  // namespace auto_trader

#endif  // AUTO_TRADER_MAIN_WINDOW_H
