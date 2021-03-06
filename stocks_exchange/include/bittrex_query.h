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


#ifndef STOCKS_EXCHANGE_BITTREX_QUERY_H_
#define STOCKS_EXCHANGE_BITTREX_QUERY_H_

#include "base_query.h"
#include "common/bittrex_currency.h"
#include "query.h"

namespace auto_trader {
namespace stock_exchange {

class BittrexQuery : public BaseQuery<Query> {
 public:
  common::MarketOrder sellOrder(common::Currency::Enum fromCurrency,
                                common::Currency::Enum toCurrency, double quantity,
                                double rate) override;
  common::MarketOrder buyOrder(common::Currency::Enum fromCurrency,
                               common::Currency::Enum toCurrency, double quantity,
                               double rate) override;

  bool cancelOrder(common::Currency::Enum fromCurrency, common::Currency::Enum toCurrency,
                   const std::string& uuid) override;

  std::vector<common::MarketOrder> getAccountOpenOrders(common::Currency::Enum fromCurrency,
                                                        common::Currency::Enum toCurrency) override;
  std::vector<common::MarketOrder> getMarketOpenOrders(common::Currency::Enum fromCurrency,
                                                       common::Currency::Enum toCurrency) override;
  common::MarketOrder getAccountOrder(common::Currency::Enum fromCurrency,
                                      common::Currency::Enum toCurrency,
                                      const std::string& uuid) override;

  common::MarketHistoryPtr getMarketHistory(common::Currency::Enum fromCurrency,
                                            common::Currency::Enum toCurrency,
                                            common::TickInterval::Enum interval) override;

  common::CurrencyTick getCurrencyTick(common::Currency::Enum fromCurrency,
                                       common::Currency::Enum toCurrency) override;
  double getBalance(common::Currency::Enum currency) override;

  CurrencyLotsHolder getCurrencyLotsHolder() override;

 private:
  common::MarketHistoryPtr parseMarketHistory(const std::string& response) const;

 private:
  common::BittrexCurrency bittrexCurrency_;
};

}  // namespace stock_exchange
}  // namespace auto_trader

#endif /* STOCKS_EXCHANGE_BITTREX_QUERY_H_ */
