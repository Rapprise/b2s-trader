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

#include "include/trade_config_json_serializer.h"

namespace auto_trader {
namespace serializer {

void TradeConfigJSONSerializer::serialize(const model::TradeConfiguration& configuration,
                                          std::ostream& stream) {
  Poco::JSON::PrintHandler printHandler(stream, 1);

  printHandler.startObject();
  printHandler.key("name");
  printHandler.value(configuration.getName());

  printHandler.key("description");
  printHandler.value(configuration.getDescription());

  printHandler.key("strategy_name");
  printHandler.value(configuration.getStrategyName());

  printHandler.key("is_active");
  printHandler.value(configuration.isActive());

  serializeBuySettings(printHandler, configuration.getBuySettings());
  serializeSellSettings(printHandler, configuration.getSellSettings());
  serializeCoinSettings(printHandler, configuration.getCoinSettings());
  serializeStockExchangeSettings(printHandler, configuration.getStockExchangeSettings());

  printHandler.endObject();
}

std::unique_ptr<model::TradeConfiguration> TradeConfigJSONSerializer::deserialize(
    std::istream& stream) {
  Poco::JSON::Parser parser;
  auto tradeConfiguration = std::make_unique<model::TradeConfiguration>();
  auto jsonObject = parser.parse(stream);
  auto object = jsonObject.extract<Poco::JSON::Object::Ptr>();
  const std::string& name = object->getValue<std::string>("name");
  const std::string& description = object->getValue<std::string>("description");
  const std::string& strategyName = object->getValue<std::string>("strategy_name");
  const bool isActive = object->getValue<bool>("is_active");

  tradeConfiguration->setName(name);
  tradeConfiguration->setDescription(description);
  tradeConfiguration->setStrategyName(strategyName);
  tradeConfiguration->setActive(isActive);

  deserializeBuySettings(object, tradeConfiguration->takeBuySettings());
  deserializeSellSettings(object, tradeConfiguration->takeSellSettings());
  deserializeCoinSettings(object, tradeConfiguration->takeCoinSettings());
  deserializeStockExchangeSettings(object, tradeConfiguration->takeStockExchangeSettings());

  return std::move(tradeConfiguration);
}

void TradeConfigJSONSerializer::serializeBuySettings(Poco::JSON::PrintHandler& printHandler,
                                                     const model::BuySettings& buySettings) {
  printHandler.key("buy_settings");
  printHandler.startObject();

  printHandler.key("max_coin_amount");
  printHandler.value(buySettings.maxCoinAmount_);

  printHandler.key("max_open_orders");
  printHandler.value(buySettings.maxOpenOrders_);

  printHandler.key("max_open_order_time");
  printHandler.value(buySettings.maxOpenTime_);

  printHandler.key("min_order_price");
  printHandler.value(buySettings.minOrderPrice_);

  printHandler.key("orders_amount_per_coin");
  printHandler.value(buySettings.openPositionAmountPerCoins_);

  printHandler.key("percentage_buy_amount");
  printHandler.value(buySettings.percentageBuyAmount_);

  printHandler.key("open_order_when_any_indicator_is_triggered");
  printHandler.value(buySettings.openOrderWhenAnyIndicatorIsTriggered_);

  printHandler.endObject();
}

void TradeConfigJSONSerializer::serializeSellSettings(Poco::JSON::PrintHandler& printHandler,
                                                      const model::SellSettings& sellSettings) {
  printHandler.key("sell_settings");
  printHandler.startObject();

  printHandler.key("open_order_time");
  printHandler.value(sellSettings.openOrderTime_);

  printHandler.key("profit_percentage");
  printHandler.value(sellSettings.profitPercentage_);

  printHandler.endObject();
}

void TradeConfigJSONSerializer::serializeCoinSettings(Poco::JSON::PrintHandler& printHandler,
                                                      const model::CoinSettings& coinSettings) {
  printHandler.key("coin_settings");
  printHandler.startObject();

  printHandler.key("base_currency");
  printHandler.value(coinSettings.baseCurrency_);

  printHandler.key("traded_currencies");
  printHandler.startArray();

  auto tradedCurrenciesSize = coinSettings.tradedCurrencies_.size();

  for (int index = 0; index < tradedCurrenciesSize; ++index) {
    printHandler.startObject();
    auto currency = coinSettings.tradedCurrencies_[index];
    printHandler.key(std::to_string(index));
    printHandler.value(currency);
    printHandler.endObject();
  }

  printHandler.endArray();
  printHandler.endObject();
}

void TradeConfigJSONSerializer::serializeStockExchangeSettings(
    Poco::JSON::PrintHandler& printHandler,
    const model::StockExchangeSettings& stockExchangeSettings) {
  printHandler.key("stock_exchange_settings");

  printHandler.startObject();

  printHandler.key("stock_exchange_type");
  printHandler.value(static_cast<int>(stockExchangeSettings.stockExchangeType_));

  printHandler.key("api_key");
  printHandler.value(stockExchangeSettings.apiKey_);

  printHandler.key("secret_key");
  printHandler.value(stockExchangeSettings.secretKey_);

  printHandler.endObject();
}

void TradeConfigJSONSerializer::deserializeBuySettings(Poco::JSON::Object::Ptr jsonObject,
                                                       model::BuySettings& buySettings) {
  auto buyObject = jsonObject->getObject("buy_settings");
  auto maxCointAmount = buyObject->getValue<double>("max_coin_amount");
  buySettings.maxCoinAmount_ = maxCointAmount;
  auto maxOpenOrders = buyObject->getValue<unsigned int>("max_open_orders");
  buySettings.maxOpenOrders_ = maxOpenOrders;
  auto maxOpenOrderTime = buyObject->getValue<unsigned int>("max_open_order_time");
  buySettings.maxOpenTime_ = maxOpenOrderTime;
  auto minOrderPrice = buyObject->getValue<double>("min_order_price");
  buySettings.minOrderPrice_ = minOrderPrice;
  auto ordersAmountPerCoin = buyObject->getValue<unsigned int>("orders_amount_per_coin");
  buySettings.openPositionAmountPerCoins_ = ordersAmountPerCoin;
  auto percentageBuyAmount = buyObject->getValue<double>("percentage_buy_amount");
  buySettings.percentageBuyAmount_ = percentageBuyAmount;
  auto openOrderWhenAnyIndicatorIsTriggered =
      buyObject->getValue<bool>("open_order_when_any_indicator_is_triggered");
  buySettings.openOrderWhenAnyIndicatorIsTriggered_ = openOrderWhenAnyIndicatorIsTriggered;
}

void TradeConfigJSONSerializer::deserializeSellSettings(Poco::JSON::Object::Ptr jsonObject,
                                                        model::SellSettings& sellSettings) {
  auto sellObject = jsonObject->getObject("sell_settings");

  auto openOrderTime = sellObject->getValue<unsigned int>("open_order_time");
  sellSettings.openOrderTime_ = openOrderTime;

  auto percentageProfit = sellObject->getValue<double>("profit_percentage");
  sellSettings.profitPercentage_ = percentageProfit;
}

void TradeConfigJSONSerializer::deserializeCoinSettings(Poco::JSON::Object::Ptr jsonObject,
                                                        model::CoinSettings& coinSettings) {
  auto coinObject = jsonObject->getObject("coin_settings");
  auto baseCurrency = coinObject->getValue<int>("base_currency");
  coinSettings.baseCurrency_ = static_cast<common::Currency::Enum>(baseCurrency);

  auto tradedCurrencies = coinObject->getArray("traded_currencies");
  size_t arraySize = tradedCurrencies->size();
  for (unsigned int index = 0; index < arraySize; ++index) {
    auto tradedCurrency = tradedCurrencies->get(index);
    auto currencyObject = tradedCurrency.extract<Poco::JSON::Object::Ptr>();
    auto currency = currencyObject->getValue<int>(std::to_string(index));
    coinSettings.tradedCurrencies_.push_back(static_cast<common::Currency::Enum>(currency));
  }
}

void TradeConfigJSONSerializer::deserializeStockExchangeSettings(
    Poco::JSON::Object::Ptr jsonObject, model::StockExchangeSettings& stockExchangeSettings) {
  auto stockExchangeObject = jsonObject->getObject("stock_exchange_settings");
  auto stockExchangeType = stockExchangeObject->getValue<int>("stock_exchange_type");
  stockExchangeSettings.stockExchangeType_ =
      static_cast<common::StockExchangeType>(stockExchangeType);

  auto apiKey = stockExchangeObject->getValue<std::string>("api_key");
  stockExchangeSettings.apiKey_ = apiKey;

  auto secretKey = stockExchangeObject->getValue<std::string>("secret_key");
  stockExchangeSettings.secretKey_ = secretKey;
}

}  // namespace serializer
}  // namespace auto_trader