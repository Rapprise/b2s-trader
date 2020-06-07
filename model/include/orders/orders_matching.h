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

#ifndef AUTO_TRADER_ORDERS_MATCHING_H
#define AUTO_TRADER_ORDERS_MATCHING_H

#include <map>

#include "common/market_order.h"

namespace auto_trader {
namespace model {

class OrderMatching {
 public:
  OrderMatching(common::OrderType fromOrderType, common::OrderType toOrderType);

 public:
  void addOrderMatching(const common::MarketOrder &fromOrder, const common::MarketOrder &toOrder);
  void removeMatching(const common::MarketOrder &fromOrder);
  void forEachMatching(
      std::function<void(const common::MarketOrder &, const common::MarketOrder &)> callback) const;

  common::MarketOrder getMatchedOrder(const common::MarketOrder &fromOrder) const;

  void clear();

 private:
  std::map<common::MarketOrder, common::MarketOrder> orders_;

  common::OrderType fromOrderType_;
  common::OrderType toOrderType_;
};

}  // namespace model
}  // namespace auto_trader

#endif  // AUTO_TRADER_ORDERS_MATCHING_H
