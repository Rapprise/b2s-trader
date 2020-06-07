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

#ifndef AUTO_TRADER_TRADER_APP_STATS_UPDATER_H
#define AUTO_TRADER_TRADER_APP_STATS_UPDATER_H

#include <QDateTime>
#include <QObject>
#include <condition_variable>
#include <mutex>
#include <queue>
#include <vector>

#include "common/date.h"
#include "common/enumerations/refresh_ui_type.h"
#include "common/listeners/app_listener.h"
#include "common/listeners/gui_listener.h"
#include "model/include/trade_configuration.h"

namespace auto_trader {
namespace trader {

class AppStatsUpdater : public QObject {
  Q_OBJECT

 public:
  AppStatsUpdater(common::AppListener& appListener, common::GuiListener& guiListener);

  void initialize();
  void addMessage(common::RefreshUiType message);

  void refreshStockExchangeViewData();
  void refreshTradingViewData();

  void setUpdateInterruptable();

 public:
  void refreshTradingCurrenciesUI();
  void refreshAllCurrenciesUI();
  void refreshAccountBalanceUI();
  void refreshAllOrdersUI();
  void refreshOpenOrdersUI();
  void refreshStatsInterruptableUI();

 private:
  void refreshTradingCurrencies();
  void refreshAllCurrencies();
  void refreshAccountBalance();
  void refreshAllOrders();
  void refreshOpenOrders();

  void refreshUI();

  void runMessageQueue();
  void runTelegramQueue();

  bool checkTradingInterval();

 public slots:
  void start();
  void stop();

 signals:
  void tradingCurrenciesChanged();
  void allCurrenciesChanged();
  void accountBalanceChanged();
  void allOrdersChanged();
  void openOrdersChanged();
  void progressStarted(int);
  void progressFinished();
  void incProgress(int);
  void statsUpdateInterrupted();

  void tradingStartedFromTelegram();
  void tradingStoppedFromTelegram();

 private:
  common::AppListener& appListener_;
  common::GuiListener& guiListener_;

  std::queue<common::RefreshUiType> messages_;

  std::vector<common::CurrencyTick> tradingCurrencies_;
  std::vector<common::CurrencyTick> allCurrencies_;
  std::vector<common::GuiListener::AccountBalance> accountBalance_;
  std::vector<common::MarketOrder> allOrders_;
  std::vector<common::MarketOrder> openOrders_;

  QDateTime lastUpdatedTick_;

  std::mutex uiLocker_;
  std::condition_variable condVar_;

  std::atomic_bool isRunning_;
  std::atomic_bool isProgressBarActive_;
  std::atomic_bool isStatsInterruptable_;

  std::atomic_int currentProgress_{0};
};

}  // namespace trader
}  // namespace auto_trader

#endif  // AUTO_TRADER_TRADER_APP_STATS_UPDATER_H
