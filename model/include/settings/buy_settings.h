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

#ifndef AUTO_TRADER_MODEL_BUY_SETTINGS_H
#define AUTO_TRADER_MODEL_BUY_SETTINGS_H

namespace auto_trader {
namespace model {

struct BuySettings {
  // Maximum time for active buying order.
  unsigned int maxOpenTime_;

  // Maximum count for buying orders of all traded currencies.
  unsigned int maxOpenOrders_;

  // Max open positions count per each traded currencies.
  unsigned int openPositionAmountPerCoins_;

  // Maximum amount of base currencies for trading.
  double maxCoinAmount_;

  // Percent to calculate quantity for each buying order.
  double percentageBuyAmount_;

  // Minimum order amount
  double minOrderPrice_;

  double getBaseCurrencyAmountPerEachOrder() const {
    return maxCoinAmount_ * (percentageBuyAmount_ / 100);
  }

  bool openOrderWhenAnyIndicatorIsTriggered_{false};
};

}  // namespace model
}  // namespace auto_trader

#endif  // AUTO_TRADER_MODEL_BUY_SETTINGS_H
