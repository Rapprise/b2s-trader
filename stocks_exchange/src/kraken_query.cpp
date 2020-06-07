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

#include "include/kraken_query.h"

#include <Poco/JSON/Parser.h>
#include <Poco/URI.h>
#include <openssl/sha.h>

#include "common/exceptions/stock_exchange_exception/incorrect_json_exception.h"
#include "common/exceptions/stock_exchange_exception/invalid_stock_exchange_response_exception.h"
#include "common/loggers/file_logger.h"
#include "common/utils.h"
#include "include/stock_exchange_utils.h"
#include "resources/resources.h"

namespace auto_trader {
namespace stock_exchange {

void KrakenQuery::checkKrakenResponseMessage(Poco::JSON::Object::Ptr& object, curl_slist* chunk) {
  auto messageObject = object->getArray(resources::words::ERROR_WORD);
  if (messageObject->size() > 0) {
    const std::string message = messageObject->get(0).toString();
    common::loggers::FileLogger::getLogger()
        << resources::messages::FAILED_TO_UPLOAD_MESSAGE << message;

    if (chunk != nullptr) {
      curl_slist_free_all(chunk);
    }

    throw common::exceptions::InvalidStockExchangeResponse(message);
  }
}

Poco::JSON::Object::Ptr KrakenQuery::getJsonObjectAndCheckOnIncorrectJson(
    const std::string& response, curl_slist* chunk) {
  Poco::JSON::Parser parser;
  if (response.at(0) != resources::symbols::LEFT_CURLY_BRACE) {
    if (chunk != nullptr) {
      curl_slist_free_all(chunk);
    }

    throw common::exceptions::IncorrectJsonException("Incorrect json");
  }

  return parser.parse(response).extract<Poco::JSON::Object::Ptr>();
}

static size_t writeCurlSize(char* ptr, size_t size, size_t nmemb, void* userdata) {
  std::string* response = reinterpret_cast<std::string*>(userdata);
  size_t real_size = size * nmemb;

  response->append(ptr, real_size);
  return real_size;
}

static common::OrderType convertOrderTypeToEnum(const std::string& type_str) {
  common::OrderType type;
  if (type_str == resources::kraken::KRAKEN_BUY_ORDER)
    type = common::OrderType::BUY;
  else
    type = common::OrderType::SELL;

  return type;
}

common::MarketOrder KrakenQuery::sellOrder(common::Currency::Enum fromCurrency,
                                           common::Currency::Enum toCurrency, double quantity,
                                           double rate) {
  using namespace Poco;

  CURL* curl = curl_easy_init();
  checkCurl(curl);

  std::string path = resources::kraken::KRAKEN_SLASH_ZERO_SLASH_REQUEST +
                     resources::kraken::KRAKEN_PRIVATE_METHODS + resources::symbols::SLASH +
                     resources::kraken::KRAKEN_NEW_ORDER_KEYWORD;

  std::string method_url = resources::kraken::KRAKEN_API_URI + path;
  curl_easy_setopt(curl, CURLOPT_URL, method_url.c_str());

  std::string nonce = std::to_string(common::getCurrentMSEpoch());
  std::string post_data =
      resources::kraken::KRAKEN_CURRENCY_PAIR_KEYWORD + resources::symbols::EQUAL +
      krakenCurrency_.getKrakenPair(fromCurrency, toCurrency) + resources::symbols::AND +
      resources::kraken::KRAKEN_ORDER_TYPE + resources::symbols::EQUAL +
      resources::kraken::KRAKEN_SELL_ORDER + resources::symbols::AND +
      resources::kraken::KRAKEN_MARKET_TYPE + resources::symbols::EQUAL +
      resources::kraken::KRAKEN_TAKE_PROFIT + resources::symbols::AND + resources::words::PRICE +
      resources::symbols::EQUAL + common::MarketOrder::convertCoinToString(rate) +
      resources::symbols::AND + resources::words::VOLUME + resources::symbols::EQUAL +
      common::MarketOrder::convertCoinToString(quantity) + resources::symbols::AND +
      resources::words::NONCE + resources::symbols::EQUAL + nonce;

  curl_easy_setopt(curl, CURLOPT_POSTFIELDS, post_data.c_str());

  auto signature = common::generateSignatureSHA512(path, nonce, post_data, secret_key_);
  std::string api_key_header = resources::kraken::KRAKEN_API_KEY_HEADER + ":" + api_key_;
  std::string signature_header = resources::kraken::KRAKEN_PRIVATE_KEY_HEADER + ":" + signature;

  curl_slist* chunk = nullptr;
  chunk = curl_slist_append(chunk, api_key_header.c_str());
  chunk = curl_slist_append(chunk, signature_header.c_str());

  curl_easy_setopt(curl, CURLOPT_HTTPHEADER, chunk);

  std::string response = sendRequest(curl);

  JSON::Object::Ptr jsonMainObject = getJsonObjectAndCheckOnIncorrectJson(response, chunk);
  checkKrakenResponseMessage(jsonMainObject, chunk);

  curl_slist_free_all(chunk);

  common::MarketOrder currentOrder;
  currentOrder.orderType_ = common::OrderType::SELL;
  currentOrder.quantity_ = quantity;
  currentOrder.price_ = rate;
  currentOrder.stockExchangeType_ = common::StockExchangeType::Kraken;
  currentOrder.isCanceled_ = false;
  currentOrder.fromCurrency_ = fromCurrency;
  currentOrder.toCurrency_ = toCurrency;
  currentOrder.opened_ = common::Date::getCurrentTime();

  auto result = jsonMainObject->getObject(resources::words::RESULT);
  auto txid = result->getArray(resources::kraken::KRAKEN_ORDER_UUID_KEYWORD);
  if (txid->size() > 0) {
    currentOrder.uuid_ = txid->get(0).toString();
  } else {
    throw common::exceptions::StockExchangeException("Sell order: can't find order uuid");
  }

  return currentOrder;
}

common::MarketOrder KrakenQuery::buyOrder(common::Currency::Enum fromCurrency,
                                          common::Currency::Enum toCurrency, double quantity,
                                          double rate) {
  using namespace Poco;

  CURL* curl = curl_easy_init();
  checkCurl(curl);

  std::string path = resources::kraken::KRAKEN_SLASH_ZERO_SLASH_REQUEST +
                     resources::kraken::KRAKEN_PRIVATE_METHODS + resources::symbols::SLASH +
                     resources::kraken::KRAKEN_NEW_ORDER_KEYWORD;

  std::string method_url = resources::kraken::KRAKEN_API_URI + path;
  curl_easy_setopt(curl, CURLOPT_URL, method_url.c_str());

  std::string nonce = std::to_string(common::getCurrentMSEpoch());
  std::string post_data =
      resources::kraken::KRAKEN_CURRENCY_PAIR_KEYWORD + resources::symbols::EQUAL +
      krakenCurrency_.getKrakenPair(fromCurrency, toCurrency) + resources::symbols::AND +
      resources::kraken::KRAKEN_ORDER_TYPE + resources::symbols::EQUAL +
      resources::kraken::KRAKEN_BUY_ORDER + resources::symbols::AND +
      resources::kraken::KRAKEN_MARKET_TYPE + resources::symbols::EQUAL +
      resources::kraken::KRAKEN_TAKE_PROFIT + resources::symbols::AND + resources::words::PRICE +
      resources::symbols::EQUAL + common::MarketOrder::convertCoinToString(rate) +
      resources::symbols::AND + resources::words::VOLUME + resources::symbols::EQUAL +
      common::MarketOrder::convertCoinToString(quantity) + resources::symbols::AND +
      resources::words::NONCE + resources::symbols::EQUAL + nonce;

  curl_easy_setopt(curl, CURLOPT_POSTFIELDS, post_data.c_str());

  auto signature = common::generateSignatureSHA512(path, nonce, post_data, secret_key_);
  std::string api_key_header = resources::kraken::KRAKEN_API_KEY_HEADER + ":" + api_key_;
  std::string signature_header = resources::kraken::KRAKEN_PRIVATE_KEY_HEADER + ":" + signature;

  curl_slist* chunk = nullptr;
  chunk = curl_slist_append(chunk, api_key_header.c_str());
  chunk = curl_slist_append(chunk, signature_header.c_str());

  curl_easy_setopt(curl, CURLOPT_HTTPHEADER, chunk);

  std::string response = sendRequest(curl);

  JSON::Object::Ptr jsonMainObject = getJsonObjectAndCheckOnIncorrectJson(response, chunk);
  checkKrakenResponseMessage(jsonMainObject, chunk);

  curl_slist_free_all(chunk);

  common::MarketOrder currentOrder;
  currentOrder.orderType_ = common::OrderType::BUY;
  currentOrder.quantity_ = quantity;
  currentOrder.price_ = rate;
  currentOrder.stockExchangeType_ = common::StockExchangeType::Kraken;
  currentOrder.isCanceled_ = false;
  currentOrder.fromCurrency_ = fromCurrency;
  currentOrder.toCurrency_ = toCurrency;
  currentOrder.opened_ = common::Date::getCurrentTime();

  auto result = jsonMainObject->getObject(resources::words::RESULT);
  auto txid = result->getArray(resources::kraken::KRAKEN_ORDER_UUID_KEYWORD);
  if (txid->size() > 0) {
    currentOrder.uuid_ = txid->get(0).toString();
  } else {
    throw common::exceptions::StockExchangeException("Buy order: can't find order uuid");
  }

  return currentOrder;
}

bool KrakenQuery::cancelOrder(common::Currency::Enum fromCurrency,
                              common::Currency::Enum toCurrency, const std::string& uuid) {
  using namespace Poco;

  CURL* curl = curl_easy_init();
  checkCurl(curl);

  std::string path = resources::kraken::KRAKEN_SLASH_ZERO_SLASH_REQUEST +
                     resources::kraken::KRAKEN_PRIVATE_METHODS + resources::symbols::SLASH +
                     resources::kraken::KRAKEN_CANCEL_ORDER_KEYWORD;

  std::string method_url = resources::kraken::KRAKEN_API_URI + path;
  curl_easy_setopt(curl, CURLOPT_URL, method_url.c_str());

  std::string nonce = std::to_string(common::getCurrentMSEpoch());
  std::string post_data = resources::kraken::KRAKEN_ORDER_UUID_KEYWORD + resources::symbols::EQUAL +
                          uuid + resources::symbols::AND + resources::words::NONCE +
                          resources::symbols::EQUAL + nonce;

  curl_easy_setopt(curl, CURLOPT_POSTFIELDS, post_data.c_str());

  auto signature = common::generateSignatureSHA512(path, nonce, post_data, secret_key_);
  std::string api_key_header = resources::kraken::KRAKEN_API_KEY_HEADER + ":" + api_key_;
  std::string signature_header = resources::kraken::KRAKEN_PRIVATE_KEY_HEADER + ":" + signature;

  curl_slist* chunk = nullptr;
  chunk = curl_slist_append(chunk, api_key_header.c_str());
  chunk = curl_slist_append(chunk, signature_header.c_str());

  curl_easy_setopt(curl, CURLOPT_HTTPHEADER, chunk);

  std::string response = sendRequest(curl);

  JSON::Object::Ptr jsonMainObject = getJsonObjectAndCheckOnIncorrectJson(response, chunk);
  checkKrakenResponseMessage(jsonMainObject, chunk);

  curl_slist_free_all(chunk);

  auto result = jsonMainObject->getObject(resources::words::RESULT);
  int count = result->get(resources::words::COUNT);
  if (count > 0) return true;

  return false;
}

common::MarketHistoryPtr KrakenQuery::getMarketHistory(common::Currency::Enum fromCurrency,
                                                       common::Currency::Enum toCurrency,
                                                       common::TickInterval::Enum interval) {
  using namespace Poco;

  CURL* curl = curl_easy_init();
  checkCurl(curl);

  std::string path = resources::kraken::KRAKEN_SLASH_ZERO_SLASH_REQUEST +
                     resources::kraken::KRAKEN_PUBLIC_METHODS + resources::symbols::SLASH +
                     resources::kraken::KRAKEN_MARKET_HISTORY;

  std::string method_url = resources::kraken::KRAKEN_API_URI + path;
  curl_easy_setopt(curl, CURLOPT_URL, method_url.c_str());

  std::string post_data =
      resources::kraken::KRAKEN_CURRENCY_PAIR_KEYWORD + resources::symbols::EQUAL +
      krakenCurrency_.getKrakenPair(fromCurrency, toCurrency) + resources::symbols::AND +
      resources::kraken::KRAKEN_INTERVAL_KEYWORD + resources::symbols::EQUAL +
      common::convertTickInterval(interval, common::StockExchangeType::Kraken);

  curl_easy_setopt(curl, CURLOPT_POSTFIELDS, post_data.c_str());
  curl_easy_setopt(curl, CURLOPT_HTTPHEADER, NULL);

  std::string response = sendRequest(curl);

  JSON::Object::Ptr jsonMainObject = getJsonObjectAndCheckOnIncorrectJson(response, nullptr);
  checkKrakenResponseMessage(jsonMainObject, nullptr);

  auto marketHistoryData = parseMarketHistory(response);
  marketHistoryData->toSell_ = fromCurrency;
  marketHistoryData->toBuy_ = toCurrency;

  return marketHistoryData;
}

std::vector<common::MarketOrder> KrakenQuery::getMarketOpenOrders(
    common::Currency::Enum fromCurrency, common::Currency::Enum toCurrency) {
  using namespace Poco;

  CURL* curl = curl_easy_init();
  checkCurl(curl);

  std::string path = resources::kraken::KRAKEN_SLASH_ZERO_SLASH_REQUEST +
                     resources::kraken::KRAKEN_PUBLIC_METHODS + resources::symbols::SLASH +
                     resources::kraken::KRAKEN_MARKET_OPENED_ORDERS;

  std::string method_url = resources::kraken::KRAKEN_API_URI + path;
  curl_easy_setopt(curl, CURLOPT_URL, method_url.c_str());

  std::string currencyPair = krakenCurrency_.getKrakenPair(fromCurrency, toCurrency);
  std::string post_data =
      resources::kraken::KRAKEN_CURRENCY_PAIR_KEYWORD + resources::symbols::EQUAL + currencyPair;

  curl_easy_setopt(curl, CURLOPT_POSTFIELDS, post_data.c_str());
  curl_easy_setopt(curl, CURLOPT_HTTPHEADER, NULL);

  std::string response = sendRequest(curl);

  JSON::Object::Ptr jsonMainObject = getJsonObjectAndCheckOnIncorrectJson(response, nullptr);
  checkKrakenResponseMessage(jsonMainObject, nullptr);

  auto resultObject = jsonMainObject->getObject(resources::words::RESULT);
  auto currencyObject = resultObject->getObject(currencyPair);

  auto buyObjects = currencyObject->getArray(resources::words::BIDS);
  auto sellObjects = currencyObject->getArray(resources::words::ASKS);

  std::vector<common::MarketOrder> marketOpenOrders;
  stock_exchange_utils::parseOpenedOrdersForType(
      buyObjects, marketOpenOrders, common::OrderType::BUY, common::StockExchangeType::Kraken);
  stock_exchange_utils::parseOpenedOrdersForType(
      sellObjects, marketOpenOrders, common::OrderType::SELL, common::StockExchangeType::Kraken);

  for (auto& order : marketOpenOrders) {
    order.stockExchangeType_ = common::StockExchangeType::Binance;
    order.fromCurrency_ = fromCurrency;
    order.toCurrency_ = toCurrency;
    order.isCanceled_ = false;
  }

  return marketOpenOrders;
}

common::MarketOrder KrakenQuery::getAccountOrder(common::Currency::Enum fromCurrency,
                                                 common::Currency::Enum toCurrency,
                                                 const std::string& uuid) {
  using namespace Poco;

  auto open_orders = getAccountOpenOrders(fromCurrency, toCurrency);
  auto closed_orders = getAccountClosedOrders(fromCurrency, toCurrency);

  common::MarketOrder orderInfo;

  auto searchingOrderInOpenOrders =
      std::find_if(open_orders.begin(), open_orders.end(),
                   [uuid](const common::MarketOrder& order) { return order.uuid_ == uuid; });

  if (searchingOrderInOpenOrders == open_orders.end()) {
    auto searchingOrderInClosedOrders =
        std::find_if(closed_orders.begin(), closed_orders.end(),
                     [uuid](const common::MarketOrder& order) { return order.uuid_ == uuid; });
    if (searchingOrderInClosedOrders != closed_orders.end()) {
      orderInfo = *searchingOrderInClosedOrders;
    }
  } else {
    orderInfo = *searchingOrderInOpenOrders;
  }

  return orderInfo;
}

common::CurrencyTick KrakenQuery::getCurrencyTick(common::Currency::Enum fromCurrency,
                                                  common::Currency::Enum toCurrency) {
  using namespace Poco;

  CURL* curl = curl_easy_init();
  checkCurl(curl);

  std::string path = resources::kraken::KRAKEN_SLASH_ZERO_SLASH_REQUEST +
                     resources::kraken::KRAKEN_PUBLIC_METHODS + resources::symbols::SLASH +
                     resources::kraken::KRAKEN_TICKER_KEYWORD;

  std::string method_url = resources::kraken::KRAKEN_API_URI + path;
  curl_easy_setopt(curl, CURLOPT_URL, method_url.c_str());

  std::string post_data = resources::kraken::KRAKEN_CURRENCY_PAIR_KEYWORD +
                          resources::symbols::EQUAL +
                          krakenCurrency_.getKrakenPair(fromCurrency, toCurrency);

  curl_easy_setopt(curl, CURLOPT_POSTFIELDS, post_data.c_str());
  curl_easy_setopt(curl, CURLOPT_HTTPHEADER, NULL);

  std::string response = sendRequest(curl);

  JSON::Object::Ptr jsonMainObject = getJsonObjectAndCheckOnIncorrectJson(response, nullptr);
  checkKrakenResponseMessage(jsonMainObject, nullptr);

  auto result = jsonMainObject->getObject(resources::words::RESULT);
  auto resultNameList = result->getNames();
  auto currencyPairData = result->getObject(resultNameList.front());

  common::CurrencyTick currencyTick;
  auto askArray = currencyPairData->getArray(resources::kraken::KRAKEN_PRICE_TICKER_ASK);
  currencyTick.ask_ = askArray->get(resources::kraken::KRAKEN_PRICE_TIKER_INDEX);
  auto bidArray = currencyPairData->getArray(resources::kraken::KRAKEN_PRICE_TICKER_BID);
  currencyTick.bid_ = bidArray->get(resources::kraken::KRAKEN_PRICE_TIKER_INDEX);

  currencyTick.fromCurrency_ = fromCurrency;
  currencyTick.toCurrency_ = toCurrency;

  return currencyTick;
}

std::vector<common::MarketOrder> KrakenQuery::getAccountOpenOrders(
    common::Currency::Enum fromCurrency, common::Currency::Enum toCurrency) {
  using namespace Poco;

  CURL* curl = curl_easy_init();
  checkCurl(curl);

  std::string path = resources::kraken::KRAKEN_SLASH_ZERO_SLASH_REQUEST +
                     resources::kraken::KRAKEN_PRIVATE_METHODS + resources::symbols::SLASH +
                     resources::kraken::KRAKEN_OPEN_ORDERS_KEYWORD;

  std::string method_url = resources::kraken::KRAKEN_API_URI + path;
  curl_easy_setopt(curl, CURLOPT_URL, method_url.c_str());

  std::string nonce = std::to_string(common::getCurrentMSEpoch());
  std::string post_data = resources::words::NONCE + resources::symbols::EQUAL + nonce;

  curl_easy_setopt(curl, CURLOPT_POSTFIELDS, post_data.c_str());

  auto signature = common::generateSignatureSHA512(path, nonce, post_data, secret_key_);
  std::string api_key_header = resources::kraken::KRAKEN_API_KEY_HEADER + ":" + api_key_;
  std::string signature_header = resources::kraken::KRAKEN_PRIVATE_KEY_HEADER + ":" + signature;

  curl_slist* chunk = nullptr;
  chunk = curl_slist_append(chunk, api_key_header.c_str());
  chunk = curl_slist_append(chunk, signature_header.c_str());

  curl_easy_setopt(curl, CURLOPT_HTTPHEADER, chunk);

  std::string response = sendRequest(curl);

  JSON::Object::Ptr jsonMainObject = getJsonObjectAndCheckOnIncorrectJson(response, chunk);
  checkKrakenResponseMessage(jsonMainObject, chunk);

  curl_slist_free_all(chunk);

  std::vector<common::MarketOrder> openOrders =
      parseOrderList(jsonMainObject, resources::kraken::KRAKEN_GET_OPENED_ORDERS_LIST_KEYWORD,
                     fromCurrency, toCurrency);

  return openOrders;
}

double KrakenQuery::getBalance(common::Currency::Enum currency) {
  using namespace Poco;

  CURL* curl = curl_easy_init();
  checkCurl(curl);

  std::string path = resources::kraken::KRAKEN_SLASH_ZERO_SLASH_REQUEST +
                     resources::kraken::KRAKEN_PRIVATE_METHODS + resources::symbols::SLASH +
                     resources::kraken::KRAKEN_ACCOUNT_BALANCE;

  std::string method_url = resources::kraken::KRAKEN_API_URI + path;
  curl_easy_setopt(curl, CURLOPT_URL, method_url.c_str());

  std::string nonce = std::to_string(common::getCurrentMSEpoch());
  std::string post_data = resources::words::NONCE + resources::symbols::EQUAL + nonce;

  curl_easy_setopt(curl, CURLOPT_POSTFIELDS, post_data.c_str());

  auto signature = common::generateSignatureSHA512(path, nonce, post_data, secret_key_);
  std::string api_key_header = resources::kraken::KRAKEN_API_KEY_HEADER + ":" + api_key_;
  std::string signature_header = resources::kraken::KRAKEN_PRIVATE_KEY_HEADER + ":" + signature;

  curl_slist* chunk = nullptr;
  chunk = curl_slist_append(chunk, api_key_header.c_str());
  chunk = curl_slist_append(chunk, signature_header.c_str());

  curl_easy_setopt(curl, CURLOPT_HTTPHEADER, chunk);

  std::string response = sendRequest(curl);

  JSON::Object::Ptr jsonMainObject = getJsonObjectAndCheckOnIncorrectJson(response, chunk);
  checkKrakenResponseMessage(jsonMainObject, chunk);

  curl_slist_free_all(chunk);

  auto resultObject = jsonMainObject->getObject(resources::words::RESULT);
  auto coinsWithNonZeroBalance =
      resultObject->optValue(stock_exchange_utils::getKrakenCurrencyStringFromEnum(currency), 0.0);

  return coinsWithNonZeroBalance;
}

CurrencyLotsHolder KrakenQuery::getCurrencyLotsHolder() {
  CurrencyLotsHolder lotsSizes;
  return lotsSizes;
}

common::MarketHistoryPtr KrakenQuery::parseMarketHistory(const std::string& response) const {
  using namespace Poco;

  JSON::Parser parser;
  JSON::Object::Ptr object = parser.parse(response).extract<JSON::Object::Ptr>();

  auto result = object->getObject(resources::words::RESULT);
  auto resultNamesList = result->getNames();
  auto marketHistory = std::make_unique<common::MarketHistory>();

  JSON::Array::Ptr candlesticks = result->getArray(resultNamesList.at(0));
  for (unsigned int index = 0; index < candlesticks->size(); ++index) {
    auto marketObject = candlesticks->getArray(index);

    common::MarketData marketData;
    auto timestamp = marketObject->get(resources::kraken::KRAKEN_TIME_INDEX);
    marketData.date_ = stock_exchange_utils::getDataFromTimestamp(timestamp, false);
    marketData.openPrice_ = marketObject->get(resources::kraken::KRAKEN_OPEN_PRICE_INDEX);
    marketData.closePrice_ = marketObject->get(resources::kraken::KRAKEN_CLOSE_PRICE_INDEX);
    marketData.lowPrice_ = marketObject->get(resources::kraken::KRAKEN_LOW_PRICE_INDEX);
    marketData.highPrice_ = marketObject->get(resources::kraken::KRAKEN_HIGH_PRICE_INDEX);
    marketData.volume_ = marketObject->get(resources::kraken::KRAKEN_VOLUME_INDEX);

    marketHistory->marketData_.push_back(marketData);
  }

  return marketHistory;
}

std::vector<common::MarketOrder> KrakenQuery::getAccountClosedOrders(
    common::Currency::Enum fromCurrency, common::Currency::Enum toCurrency) {
  using namespace Poco;

  CURL* curl = curl_easy_init();
  checkCurl(curl);

  std::string path = resources::kraken::KRAKEN_SLASH_ZERO_SLASH_REQUEST +
                     resources::kraken::KRAKEN_PRIVATE_METHODS + resources::symbols::SLASH +
                     resources::kraken::KRAKEN_GET_CLOSED_ORDERS_KEYWORD;

  std::string method_url = resources::kraken::KRAKEN_API_URI + path;
  curl_easy_setopt(curl, CURLOPT_URL, method_url.c_str());

  std::string nonce = std::to_string(common::getCurrentMSEpoch());
  std::string post_data = resources::words::NONCE + resources::symbols::EQUAL + nonce;

  curl_easy_setopt(curl, CURLOPT_POSTFIELDS, post_data.c_str());

  auto signature = common::generateSignatureSHA512(path, nonce, post_data, secret_key_);
  std::string api_key_header = resources::kraken::KRAKEN_API_KEY_HEADER + ":" + api_key_;
  std::string signature_header = resources::kraken::KRAKEN_PRIVATE_KEY_HEADER + ":" + signature;

  curl_slist* chunk = nullptr;
  chunk = curl_slist_append(chunk, api_key_header.c_str());
  chunk = curl_slist_append(chunk, signature_header.c_str());

  curl_easy_setopt(curl, CURLOPT_HTTPHEADER, chunk);

  std::string response = sendRequest(curl);

  JSON::Object::Ptr jsonMainObject = getJsonObjectAndCheckOnIncorrectJson(response, chunk);
  checkKrakenResponseMessage(jsonMainObject, chunk);

  curl_slist_free_all(chunk);

  std::vector<common::MarketOrder> closedOrders =
      parseOrderList(jsonMainObject, resources::kraken::KRAKEN_GET_CLOSED_ORDERS_LIST_KEYWORD,
                     fromCurrency, toCurrency);

  return closedOrders;
}

std::vector<common::MarketOrder> KrakenQuery::parseOrderList(
    const Poco::JSON::Object::Ptr jsonMainObject, const std::string& ordersType,
    common::Currency::Enum fromCurrency, common::Currency::Enum toCurrency) {
  std::vector<common::MarketOrder> orders;

  auto result = jsonMainObject->getObject(resources::words::RESULT);
  auto openJsonBlock = result->getObject(ordersType);
  auto ordersList = openJsonBlock->getNames();

  for (int i = 0; i < ordersList.size(); ++i) {
    auto orderInfo = openJsonBlock->getObject(ordersList.at(i));
    auto orderDescription = orderInfo->getObject(resources::kraken::KRAKEN_OPEN_ORDER_DESCRIPTION);

    common::OrderType order_type = convertOrderTypeToEnum(
        orderDescription->get(resources::kraken::KRAKEN_ORDER_TYPE).toString());
    common::MarketOrder currentOrder;

    auto currency_pair =
        orderDescription->get(resources::kraken::KRAKEN_CURRENCY_PAIR_KEYWORD).toString();
    auto pair = stock_exchange_utils::parseKrakenCyrrencyPair(currency_pair);
    currentOrder.uuid_ = ordersList.at(i);
    currentOrder.fromCurrency_ = common::Currency::fromString(pair.first);
    currentOrder.toCurrency_ = common::Currency::fromString(pair.second);

    if (currentOrder.fromCurrency_ != fromCurrency || currentOrder.toCurrency_ != toCurrency) {
      continue;
    }
    currentOrder.orderType_ = order_type;

    currentOrder.stockExchangeType_ = common::StockExchangeType::Kraken;
    currentOrder.quantity_ =
        std::stod(orderInfo->get(resources::kraken::KRAKEN_OPEN_ORDER_QUANTITY).toString());
    currentOrder.price_ = std::stod(orderDescription->get(resources::words::PRICE).toString());

    auto status = orderInfo->get(resources::kraken::KRAKEN_ORDER_STATUS_KEYWORD).toString();
    currentOrder.isCanceled_ =
        status == resources::kraken::KRAKEN_ORDER_CANCELED_STATUS ? true : false;

    auto open_time =
        std::stoi(orderInfo->get(resources::kraken::KRAKEN_ORDER_OPEN_TIME_KEYWORD).toString());
    currentOrder.opened_ = stock_exchange_utils::getDataFromTimestamp(open_time, false);

    orders.emplace_back(currentOrder);
  }

  return orders;
}

void KrakenQuery::checkCurl(CURL* curl) {
  if (curl) {
    curl_easy_setopt(curl, CURLOPT_VERBOSE, 0L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 2L);
    curl_easy_setopt(curl, CURLOPT_POST, 1L);

    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCurlSize);
  } else {
    throw common::exceptions::BaseException("Can't create curl handle");
  }
}

std::string KrakenQuery::sendRequest(CURL* curl) {
  std::string response;
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, static_cast<void*>(&response));

  CURLcode responseResult = curl_easy_perform(curl);
  curl_easy_cleanup(curl);

  return response;
}

}  // namespace stock_exchange
}  // namespace auto_trader
