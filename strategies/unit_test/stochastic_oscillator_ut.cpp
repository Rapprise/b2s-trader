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

#include "stochastic_oscillator_ut.h"

#include <gtest/gtest.h>

#include "common/exceptions/strategy_exception/strategy_exception.h"
#include "include/strategy_facade.h"
#include "include/strategy_factory.h"

namespace auto_trader {
namespace strategies {
namespace unit_test {

namespace stochastic_to_buy_signal {
const int quick_first_point = 72;
const int quick_second_point = 42;
const int quick_third_point = 65;

const int slow_first_point = 60;
const int slow_second_point = 51;
const int slow_third_point = 51;
}  // namespace stochastic_to_buy_signal

TEST(QuickStochastic, ExpectTrueOnBuyOrder) {
  StrategyFacade facade;

  auto stoch = facade.getStochasticOscillatorStrategy();
  stoch->setTopLevel(80);
  stoch->setBottomLevel(20);

  stoch->createLines(stochasticToBuy, common::StochasticOscillatorType::Quick, 10);

  EXPECT_EQ(stoch->isNeedToBuy(), false);
  EXPECT_EQ(stoch->isNeedToSell(), true);
}

TEST(QuickStochastic, EqualsForLinePoints) {
  StrategyFacade facade;

  auto stoch = facade.getStochasticOscillatorStrategy();
  stoch->setTopLevel(80);
  stoch->setBottomLevel(20);

  stoch->createLines(stochasticToBuy, common::StochasticOscillatorType::Quick, 10);

  auto quickLine = stoch->getQuickLine();
  auto slowLine = stoch->getSlowLine();

  EXPECT_EQ(static_cast<int>(quickLine.getPoint(0)), stochastic_to_buy_signal::quick_first_point);
  EXPECT_EQ(static_cast<int>(quickLine.getPoint(1)), stochastic_to_buy_signal::quick_second_point);
  EXPECT_EQ(static_cast<int>(quickLine.getPoint(2)), stochastic_to_buy_signal::quick_third_point);

  EXPECT_EQ(static_cast<int>(slowLine.getPoint(0)), stochastic_to_buy_signal::slow_first_point);
  EXPECT_EQ(static_cast<int>(slowLine.getPoint(1)), stochastic_to_buy_signal::slow_second_point);
  EXPECT_EQ(static_cast<int>(slowLine.getPoint(2)), stochastic_to_buy_signal::slow_third_point);
}

TEST(QuickStochastic, DoubleBuySignal) {
  StrategyFacade facade;

  auto stoch = facade.getStochasticOscillatorStrategy();
  stoch->setTopLevel(80);
  stoch->setBottomLevel(20);

  stoch->createLines(stochasticToBuy, common::StochasticOscillatorType::Quick, 10);

  EXPECT_EQ(stoch->isNeedToSell(), true);

  stoch->createLines(stochasticToBuy, common::StochasticOscillatorType::Quick, 10, 3, 3, 3,
                     stoch->getLastBuyCrossingPoint(), stoch->getLastSellCrossingPoint());

  EXPECT_EQ(stoch->isNeedToSell(), false);
}

TEST(SlowStochastic, BuySignal) {
  StrategyFacade facade;

  auto stoch = facade.getStochasticOscillatorStrategy();
  stoch->setTopLevel(80);
  stoch->setBottomLevel(20);
  stoch->createLines(slowStochasticToBuy, common::StochasticOscillatorType::Slow, 14);

  EXPECT_TRUE(stoch->isNeedToBuy());
  EXPECT_FALSE(stoch->isNeedToSell());
}

TEST(SlowStochastic, BuySignal_False_Without_One_candle) {
  StrategyFacade facade;

  auto stoch = facade.getStochasticOscillatorStrategy();
  stoch->setTopLevel(80);
  stoch->setBottomLevel(20);

  auto stochasticToBuyWithoutOneCandle = slowStochasticToBuy;
  stochasticToBuyWithoutOneCandle.erase(stochasticToBuyWithoutOneCandle.end() - 1);

  stoch->createLines(stochasticToBuyWithoutOneCandle, common::StochasticOscillatorType::Slow, 14);

  EXPECT_FALSE(stoch->isNeedToBuy());
  EXPECT_FALSE(stoch->isNeedToSell());

  stoch->createLines(slowStochasticToBuy, common::StochasticOscillatorType::Slow, 14);

  EXPECT_TRUE(stoch->isNeedToBuy());
  EXPECT_FALSE(stoch->isNeedToSell());
}

TEST(SlowStochastic, SellSignal) {
  StrategyFacade facade;

  auto stoch = facade.getStochasticOscillatorStrategy();
  stoch->setTopLevel(80);
  stoch->setBottomLevel(20);
  stoch->createLines(slowStochasticToSell, common::StochasticOscillatorType::Slow, 14);

  EXPECT_TRUE(stoch->isNeedToSell());
  EXPECT_FALSE(stoch->isNeedToBuy());
}

TEST(SlowStochastic, DoubleSellSignal) {
  StrategyFacade facade;

  auto stoch = facade.getStochasticOscillatorStrategy();
  stoch->setTopLevel(80);
  stoch->setBottomLevel(20);
  stoch->createLines(slowStochasticToSell, common::StochasticOscillatorType::Slow, 14);

  EXPECT_TRUE(stoch->isNeedToSell());
  EXPECT_FALSE(stoch->isNeedToBuy());

  auto doubleSellSignal = slowStochasticToSell;
  doubleSellSignal.emplace_back(additionalToSellPoint);

  stoch->createLines(slowStochasticToSell, common::StochasticOscillatorType::Slow, 14, 3, 3, 3,
                     stoch->getLastBuyCrossingPoint(), stoch->getLastSellCrossingPoint());

  EXPECT_FALSE(stoch->isNeedToSell());
  EXPECT_FALSE(stoch->isNeedToBuy());
}

TEST(FullStochastic, BuySignal) {
  StrategyFacade facade;

  auto stoch = facade.getStochasticOscillatorStrategy();
  stoch->setTopLevel(80);
  stoch->setBottomLevel(20);
  stoch->createLines(slowStochasticToBuy, common::StochasticOscillatorType::Full, 14, 3, 3);

  EXPECT_TRUE(stoch->isNeedToBuy());
  EXPECT_FALSE(stoch->isNeedToSell());
}

TEST(FullStochastic, BuySignal_False_Without_One_candle) {
  StrategyFacade facade;

  auto stoch = facade.getStochasticOscillatorStrategy();
  stoch->setTopLevel(80);
  stoch->setBottomLevel(20);

  auto stochasticToBuyWithoutOneCandle = slowStochasticToBuy;
  stochasticToBuyWithoutOneCandle.erase(stochasticToBuyWithoutOneCandle.end() - 1);

  stoch->createLines(stochasticToBuyWithoutOneCandle, common::StochasticOscillatorType::Full, 14, 3,
                     3);

  EXPECT_FALSE(stoch->isNeedToBuy());
  EXPECT_FALSE(stoch->isNeedToSell());

  stoch->createLines(slowStochasticToBuy, common::StochasticOscillatorType::Full, 14, 3, 3);

  EXPECT_TRUE(stoch->isNeedToBuy());
  EXPECT_FALSE(stoch->isNeedToSell());
}

TEST(SlowStochastic, SlowStochastic_EmptyMarketData_Test) {
  StrategyFacade facade;

  auto stoch = facade.getStochasticOscillatorStrategy();
  stoch->setTopLevel(80);
  stoch->setBottomLevel(20);

  EXPECT_THROW(
      stoch->createLines(stochasticEmptyMarketData, common::StochasticOscillatorType::Slow, 14),
      common::exceptions::StrategyException);
}

TEST(SlowStochastic, SlowStochastic_PeriodBiggerThanMarketDataSize_Test) {
  StrategyFacade facade;

  auto stoch = facade.getStochasticOscillatorStrategy();
  stoch->setTopLevel(80);
  stoch->setBottomLevel(20);

  EXPECT_THROW(
      stoch->createLines(stochasticFiveCandles, common::StochasticOscillatorType::Slow, 14),
      common::exceptions::StrategyException);
}

}  // namespace unit_test
}  // namespace strategies
}  // namespace auto_trader
