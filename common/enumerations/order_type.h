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

#ifndef AUTO_TRADER_COMMON_ORDER_TYPE_H
#define AUTO_TRADER_COMMON_ORDER_TYPE_H

#include <map>
#include <string>

#include "common/exceptions/undefined_type_exception.h"

namespace auto_trader {
namespace common {

enum class OrderType { BUY, SELL, UNKNOWN };

static OrderType parseBinanceOrderType(std::string type) {
  if (type == "SELL")
    return OrderType::SELL;
  else if (type == "BUY")
    return OrderType::BUY;
  else
    return OrderType::UNKNOWN;
}

static OrderType parseBittrexOrderType(std::string type) {
  if (type == "LIMIT_SELL")
    return OrderType::SELL;
  else if (type == "LIMIT_BUY")
    return OrderType::BUY;
  else
    return OrderType::UNKNOWN;
}

static std::string convertOrderTypeToString(OrderType type) {
  switch (type) {
    case OrderType::BUY:
      return "BUY";
    case OrderType::SELL:
      return "SELL";
    default:
      return "UNKNOWN";
  }
}

static OrderType convertOrderTypeFromString(const std::string& type) {
  if (type == "SELL")
    return OrderType::SELL;
  else if (type == "BUY")
    return OrderType::BUY;
  else
    return OrderType::UNKNOWN;
}

}  // namespace common
}  // namespace auto_trader

#endif  // AUTO_TRADER_DATA_ORDER_TYPE_H
