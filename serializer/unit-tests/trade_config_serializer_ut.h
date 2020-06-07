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

#ifndef AUTO_TRADER_TRADE_CONFIG_SERIALIZER_UT_H
#define AUTO_TRADER_TRADE_CONFIG_SERIALIZER_UT_H

#include <gtest/gtest.h>

#include "include/trade_config_json_serializer.h"

namespace auto_trader {
namespace serializer {
namespace unit_tests {

class TradeConfigSerializerUTFixture : public ::testing::Test {
 public:
  void SetUp() override { tradeConfigSerializer_ = std::make_unique<TradeConfigJSONSerializer>(); }

  void TearDown() override { tradeConfigSerializer_.reset(); }

  TradeConfigJSONSerializer& getTradeConfigSerializer() const {
    EXPECT_TRUE(tradeConfigSerializer_);
    return *tradeConfigSerializer_;
  }

 private:
  std::unique_ptr<TradeConfigJSONSerializer> tradeConfigSerializer_;
};

}  // namespace unit_tests
}  // namespace serializer
}  // namespace auto_trader

#endif  // AUTO_TRADER_TRADE_CONFIG_SERIALIZER_UT_H
