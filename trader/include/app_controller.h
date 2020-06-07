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

#ifndef AUTO_TRADER_APP_CONTROLLER_H
#define AUTO_TRADER_APP_CONTROLLER_H

#include <QtCore/QThread>
#include <QtWidgets/QApplication>
#include <thread>

#include "app_chart_updater.h"
#include "app_stats_updater.h"
#include "common/enumerations/refresh_ui_type.h"
#include "common/listeners/app_listener.h"
#include "common/listeners/gui_listener.h"
#include "database/include/database.h"
#include "model/include/holders/strategies_settings_holder.h"
#include "model/include/holders/trade_configs_holder.h"
#include "model/include/holders/trade_orders_holder.h"
#include "model/include/holders/trade_signaled_strategy_market_holder.h"
#include "model/include/settings/app_settings.h"
#include "stocks_exchange/include/stock_exchange_library.h"
#include "strategies/include/strategy_facade.h"
#include "trading_manager.h"
#include "trading_message_sender.h"

namespace auto_trader {
namespace trader {

class AppController : public QObject, public common::AppListener {
  Q_OBJECT

 public:
  AppController(QApplication &application);
  ~AppController();

  AppController(const AppController &) = delete;
  AppController &operator=(const AppController &) = delete;

  void loadStrategies();
  void loadTradeConfigurations();
  void loadFeaturesSettings();
  void loadAppSettings();

  void saveStrategiesSettingsFiles() const override;
  void saveTradeConfigurationsFiles() const override;
  void saveFeaturesSettings() const override;
  void saveAppSettings() const override;

  void changeTheme(common::ApplicationThemeType themeType) override;

  common::GuiListener &getGuiListener();

  model::StrategiesSettingsHolder &getStrategySettingsHolder() override;
  model::TradeConfigsHolder &getTradeConfigsHolder() override;
  model::AppSettings &getAppSettings() override;
  stock_exchange::QueryProcessor &getQueryProcessor() override;

  void runTrading() override;
  void stopTrading() override;

  void refreshUIMessage(common::RefreshUiType refreshUiType) override;

  void refreshMarketHistory(common::Currency::Enum baseCurrency,
                            common::Currency::Enum tradedCurrency,
                            common::TickInterval::Enum interval) override;

  void refreshStockExchangeView() override;
  void refreshTradingView() override;

  void refreshApiKeys(const model::TradeConfiguration &configuration) override;
  void interruptStatsUpdate() override;

  void startStatsUpdater();
  void stopStatsUpdater();
  void stopChartUpdater();

  void stopTradingThread();

 public slots:
  void refreshTradingCurrenciesUI();
  void refreshAllCurrenciesUI();
  void refreshMarketHistoryUI(unsigned int stockExchangeType);
  void refreshAccountBalanceUI();
  void refreshAllOrdersUI();
  void refreshOpenOrdersUI();
  void refreshStatsInterruptableUI();
  void printMessage(const QString &message);
  void dispatchProgressBarStartEvent(int maximum);
  void dispatchProgressBarFinishEvent();
  void incrementProgressBarValue(int value);

  void refreshTradingStartButton();
  void refreshTradingStopButton();
  void refreshTradingOutdatedData();

  void tradingStartedFromTelegramUI();
  void tradingStoppedFromTelegramUI();

 signals:
  void runStatsUpdaterThread();
  void runTradingThread();
  void runChartUpdaterThread(unsigned int baseCurrency, unsigned int tradedCurrency,
                             unsigned int interval);

 private:
  QApplication &application_;

  std::unique_ptr<model::StrategiesSettingsHolder> strategiesSettingsHolder_;
  std::unique_ptr<model::TradeConfigsHolder> tradeConfigurationsHolder_;
  std::unique_ptr<model::TradeOrdersHolder> tradeOrdersHolder_;
  std::unique_ptr<model::TradeSignaledStrategyMarketHolder> tradeSignaledStrategyMarketHolder_;

  std::unique_ptr<common::GuiListener> guiListener_;

  std::unique_ptr<strategies::StrategyFacade> strategyFacade_;
  std::unique_ptr<stock_exchange::StockExchangeLibrary> stockExchangeLibrary_;

  std::unique_ptr<AppStatsUpdater> appStatsUpdater_;
  std::unique_ptr<AppChartUpdater> appChartUpdater_;

  std::unique_ptr<TradingMessageSender> messageSender_;
  std::unique_ptr<TradingManager> tradingManager_;

  std::unique_ptr<database::Database> databaseProvider_;

  model::AppSettings appSettings_;

  QThread statsUpdateThread_;
  QThread chartUpdateThread_;
  QThread tradingThread_;
};

}  // namespace trader
}  // namespace auto_trader

#endif  // AUTO_TRADER_APP_CONTROLLER_H
