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

#ifndef AUTO_TRADER_COMMON_APP_LISTENER_H
#define AUTO_TRADER_COMMON_APP_LISTENER_H

#include <memory>

#include "common/enumerations/application_theme_type.h"
#include "common/enumerations/refresh_ui_type.h"
#include "model/include/holders/strategies_settings_holder.h"
#include "model/include/holders/trade_configs_holder.h"
#include "model/include/settings/app_settings.h"
#include "model/include/trade_configuration.h"

namespace auto_trader {
namespace stock_exchange {
class QueryProcessor;
}
namespace common {

class AppListener {
 public:
  virtual ~AppListener() = default;

  virtual void changeTheme(common::ApplicationThemeType themeType) = 0;

  virtual void saveStrategiesSettingsFiles() const = 0;
  virtual void saveTradeConfigurationsFiles() const = 0;
  virtual void saveFeaturesSettings() const = 0;
  virtual void saveAppSettings() const = 0;

  virtual model::StrategiesSettingsHolder& getStrategySettingsHolder() = 0;
  virtual model::TradeConfigsHolder& getTradeConfigsHolder() = 0;

  virtual stock_exchange::QueryProcessor& getQueryProcessor() = 0;
  virtual model::AppSettings& getAppSettings() = 0;

  virtual void runTrading() = 0;
  virtual void stopTrading() = 0;

  virtual void refreshUIMessage(common::RefreshUiType type) = 0;

  virtual void refreshMarketHistory(common::Currency::Enum baseCurrency,
                                    common::Currency::Enum tradedCurrency,
                                    common::TickInterval::Enum interval) = 0;

  virtual void refreshStockExchangeView() = 0;
  virtual void refreshTradingView() = 0;

  virtual void refreshApiKeys(const model::TradeConfiguration& configuration) = 0;

  virtual void interruptStatsUpdate() = 0;
};

}  // namespace common
}  // namespace auto_trader

#endif  // AUTO_TRADER_COMMON_APP_LISTENER_H
