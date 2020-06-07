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

#ifndef COMMON_BOLLINGERFIELD_INPUT_TYPE__H_
#define COMMON_BOLLINGERFIELD_INPUT_TYPE__H_

#include <string>

#include "common/exceptions/undefined_type_exception.h"
#include "common/market_data.h"

namespace auto_trader {
namespace common {

enum class BollingerInputType {
  price_,
  openPosition_,
  closePosition_,
  lowPrice_,
  highPrice_,
  volume_,
  unknown,
};

static std::string convertBBInputTypeToString(BollingerInputType type) {
  switch (type) {
    case BollingerInputType::price_:
      return "price";
    case BollingerInputType::openPosition_:
      return "open position";
    case BollingerInputType::closePosition_:
      return "close position";
    case BollingerInputType::lowPrice_:
      return "low price";
    case BollingerInputType::highPrice_:
      return "high price";
    case BollingerInputType::volume_:
      return "volume";

    default:
      return "UNKNOWN";
  }
}

static BollingerInputType convertBBInputTypeFromString(const std::string& type) {
  if (type == "price") {
    return BollingerInputType::price_;
  } else if (type == "open position") {
    return BollingerInputType::openPosition_;
  } else if (type == "close position") {
    return BollingerInputType::closePosition_;
  } else if (type == "low price") {
    return BollingerInputType::lowPrice_;
  } else if (type == "high price") {
    return BollingerInputType::highPrice_;
  } else if (type == "volume") {
    return BollingerInputType::volume_;
  } else {
    return BollingerInputType::unknown;
  }
}

}  // namespace common
}  // namespace auto_trader

#endif  // DATA_BOLLINGERFIELD_H
