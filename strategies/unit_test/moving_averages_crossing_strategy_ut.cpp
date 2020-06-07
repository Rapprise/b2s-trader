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

#include "moving_averages_crossing_strategy_ut.h"

#include <gtest/gtest.h>

#include "common/exceptions/strategy_exception/strategy_exception.h"
#include "include/strategy_facade.h"
#include "include/strategy_factory.h"

namespace auto_trader {
namespace strategies {
namespace unit_test {

TEST(Ma_Crossing_Sma, Ma_Crossing_Sma_OrderToSellTrue_Test) {
  StrategyFacade facade;

  auto maCrossingStrategy = facade.getMACrossingStrategy();
  maCrossingStrategy->setCrossingInterval(3);
  maCrossingStrategy->createLines(macCandlesToSell, 9, 18, 0, 0, common::MovingAverageType::SIMPLE);

  EXPECT_TRUE(maCrossingStrategy->isNeedToSell());
  EXPECT_FALSE(maCrossingStrategy->isNeedToBuy());
}

TEST(Ma_Crossing_Sma, Ma_Crossing_Sma_OrderToSellFalse_Crossing_Test) {
  StrategyFacade facade;

  auto maCrossingStrategy = facade.getMACrossingStrategy();
  maCrossingStrategy->setCrossingInterval(3);
  maCrossingStrategy->createLines(macCandlesToSell, 9, 18, 0, 0, common::MovingAverageType::SIMPLE);

  EXPECT_TRUE(maCrossingStrategy->isNeedToSell());
  EXPECT_FALSE(maCrossingStrategy->isNeedToBuy());

  auto candlesToCheckCrossing = macCandlesToSell;
  candlesToCheckCrossing.emplace_back(additionalCandleToSell_crossing);

  maCrossingStrategy->createLines(
      macCandlesToSell, 9, 18, maCrossingStrategy->getLastBuyCrossingPoint(),
      maCrossingStrategy->getLastSellCrossingPoint(), common::MovingAverageType::SIMPLE);

  EXPECT_FALSE(maCrossingStrategy->isNeedToSell());
  EXPECT_FALSE(maCrossingStrategy->isNeedToBuy());
}

TEST(Ma_Crossing_Sma, Ma_Crossing_Sma_OrderToBuyTrue_Test) {
  StrategyFacade facade;

  auto maCrossingStrategy = facade.getMACrossingStrategy();
  maCrossingStrategy->setCrossingInterval(3);
  maCrossingStrategy->createLines(macCandlesToBuy, 9, 18, 0, 0, common::MovingAverageType::SIMPLE);

  EXPECT_TRUE(maCrossingStrategy->isNeedToBuy());
  EXPECT_FALSE(maCrossingStrategy->isNeedToSell());
}

TEST(Ma_Crossing_Sma, Ma_Crossing_Sma_OrderToSellTrue_AfterBuy_Test) {
  StrategyFacade facade;

  auto maCrossingStrategy = facade.getMACrossingStrategy();
  maCrossingStrategy->setCrossingInterval(3);
  maCrossingStrategy->createLines(macCandlesToBuy, 9, 18, 0, 0, common::MovingAverageType::SIMPLE);

  EXPECT_TRUE(maCrossingStrategy->isNeedToBuy());
  EXPECT_FALSE(maCrossingStrategy->isNeedToSell());

  auto candlesToSellTrue = macCandlesToBuy;
  candlesToSellTrue.emplace_back(additionalCandleToSell_True);
  maCrossingStrategy->createLines(candlesToSellTrue, 9, 18, 0, 0,
                                  common::MovingAverageType::SIMPLE);

  EXPECT_FALSE(maCrossingStrategy->isNeedToBuy());
  EXPECT_TRUE(maCrossingStrategy->isNeedToSell());
}

TEST(Ma_Crossing_Sma, Ma_Crossing_Sma_NoSignalsAfterEvents_Test) {
  StrategyFacade facade;

  auto maCrossingStrategy = facade.getMACrossingStrategy();
  maCrossingStrategy->setCrossingInterval(3);
  maCrossingStrategy->createLines(macCandlesToBuy, 9, 18, 0, 0, common::MovingAverageType::SIMPLE);

  EXPECT_TRUE(maCrossingStrategy->isNeedToBuy());
  EXPECT_FALSE(maCrossingStrategy->isNeedToSell());

  auto candlesToSellTrue = macCandlesToBuy;
  candlesToSellTrue.emplace_back(additionalCandleToSell_True);
  maCrossingStrategy->createLines(candlesToSellTrue, 9, 18, 0, 0,
                                  common::MovingAverageType::SIMPLE);

  EXPECT_FALSE(maCrossingStrategy->isNeedToBuy());
  EXPECT_TRUE(maCrossingStrategy->isNeedToSell());

  auto candlesNotSignals = candlesToSellTrue;
  candlesNotSignals.emplace_back(additionalCandleToSellFalse_crossing);
  maCrossingStrategy->createLines(
      candlesToSellTrue, 9, 18, maCrossingStrategy->getLastBuyCrossingPoint(),
      maCrossingStrategy->getLastSellCrossingPoint(), common::MovingAverageType::SIMPLE);

  EXPECT_FALSE(maCrossingStrategy->isNeedToBuy());
  EXPECT_FALSE(maCrossingStrategy->isNeedToSell());
}

TEST(Ma_Crossing_Ema, Ma_Crossing_Ema_OrderToSellTrue_Test) {
  StrategyFacade facade;

  auto maCrossingStrategy = facade.getMACrossingStrategy();
  maCrossingStrategy->setCrossingInterval(3);
  maCrossingStrategy->createLines(macEmaCandlesToSell, 9, 18, 0, 0,
                                  common::MovingAverageType::EXPONENTIAL);

  EXPECT_TRUE(maCrossingStrategy->isNeedToSell());
  EXPECT_FALSE(maCrossingStrategy->isNeedToBuy());
}

TEST(Ma_Crossing_Ema, Ma_Crossing_Ema_OrderToSellFalse_Crossing_Test) {
  StrategyFacade facade;

  auto maCrossingStrategy = facade.getMACrossingStrategy();
  maCrossingStrategy->setCrossingInterval(3);
  maCrossingStrategy->createLines(macEmaCandlesToSell, 9, 18, 0, 0,
                                  common::MovingAverageType::EXPONENTIAL);

  EXPECT_TRUE(maCrossingStrategy->isNeedToSell());
  EXPECT_FALSE(maCrossingStrategy->isNeedToBuy());

  auto emaCandlesToCHeckCrossing = macEmaCandlesToSell;
  emaCandlesToCHeckCrossing.emplace_back(additionalEmaCandleToSellTrue);
  maCrossingStrategy->createLines(emaCandlesToCHeckCrossing, 9, 18, 0, 0,
                                  common::MovingAverageType::EXPONENTIAL);

  EXPECT_FALSE(maCrossingStrategy->isNeedToSell());
  EXPECT_FALSE(maCrossingStrategy->isNeedToBuy());
}

TEST(Ma_Crossing_Ema, Ma_Crossing_Ema_OrderToBuyTrue_Test) {
  StrategyFacade facade;

  auto maCrossingStrategy = facade.getMACrossingStrategy();
  maCrossingStrategy->setCrossingInterval(3);
  maCrossingStrategy->createLines(macEmaCandlesToBuy, 9, 18, 0, 0,
                                  common::MovingAverageType::EXPONENTIAL);

  EXPECT_FALSE(maCrossingStrategy->isNeedToSell());
  EXPECT_TRUE(maCrossingStrategy->isNeedToBuy());
}

TEST(Ma_Crossing_Ema, Ma_Crossing_Ema_EmptyMarketData_Test) {
  StrategyFacade facade;

  auto maCrossingStrategy = facade.getMACrossingStrategy();
  maCrossingStrategy->setCrossingInterval(3);
  EXPECT_THROW(maCrossingStrategy->createLines(macEmptyMarketData, 9, 18, 0, 0,
                                               common::MovingAverageType::EXPONENTIAL),
               common::exceptions::StrategyException);
}

TEST(Ma_Crossing_Ema, Ma_Crossing_Ema_PeriodBiggerThanMarketDataSize_Test) {
  StrategyFacade facade;

  auto maCrossingStrategy = facade.getMACrossingStrategy();
  maCrossingStrategy->setCrossingInterval(3);
  EXPECT_THROW(maCrossingStrategy->createLines(macFiveCandles, 9, 18, 0, 0,
                                               common::MovingAverageType::EXPONENTIAL),
               common::exceptions::StrategyException);
}

}  // namespace unit_test
}  // namespace strategies
}  // namespace auto_trader