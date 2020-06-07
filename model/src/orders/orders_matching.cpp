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

#include "include/orders/orders_matching.h"

#include <assert.h>

#include "common/exceptions/trading_execution_exception/undefined_open_order_exception.h"

namespace auto_trader {
namespace model {

OrderMatching::OrderMatching(common::OrderType fromOrderType, common::OrderType toOrderType)
    : fromOrderType_(fromOrderType), toOrderType_(toOrderType) {}

void OrderMatching::addOrderMatching(const common::MarketOrder &fromOrder,
                                     const common::MarketOrder &toOrder) {
  assert(fromOrder.orderType_ == fromOrderType_);
  assert(toOrder.orderType_ == toOrderType_);

  orders_.insert(std::make_pair<>(fromOrder, toOrder));
}

void OrderMatching::removeMatching(const common::MarketOrder &fromOrder) {
  assert(fromOrder.orderType_ == fromOrderType_);

  orders_.erase(fromOrder);
}

void OrderMatching::forEachMatching(
    std::function<void(const common::MarketOrder &, const common::MarketOrder &)> callback) const {
  for (auto it = orders_.begin(); it != orders_.end(); ++it) {
    callback(it->first, it->second);
  }
}

common::MarketOrder OrderMatching::getMatchedOrder(const common::MarketOrder &fromOrder) const {
  assert(fromOrder.orderType_ == fromOrderType_);
  auto it = orders_.find(fromOrder);
  if (it != orders_.end()) {
    return it->second;
  }

  throw common::exceptions::UndefinedOpenOrderException(fromOrder.uuid_);
}

void OrderMatching::clear() { orders_.clear(); }

}  // namespace model
}  // namespace auto_trader