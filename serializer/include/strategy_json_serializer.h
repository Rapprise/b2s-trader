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

#ifndef AUTO_TRADER_SERIALIZER_STRATEGY_SERIALIZER_H
#define AUTO_TRADER_SERIALIZER_STRATEGY_SERIALIZER_H

#include <Poco/JSON/Parser.h>
#include <Poco/JSON/PrintHandler.h>

#include "model/include/settings/strategies_settings/strategy_settings.h"
#include "model/include/settings/strategies_settings/strategy_settings_visitor.h"

namespace auto_trader {

namespace model {
struct StrategySettings;
}

namespace serializer {

class StrategyJSONSerializer : public model::StrategySettingsVisitor {
 public:
  void serialize(const model::StrategySettings& strategySettings, std::ostream& stream);
  std::unique_ptr<model::StrategySettings> deserialize(std::istream& stream);

 private:
  void visit(const model::BollingerBandsSettings& bollingerBandsSettings);
  void visit(const model::BollingerBandsAdvancedSettings& bollingerBandsAdvancedSettings);
  void visit(const model::MovingAveragesCrossingSettings& movingAveragesCrossingSettings);
  void visit(const model::RsiSettings& rsiSettings);
  void visit(const model::SmaSettings& smaSettings);
  void visit(const model::EmaSettings& emaSettings);
  void visit(const model::StochasticOscillatorSettings& stochasticOscillatorSettings);
  void visit(const model::CustomStrategySettings& customStrategySettings);

  void deserializeBollingerBandsSettings(Poco::JSON::Object::Ptr jsonObject,
                                         model::BollingerBandsSettings& bollingerBandsSettings);
  void deserializeBollingerBandsAdvancedSettings(
      Poco::JSON::Object::Ptr jsonObject,
      model::BollingerBandsAdvancedSettings& bollingerBandsAdvancedSettings);
  void deserializeMovingAveragesCrossingSettings(
      Poco::JSON::Object::Ptr jsonObject,
      model::MovingAveragesCrossingSettings& movingAveragesCrossingSettings);
  void deserializeRsiSettings(Poco::JSON::Object::Ptr jsonObject, model::RsiSettings& rsiSettings);
  void deserializeSmaSettings(Poco::JSON::Object::Ptr jsonObject, model::SmaSettings& smaSettings);
  void deserializeEmaSettings(Poco::JSON::Object::Ptr jsonObject, model::EmaSettings& emaSettings);
  void deserializeStochasticOscillatorSettings(
      Poco::JSON::Object::Ptr jsonObject,
      model::StochasticOscillatorSettings& stochasticOscillatorSettings);
  void deserializeCustomSettings(Poco::JSON::Object::Ptr jsonObject,
                                 model::CustomStrategySettings& customStrategySettings);

  std::unique_ptr<model::StrategySettings> deserializeStrategy(Poco::JSON::Object::Ptr jsonObject,
                                                               common::StrategiesType type);

 private:
  std::unique_ptr<Poco::JSON::PrintHandler> printHandler_;
};

}  // namespace serializer
}  // namespace auto_trader

#endif  // AUTO_TRADER_SERIALIZER_STRATEGY_SERIALIZER_H
