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

#ifndef AUTO_TRADER_SERIALIZER_TRADE_CONFIG_SERIALIZER_H
#define AUTO_TRADER_SERIALIZER_TRADE_CONFIG_SERIALIZER_H

#include <Poco/JSON/Parser.h>

#include <ostream>

#include "Poco/JSON/PrintHandler.h"
#include "model/include/trade_configuration.h"
#include "strategy_json_serializer.h"

namespace auto_trader {
namespace serializer {

class TradeConfigJSONSerializer {
 public:
  void serialize(const model::TradeConfiguration& configuration, std::ostream& stream);
  std::unique_ptr<model::TradeConfiguration> deserialize(std::istream& stream);

 private:
  void serializeBuySettings(Poco::JSON::PrintHandler& printHandler,
                            const model::BuySettings& buySettings);
  void serializeSellSettings(Poco::JSON::PrintHandler& printHandler,
                             const model::SellSettings& sellSettings);
  void serializeCoinSettings(Poco::JSON::PrintHandler& printHandler,
                             const model::CoinSettings& coinSettings);
  void serializeStockExchangeSettings(Poco::JSON::PrintHandler& printHandler,
                                      const model::StockExchangeSettings& stockExchangeSettings);

  void deserializeBuySettings(Poco::JSON::Object::Ptr jsonObject, model::BuySettings& buySettings);
  void deserializeSellSettings(Poco::JSON::Object::Ptr jsonObject,
                               model::SellSettings& sellSettings);
  void deserializeCoinSettings(Poco::JSON::Object::Ptr jsonObject,
                               model::CoinSettings& coinSettings);
  void deserializeStockExchangeSettings(Poco::JSON::Object::Ptr jsonObject,
                                        model::StockExchangeSettings& stockExchangeSettings);
};

}  // namespace serializer
}  // namespace auto_trader

#endif  // AUTO_TRADER_SERIALIZER_TRADE_CONFIG_SERIALIZER_H
