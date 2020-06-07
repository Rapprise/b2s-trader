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

#ifndef AUTO_TRADER_FAKE_QUERY_PROCESSOR_H
#define AUTO_TRADER_FAKE_QUERY_PROCESSOR_H

#include "fake_stock_exchange_query.h"
#include "stocks_exchange/include/query.h"
#include "stocks_exchange/include/query_factory.h"
#include "stocks_exchange/include/query_processor.h"

namespace auto_trader {
namespace trader {
namespace unit_test {

class FakeQueryProcessor : public stock_exchange::QueryProcessor {
 public:
  explicit FakeQueryProcessor(stock_exchange::QueryFactory& factory);
  stock_exchange::QueryPtr getQuery(common::StockExchangeType type) override;

 private:
  std::shared_ptr<stock_exchange::Query> fakeQuery_;
};

}  // namespace unit_test
}  // namespace trader
}  // namespace auto_trader

#endif  // AUTO_TRADER_FAKE_QUERY_PROCESSOR_H
