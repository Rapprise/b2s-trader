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

#include "ema_strategy_ut.h"

#include <gtest/gtest.h>

#include "common/exceptions/strategy_exception/strategy_exception.h"
#include "include/strategy_facade.h"
#include "include/strategy_factory.h"

namespace auto_trader {
namespace strategies {
namespace unit_test {

TEST(Ema, Ema_Ema_Matching_Test) {
  StrategyFacade facade;
  auto emaStrategy = facade.getEmaStrategy();
  emaStrategy->createLine(emaCandlesToBuy, 10, 1, 0, 0);

  auto line = emaStrategy->getLine();
  EXPECT_EQ(line.getSize(), 2);
  EXPECT_EQ(line.getPoint(0), 22.220999999999997);
  EXPECT_EQ(line.getPoint(1), 22.208090909090906);
}

TEST(Ema, Ema_Ema_Matching_Second_Test) {
  StrategyFacade facade;
  auto emaStrategy = facade.getEmaStrategy();
  emaStrategy->createLine(emaCandlesToSell, 10, 3, 0, 0);

  auto line = emaStrategy->getLine();
  EXPECT_EQ(line.getSize(), 5);
  EXPECT_EQ(line.getPoint(0), 22.220999999999997);
  EXPECT_EQ(line.getPoint(1), 22.208090909090906);
  EXPECT_EQ(line.getPoint(2), 22.241165289256195);
  EXPECT_EQ(line.getPoint(3), 22.266407963936885);
  EXPECT_EQ(line.getPoint(4), 22.328879243221088);
}

TEST(Ema, OrderSma_DoNothing_With_Current_Stock_Data) {
  StrategyFacade facade;
  auto emaStrategy = facade.getEmaStrategy();
  emaStrategy->createLine(emaCandlesToSell, 10, 2, 0, 0);

  auto line = emaStrategy->getLine();
  EXPECT_EQ(line.getSize(), 5);
  EXPECT_EQ(line.getPoint(0), 22.220999999999997);
  EXPECT_EQ(line.getPoint(1), 22.208090909090906);
  EXPECT_EQ(line.getPoint(2), 22.241165289256195);
  EXPECT_EQ(line.getPoint(3), 22.266407963936885);
  EXPECT_EQ(line.getPoint(4), 22.328879243221088);

  EXPECT_FALSE(emaStrategy->isNeedToSell());
  EXPECT_FALSE(emaStrategy->isNeedToBuy());
}

TEST(Ema, Ema_CheckPoints_Test) {
  StrategyFacade facade;
  auto emaStrategy = facade.getEmaStrategy();
  emaStrategy->createLine(emaCandlesToCheckPoints, 26, 3, 0, 0);

  auto line = emaStrategy->getLine();

  EXPECT_EQ(line.getPoint(0), 27.286923076923078);
  EXPECT_EQ(line.getPoint(1), 27.306410256410256);
  EXPECT_EQ(line.getPoint(2), 27.34741690408357);
  EXPECT_EQ(line.getPoint(3), 27.386867503781083);
  EXPECT_EQ(line.getPoint(4), 27.434506947945447);
  EXPECT_EQ(line.getPoint(5), 27.479358285134673);
  EXPECT_EQ(line.getPoint(6), 27.518665078828402);
}

TEST(Ema, Ema_CandlesToBuyTrue_Test) {
  StrategyFacade facade;
  auto emaStrategy = facade.getEmaStrategy();
  emaStrategy->createLine(emaCandlesToBuySecond, 10, 3, 0, 0);

  EXPECT_TRUE(emaStrategy->isNeedToBuy());
  EXPECT_FALSE(emaStrategy->isNeedToSell());
}

TEST(Ema, Ema_CandlesToSellTrue_Test) {
  StrategyFacade facade;
  auto emaStrategy = facade.getEmaStrategy();
  emaStrategy->createLine(emaCandlesToSellSecond, 10, 3, 0, 0);

  EXPECT_FALSE(emaStrategy->isNeedToBuy());
  EXPECT_TRUE(emaStrategy->isNeedToSell());
}

TEST(Ema, Ema_EmptyMarketData_Test) {
  StrategyFacade facade;

  auto emaStrategy = facade.getEmaStrategy();
  EXPECT_THROW(emaStrategy->createLine(emaEmptyMarketData, 10, 3, 0, 0),
               common::exceptions::StrategyException);
}

TEST(Ema, Ema_PeriodBiggerThanMarketDataSize_Test) {
  StrategyFacade facade;

  auto emaStrategy = facade.getEmaStrategy();
  EXPECT_THROW(emaStrategy->createLine(emaFiveCandles, 10, 3, 0, 0),
               common::exceptions::StrategyException);
}

}  // namespace unit_test
}  // namespace strategies
}  // namespace auto_trader