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

#include "sma_strategy_ut.h"

#include <gtest/gtest.h>

#include "common/exceptions/strategy_exception/strategy_exception.h"
#include "include/strategy_facade.h"
#include "include/strategy_factory.h"

namespace auto_trader {
namespace strategies {
namespace unit_test {

TEST(Sma, Sma_LineMatching_Test) {
  StrategyFacade facade;
  auto smaStrategy = facade.getSmaStrategy();
  smaStrategy->createLine(smaCandlesToBuy, 10, 1, 0, 0);

  auto line = smaStrategy->getLine();

  EXPECT_EQ(line.getSize(), 2);
  EXPECT_EQ(line.getPoint(0), 22.220999999999997);
  EXPECT_EQ(line.getPoint(1), 22.209);
}

TEST(Sma, OrderSmaToSell) {
  StrategyFacade facade;
  auto smaStrategy = facade.getSmaStrategy();
  smaStrategy->createLine(smaCandlesToSell, 10, 3, 0, 0);

  auto line = smaStrategy->getLine();
  EXPECT_EQ(line.getSize(), 5);
  EXPECT_EQ(line.getPoint(0), 22.220999999999997);
  EXPECT_EQ(line.getPoint(1), 22.209);
  EXPECT_EQ(line.getPoint(2), 22.229000000000003);
  EXPECT_EQ(line.getPoint(3), 22.259000000000004);
  EXPECT_EQ(line.getPoint(4), 22.303000000000004);
}

TEST(Sma, Sma_Second_LineMatching_Test) {
  StrategyFacade facade;
  auto smaStrategy = facade.getSmaStrategy();
  smaStrategy->createLine(smaCandlesToSell, 10, 2, 0, 0);

  auto line = smaStrategy->getLine();
  EXPECT_EQ(line.getSize(), 5);
  EXPECT_EQ(line.getPoint(0), 22.220999999999997);
  EXPECT_EQ(line.getPoint(1), 22.209);
  EXPECT_EQ(line.getPoint(2), 22.229000000000003);
  EXPECT_EQ(line.getPoint(3), 22.259000000000004);
  EXPECT_EQ(line.getPoint(4), 22.303000000000004);

  EXPECT_FALSE(smaStrategy->isNeedToSell());
  EXPECT_FALSE(smaStrategy->isNeedToBuy());
}

TEST(Sma, Sma_BuyTrue_Test) {
  StrategyFacade facade;
  auto smaStrategy = facade.getSmaStrategy();
  smaStrategy->createLine(smaCandlesToBuySecond, 10, 3, 0, 0);

  EXPECT_FALSE(smaStrategy->isNeedToSell());
  EXPECT_TRUE(smaStrategy->isNeedToBuy());
}

TEST(Sma, Sma_SellTrue_Test) {
  StrategyFacade facade;
  auto smaStrategy = facade.getSmaStrategy();
  smaStrategy->createLine(smaCandlesToSellSecond, 10, 3, 0, 0);

  EXPECT_TRUE(smaStrategy->isNeedToSell());
  EXPECT_FALSE(smaStrategy->isNeedToBuy());
}

TEST(Sma, Sma_EmptyMarketData_Test) {
  StrategyFacade facade;

  auto smaStrategy = facade.getSmaStrategy();
  EXPECT_THROW(smaStrategy->createLine(smaEmptyMarketData, 10, 3, 0, 0),
               common::exceptions::StrategyException);
}

TEST(Sma, Sma_PeriodBiggerThanMarketDataSize_Test) {
  StrategyFacade facade;

  auto smaStrategy = facade.getSmaStrategy();
  EXPECT_THROW(smaStrategy->createLine(smaFiveCandles, 10, 3, 0, 0),
               common::exceptions::StrategyException);
}

}  // namespace unit_test
}  // namespace strategies
}  // namespace auto_trader