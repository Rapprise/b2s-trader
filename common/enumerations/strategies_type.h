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

#ifndef COMMON_ENUMERATIONS_H_
#define COMMON_ENUMERATIONS_H_

#include <string>

namespace auto_trader {
namespace common {

enum class StrategiesType {
  BOLLINGER_BANDS,
  BOLLINGER_BANDS_ADVANCED,
  RSI,
  SMA,
  EMA,
  MA_CROSSING,
  STOCHASTIC_OSCILLATOR,
  MACD,
  CUSTOM,
  UNKNOWN
};

static std::string convertStrategyTypeToString(StrategiesType type) {
  switch (type) {
    case StrategiesType::BOLLINGER_BANDS:
      return "Bollinger Bands";
    case StrategiesType::BOLLINGER_BANDS_ADVANCED:
      return "Bollinger Bands Advanced";
    case StrategiesType::RSI:
      return "RSI";
    case StrategiesType::SMA:
      return "SMA";
    case StrategiesType::EMA:
      return "EMA";
    case StrategiesType::MA_CROSSING:
      return "MA Crossing";
    case StrategiesType::STOCHASTIC_OSCILLATOR:
      return "Stochastic Oscillator";
    case StrategiesType::MACD:
      return "MACD";
    case StrategiesType::CUSTOM:
      return "Custom";
    default:
      return "Unknown";
  }
}

static StrategiesType convertStrategyTypeFromString(const std::string& type) {
  if (type == "Bollinger Bands")
    return StrategiesType::BOLLINGER_BANDS;
  else if (type == "Bollinger Bands Advanced")
    return StrategiesType::BOLLINGER_BANDS_ADVANCED;
  else if (type == "RSI")
    return StrategiesType::RSI;
  else if (type == "SMA")
    return StrategiesType::SMA;
  else if (type == "EMA")
    return StrategiesType::EMA;
  else if (type == "MA Crossing")
    return StrategiesType::MA_CROSSING;
  else if (type == "Stochastic Oscillator")
    return StrategiesType::STOCHASTIC_OSCILLATOR;
  else if (type == "Custom")
    return StrategiesType::CUSTOM;
  else
    return StrategiesType::UNKNOWN;
}

}  // namespace common
}  // namespace auto_trader

#endif  // ENUMERATIONS_H_
