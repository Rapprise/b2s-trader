/*
 * Copyright (c) 2019, Rapprise - All rights reserved.
 * Unauthorized copying of this file, via any medium is strictly prohibited.
 * Proprietary and confidential.
 */

#ifndef AUTO_TRADER_COMMON_CHART_PERIOD_TYPE_H
#define AUTO_TRADER_COMMON_CHART_PERIOD_TYPE_H

#include <string>

namespace auto_trader {
namespace common {
namespace charts {

class ChartPeriodType {
 public:
  enum Enum { ONE_DAY, ONE_WEEK, ONE_MONTH, THREE_MONTH, SIX_MONTH, ONE_YEAR, ALL, UNKNOWN };

  static std::string toString(ChartPeriodType::Enum periodType) {
    switch (periodType) {
      case ONE_DAY:
        return "ONE DAY";
      case ONE_WEEK:
        return "ONE WEEK";
      case ONE_MONTH:
        return "ONE MONTH";
      case THREE_MONTH:
        return "THREE MONTH";
      case SIX_MONTH:
        return "SIX MONTH";
      case ONE_YEAR:
        return "ONE YEAR";
      case ALL:
        return "ALL";
      default:
        return "UNKNOWN";
    }
  }

  static ChartPeriodType::Enum fromString(const std::string& type) {
    if (type == "ONE DAY") {
      return ChartPeriodType::ONE_DAY;
    } else if (type == "ONE WEEK") {
      return ChartPeriodType::ONE_WEEK;
    } else if (type == "ONE MONTH") {
      return ChartPeriodType::ONE_MONTH;
    } else if (type == "THREE MONTH") {
      return ChartPeriodType::THREE_MONTH;
    } else if (type == "SIX MONTH") {
      return ChartPeriodType::SIX_MONTH;
    } else if (type == "ONE YEAR") {
      return ChartPeriodType::ONE_YEAR;
    } else if (type == "ALL") {
      return ChartPeriodType::ALL;
    } else {
      return ChartPeriodType::UNKNOWN;
    }
  }
};

}  // namespace charts
}  // namespace common
}  // namespace auto_trader

#endif  // AUTO_TRADER_COMMON_CHART_PERIOD_TYPE_H
