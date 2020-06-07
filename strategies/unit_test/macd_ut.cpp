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

#include "macd_ut.h"

#include <gtest/gtest.h>

#include "common/exceptions/strategy_exception/bad_periods_for_lines_exception.h"
#include "common/exceptions/strategy_exception/not_correct_lines_size_exception.h"
#include "include/strategy_facade.h"
#include "include/strategy_factory.h"

namespace auto_trader {
namespace strategies {
namespace unit_test {

TEST(MacdStrategies, signalToSell) {
  StrategyFacade facade;

  auto macd = facade.getMacdStrategy();
  macd->createLines(macdToSell, 12, 26, 9, 3);

  EXPECT_EQ(macd->isNeedToSell(), true);
  EXPECT_EQ(macd->isNeedToBuy(), false);
}

TEST(MacdStrategies, signalToBuy) {
  StrategyFacade facade;

  auto macd = facade.getMacdStrategy();
  macd->createLines(macdToBuySignal, 12, 26, 9, 3);

  EXPECT_EQ(macd->isNeedToSell(), false);
  EXPECT_EQ(macd->isNeedToBuy(), true);
}

TEST(MacdStrategies, signalToBuy_testCrossingAfter) {
  StrategyFacade facade;

  auto macd = facade.getMacdStrategy();
  std::vector<common::MarketData> candlesToBuy = macdToBuySignal;

  macd->createLines(candlesToBuy, 12, 26, 9, 3);

  EXPECT_EQ(macd->isNeedToBuy(), true);
  EXPECT_EQ(macd->isNeedToSell(), false);

  candlesToBuy.emplace_back(additionalCandleForBuy);

  macd->createLines(candlesToBuy, 12, 26, 9, 3);

  EXPECT_EQ(macd->isNeedToBuy(), false);
  EXPECT_EQ(macd->isNeedToSell(), false);
}

TEST(MacdStrategies, equalPeriods) {
  StrategyFacade facade;

  auto macd = facade.getMacdStrategy();
  EXPECT_THROW(macd->createLines(macdToSell, 5, 5, 9, 3),
               common::exceptions::BadPeriodsForLinesException);
}

TEST(MacdStrategies, fastPeriodBiggerThenSlowPeriod) {
  StrategyFacade facade;

  auto macd = facade.getMacdStrategy();
  EXPECT_THROW(macd->createLines(macdToSell, 10, 6, 9, 3),
               common::exceptions::BadPeriodsForLinesException);
}

}  // namespace unit_test
}  // namespace strategies
}  // namespace auto_trader
