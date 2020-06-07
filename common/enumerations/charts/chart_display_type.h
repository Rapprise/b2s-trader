/*
 * Copyright (c) 2019, Rapprise - All rights reserved.
 * Unauthorized copying of this file, via any medium is strictly prohibited.
 * Proprietary and confidential.
 */

#ifndef AUTO_TRADER_COMMON_CHART_DISPLAY_TYPE_H
#define AUTO_TRADER_COMMON_CHART_DISPLAY_TYPE_H

#include <string>

namespace auto_trader {
namespace common {
namespace charts {

class ChartDisplayType {
 public:
  enum Enum { MOUNTAINS, CANDLES, UNKNOWN };

  static std::string toString(ChartDisplayType::Enum type) {
    switch (type) {
      case MOUNTAINS:
        return "Mountains";
      case CANDLES:
        return "Candles";
      default:
        return "Unknown";
    }
  }

  static Enum fromString(const std::string &type) {
    if (type == "Mountains") {
      return MOUNTAINS;
    } else if (type == "Candles") {
      return CANDLES;
    } else {
      return UNKNOWN;
    }
  }
};

}  // namespace charts
}  // namespace common
}  // namespace auto_trader

#endif  // AUTO_TRADER_CHART_DISPLAY_TYPE_H
