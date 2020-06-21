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

#ifndef B2S_TRADER_TRADING_SELLING_STRATEGY_PROCESSOR_H
#define B2S_TRADER_TRADING_SELLING_STRATEGY_PROCESSOR_H

#include <memory>
#include <set>

#include "common/enumerations/strategies_type.h"
#include "common/listeners/gui_listener.h"
#include "common/market_history.h"
#include "common/market_order.h"
#include "database/include/database.h"
#include "model/include/holders/strategies_settings_holder.h"
#include "model/include/holders/trade_orders_holder.h"
#include "model/include/holders/trade_signaled_strategy_market_holder.h"
#include "model/include/settings/strategies_settings/strategy_settings.h"
#include "model/include/settings/strategies_settings/strategy_settings_visitor.h"
#include "model/include/trade_configuration.h"
#include "stocks_exchange/include/currency_lots_holder.h"
#include "stocks_exchange/include/stock_exchange_library.h"
#include "strategies/include/strategy_facade.h"
#include "trading_manager.h"
#include "trading_message_sender.h"

namespace auto_trader {

namespace model {
struct BollingerBandsSettings;
struct RsiSettings;
struct EmaSettings;
struct SmaSettings;
struct MovingAveragesCrossingSettings;
struct CustomStrategySettings;
}  // namespace model

namespace trader {

class TradingSellStrategyProcessor : private model::StrategySettingsVisitor {
 public:
  TradingSellStrategyProcessor(
      stock_exchange::QueryProcessor& queryProcessor, strategies::StrategyFacade& strategiesLibrary,
      database::Database& databaseProvider, common::AppListener& appListener,
      const model::StrategiesSettingsHolder& strategiesSettingsHolder,
      model::TradeOrdersHolder& tradeOrdersHolder, model::TradeConfigsHolder& tradeConfigsHolder,
      model::TradeSignaledStrategyMarketHolder& tradeSignaledStrategyMarketHolder,
      const model::TradeConfiguration& tradeConfiguration, TradingMessageSender& messageSender,
      const stock_exchange::CurrencyLotsHolder& lotsHolder, const TradingManager& tradingManager);

  void runStrategyProcessor();
  void runStopLossProcessor();
  void runTakeProfitProcessor();

  common::MarketOrder openOrder(const common::MarketOrder& buyOrder, double quantity, double price,
                                const std::string& message);

 private:
  void resetOrderProfit(model::OrdersProfit& orderProfit);

  double calculateOrderQuantity(const common::MarketOrder& order, model::OrdersProfit& orderProfit);
  double calculateLotSize(common::Currency::Enum tradedCurrency, double quantity) const;

 private:
  void visit(const model::BollingerBandsSettings& bandsSettings) final;
  void visit(const model::BollingerBandsAdvancedSettings& bandsAdvancedSettings) final;
  void visit(const model::RsiSettings& rsiSettings) final;
  void visit(const model::EmaSettings& emaSettings) final;
  void visit(const model::SmaSettings& smaSettings) final;
  void visit(const model::MovingAveragesCrossingSettings& movingAveragesCrossingSettings) final;
  void visit(const model::StochasticOscillatorSettings& stochasticOscillatorSettings) final;
  void visit(const model::CustomStrategySettings& customStrategySettings) final;

 private:
  common::MarketHistoryPtr getMarketHistory(common::TickInterval::Enum interval) const;

 private:
  stock_exchange::QueryProcessor& queryProcessor_;
  strategies::StrategyFacade& strategiesLibrary_;
  database::Database& databaseProvider_;
  common::AppListener& appListener_;
  const model::TradeConfiguration& tradeConfiguration_;
  const model::StrategiesSettingsHolder& strategiesSettingsHolder_;
  model::TradeOrdersHolder& tradeOrdersHolder_;
  model::TradeConfigsHolder& tradeConfigsHolder_;
  model::TradeSignaledStrategyMarketHolder& tradeSignaledStrategyMarketHolder_;
  TradingMessageSender& messageSender_;
  const stock_exchange::CurrencyLotsHolder& lotsHolder_;
  const TradingManager& tradingManager_;

  common::Currency::Enum currentTradedCurrency_;

  bool processingResult;
};

}  // namespace trader
}  // namespace auto_trader

#endif  // B2S_TRADER_TRADING_SELLING_STRATEGY_PROCESSOR_H
