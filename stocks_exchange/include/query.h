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

#ifndef STOCKS_EXCHANGE_STOCK_QUERY_H_
#define STOCKS_EXCHANGE_STOCK_QUERY_H_

#include <Poco/Net/HTTPSClientSession.h>
#include <Poco/URI.h>

#include <unordered_set>

#include "common/currency.h"
#include "common/enumerations/stock_exchange_type.h"
#include "common/enumerations/tick_interval.h"
#include "common/market_history.h"
#include "common/market_order.h"
#include "currency_lots_holder.h"

namespace auto_trader {
namespace stock_exchange {

class Query {
 public:
  virtual ~Query() = default;

  virtual common::MarketOrder sellOrder(common::Currency::Enum fromCurrency,
                                        common::Currency::Enum toCurrency, double quantity,
                                        double rate) = 0;
  virtual common::MarketOrder buyOrder(common::Currency::Enum fromCurrency,
                                       common::Currency::Enum toCurrency, double quantity,
                                       double rate) = 0;

  virtual bool cancelOrder(common::Currency::Enum fromCurrency, common::Currency::Enum toCurrency,
                           const std::string& uuid) = 0;
  virtual void updateApiKey(const std::string& api_key) = 0;
  virtual void updateSecretKey(const std::string& secret_key) = 0;

  virtual common::MarketHistoryPtr getMarketHistory(common::Currency::Enum fromCurrency,
                                                    common::Currency::Enum toCurrency,
                                                    common::TickInterval::Enum interval) = 0;

  virtual std::vector<common::MarketOrder> getMarketOpenOrders(
      common::Currency::Enum fromCurrency, common::Currency::Enum toCurrency) = 0;
  virtual std::vector<common::MarketOrder> getAccountOpenOrders(
      common::Currency::Enum fromCurrency, common::Currency::Enum toCurrency) = 0;
  virtual common::MarketOrder getAccountOrder(common::Currency::Enum fromCurrency,
                                              common::Currency::Enum toCurrency,
                                              const std::string& uuid) = 0;

  virtual common::CurrencyTick getCurrencyTick(common::Currency::Enum fromCurrency,
                                               common::Currency::Enum toCurrency) = 0;
  virtual double getBalance(common::Currency::Enum currency) = 0;

  virtual CurrencyLotsHolder getCurrencyLotsHolder() = 0;
};

typedef std::shared_ptr<Query> QueryPtr;

}  // namespace stock_exchange
}  // namespace auto_trader

#endif /* STOCKS_EXCHANGE_STOCK_QUERY_H_ */
