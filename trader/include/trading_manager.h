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

#ifndef AUTO_TRADER_TRADING_MANAGER_H
#define AUTO_TRADER_TRADING_MANAGER_H

#include <QObject>
#include <condition_variable>
#include <mutex>
#include <set>
#include <unordered_set>

#include "common/currency.h"
#include "common/enumerations/stock_exchange_type.h"
#include "common/listeners/gui_listener.h"
#include "common/market_order.h"
#include "database/include/database.h"
#include "model/include/holders/strategies_settings_holder.h"
#include "model/include/holders/trade_configs_holder.h"
#include "model/include/holders/trade_orders_holder.h"
#include "model/include/holders/trade_signaled_strategy_market_holder.h"
#include "model/include/orders/orders_matching.h"
#include "model/include/orders/orders_profit.h"
#include "model/include/settings/app_settings.h"
#include "model/include/trade_configuration.h"
#include "stocks_exchange/include/currency_lots_holder.h"
#include "stocks_exchange/include/stock_exchange_library.h"
#include "strategies/include/strategy_facade.h"
#include "trading_message_sender.h"

namespace auto_trader {
namespace trader {

class TradingManager : public QObject {
  Q_OBJECT

 public:
  TradingManager(stock_exchange::QueryProcessor& queryProcessor,
                 strategies::StrategyFacade& strategyFacade, database::Database& databaseProvider,
                 common::AppListener& appListener, common::GuiListener& guiListener,
                 model::AppSettings& appSettings, TradingMessageSender& messageSender_,
                 model::StrategiesSettingsHolder& strategiesSettingsHolder,
                 model::TradeOrdersHolder& tradeOrdersHolder,
                 model::TradeConfigsHolder& tradeConfigsHolder,
                 model::TradeSignaledStrategyMarketHolder& tradeSignaledStrategyMarketHolder);

  void reset(bool value);

 public slots:
  void startTradingSlot();
  void stopTradingSlot();

 public:
  bool isRunning() const;

 private:
  void prepareBuying();
  void prepareSelling();

  void loadOrders();

  void cancelOutdatedBuyingOrders(const std::set<common::MarketOrder>& orders);
  void cancelOutdatedSellingOrders(const std::set<common::MarketOrder>& orders);

  void updateClosedBuyingOrders(const std::set<common::MarketOrder>& orders);
  void updateClosedSellingOrders(const std::set<common::MarketOrder>& orders);

  const std::set<common::MarketOrder> updateManuallyOpenedBuyingOrders(
      const std::set<common::MarketOrder>& orders);
  const std::set<common::MarketOrder> updateManuallyOpenedSellingOrders(
      const std::set<common::MarketOrder>& orders);

  void uploadBuyingOrders();
  void uploadSellOrders();

  void runStopLoss();

  bool isOrderManuallyCanceled(const common::MarketOrder& order) const;

 private:
  bool isDataExists() const;
  void resetData();

 signals:
  void tradingStarted();
  void tradingStopped();

  void tradingDataOutdated();

 private:
  stock_exchange::QueryProcessor& queryProcessor_;
  strategies::StrategyFacade& strategyFacade_;
  database::Database& databaseProvider_;
  common::AppListener& appListener_;
  common::GuiListener& guiListener_;
  model::AppSettings& appSettings_;
  model::StrategiesSettingsHolder& strategiesSettingsHolder_;
  model::TradeConfigsHolder& tradeConfigsHolder_;
  model::TradeOrdersHolder& tradeOrdersHolder_;
  model::TradeSignaledStrategyMarketHolder& tradeSignaledStrategyMarketHolder_;
  TradingMessageSender& messageSender_;
  stock_exchange::CurrencyLotsHolder currencyLotsHolder_;

  std::mutex locker_;
  std::condition_variable condVar_;
  std::atomic_bool isRunning_;
  std::atomic_bool isReset_;
};

}  // namespace trader
}  // namespace auto_trader

#endif  // AUTO_TRADER_TRADING_MANAGER_H
