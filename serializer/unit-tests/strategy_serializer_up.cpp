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

#include <fstream>

#include "model/include/settings/strategies_settings/bollinger_bands_settings.h"
#include "model/include/settings/strategies_settings/custom_strategy_settings.h"
#include "model/include/settings/strategies_settings/ema_settings.h"
#include "model/include/settings/strategies_settings/ma_crossing_settings.h"
#include "model/include/settings/strategies_settings/rsi_settings.h"
#include "model/include/settings/strategies_settings/sma_settings.h"
#include "model/include/settings/strategies_settings/strategy_settings.h"
#include "strategy_serializer_ut.h"

namespace auto_trader {
namespace serializer {
namespace unit_tests {

/**
 *      Test Plan
 *
 * 1. Bollinger bands strategy settings.
 * 2. RSI strategy settings.
 * 3. SMA settings.
 * 4. EMA settings.
 * 5. MA crossings settings.
 * 6. Ping Pong settings.
 * 7. Custom RSI + BB settings.
 * 8. Custom RSI + BB + EMA settings.
 * 9. Custom MACrossing + PingPong settings.
 *
 **/

TEST_F(StrategySerializerUTFixture, BollingerBandsSettings) {
  auto bbSettingsToFile = std::make_unique<model::BollingerBandsSettings>();
  bbSettingsToFile->name_ = "bollinger_bands";
  bbSettingsToFile->bbInputType_ = common::BollingerInputType::closePosition_;
  bbSettingsToFile->period_ = 2;
  bbSettingsToFile->standardDeviations_ = 4;
  bbSettingsToFile->tickInterval_ = common::TickInterval::FIVE_MIN;
  bbSettingsToFile->strategiesType_ = common::StrategiesType::BOLLINGER_BANDS;

  const std::string filename = "bb_settings.json";
  std::ofstream outputStream(filename);
  getStrategySerializer().serialize(*bbSettingsToFile, outputStream);
  outputStream.close();

  std::ifstream inputStream(filename);
  auto restoredSettings = getStrategySerializer().deserialize(inputStream);

  EXPECT_EQ(restoredSettings->strategiesType_, common::StrategiesType::BOLLINGER_BANDS);

  const auto &bbSettingsFromFile =
      dynamic_cast<const model::BollingerBandsSettings &>(*restoredSettings);

  EXPECT_EQ(bbSettingsFromFile.name_, "bollinger_bands");
  EXPECT_EQ(bbSettingsFromFile.tickInterval_, common::TickInterval::FIVE_MIN);
  EXPECT_EQ(bbSettingsFromFile.period_, 2);
  EXPECT_EQ(bbSettingsFromFile.standardDeviations_, 4);
  EXPECT_EQ(bbSettingsFromFile.bbInputType_, common::BollingerInputType::closePosition_);

  remove(filename.c_str());
}

TEST_F(StrategySerializerUTFixture, RsiSettings) {
  auto rsiSettings = std::make_unique<model::RsiSettings>();
  rsiSettings->period_ = 2;
  rsiSettings->tickInterval_ = common::TickInterval::FIVE_MIN;
  rsiSettings->strategiesType_ = common::StrategiesType::RSI;
  rsiSettings->name_ = "rsi_settings";
  rsiSettings->bottomLevel_ = 2;
  rsiSettings->topLevel_ = 3;
  rsiSettings->crossingInterval_ = 4;

  const std::string filename = "rsi_settings.json";

  std::ofstream outputStream(filename);
  getStrategySerializer().serialize(*rsiSettings, outputStream);
  outputStream.close();

  std::ifstream inputStream(filename);
  auto restoredSettings = getStrategySerializer().deserialize(inputStream);

  EXPECT_EQ(restoredSettings->strategiesType_, common::StrategiesType::RSI);

  const auto &rsiSettingsFromFile = dynamic_cast<const model::RsiSettings &>(*restoredSettings);

  EXPECT_EQ(rsiSettingsFromFile.strategiesType_, common::StrategiesType::RSI);
  EXPECT_EQ(rsiSettingsFromFile.name_, "rsi_settings");
  EXPECT_EQ(rsiSettingsFromFile.period_, 2);
  EXPECT_EQ(rsiSettingsFromFile.tickInterval_, common::TickInterval::FIVE_MIN);
  EXPECT_EQ(rsiSettingsFromFile.bottomLevel_, 2);
  EXPECT_EQ(rsiSettingsFromFile.topLevel_, 3);
  EXPECT_EQ(rsiSettingsFromFile.crossingInterval_, 4);

  remove(filename.c_str());
}

TEST_F(StrategySerializerUTFixture, SmaSettings) {
  auto smaSettings = std::make_unique<model::SmaSettings>();
  smaSettings->name_ = "sma_settings";
  smaSettings->tickInterval_ = common::TickInterval::FIVE_MIN;
  smaSettings->period_ = 2;
  smaSettings->crossingInterval_ = 4;
  smaSettings->strategiesType_ = common::StrategiesType::SMA;

  const std::string filename = "sma_settings.json";
  std::ofstream outputStream(filename);
  getStrategySerializer().serialize(*smaSettings, outputStream);
  outputStream.close();

  std::ifstream inputStream(filename);
  auto restoredSettings = getStrategySerializer().deserialize(inputStream);

  EXPECT_EQ(restoredSettings->strategiesType_, common::StrategiesType::SMA);

  const auto &smaSettingsFromFile = dynamic_cast<const model::SmaSettings &>(*restoredSettings);

  EXPECT_EQ(smaSettingsFromFile.strategiesType_, common::StrategiesType::SMA);
  EXPECT_EQ(smaSettingsFromFile.name_, "sma_settings");
  EXPECT_EQ(smaSettingsFromFile.period_, 2);
  EXPECT_EQ(smaSettingsFromFile.crossingInterval_, 4);
  EXPECT_EQ(smaSettingsFromFile.tickInterval_, common::TickInterval::FIVE_MIN);

  remove(filename.c_str());
}

TEST_F(StrategySerializerUTFixture, EmaSettings) {
  auto emaSettings = std::make_unique<model::EmaSettings>();
  emaSettings->name_ = "ema_settings";
  emaSettings->tickInterval_ = common::TickInterval::FIVE_MIN;
  emaSettings->period_ = 4;
  emaSettings->crossingInterval_ = 5;
  emaSettings->strategiesType_ = common::StrategiesType::EMA;

  const std::string filename = "ema_settings.json";

  std::ofstream outputStream(filename);
  getStrategySerializer().serialize(*emaSettings, outputStream);
  outputStream.close();

  std::ifstream inputStream(filename);
  auto restoredSettings = getStrategySerializer().deserialize(inputStream);

  EXPECT_EQ(restoredSettings->strategiesType_, common::StrategiesType::EMA);

  const auto &emaSettingsFromFile = dynamic_cast<const model::EmaSettings &>(*restoredSettings);

  EXPECT_EQ(emaSettingsFromFile.name_, "ema_settings");
  EXPECT_EQ(emaSettingsFromFile.period_, 4);
  EXPECT_EQ(emaSettingsFromFile.strategiesType_, common::StrategiesType::EMA);
  EXPECT_EQ(emaSettingsFromFile.tickInterval_, common::TickInterval::FIVE_MIN);
  EXPECT_EQ(emaSettingsFromFile.crossingInterval_, 5);
}

TEST_F(StrategySerializerUTFixture, MaCrossingSettings) {
  auto maCrossingSettings = std::make_unique<model::MovingAveragesCrossingSettings>();
  maCrossingSettings->name_ = "moving_average_crossing_settings";
  maCrossingSettings->tickInterval_ = common::TickInterval::FIVE_MIN;
  maCrossingSettings->strategiesType_ = common::StrategiesType::MA_CROSSING;
  maCrossingSettings->movingAverageType_ = common::MovingAverageType::SIMPLE;

  maCrossingSettings->smallerPeriod_ = 1;
  maCrossingSettings->biggerPeriod_ = 3;

  const std::string filename = "moving_average_crossing_settings.json";

  std::ofstream outputStream(filename);
  getStrategySerializer().serialize(*maCrossingSettings, outputStream);
  outputStream.close();

  std::ifstream inputStream(filename);
  auto restoredSettings = getStrategySerializer().deserialize(inputStream);

  EXPECT_EQ(restoredSettings->strategiesType_, common::StrategiesType::MA_CROSSING);

  const auto &maCrossingSettingsFromFile =
      dynamic_cast<const model::MovingAveragesCrossingSettings &>(*restoredSettings);

  EXPECT_EQ(maCrossingSettingsFromFile.name_, "moving_average_crossing_settings");
  EXPECT_EQ(maCrossingSettingsFromFile.tickInterval_, common::TickInterval::FIVE_MIN);
  EXPECT_EQ(maCrossingSettingsFromFile.movingAverageType_, common::MovingAverageType::SIMPLE);
  EXPECT_EQ(maCrossingSettingsFromFile.strategiesType_, common::StrategiesType::MA_CROSSING);
  EXPECT_EQ(maCrossingSettingsFromFile.smallerPeriod_, 1);
  EXPECT_EQ(maCrossingSettingsFromFile.biggerPeriod_, 3);

  remove(filename.c_str());
}

TEST_F(StrategySerializerUTFixture, CustomRsi_BB_Settings) {
  auto bbSettingsToFile = std::make_unique<model::BollingerBandsSettings>();
  bbSettingsToFile->name_ = "bollinger_bands";
  bbSettingsToFile->strategiesType_ = common::StrategiesType::BOLLINGER_BANDS;
  bbSettingsToFile->bbInputType_ = common::BollingerInputType::closePosition_;
  bbSettingsToFile->period_ = 2;
  bbSettingsToFile->standardDeviations_ = 4;
  bbSettingsToFile->tickInterval_ = common::TickInterval::FIVE_MIN;

  auto rsiSettings = std::make_unique<model::RsiSettings>();
  rsiSettings->period_ = 2;
  rsiSettings->tickInterval_ = common::TickInterval::FIVE_MIN;
  rsiSettings->strategiesType_ = common::StrategiesType::RSI;
  rsiSettings->name_ = "rsi_settings";
  rsiSettings->bottomLevel_ = 2;
  rsiSettings->topLevel_ = 3;
  rsiSettings->crossingInterval_ = 4;

  auto customSettings = std::make_unique<model::CustomStrategySettings>();
  customSettings->name_ = "bb_rsi_settings";
  customSettings->strategiesType_ = common::StrategiesType::CUSTOM;
  customSettings->strategies_.push_back(std::move(bbSettingsToFile));
  customSettings->strategies_.push_back(std::move(rsiSettings));

  const std::string filename = "custom_settings.json";

  std::ofstream outputStream(filename);
  getStrategySerializer().serialize(*customSettings, outputStream);
  outputStream.close();

  std::ifstream inputStream(filename);
  auto restoredSettings = getStrategySerializer().deserialize(inputStream);

  EXPECT_EQ(restoredSettings->strategiesType_, common::StrategiesType::CUSTOM);

  const auto &customSettingsFromFile =
      dynamic_cast<const model::CustomStrategySettings &>(*restoredSettings);

  EXPECT_EQ(customSettingsFromFile.name_, "bb_rsi_settings");
  EXPECT_EQ(customSettingsFromFile.strategiesType_, common::StrategiesType::CUSTOM);
  EXPECT_EQ(customSettingsFromFile.getStrategiesCount(), 2);

  auto &customFromFileOne = customSettingsFromFile.getStrategy(0);
  EXPECT_EQ(customFromFileOne->strategiesType_, common::StrategiesType::BOLLINGER_BANDS);

  const auto &bbSettingsFromFile =
      dynamic_cast<const model::BollingerBandsSettings &>(*customFromFileOne);
  EXPECT_EQ(bbSettingsFromFile.name_, "bollinger_bands");
  EXPECT_EQ(bbSettingsFromFile.period_, 2);
  EXPECT_EQ(bbSettingsFromFile.standardDeviations_, 4);
  EXPECT_EQ(bbSettingsFromFile.tickInterval_, common::TickInterval::FIVE_MIN);
  EXPECT_EQ(bbSettingsFromFile.bbInputType_, common::BollingerInputType::closePosition_);

  auto &customFromFileTwo = customSettingsFromFile.getStrategy(1);
  EXPECT_EQ(customFromFileTwo->strategiesType_, common::StrategiesType::RSI);
  const auto &rsiSettingsFromFile = dynamic_cast<const model::RsiSettings &>(*customFromFileTwo);

  EXPECT_EQ(rsiSettingsFromFile.name_, "rsi_settings");
  EXPECT_EQ(rsiSettingsFromFile.period_, 2);
  EXPECT_EQ(rsiSettingsFromFile.tickInterval_, common::TickInterval::FIVE_MIN);
  EXPECT_EQ(rsiSettingsFromFile.strategiesType_, common::StrategiesType::RSI);
  EXPECT_EQ(rsiSettingsFromFile.bottomLevel_, 2);
  EXPECT_EQ(rsiSettingsFromFile.topLevel_, 3);
  EXPECT_EQ(rsiSettingsFromFile.crossingInterval_, 4);

  remove(filename.c_str());
}

TEST_F(StrategySerializerUTFixture, Custom_Rsi_BB_EMA) {
  auto bbSettingsToFile = std::make_unique<model::BollingerBandsSettings>();
  bbSettingsToFile->name_ = "bollinger_bands";
  bbSettingsToFile->bbInputType_ = common::BollingerInputType::closePosition_;
  bbSettingsToFile->period_ = 2;
  bbSettingsToFile->standardDeviations_ = 4;
  bbSettingsToFile->tickInterval_ = common::TickInterval::FIVE_MIN;
  bbSettingsToFile->strategiesType_ = common::StrategiesType::BOLLINGER_BANDS;

  auto rsiSettings = std::make_unique<model::RsiSettings>();
  rsiSettings->period_ = 2;
  rsiSettings->tickInterval_ = common::TickInterval::FIVE_MIN;
  rsiSettings->strategiesType_ = common::StrategiesType::RSI;
  rsiSettings->name_ = "rsi_settings";
  rsiSettings->bottomLevel_ = 2;
  rsiSettings->topLevel_ = 3;
  rsiSettings->crossingInterval_ = 4;

  auto emaSettings = std::make_unique<model::EmaSettings>();
  emaSettings->strategiesType_ = common::StrategiesType::EMA;
  emaSettings->name_ = "ema_settings";
  emaSettings->tickInterval_ = common::TickInterval::ONE_HOUR;
  emaSettings->period_ = 4;
  emaSettings->crossingInterval_ = 3;

  auto customSettings = std::make_unique<model::CustomStrategySettings>();
  customSettings->name_ = "bb_rsi_ema_settings";
  customSettings->strategiesType_ = common::StrategiesType::CUSTOM;
  customSettings->strategies_.push_back(std::move(bbSettingsToFile));
  customSettings->strategies_.push_back(std::move(rsiSettings));
  customSettings->strategies_.push_back(std::move(emaSettings));

  const std::string filename = "custom_settings.json";

  std::ofstream outputStream(filename);
  getStrategySerializer().serialize(*customSettings, outputStream);
  outputStream.close();

  std::ifstream inputStream(filename);
  auto restoredSettings = getStrategySerializer().deserialize(inputStream);

  EXPECT_EQ(restoredSettings->strategiesType_, common::StrategiesType::CUSTOM);

  const auto &customSettingsFromFile =
      dynamic_cast<const model::CustomStrategySettings &>(*restoredSettings);

  EXPECT_EQ(customSettingsFromFile.name_, "bb_rsi_ema_settings");
  EXPECT_EQ(customSettingsFromFile.strategiesType_, common::StrategiesType::CUSTOM);
  EXPECT_EQ(customSettingsFromFile.getStrategiesCount(), 3);

  auto &customFromFileOne = customSettingsFromFile.getStrategy(0);
  EXPECT_EQ(customFromFileOne->strategiesType_, common::StrategiesType::BOLLINGER_BANDS);

  const auto &bbSettingsFromFile =
      dynamic_cast<const model::BollingerBandsSettings &>(*customFromFileOne);
  EXPECT_EQ(bbSettingsFromFile.name_, "bollinger_bands");
  EXPECT_EQ(bbSettingsFromFile.period_, 2);
  EXPECT_EQ(bbSettingsFromFile.standardDeviations_, 4);
  EXPECT_EQ(bbSettingsFromFile.tickInterval_, common::TickInterval::FIVE_MIN);
  EXPECT_EQ(bbSettingsFromFile.bbInputType_, common::BollingerInputType::closePosition_);

  const auto &customFromFileTwo = customSettingsFromFile.getStrategy(1);
  EXPECT_EQ(customFromFileTwo->strategiesType_, common::StrategiesType::RSI);
  const auto &rsiSettingsFromFile = dynamic_cast<const model::RsiSettings &>(*customFromFileTwo);

  EXPECT_EQ(rsiSettingsFromFile.name_, "rsi_settings");
  EXPECT_EQ(rsiSettingsFromFile.period_, 2);
  EXPECT_EQ(rsiSettingsFromFile.tickInterval_, common::TickInterval::FIVE_MIN);
  EXPECT_EQ(rsiSettingsFromFile.strategiesType_, common::StrategiesType::RSI);
  EXPECT_EQ(rsiSettingsFromFile.bottomLevel_, 2);
  EXPECT_EQ(rsiSettingsFromFile.topLevel_, 3);
  EXPECT_EQ(rsiSettingsFromFile.crossingInterval_, 4);

  const auto &customFromFileThree = customSettingsFromFile.getStrategy(2);
  EXPECT_EQ(customFromFileThree->strategiesType_, common::StrategiesType::EMA);
  const auto &emaSettingsFromFile = dynamic_cast<const model::EmaSettings &>(*customFromFileThree);

  EXPECT_EQ(emaSettingsFromFile.name_, "ema_settings");
  EXPECT_EQ(emaSettingsFromFile.strategiesType_, common::StrategiesType::EMA);
  EXPECT_EQ(emaSettingsFromFile.crossingInterval_, 3);
  EXPECT_EQ(emaSettingsFromFile.period_, 4);
  EXPECT_EQ(emaSettingsFromFile.tickInterval_, common::TickInterval::ONE_HOUR);

  remove(filename.c_str());
}

TEST_F(StrategySerializerUTFixture, MaCrossing_PingPong) {
  auto maCrossing = std::make_unique<model::MovingAveragesCrossingSettings>();
  maCrossing->name_ = "ma_crossing_settings";
  maCrossing->tickInterval_ = common::TickInterval::FIVE_MIN;
  maCrossing->strategiesType_ = common::StrategiesType::MA_CROSSING;
  maCrossing->movingAverageType_ = common::MovingAverageType::SIMPLE;
  maCrossing->smallerPeriod_ = 1;
  maCrossing->biggerPeriod_ = 3;

  auto customSettings = std::make_unique<model::CustomStrategySettings>();
  customSettings->name_ = "ma_crossing_ping_pong_settings";
  customSettings->strategiesType_ = common::StrategiesType::CUSTOM;
  customSettings->strategies_.push_back(std::move(maCrossing));

  const std::string filename = "custom_settings.json";

  std::ofstream outputStream(filename);
  getStrategySerializer().serialize(*customSettings, outputStream);
  outputStream.close();

  std::ifstream inputStream(filename);
  auto restoredSettings = getStrategySerializer().deserialize(inputStream);

  EXPECT_EQ(restoredSettings->strategiesType_, common::StrategiesType::CUSTOM);

  const auto &customSettingsFromFile =
      dynamic_cast<const model::CustomStrategySettings &>(*restoredSettings);

  EXPECT_EQ(customSettingsFromFile.name_, "ma_crossing_ping_pong_settings");
  EXPECT_EQ(customSettingsFromFile.strategiesType_, common::StrategiesType::CUSTOM);
  EXPECT_EQ(customSettingsFromFile.getStrategiesCount(), 1);
  auto &customFromFileOne = customSettingsFromFile.getStrategy(0);
  EXPECT_EQ(customFromFileOne->strategiesType_, common::StrategiesType::MA_CROSSING);

  const auto &maCrossingFromFileSetting =
      dynamic_cast<const model::MovingAveragesCrossingSettings &>(*customFromFileOne);

  EXPECT_EQ(maCrossingFromFileSetting.name_, "ma_crossing_settings");
  EXPECT_EQ(maCrossingFromFileSetting.strategiesType_, common::StrategiesType::MA_CROSSING);
  EXPECT_EQ(maCrossingFromFileSetting.movingAverageType_, common::MovingAverageType::SIMPLE);
  EXPECT_EQ(maCrossingFromFileSetting.tickInterval_, common::TickInterval::FIVE_MIN);
  EXPECT_EQ(maCrossingFromFileSetting.smallerPeriod_, 1);
  EXPECT_EQ(maCrossingFromFileSetting.biggerPeriod_, 3);

  remove(filename.c_str());
}

}  // namespace unit_tests
}  // namespace serializer
}  // namespace auto_trader