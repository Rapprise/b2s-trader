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

#ifndef AUTO_TRADER_COMMON_TICK_INTERVAL_H
#define AUTO_TRADER_COMMON_TICK_INTERVAL_H

#include <string>

namespace auto_trader {
namespace common {

class TickInterval {
 public:
  enum Enum {
    ONE_MIN,
    THREE_MIN,
    FIVE_MIN,
    FIFTEEN_MIN,
    THIRTY_MIN,
    ONE_HOUR,
    TWO_HOURS,
    FOUR_HOURS,
    SIX_HOURS,
    EIGHT_HOURS,
    TWELVE_HOURS,
    ONE_DAY,
    THREE_DAYS,
    ONE_WEEK,
    TWO_WEEKS,
    ONE_MONTH,
    ONE_YEAR,
    UNKNOWN
  };

  static std::string toString(Enum tickInteval) {
    switch (tickInteval) {
      case ONE_MIN:
        return "ONE MIN";
      case THREE_MIN:
        return "THREE MIN";
      case FIVE_MIN:
        return "FIVE MIN";
      case FIFTEEN_MIN:
        return "FIFTEEN MIN";
      case THIRTY_MIN:
        return "THIRTY MIN";
      case ONE_HOUR:
        return "ONE HOUR";
      case TWO_HOURS:
        return "TWO HOURS";
      case FOUR_HOURS:
        return "FOUR HOURS";
      case SIX_HOURS:
        return "SIX HOURS";
      case EIGHT_HOURS:
        return "EIGHT HOURS";
      case TWELVE_HOURS:
        return "TWELVE HOURS";
      case ONE_DAY:
        return "ONE DAY";
      case THREE_DAYS:
        return "THREE DAYS";
      case ONE_WEEK:
        return "ONE WEEK";
      case ONE_MONTH:
        return "ONE MONTH";
      case TWO_WEEKS:
        return "TWO WEEKS";
      default:
        return "UNKNOWN";
    }
  }

  static TickInterval::Enum fromString(const std::string& tickInteval) {
    if (tickInteval == "ONE MIN") {
      return TickInterval::ONE_MIN;
    } else if (tickInteval == "THREE MIN") {
      return TickInterval::THREE_MIN;
    } else if (tickInteval == "FIVE MIN") {
      return TickInterval::FIVE_MIN;
    } else if (tickInteval == "FIFTEEN MIN") {
      return TickInterval::FIFTEEN_MIN;
    } else if (tickInteval == "THIRTY MIN") {
      return TickInterval::THIRTY_MIN;
    } else if (tickInteval == "ONE HOUR") {
      return TickInterval::ONE_HOUR;
    } else if (tickInteval == "TWO HOURS") {
      return TickInterval::TWO_HOURS;
    } else if (tickInteval == "FOUR HOURS") {
      return TickInterval::FOUR_HOURS;
    } else if (tickInteval == "SIX HOURS") {
      return TickInterval::SIX_HOURS;
    } else if (tickInteval == "EIGHT HOURS") {
      return TickInterval::EIGHT_HOURS;
    } else if (tickInteval == "TWELVE HOURS") {
      return TickInterval::TWELVE_HOURS;
    } else if (tickInteval == "ONE DAY") {
      return TickInterval::ONE_DAY;
    } else if (tickInteval == "THREE DAYS") {
      return TickInterval::THREE_DAYS;
    } else if (tickInteval == "ONE WEEK") {
      return TickInterval::ONE_WEEK;
    } else if (tickInteval == "TWO WEEKS") {
      return TickInterval::TWO_WEEKS;
    } else if (tickInteval == "ONE MONTH") {
      return TickInterval::ONE_MONTH;
    } else {
      return TickInterval::UNKNOWN;
    }
  }
};

}  // namespace common
}  // namespace auto_trader

#endif  // AUTO_TRADER_COMMON_TICK_INTERVAL_H
