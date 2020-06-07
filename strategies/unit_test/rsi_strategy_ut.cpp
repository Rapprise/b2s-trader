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

#include "rsi_strategy_ut.h"

#include <gtest/gtest.h>

#include "common/exceptions/strategy_exception/strategy_exception.h"
#include "include/strategy_facade.h"
#include "include/strategy_factory.h"

namespace auto_trader {
namespace strategies {
namespace unit_test {

TEST(Rsi, Rsi_SellSignal_Test) {
  StrategyFacade facade;

  auto rsiStrategy = facade.getRsiStrategy();

  rsiStrategy->setTopRsiIndex(80);
  rsiStrategy->setBottomRsiIndex(20);

  rsiStrategy->createLine(rsiToSellSignal, 14, 3);

  EXPECT_FALSE(rsiStrategy->isNeedToBuy());
  EXPECT_TRUE(rsiStrategy->isNeedToSell());
}

TEST(Rsi, Rsi_SellSignal_And_DoubleCrossing_Test) {
  StrategyFacade facade;

  auto rsiStrategy = facade.getRsiStrategy();

  rsiStrategy->setTopRsiIndex(80);
  rsiStrategy->setBottomRsiIndex(20);

  rsiStrategy->createLine(rsiToSellSignal, 14, 3);

  EXPECT_FALSE(rsiStrategy->isNeedToBuy());
  EXPECT_TRUE(rsiStrategy->isNeedToSell());

  auto candlesToSellFalse = rsiToSellSignal;
  candlesToSellFalse.emplace_back(additionalCandleToSell);

  rsiStrategy->createLine(candlesToSellFalse, 14, 3, rsiStrategy->getLastBuyCrossingPoint(),
                          rsiStrategy->getLastSellCrossingPoint());

  EXPECT_FALSE(rsiStrategy->isNeedToBuy());
  EXPECT_FALSE(rsiStrategy->isNeedToSell());
}

TEST(Rsi, Rsi_BuySignal_Test) {
  StrategyFacade facade;

  auto rsiStrategy = facade.getRsiStrategy();

  rsiStrategy->setTopRsiIndex(80);
  rsiStrategy->setBottomRsiIndex(20);

  rsiStrategy->createLine(rsiToBuySignal, 14, 3);

  EXPECT_TRUE(rsiStrategy->isNeedToBuy());
  EXPECT_FALSE(rsiStrategy->isNeedToSell());
}

TEST(Rsi, Rsi_BuySignal_DoubleCrossing_Test) {
  StrategyFacade facade;

  auto rsiStrategy = facade.getRsiStrategy();

  rsiStrategy->setTopRsiIndex(80);
  rsiStrategy->setBottomRsiIndex(25);

  rsiStrategy->createLine(rsiToBuyForDoubleCrossingSignal, 14, 3);

  EXPECT_TRUE(rsiStrategy->isNeedToBuy());
  EXPECT_FALSE(rsiStrategy->isNeedToSell());

  auto doubleCrossing = rsiToBuyForDoubleCrossingSignal;
  doubleCrossing.emplace_back(additionalCandleForBoyDoubleCrossing);

  rsiStrategy->createLine(doubleCrossing, 14, 3, rsiStrategy->getLastBuyCrossingPoint(),
                          rsiStrategy->getLastSellCrossingPoint());

  EXPECT_FALSE(rsiStrategy->isNeedToBuy());
  EXPECT_FALSE(rsiStrategy->isNeedToSell());
}

TEST(Rsi, Rsi_EmptyMarketData_Test) {
  StrategyFacade facade;
  auto rsiStrategy = facade.getRsiStrategy();

  rsiStrategy->setTopRsiIndex(80);
  rsiStrategy->setBottomRsiIndex(25);

  EXPECT_THROW(rsiStrategy->createLine(rsiEmptyMarketData, 14, 3),
               common::exceptions::StrategyException);
}

TEST(Rsi, Rsi_PeriodBiggerThanMarketDataSize_Test) {
  StrategyFacade facade;
  auto rsiStrategy = facade.getRsiStrategy();

  rsiStrategy->setTopRsiIndex(80);
  rsiStrategy->setBottomRsiIndex(25);

  EXPECT_THROW(rsiStrategy->createLine(rsiFiveCandles, 14, 3),
               common::exceptions::StrategyException);
}

}  // namespace unit_test
}  // namespace strategies
}  // namespace auto_trader
