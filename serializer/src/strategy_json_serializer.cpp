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

#include "include/strategy_json_serializer.h"

#include "model/include/settings/strategies_settings/bollinger_bands_advanced_settings.h"
#include "model/include/settings/strategies_settings/bollinger_bands_settings.h"
#include "model/include/settings/strategies_settings/custom_strategy_settings.h"
#include "model/include/settings/strategies_settings/ema_settings.h"
#include "model/include/settings/strategies_settings/ma_crossing_settings.h"
#include "model/include/settings/strategies_settings/rsi_settings.h"
#include "model/include/settings/strategies_settings/sma_settings.h"
#include "model/include/settings/strategies_settings/stochastic_oscillator_settings.h"
#include "model/include/settings/strategies_settings/strategy_settings.h"

namespace auto_trader {
namespace serializer {

void StrategyJSONSerializer::serialize(const model::StrategySettings &strategySettings,
                                       std::ostream &stream) {
  printHandler_ = std::make_unique<Poco::JSON::PrintHandler>(stream, 1);
  printHandler_->startObject();

  printHandler_->key("strategy_name");
  printHandler_->value(strategySettings.name_);

  printHandler_->key("description");
  printHandler_->value(strategySettings.description_);

  printHandler_->key("strategy_type");
  printHandler_->value(static_cast<int>(strategySettings.strategiesType_));

  printHandler_->key("tick_interval");
  printHandler_->value(static_cast<int>(strategySettings.tickInterval_));

  strategySettings.accept(*this);

  printHandler_->endObject();
}

std::unique_ptr<model::StrategySettings> StrategyJSONSerializer::deserialize(std::istream &stream) {
  Poco::JSON::Parser parser;
  auto jsonObject = parser.parse(stream);
  auto object = jsonObject.extract<Poco::JSON::Object::Ptr>();

  const std::string name = object->getValue<std::string>("strategy_name");
  const std::string description = object->getValue<std::string>("description");

  common::TickInterval::Enum interval =
      static_cast<common::TickInterval::Enum>(object->getValue<int>("tick_interval"));
  common::StrategiesType type =
      static_cast<common::StrategiesType>(object->getValue<int>("strategy_type"));
  auto strategy = deserializeStrategy(object, type);
  strategy->name_ = name;
  strategy->tickInterval_ = interval;
  strategy->strategiesType_ = type;

  return std::move(strategy);
}

void StrategyJSONSerializer::visit(const model::BollingerBandsSettings &bollingerBandsSettings) {
  printHandler_->key("bollinger_bands_settings");

  printHandler_->startObject();

  printHandler_->key("strategy_name");
  printHandler_->value(bollingerBandsSettings.name_);

  printHandler_->key("strategy_type");
  printHandler_->value(static_cast<int>(bollingerBandsSettings.strategiesType_));

  printHandler_->key("tick_interval");
  printHandler_->value(bollingerBandsSettings.tickInterval_);

  printHandler_->key("bb_input_type");
  printHandler_->value(static_cast<int>(bollingerBandsSettings.bbInputType_));

  printHandler_->key("standard_deviations");
  printHandler_->value(bollingerBandsSettings.standardDeviations_);

  printHandler_->key("period");
  printHandler_->value(bollingerBandsSettings.period_);

  printHandler_->endObject();
}

void StrategyJSONSerializer::visit(
    const model::BollingerBandsAdvancedSettings &bollingerBandsAdvancedSettings) {
  printHandler_->key("bollinger_bands_advanced_settings");

  printHandler_->startObject();

  printHandler_->key("strategy_name");
  printHandler_->value(bollingerBandsAdvancedSettings.name_);

  printHandler_->key("strategy_type");
  printHandler_->value(static_cast<int>(bollingerBandsAdvancedSettings.strategiesType_));

  printHandler_->key("tick_interval");
  printHandler_->value(bollingerBandsAdvancedSettings.tickInterval_);

  printHandler_->key("bb_input_type");
  printHandler_->value(static_cast<int>(bollingerBandsAdvancedSettings.bbInputType_));

  printHandler_->key("standard_deviations");
  printHandler_->value(bollingerBandsAdvancedSettings.standardDeviations_);

  printHandler_->key("period");
  printHandler_->value(bollingerBandsAdvancedSettings.period_);

  printHandler_->key("top_line_percentage");
  printHandler_->value(bollingerBandsAdvancedSettings.topLinePercentage_);

  printHandler_->key("bottom_line_percentage");
  printHandler_->value(bollingerBandsAdvancedSettings.bottomLinePercentage_);

  printHandler_->endObject();
}

void StrategyJSONSerializer::visit(
    const model::StochasticOscillatorSettings &stochasticOscillatorSettings) {
  printHandler_->key("stochastic_oscillator_settings");

  printHandler_->startObject();

  printHandler_->key("strategy_name");
  printHandler_->value(stochasticOscillatorSettings.name_);

  printHandler_->key("strategy_type");
  printHandler_->value(static_cast<int>(stochasticOscillatorSettings.strategiesType_));

  printHandler_->key("tick_interval");
  printHandler_->value(stochasticOscillatorSettings.tickInterval_);

  printHandler_->key("stochastic_oscillator_type");
  printHandler_->value(static_cast<int>(stochasticOscillatorSettings.stochasticType_));

  printHandler_->key("period_for_classic_line");
  printHandler_->value(stochasticOscillatorSettings.periodsForClassicLine_);

  printHandler_->key("smooth_fast_period");
  printHandler_->value(stochasticOscillatorSettings.smoothFastPeriod_);

  printHandler_->key("smooth_slow_period");
  printHandler_->value(stochasticOscillatorSettings.smoothSlowPeriod_);

  printHandler_->key("candles_after_crossings");
  printHandler_->value(stochasticOscillatorSettings.crossingInterval_);

  printHandler_->key("top_level");
  printHandler_->value(stochasticOscillatorSettings.topLevel);

  printHandler_->key("bottom_level");
  printHandler_->value(stochasticOscillatorSettings.bottomLevel);

  printHandler_->endObject();
}

void StrategyJSONSerializer::visit(
    const model::MovingAveragesCrossingSettings &movingAveragesCrossingSettings) {
  printHandler_->key("moving_averages_crossings_settings");

  printHandler_->startObject();

  printHandler_->key("strategy_name");
  printHandler_->value(movingAveragesCrossingSettings.name_);

  printHandler_->key("strategy_type");
  printHandler_->value(static_cast<int>(movingAveragesCrossingSettings.strategiesType_));

  printHandler_->key("tick_interval");
  printHandler_->value(movingAveragesCrossingSettings.tickInterval_);

  printHandler_->key("moving_average_type");
  printHandler_->value(static_cast<int>(movingAveragesCrossingSettings.movingAverageType_));

  printHandler_->key("smaller_period");
  printHandler_->value(movingAveragesCrossingSettings.smallerPeriod_);

  printHandler_->key("bigger_period");
  printHandler_->value(movingAveragesCrossingSettings.biggerPeriod_);

  printHandler_->key("crossing_interval");
  printHandler_->value(movingAveragesCrossingSettings.crossingInterval_);

  printHandler_->endObject();
}

void StrategyJSONSerializer::visit(const model::RsiSettings &rsiSettings) {
  printHandler_->key("rsi_settings");
  printHandler_->startObject();

  printHandler_->key("strategy_name");
  printHandler_->value(rsiSettings.name_);

  printHandler_->key("strategy_type");
  printHandler_->value(static_cast<int>(rsiSettings.strategiesType_));

  printHandler_->key("tick_interval");
  printHandler_->value(rsiSettings.tickInterval_);

  printHandler_->key("period");
  printHandler_->value(rsiSettings.period_);

  printHandler_->key("candles_after_crossing");
  printHandler_->value(rsiSettings.crossingInterval_);

  printHandler_->key("top_level");
  printHandler_->value(rsiSettings.topLevel_);

  printHandler_->key("bottom_level");
  printHandler_->value(rsiSettings.bottomLevel_);

  printHandler_->endObject();
}

void StrategyJSONSerializer::visit(const model::SmaSettings &smaSettings) {
  printHandler_->key("sma_settings");
  printHandler_->startObject();

  printHandler_->key("strategy_name");
  printHandler_->value(smaSettings.name_);

  printHandler_->key("strategy_type");
  printHandler_->value(static_cast<int>(smaSettings.strategiesType_));

  printHandler_->key("tick_interval");
  printHandler_->value(smaSettings.tickInterval_);

  printHandler_->key("period");
  printHandler_->value(smaSettings.period_);

  printHandler_->key("candles_after_crossing");
  printHandler_->value(smaSettings.crossingInterval_);

  printHandler_->endObject();
}

void StrategyJSONSerializer::visit(const model::EmaSettings &emaSettings) {
  printHandler_->key("ema_settings");
  printHandler_->startObject();

  printHandler_->key("strategy_name");
  printHandler_->value(emaSettings.name_);

  printHandler_->key("strategy_type");
  printHandler_->value(static_cast<int>(emaSettings.strategiesType_));

  printHandler_->key("tick_interval");
  printHandler_->value(emaSettings.tickInterval_);

  printHandler_->key("period");
  printHandler_->value(emaSettings.period_);

  printHandler_->key("candles_after_crossing");
  printHandler_->value(emaSettings.crossingInterval_);

  printHandler_->endObject();
}

void StrategyJSONSerializer::visit(const model::CustomStrategySettings &customStrategySettings) {
  printHandler_->key("custom_strategy");
  printHandler_->startObject();

  printHandler_->key("strategy_type");
  printHandler_->value(static_cast<int>(customStrategySettings.strategiesType_));

  printHandler_->key("strategy_name");
  printHandler_->value(customStrategySettings.name_);

  size_t strategiesCount = customStrategySettings.strategies_.size();

  printHandler_->key("strategies");
  printHandler_->startArray();
  for (int index = 0; index < strategiesCount; ++index) {
    printHandler_->startObject();
    printHandler_->key("strategy_type");
    auto type = static_cast<int>(customStrategySettings.strategies_[index]->strategiesType_);
    printHandler_->value(type);
    customStrategySettings.strategies_[index]->accept(*this);
    printHandler_->endObject();
  }

  printHandler_->endArray();
  printHandler_->endObject();
}

void StrategyJSONSerializer::deserializeBollingerBandsSettings(
    Poco::JSON::Object::Ptr jsonObject, model::BollingerBandsSettings &bollingerBandsSettings) {
  auto bbObject = jsonObject->getObject("bollinger_bands_settings");
  auto name = bbObject->getValue<std::string>("strategy_name");
  bollingerBandsSettings.name_ = name;

  auto type = bbObject->getValue<int>("strategy_type");
  bollingerBandsSettings.strategiesType_ = static_cast<common::StrategiesType>(type);

  auto tickInterval = bbObject->getValue<int>("tick_interval");
  bollingerBandsSettings.tickInterval_ = static_cast<common::TickInterval::Enum>(tickInterval);

  auto bbInputType = bbObject->getValue<int>("bb_input_type");
  bollingerBandsSettings.bbInputType_ = static_cast<common::BollingerInputType>(bbInputType);

  auto standardDeviation = bbObject->getValue<unsigned int>("standard_deviations");
  bollingerBandsSettings.standardDeviations_ = standardDeviation;

  auto period = bbObject->getValue<unsigned int>("period");
  bollingerBandsSettings.period_ = period;
}

void StrategyJSONSerializer::deserializeBollingerBandsAdvancedSettings(
    Poco::JSON::Object::Ptr jsonObject,
    model::BollingerBandsAdvancedSettings &bollingerBandsAdvancedSettings) {
  auto bbAdvancedObject = jsonObject->getObject("bollinger_bands_advanced_settings");
  auto name = bbAdvancedObject->getValue<std::string>("strategy_name");
  bollingerBandsAdvancedSettings.name_ = name;

  auto type = bbAdvancedObject->getValue<int>("strategy_type");
  bollingerBandsAdvancedSettings.strategiesType_ = static_cast<common::StrategiesType>(type);

  auto tickInterval = bbAdvancedObject->getValue<int>("tick_interval");
  bollingerBandsAdvancedSettings.tickInterval_ =
      static_cast<common::TickInterval::Enum>(tickInterval);

  auto bbInputType = bbAdvancedObject->getValue<int>("bb_input_type");
  bollingerBandsAdvancedSettings.bbInputType_ =
      static_cast<common::BollingerInputType>(bbInputType);

  auto standardDeviation = bbAdvancedObject->getValue<unsigned int>("standard_deviations");
  bollingerBandsAdvancedSettings.standardDeviations_ = standardDeviation;

  auto period = bbAdvancedObject->getValue<unsigned int>("period");
  bollingerBandsAdvancedSettings.period_ = period;

  auto topLinePercent = bbAdvancedObject->getValue<int>("top_line_percentage");
  bollingerBandsAdvancedSettings.topLinePercentage_ = topLinePercent;

  auto bottomLinePercent = bbAdvancedObject->getValue<int>("bottom_line_percentage");
  bollingerBandsAdvancedSettings.bottomLinePercentage_ = bottomLinePercent;
}

void StrategyJSONSerializer::deserializeStochasticOscillatorSettings(
    Poco::JSON::Object::Ptr jsonObject,
    model::StochasticOscillatorSettings &stochasticOscillatorSettings) {
  auto stochasticOscillatorObject = jsonObject->getObject("stochastic_oscillator_settings");
  auto name = stochasticOscillatorObject->getValue<std::string>("strategy_name");
  stochasticOscillatorSettings.name_ = name;

  auto type = stochasticOscillatorObject->getValue<int>("strategy_type");
  stochasticOscillatorSettings.strategiesType_ = static_cast<common::StrategiesType>(type);

  auto tickInterval = stochasticOscillatorObject->getValue<int>("tick_interval");
  stochasticOscillatorSettings.tickInterval_ =
      static_cast<common::TickInterval::Enum>(tickInterval);

  auto stochasticOscillatorType =
      stochasticOscillatorObject->getValue<int>("stochastic_oscillator_type");
  stochasticOscillatorSettings.stochasticType_ =
      static_cast<common::StochasticOscillatorType>(stochasticOscillatorType);

  auto periodForClassicLine = stochasticOscillatorObject->getValue<int>("period_for_classic_line");
  stochasticOscillatorSettings.periodsForClassicLine_ = periodForClassicLine;

  auto smoothFastPeriod = stochasticOscillatorObject->getValue<int>("smooth_fast_period");
  stochasticOscillatorSettings.smoothFastPeriod_ = smoothFastPeriod;

  auto smoothSlowPeriod = stochasticOscillatorObject->getValue<int>("smooth_slow_period");
  stochasticOscillatorSettings.smoothSlowPeriod_ = smoothSlowPeriod;

  auto candlesAfterCrossing = stochasticOscillatorObject->getValue<int>("candles_after_crossings");
  stochasticOscillatorSettings.crossingInterval_ = candlesAfterCrossing;

  auto topLevel = stochasticOscillatorObject->getValue<int>("top_level");
  stochasticOscillatorSettings.topLevel = topLevel;

  auto bottomLevel = stochasticOscillatorObject->getValue<int>("bottom_level");
  stochasticOscillatorSettings.bottomLevel = bottomLevel;
}

void StrategyJSONSerializer::deserializeMovingAveragesCrossingSettings(
    Poco::JSON::Object::Ptr jsonObject,
    model::MovingAveragesCrossingSettings &movingAveragesCrossingSettings) {
  auto movingAveragesCrossingObject = jsonObject->getObject("moving_averages_crossings_settings");
  auto name = movingAveragesCrossingObject->getValue<std::string>("strategy_name");
  movingAveragesCrossingSettings.name_ = name;

  auto type = movingAveragesCrossingObject->getValue<int>("strategy_type");
  movingAveragesCrossingSettings.strategiesType_ = static_cast<common::StrategiesType>(type);

  auto tickInterval = movingAveragesCrossingObject->getValue<int>("tick_interval");
  movingAveragesCrossingSettings.tickInterval_ =
      static_cast<common::TickInterval::Enum>(tickInterval);

  auto movingAverageType = movingAveragesCrossingObject->getValue<int>("moving_average_type");
  movingAveragesCrossingSettings.movingAverageType_ =
      static_cast<common::MovingAverageType>(movingAverageType);

  auto smallerPeriod = movingAveragesCrossingObject->getValue<int>("smaller_period");
  movingAveragesCrossingSettings.smallerPeriod_ = smallerPeriod;

  auto biggerPeriod = movingAveragesCrossingObject->getValue<int>("bigger_period");
  movingAveragesCrossingSettings.biggerPeriod_ = biggerPeriod;

  auto crossingInterval = movingAveragesCrossingObject->getValue<int>("crossing_interval");
  movingAveragesCrossingSettings.crossingInterval_ = crossingInterval;
}

void StrategyJSONSerializer::deserializeRsiSettings(Poco::JSON::Object::Ptr jsonObject,
                                                    model::RsiSettings &rsiSettings) {
  auto rsiObject = jsonObject->getObject("rsi_settings");
  auto name = rsiObject->getValue<std::string>("strategy_name");
  rsiSettings.name_ = name;

  auto type = rsiObject->getValue<int>("strategy_type");
  rsiSettings.strategiesType_ = static_cast<common::StrategiesType>(type);

  auto tickInterval = rsiObject->getValue<int>("tick_interval");
  rsiSettings.tickInterval_ = static_cast<common::TickInterval::Enum>(tickInterval);

  auto period = rsiObject->getValue<unsigned int>("period");
  rsiSettings.period_ = period;

  auto candlesAfterCrossing = rsiObject->getValue<unsigned int>("candles_after_crossing");
  rsiSettings.crossingInterval_ = candlesAfterCrossing;

  auto topLevel = rsiObject->getValue<unsigned int>("top_level");
  rsiSettings.topLevel_ = topLevel;

  auto bottomlevel = rsiObject->getValue<unsigned int>("bottom_level");
  rsiSettings.bottomLevel_ = bottomlevel;
}

void StrategyJSONSerializer::deserializeSmaSettings(Poco::JSON::Object::Ptr jsonObject,
                                                    model::SmaSettings &smaSettings) {
  auto smaObject = jsonObject->getObject("sma_settings");
  auto name = smaObject->getValue<std::string>("strategy_name");
  smaSettings.name_ = name;

  auto type = smaObject->getValue<int>("strategy_type");
  smaSettings.strategiesType_ = static_cast<common::StrategiesType>(type);

  auto tickInterval = smaObject->getValue<int>("tick_interval");
  smaSettings.tickInterval_ = static_cast<common::TickInterval::Enum>(tickInterval);
  auto period = smaObject->getValue<unsigned int>("period");
  smaSettings.period_ = period;
  auto candlesAfterCrossing = smaObject->getValue<unsigned int>("candles_after_crossing");
  smaSettings.crossingInterval_ = candlesAfterCrossing;
}

void StrategyJSONSerializer::deserializeEmaSettings(Poco::JSON::Object::Ptr jsonObject,
                                                    model::EmaSettings &emaSettings) {
  auto emaObject = jsonObject->getObject("ema_settings");
  auto name = emaObject->getValue<std::string>("strategy_name");
  emaSettings.name_ = name;
  auto type = emaObject->getValue<int>("strategy_type");
  emaSettings.strategiesType_ = static_cast<common::StrategiesType>(type);
  auto tickInterval = emaObject->getValue<int>("tick_interval");
  emaSettings.tickInterval_ = static_cast<common::TickInterval::Enum>(tickInterval);
  auto period = emaObject->getValue<unsigned int>("period");
  emaSettings.period_ = period;
  auto candlesAfterCrossing = emaObject->getValue<unsigned int>("candles_after_crossing");
  emaSettings.crossingInterval_ = candlesAfterCrossing;
}

void StrategyJSONSerializer::deserializeCustomSettings(
    Poco::JSON::Object::Ptr jsonObject, model::CustomStrategySettings &customStrategySettings) {
  auto customObject = jsonObject->getObject("custom_strategy");
  auto type = customObject->getValue<int>("strategy_type");
  customStrategySettings.strategiesType_ = static_cast<common::StrategiesType>(type);
  auto name = customObject->getValue<std::string>("strategy_name");
  customStrategySettings.name_ = name;
  auto strategiesArray = customObject->getArray("strategies");
  for (unsigned int index = 0; index < strategiesArray->size(); ++index) {
    auto strategyVar = strategiesArray->get(index);
    auto strategyObject = strategyVar.extract<Poco::JSON::Object::Ptr>();
    auto strategyType = strategyObject->getValue<int>("strategy_type");
    auto strategy =
        deserializeStrategy(strategyObject, static_cast<common::StrategiesType>(strategyType));
    customStrategySettings.strategies_.emplace_back(std::move(strategy));
  }
}

std::unique_ptr<model::StrategySettings> StrategyJSONSerializer::deserializeStrategy(
    Poco::JSON::Object::Ptr jsonObject, common::StrategiesType type) {
  switch (type) {
    case common::StrategiesType::BOLLINGER_BANDS: {
      auto bbSettings = std::make_unique<model::BollingerBandsSettings>();
      deserializeBollingerBandsSettings(jsonObject, *bbSettings);
      return std::move(bbSettings);
    }
    case common::StrategiesType::BOLLINGER_BANDS_ADVANCED: {
      auto bbAdvancedSettings = std::make_unique<model::BollingerBandsAdvancedSettings>();
      deserializeBollingerBandsAdvancedSettings(jsonObject, *bbAdvancedSettings);
      return std::move(bbAdvancedSettings);
    }
    case common::StrategiesType::EMA: {
      auto emaSettings = std::make_unique<model::EmaSettings>();
      deserializeEmaSettings(jsonObject, *emaSettings);
      return std::move(emaSettings);
    }
    case common::StrategiesType::SMA: {
      auto smaSettings = std::make_unique<model::SmaSettings>();
      deserializeSmaSettings(jsonObject, *smaSettings);
      return std::move(smaSettings);
    }
    case common::StrategiesType::RSI: {
      auto rsiSettings = std::make_unique<model::RsiSettings>();
      deserializeRsiSettings(jsonObject, *rsiSettings);
      return std::move(rsiSettings);
    }
    case common::StrategiesType::MA_CROSSING: {
      auto maCrossing = std::make_unique<model::MovingAveragesCrossingSettings>();
      deserializeMovingAveragesCrossingSettings(jsonObject, *maCrossing);
      return std::move(maCrossing);
    }
    case common::StrategiesType::STOCHASTIC_OSCILLATOR: {
      auto stochasticOscillator = std::make_unique<model::StochasticOscillatorSettings>();
      deserializeStochasticOscillatorSettings(jsonObject, *stochasticOscillator);
      return std::move(stochasticOscillator);
    }
    case common::StrategiesType::CUSTOM: {
      auto customSettings = std::make_unique<model::CustomStrategySettings>();
      deserializeCustomSettings(jsonObject, *customSettings);
      return std::move(customSettings);
    }
    default:
      throw common::exceptions::UndefinedTypeException("Strategy type.");
  }
}

}  // namespace serializer
}  // namespace auto_trader