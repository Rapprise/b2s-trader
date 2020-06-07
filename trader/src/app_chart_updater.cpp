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

#include "include/app_chart_updater.h"

#include "common/loggers/file_logger.h"
#include "stocks_exchange/include/query_processor.h"

namespace auto_trader {
namespace trader {

AppChartUpdater::AppChartUpdater(common::AppListener& appListener, common::GuiListener& guiListener)
    : appListener_(appListener), guiListener_(guiListener) {
  marketHistory_.reset();
}

void AppChartUpdater::refreshMarketHistory(unsigned int baseCurrency, unsigned int tradedCurrency,
                                           unsigned int interval) {
  auto& tradeConfigurationsHolder = appListener_.getTradeConfigsHolder();
  if (tradeConfigurationsHolder.isEmpty()) {
    return;
  }

  auto baseCurrencyValue = static_cast<common::Currency::Enum>(baseCurrency);
  auto tradedCurrencyValue = static_cast<common::Currency::Enum>(tradedCurrency);
  auto tickIntervalValue = static_cast<common::TickInterval::Enum>(interval);

  try {
    const auto& currentConfig = tradeConfigurationsHolder.getCurrentTradeConfiguration();
    auto& queryProcessor = appListener_.getQueryProcessor();
    auto& stockExchangeSettings = currentConfig.getStockExchangeSettings();
    auto query = queryProcessor.getQuery(stockExchangeSettings.stockExchangeType_);
    auto stockExchangeType = static_cast<unsigned int>(stockExchangeSettings.stockExchangeType_);
    try {
      marketHistory_.reset();
      marketHistory_ =
          query->getMarketHistory(baseCurrencyValue, tradedCurrencyValue, tickIntervalValue);
    } catch (std::exception& response) {
      common::loggers::FileLogger::getLogger() << response.what();
    }

    if (!marketHistory_) {
      marketHistory_ = std::make_unique<common::MarketHistory>();
    }

    emit marketHistoryChanged(stockExchangeType);
  } catch (std::exception& exception) {
    common::loggers::FileLogger::getLogger() << exception.what();
  }
}

void AppChartUpdater::refreshMarketHistoryUI(unsigned int stockExchangeType) {
  auto stockExchange = static_cast<common::StockExchangeType>(stockExchangeType);
  guiListener_.refreshChartViewFinish(std::move(marketHistory_), stockExchange);
}

}  // namespace trader
}  // namespace auto_trader