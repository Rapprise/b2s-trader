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

#include "include/binance_query.h"

#include <Poco/JSON/Parser.h>
#include <Poco/Net/HTTPRequest.h>
#include <Poco/Path.h>
#include <Poco/URI.h>

#include <typeinfo>

#include "common/exceptions/stock_exchange_exception/invalid_stock_exchange_response_exception.h"
#include "common/loggers/file_logger.h"
#include "common/utils.h"
#include "include/stock_exchange_utils.h"
#include "resources/resources.h"

namespace auto_trader {
namespace stock_exchange {

static void checkBinanceResponseMessage(Poco::JSON::Object::Ptr& object) {
  auto messageObject = object->get(resources::words::MESSAGE_SHORT);
  auto code = object->get(resources::words::CODE);
  if (code < 0) {
    const std::string message = messageObject.toString();
    common::loggers::FileLogger::getLogger()
        << resources::messages::FAILED_TO_UPLOAD_MESSAGE << message;
    throw common::exceptions::InvalidStockExchangeResponse(message);
  }
}

common::MarketOrder BinanceQuery::sellOrder(common::Currency::Enum fromCurrency,
                                            common::Currency::Enum toCurrency, double quantity,
                                            double rate) {
  using namespace Poco;

  std::string query =
      resources::words::SYMBOL + resources::symbols::EQUAL +
      binanceCurrency_.getBinancePair(fromCurrency, toCurrency) + resources::symbols::AND +
      resources::binance::BINANCE_SIDE + resources::symbols::EQUAL + resources::words::SELL_SIDE +
      resources::symbols::AND + resources::binance::BINANCE_ORDER_TYPE_NAME +
      resources::symbols::EQUAL + resources::binance::BINANCE_ORDER_TYPE_LIMIT +
      resources::symbols::AND + resources::words::QUANTITY + resources::symbols::EQUAL +
      common::MarketOrder::convertCoinToString(quantity) + resources::symbols::AND +
      resources::words::PRICE + resources::symbols::EQUAL +
      common::MarketOrder::convertCoinToString(rate) + resources::symbols::AND +
      resources::binance::BINANCE_TIME_IN_FORCE + resources::symbols::EQUAL +
      resources::binance::BINANCE_GTC + resources::symbols::AND + resources::words::TIMESTAMP +
      resources::symbols::EQUAL + std::to_string(getCurrentServerTime()) + resources::symbols::AND +
      resources::binance::BINANCE_RECIEVE_WINDOW + resources::symbols::EQUAL +
      std::to_string(resources::binance::BINANCE_RECIEVE_WINDOW_VALUE);

  Poco::HMACEngine<common::EncryptionSHA256Engine> hmac{secret_key_};
  hmac.update(query);

  std::string signature = DigestEngine::digestToHex(hmac.digest());
  std::string request_str = resources::binance::BINANCE_URL + resources::binance::BINANCE_ORDER +
                            query + resources::symbols::AND +
                            resources::binance::BINANCE_SIGNATURE + resources::symbols::EQUAL +
                            signature;

  Poco::URI uri(request_str);
  auto path = uri.getPathAndQuery();
  path = path.empty() ? resources::symbols::SLASH : path;
  Net::HTTPRequest request(Net::HTTPRequest::HTTP_POST, path, Net::HTTPMessage::HTTP_1_1);

  ConnectionAttributes attributes;
  attributes.host_ = uri.getHost();
  attributes.port_ = uri.getPort();
  std::vector<HTTP_HEADERS> headers{
      std::make_pair(resources::binance::BINANCE_X_MBX_APIKEY, api_key_)};

  auto response = processHttpRequest(attributes, request, headers);

  JSON::Parser parser;
  JSON::Object::Ptr jsonMainObject = parser.parse(response).extract<JSON::Object::Ptr>();
  checkBinanceResponseMessage(jsonMainObject);

  std::string clientOrderId = jsonMainObject->get(resources::binance::BINANCE_CLIENT_ORDER_ID);
  common::MarketOrder currentOrder;

  currentOrder.fromCurrency_ = fromCurrency;
  currentOrder.toCurrency_ = toCurrency;
  currentOrder.uuid_ = clientOrderId;
  currentOrder.isCanceled_ = false;
  currentOrder.orderType_ = common::OrderType::SELL;
  currentOrder.quantity_ = quantity;
  currentOrder.price_ = rate;
  currentOrder.opened_ = common::Date::getCurrentTime();
  currentOrder.stockExchangeType_ = common::StockExchangeType::Binance;

  return currentOrder;
}

common::MarketOrder BinanceQuery::buyOrder(common::Currency::Enum fromCurrency,
                                           common::Currency::Enum toCurrency, double quantity,
                                           double rate) {
  using namespace Poco;

  std::string query =
      resources::words::SYMBOL + resources::symbols::EQUAL +
      binanceCurrency_.getBinancePair(fromCurrency, toCurrency) + resources::symbols::AND +
      resources::binance::BINANCE_SIDE + resources::symbols::EQUAL + resources::words::BUY_SIDE +
      resources::symbols::AND + resources::binance::BINANCE_ORDER_TYPE_NAME +
      resources::symbols::EQUAL + resources::binance::BINANCE_ORDER_TYPE_LIMIT +
      resources::symbols::AND + resources::words::QUANTITY + resources::symbols::EQUAL +
      common::MarketOrder::convertCoinToString(quantity) + resources::symbols::AND +
      resources::words::PRICE + resources::symbols::EQUAL +
      common::MarketOrder::convertCoinToString(rate) + resources::symbols::AND +
      resources::binance::BINANCE_TIME_IN_FORCE + resources::symbols::EQUAL +
      resources::binance::BINANCE_GTC + resources::symbols::AND + resources::words::TIMESTAMP +
      resources::symbols::EQUAL + std::to_string(getCurrentServerTime()) + resources::symbols::AND +
      resources::binance::BINANCE_RECIEVE_WINDOW + resources::symbols::EQUAL +
      std::to_string(resources::binance::BINANCE_RECIEVE_WINDOW_VALUE);

  Poco::HMACEngine<common::EncryptionSHA256Engine> hmac{secret_key_};
  hmac.update(query);

  std::string signature = DigestEngine::digestToHex(hmac.digest());
  std::string request_str = resources::binance::BINANCE_URL + resources::binance::BINANCE_ORDER +
                            query + resources::symbols::AND +
                            resources::binance::BINANCE_SIGNATURE + resources::symbols::EQUAL +
                            signature;

  Poco::URI uri(request_str);
  auto path = uri.getPathAndQuery();
  path = path.empty() ? resources::symbols::SLASH : path;
  Net::HTTPRequest request(Net::HTTPRequest::HTTP_POST, path, Net::HTTPMessage::HTTP_1_1);

  ConnectionAttributes attributes;
  attributes.host_ = uri.getHost();
  attributes.port_ = uri.getPort();
  std::vector<HTTP_HEADERS> headers{
      std::make_pair(resources::binance::BINANCE_X_MBX_APIKEY, api_key_)};

  auto response = processHttpRequest(attributes, request, headers);

  JSON::Parser parser;
  JSON::Object::Ptr jsonMainObject = parser.parse(response).extract<JSON::Object::Ptr>();
  checkBinanceResponseMessage(jsonMainObject);

  std::string clientOrderId = jsonMainObject->get(resources::binance::BINANCE_CLIENT_ORDER_ID);
  common::MarketOrder currentOrder;

  currentOrder.fromCurrency_ = fromCurrency;
  currentOrder.toCurrency_ = toCurrency;
  currentOrder.uuid_ = clientOrderId;
  currentOrder.isCanceled_ = false;
  currentOrder.orderType_ = common::OrderType::BUY;
  currentOrder.quantity_ = quantity;
  currentOrder.price_ = rate;
  currentOrder.opened_ = common::Date::getCurrentTime();
  currentOrder.stockExchangeType_ = common::StockExchangeType::Binance;

  return currentOrder;
}

bool BinanceQuery::cancelOrder(common::Currency::Enum fromCurrency,
                               common::Currency::Enum toCurrency, const std::string& uuid) {
  using namespace Poco;

  std::string query = resources::words::SYMBOL + resources::symbols::EQUAL +
                      binanceCurrency_.getBinancePair(fromCurrency, toCurrency) +
                      resources::symbols::AND + resources::binance::BINANCE_ORIG_CLIENT_ORDER_ID +
                      resources::symbols::EQUAL + uuid + resources::symbols::AND +
                      resources::words::TIMESTAMP + resources::symbols::EQUAL +
                      std::to_string(getCurrentServerTime()) + resources::symbols::AND +
                      resources::binance::BINANCE_RECIEVE_WINDOW + resources::symbols::EQUAL +
                      std::to_string(resources::binance::BINANCE_RECIEVE_WINDOW_VALUE);

  Poco::HMACEngine<common::EncryptionSHA256Engine> hmac{secret_key_};
  hmac.update(query);

  std::string signature = DigestEngine::digestToHex(hmac.digest());
  std::string request_str = resources::binance::BINANCE_URL +
                            resources::binance::BINANCE_CANCEL_ORDER + query +
                            resources::symbols::AND + resources::binance::BINANCE_SIGNATURE +
                            resources::symbols::EQUAL + signature;

  Poco::URI uri(request_str);
  auto path = uri.getPathAndQuery();
  path = path.empty() ? resources::symbols::SLASH : path;
  Net::HTTPRequest request(Net::HTTPRequest::HTTP_DELETE, path, Net::HTTPMessage::HTTP_1_1);

  ConnectionAttributes attributes;
  attributes.host_ = uri.getHost();
  attributes.port_ = uri.getPort();
  std::vector<HTTP_HEADERS> headers{
      std::make_pair(resources::binance::BINANCE_X_MBX_APIKEY, api_key_)};

  auto response = processHttpRequest(attributes, request, headers);

  JSON::Parser parser;
  JSON::Object::Ptr jsonMainObject = parser.parse(response).extract<JSON::Object::Ptr>();
  checkBinanceResponseMessage(jsonMainObject);

  const std::string status = jsonMainObject->getValue<std::string>(resources::words::STATUS);

  return status == resources::words::CANCELED_UPERCASE;
}

common::MarketHistoryPtr BinanceQuery::getMarketHistory(common::Currency::Enum fromCurrency,
                                                        common::Currency::Enum toCurrency,
                                                        common::TickInterval::Enum interval) {
  using namespace Poco;

  std::string request_str =
      resources::binance::BINANCE_URL + resources::binance::BINANCE_KLINES +
      resources::words::SYMBOL + resources::symbols::EQUAL +
      binanceCurrency_.getBinancePair(fromCurrency, toCurrency) + resources::symbols::AND +
      resources::binance::BINANCE_INTERVAL + resources::symbols::EQUAL +
      common::convertTickInterval(interval, common::StockExchangeType::Binance);

  Poco::URI uri(request_str);
  auto path = uri.getPathAndQuery();
  path = path.empty() ? resources::symbols::SLASH : path;
  Net::HTTPRequest request(Net::HTTPRequest::HTTP_GET, path, Net::HTTPMessage::HTTP_1_1);

  ConnectionAttributes attributes;
  attributes.host_ = uri.getHost();
  attributes.port_ = uri.getPort();
  std::vector<HTTP_HEADERS> headers;

  auto response = processHttpRequest(attributes, request, headers);

  auto marketHistoryData = parseMarketHistory(response);
  marketHistoryData->toBuy_ = toCurrency;
  marketHistoryData->toSell_ = fromCurrency;

  return marketHistoryData;
}

std::vector<common::MarketOrder> BinanceQuery::getMarketOpenOrders(
    common::Currency::Enum fromCurrency, common::Currency::Enum toCurrency) {
  using namespace Poco;

  std::string request_str = resources::binance::BINANCE_URL +
                            resources::binance::BINANCE_TRADE_LIST + resources::words::SYMBOL +
                            resources::symbols::EQUAL +
                            binanceCurrency_.getBinancePair(fromCurrency, toCurrency);

  Poco::URI uri(request_str);
  auto path = uri.getPathAndQuery();
  path = path.empty() ? resources::symbols::SLASH : path;
  Net::HTTPRequest request(Net::HTTPRequest::HTTP_GET, path, Net::HTTPMessage::HTTP_1_1);

  ConnectionAttributes attributes;
  attributes.host_ = uri.getHost();
  attributes.port_ = uri.getPort();
  std::vector<HTTP_HEADERS> headers;

  auto response = processHttpRequest(attributes, request, headers);

  JSON::Parser parser;
  JSON::Object::Ptr objects = parser.parse(response).extract<JSON::Object::Ptr>();
  checkBinanceResponseMessage(objects);

  auto buyObjects = objects->getArray(resources::words::BIDS);
  auto sellObjects = objects->getArray(resources::words::ASKS);

  std::vector<common::MarketOrder> marketOpenOrders;
  stock_exchange_utils::parseOpenedOrdersForType(
      buyObjects, marketOpenOrders, common::OrderType::BUY, common::StockExchangeType::Binance);
  stock_exchange_utils::parseOpenedOrdersForType(
      sellObjects, marketOpenOrders, common::OrderType::SELL, common::StockExchangeType::Binance);

  for (auto& order : marketOpenOrders) {
    order.stockExchangeType_ = common::StockExchangeType::Binance;
    order.fromCurrency_ = fromCurrency;
    order.toCurrency_ = toCurrency;
    order.isCanceled_ = false;
  }

  return marketOpenOrders;
}

common::MarketOrder BinanceQuery::getAccountOrder(common::Currency::Enum fromCurrency,
                                                  common::Currency::Enum toCurrency,
                                                  const std::string& uuid) {
  using namespace Poco;

  std::string query = resources::words::SYMBOL + resources::symbols::EQUAL +
                      binanceCurrency_.getBinancePair(fromCurrency, toCurrency) +
                      resources::symbols::AND + resources::binance::BINANCE_ORIG_CLIENT_ORDER_ID +
                      resources::symbols::EQUAL + uuid + resources::symbols::AND +
                      resources::words::TIMESTAMP + resources::symbols::EQUAL +
                      std::to_string(getCurrentServerTime()) + resources::symbols::AND +
                      resources::binance::BINANCE_RECIEVE_WINDOW + resources::symbols::EQUAL +
                      std::to_string(resources::binance::BINANCE_RECIEVE_WINDOW_VALUE);

  Poco::HMACEngine<common::EncryptionSHA256Engine> hmac{secret_key_};
  hmac.update(query);

  std::string signature = DigestEngine::digestToHex(hmac.digest());
  std::string request_str = resources::binance::BINANCE_URL + resources::binance::BINANCE_ORDER +
                            query + resources::symbols::AND +
                            resources::binance::BINANCE_SIGNATURE + resources::symbols::EQUAL +
                            signature;

  Poco::URI uri(request_str);
  auto path = uri.getPathAndQuery();
  path = path.empty() ? resources::symbols::SLASH : path;
  Net::HTTPRequest request(Net::HTTPRequest::HTTP_GET, path, Net::HTTPMessage::HTTP_1_1);

  ConnectionAttributes attributes;
  attributes.host_ = uri.getHost();
  attributes.port_ = uri.getPort();
  std::vector<HTTP_HEADERS> headers{
      std::make_pair(resources::binance::BINANCE_X_MBX_APIKEY, api_key_)};

  auto response = processHttpRequest(attributes, request, headers);

  JSON::Parser parser;
  JSON::Object::Ptr jsonMainObject = parser.parse(response).extract<JSON::Object::Ptr>();
  checkBinanceResponseMessage(jsonMainObject);

  common::MarketOrder orderInfo;
  orderInfo.uuid_ = jsonMainObject->get(resources::binance::BINANCE_CLIENT_ORDER_ID).toString();
  orderInfo.price_ = jsonMainObject->get(resources::binance::BINANCE_CURRENCY_PRICE_FIELD);
  orderInfo.quantity_ = jsonMainObject->get(resources::binance::BINANCE_ORDER_ORIG_QUANTITY);
  orderInfo.stockExchangeType_ = common::StockExchangeType::Binance;
  orderInfo.fromCurrency_ = fromCurrency;
  orderInfo.toCurrency_ = toCurrency;

  time_t timestamp = jsonMainObject->get(resources::words::TIME);
  orderInfo.opened_ = stock_exchange_utils::getDataFromTimestamp(timestamp, true);

  auto order_type_str = jsonMainObject->get(resources::binance::BINANCE_ORDER_TYPE).toString();
  orderInfo.orderType_ = common::parseBinanceOrderType(order_type_str);

  auto status = jsonMainObject->get(resources::words::STATUS).toString();
  if (status == resources::words::CANCELED_UPERCASE)
    orderInfo.isCanceled_ = true;
  else
    orderInfo.isCanceled_ = false;

  return orderInfo;
}

common::CurrencyTick BinanceQuery::getCurrencyTick(common::Currency::Enum fromCurrency,
                                                   common::Currency::Enum toCurrency) {
  using namespace Poco;

  std::string request_str = resources::binance::BINANCE_URL +
                            resources::binance::BINANCE_CURRENTY_TICK + resources::words::SYMBOL +
                            resources::symbols::EQUAL +
                            binanceCurrency_.getBinancePair(fromCurrency, toCurrency);

  Poco::URI uri(request_str);
  auto path = uri.getPathAndQuery();
  path = path.empty() ? resources::symbols::SLASH : path;
  Net::HTTPRequest request(Net::HTTPRequest::HTTP_GET, path, Net::HTTPMessage::HTTP_1_1);

  ConnectionAttributes attributes;
  attributes.host_ = uri.getHost();
  attributes.port_ = uri.getPort();
  std::vector<HTTP_HEADERS> headers;

  auto response = processHttpRequest(attributes, request, headers);

  JSON::Parser parser;
  JSON::Object::Ptr object = parser.parse(response).extract<JSON::Object::Ptr>();
  checkBinanceResponseMessage(object);

  common::CurrencyTick currencyTick;

  currencyTick.bid_ = object->get(resources::binance::BINANCE_BID_PRICE_KEYWORD);
  currencyTick.ask_ = object->get(resources::binance::BINANCE_ASK_PRICE_KEYWORD);

  currencyTick.fromCurrency_ = fromCurrency;
  currencyTick.toCurrency_ = toCurrency;

  return currencyTick;
}

std::vector<common::MarketOrder> BinanceQuery::getAccountOpenOrders(
    common::Currency::Enum fromCurrency, common::Currency::Enum toCurrency) {
  using namespace Poco;

  std::string query = resources::words::TIMESTAMP + resources::symbols::EQUAL +
                      std::to_string(getCurrentServerTime()) + resources::symbols::AND +
                      resources::binance::BINANCE_RECIEVE_WINDOW + resources::symbols::EQUAL +
                      std::to_string(resources::binance::BINANCE_RECIEVE_WINDOW_VALUE);

  Poco::HMACEngine<common::EncryptionSHA256Engine> hmac{secret_key_};
  hmac.update(query);
  std::string signature = DigestEngine::digestToHex(hmac.digest());

  std::string request_str = resources::binance::BINANCE_URL +
                            resources::binance::BINANCE_OPEN_ORDERS + query +
                            resources::symbols::AND + resources::binance::BINANCE_SIGNATURE +
                            resources::symbols::EQUAL + signature;

  Poco::URI uri(request_str);
  auto path = uri.getPathAndQuery();
  path = path.empty() ? resources::symbols::SLASH : path;
  Net::HTTPRequest request(Net::HTTPRequest::HTTP_GET, path, Net::HTTPMessage::HTTP_1_1);

  ConnectionAttributes attributes;
  attributes.host_ = uri.getHost();
  attributes.port_ = uri.getPort();
  std::vector<HTTP_HEADERS> headers{
      std::make_pair(resources::binance::BINANCE_X_MBX_APIKEY, api_key_)};

  auto response = processHttpRequest(attributes, request, headers);

  JSON::Parser parser;
  JSON::Array::Ptr objects;
  try {
    objects = parser.parse(response).extract<JSON::Array::Ptr>();
  } catch (const std::exception& ex) {
    JSON::Object::Ptr object = parser.parse(response).extract<JSON::Object::Ptr>();
    checkBinanceResponseMessage(object);
  }

  std::vector<common::MarketOrder> openOrders;
  for (unsigned int index = 0; index < objects->size(); ++index) {
    auto object = objects->getObject(index);

    common::MarketOrder currentOrder;
    auto order_type_str = object->get(resources::binance::BINANCE_ORDER_TYPE).toString();
    currentOrder.orderType_ = common::parseBinanceOrderType(order_type_str);

    const std::string type = object->get(resources::words::SYMBOL);
    auto pairType = stock_exchange_utils::parseBinanceExchangeType(type);
    currentOrder.fromCurrency_ = common::Currency::fromString(pairType.first);
    currentOrder.toCurrency_ = common::Currency::fromString(pairType.second);

    if (currentOrder.fromCurrency_ != fromCurrency || currentOrder.toCurrency_ != toCurrency) {
      continue;
    }

    currentOrder.price_ = object->get(resources::words::PRICE);
    currentOrder.uuid_ =
        object->get(resources::binance::BINANCE_CLIENT_ORDER_ID).convert<std::string>();
    currentOrder.quantity_ = object->get(resources::binance::BINANCE_ORDER_ORIG_QUANTITY);
    currentOrder.isCanceled_ = object->get(resources::binance::BINANCE_IS_WORKING_ORDER);
    currentOrder.stockExchangeType_ = common::StockExchangeType::Binance;
    time_t timestamp = object->get(resources::words::TIME);
    currentOrder.opened_ = stock_exchange_utils::getDataFromTimestamp(timestamp, true);

    openOrders.push_back(currentOrder);
  }

  return openOrders;
}

double BinanceQuery::getBalance(common::Currency::Enum currency) {
  using namespace Poco;

  std::string query = resources::words::TIMESTAMP + resources::symbols::EQUAL +
                      std::to_string(getCurrentServerTime());

  Poco::HMACEngine<common::EncryptionSHA256Engine> hmac{secret_key_};
  hmac.update(query);
  std::string signature = DigestEngine::digestToHex(hmac.digest());

  std::string request_str = resources::binance::BINANCE_URL +
                            resources::binance::BINANCE_ACCOUNT_INFO + query +
                            resources::symbols::AND + resources::binance::BINANCE_SIGNATURE +
                            resources::symbols::EQUAL + signature;

  Poco::URI uri(request_str);
  auto path = uri.getPathAndQuery();
  path = path.empty() ? resources::symbols::SLASH : path;
  Net::HTTPRequest request(Net::HTTPRequest::HTTP_GET, path, Net::HTTPMessage::HTTP_1_1);

  ConnectionAttributes attributes;
  attributes.host_ = uri.getHost();
  attributes.port_ = uri.getPort();
  std::vector<HTTP_HEADERS> headers{
      std::make_pair(resources::binance::BINANCE_X_MBX_APIKEY, api_key_)};

  auto response = processHttpRequest(attributes, request, headers);

  JSON::Parser parser;
  JSON::Object::Ptr object = parser.parse(response).extract<JSON::Object::Ptr>();
  checkBinanceResponseMessage(object);

  double balance = 0.0;
  auto balancesInfoArray = object->getArray(resources::binance::BINANCE_BALANCE_ARRAY_BLOCK);
  for (int i = 0; i < balancesInfoArray->size(); ++i) {
    auto currencyInfoObject = balancesInfoArray->getObject(i);
    std::string currencyName = currencyInfoObject->get(resources::words::BALANCE_ASSET).toString();
    if (currencyName == common::Currency::toString(currency)) {
      std::string free =
          currencyInfoObject->get(resources::binance::BINANCE_CURRENCY_FREE).toString();
      balance = std::stod(free);

      break;
    }
  }

  return balance;
}

CurrencyLotsHolder BinanceQuery::getCurrencyLotsHolder() {
  using namespace Poco;

  std::string request_str =
      resources::binance::BINANCE_URL + resources::binance::BINANCE_EXCHANGE_INFO;

  Poco::URI uri(request_str);
  auto path = uri.getPathAndQuery();
  path = path.empty() ? resources::symbols::SLASH : path;
  Net::HTTPRequest request(Net::HTTPRequest::HTTP_GET, path, Net::HTTPMessage::HTTP_1_1);

  ConnectionAttributes attributes;
  attributes.host_ = uri.getHost();
  attributes.port_ = uri.getPort();
  std::vector<HTTP_HEADERS> headers;

  auto response = processHttpRequest(attributes, request, headers);

  JSON::Parser parser;
  JSON::Object::Ptr object = parser.parse(response).extract<JSON::Object::Ptr>();
  checkBinanceResponseMessage(object);

  auto symbolsArray = object->getArray(resources::binance::BINANCE_EXCHANGE_INFO_SYMBOLS);
  auto arraySize = symbolsArray->size();

  CurrencyLotsHolder lotsSizes;

  for (int mainIndex = 0; mainIndex < arraySize; ++mainIndex) {
    auto symbol_info = symbolsArray->getObject(mainIndex);
    std::string currencyPair = symbol_info->get(resources::words::SYMBOL).toString();

    auto filters = symbol_info->getArray(resources::binance::BINANCE_EXCHANGE_INFO_FILTERS);
    auto filtersBlockSize = filters->size();
    for (int filtersIndex = 0; filtersIndex < filtersBlockSize; ++filtersIndex) {
      auto filterType = filters->getObject(filtersIndex);
      std::string filterTypeName =
          filterType->get(resources::binance::BINANCE_EXCHANGE_INFO_FILTER_TYPE_KEYWORD).toString();
      if (filterTypeName == resources::binance::BINANCE_LOT_SIZE_KEYWORD) {
        common::LotSize lotSize;
        lotSize.minQty_ =
            filterType->get(resources::binance::BINANCE_LOT_SIZE_MIN_QUANTITY_KEYWORD);
        lotSize.maxQty_ =
            filterType->get(resources::binance::BINANCE_LOT_SIZE_MAX_QUANTITY_KEYWORD);
        lotSize.stepSize_ = filterType->get(resources::binance::BINANCE_LOT_SIZE_STEP_SIZE_KEYWORD);

        lotsSizes.addLot(currencyPair, lotSize);
      }
    }
  }

  return lotsSizes;
}

uint64_t BinanceQuery::getCurrentServerTime() {
  using namespace Poco;

  std::string request_str = resources::binance::BINANCE_URL + resources::binance::BINANCE_TIME;

  Poco::URI uri(request_str);
  auto path = uri.getPathAndQuery();
  path = path.empty() ? resources::symbols::SLASH : path;
  Net::HTTPRequest request(Net::HTTPRequest::HTTP_GET, path, Net::HTTPMessage::HTTP_1_1);

  ConnectionAttributes attributes;
  attributes.host_ = uri.getHost();
  attributes.port_ = uri.getPort();
  std::vector<HTTP_HEADERS> headers;

  auto response = processHttpRequest(attributes, request, headers);

  JSON::Parser parser;
  JSON::Object::Ptr object = parser.parse(response).extract<JSON::Object::Ptr>();
  checkBinanceResponseMessage(object);

  uint64_t serverTime = object->get(resources::binance::BINANCE_SERVER_TIME_VALUE);
  return serverTime;
}

common::MarketHistoryPtr BinanceQuery::parseMarketHistory(const std::string& response) const {
  using namespace Poco;

  JSON::Parser parser;
  JSON::Array::Ptr objects;

  try {
    objects = parser.parse(response).extract<JSON::Array::Ptr>();
  } catch (const std::exception& ex) {
    JSON::Object::Ptr object = parser.parse(response).extract<JSON::Object::Ptr>();
    checkBinanceResponseMessage(object);
  }

  auto marketHistory = std::make_unique<common::MarketHistory>();
  for (unsigned int index = 0; index < objects->size(); ++index) {
    auto object = objects->getArray(index);

    common::MarketData marketData;
    time_t timestamp = object->get(resources::binance::BINANCE_TIMESTAMP_INDEX);
    marketData.date_ = stock_exchange_utils::getDataFromTimestamp(timestamp, true);
    marketData.openPrice_ = object->get(resources::binance::BINANCE_OPEN_PRICE_INDEX);
    marketData.closePrice_ = object->get(resources::binance::BINANCE_CLOSE_PRICE_INDEX);
    marketData.lowPrice_ = object->get(resources::binance::BINANCE_LOW_PRICE_INDEX);
    marketData.highPrice_ = object->get(resources::binance::BINANCE_HIGH_PRICE_INDEX);
    marketData.volume_ = object->get(resources::binance::BINANCE_VOLUME_INDEX);

    marketHistory->marketData_.push_back(marketData);
  }

  return marketHistory;
}

}  // namespace stock_exchange
}  // namespace auto_trader
