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

#include "include/bittrex_query.h"

#include <Poco/JSON/Parser.h>
#include <Poco/Net/HTTPRequest.h>
#include <Poco/Path.h>
#include <Poco/URI.h>

#include <string>

#include "common/encryption_sha512_engine.h"
#include "common/exceptions/no_data_found_exception.h"
#include "common/exceptions/stock_exchange_exception/invalid_stock_exchange_response_exception.h"
#include "common/exceptions/stock_exchange_exception/redirect_http_exception.h"
#include "common/loggers/file_logger.h"
#include "common/utils.h"
#include "resources/resources.h"

namespace auto_trader {
namespace stock_exchange {

static void fillMarketOpenOrdersInfo(const Poco::JSON::Array::Ptr objects,
                                     std::vector<common::MarketOrder>& fillingObject,
                                     common::OrderType type) {
  for (int index = 0; index < objects->size(); ++index) {
    auto object = objects->getObject(index);

    common::MarketOrder currentOrder;
    currentOrder.orderType_ = type;
    currentOrder.stockExchangeType_ = common::StockExchangeType::Bittrex;
    currentOrder.quantity_ = object->get(resources::bittrex::BITTREX_QUANTITY_KEYWORD);
    currentOrder.price_ = object->get(resources::bittrex::BITTREX_RATE_KEYWORD);

    fillingObject.push_back(currentOrder);
  }
}

static void checkBittrexResponseMessage(Poco::JSON::Object::Ptr& object) {
  if (object->get(resources::words::SUCCESS) != true) {
    const std::string& message = object->get(resources::words::MESSAGE).toString();
    common::loggers::FileLogger::getLogger()
        << resources::messages::FAILED_TO_UPLOAD_MESSAGE << message;
    throw common::exceptions::InvalidStockExchangeResponse(message);
  }
}

static std::pair<std::string, std::string> parseBittrexExchangeType(const std::string& type) {
  auto dashTockenPosition = type.find('-');
  const std::string& fromTypeStr = type.substr(0, dashTockenPosition);
  const std::string& toTypeStr = type.substr(dashTockenPosition + 1);

  return std::make_pair<>(fromTypeStr, toTypeStr);
}

common::MarketOrder BittrexQuery::sellOrder(common::Currency::Enum fromCurrency,
                                            common::Currency::Enum toCurrency, double quantity,
                                            double rate) {
  using namespace Poco;

  auto nonce = std::to_string(common::getCurrentMSEpoch());
  std::string request_str =
      resources::bittrex::BITTREX_URL + resources::bittrex::BITTREX_MARKET_URL_API +
      resources::bittrex::BITTREX_SELL_LIMIT + resources::symbols::QUESTION +
      resources::words::API_KEY + resources::symbols::EQUAL + api_key_ + resources::symbols::AND +
      resources::words::MARKET + resources::symbols::EQUAL +
      bittrexCurrency_.getBittrexPair(fromCurrency, toCurrency) + resources::symbols::AND +
      resources::words::QUANTITY + resources::symbols::EQUAL +
      common::MarketOrder::convertCoinToString(quantity) + resources::symbols::AND +
      resources::words::RATE + resources::symbols::EQUAL +
      common::MarketOrder::convertCoinToString(rate) + resources::symbols::AND +
      resources::words::NONCE + resources::symbols::EQUAL + nonce;

  Poco::HMACEngine<common::EncryptionSHA512Engine> hmac{secret_key_};
  hmac.update(request_str);
  std::string signature = DigestEngine::digestToHex(hmac.digest());

  Poco::URI uri(request_str);
  request_str +=
      resources::symbols::AND + resources::words::HASH + resources::symbols::EQUAL + signature;

  auto path = uri.getPathAndQuery();
  path = path.empty() ? resources::symbols::SLASH : path;
  Net::HTTPRequest request(Net::HTTPRequest::HTTP_GET, path, Net::HTTPMessage::HTTP_1_1);

  ConnectionAttributes attributes;
  attributes.host_ = uri.getHost();
  attributes.port_ = uri.getPort();
  std::vector<HTTP_HEADERS> headers = {
      std::make_pair(resources::bittrex::BITTREX_HEADER_APISIGN, signature)};

  auto response = processHttpRequest(attributes, request, headers);

  JSON::Parser parser;
  JSON::Object::Ptr ret = parser.parse(response).extract<JSON::Object::Ptr>();
  checkBittrexResponseMessage(ret);

  auto result = ret->get(resources::words::RESULT);
  JSON::Object::Ptr object = result.extract<Poco::JSON::Object::Ptr>();

  std::string uuid = object->get(resources::words::UUID);

  common::MarketOrder currentOrder;
  currentOrder.fromCurrency_ = fromCurrency;
  currentOrder.toCurrency_ = toCurrency;
  currentOrder.uuid_ = uuid;
  currentOrder.isCanceled_ = false;
  currentOrder.orderType_ = common::OrderType::SELL;
  currentOrder.quantity_ = quantity;
  currentOrder.price_ = rate;
  currentOrder.stockExchangeType_ = common::StockExchangeType::Bittrex;
  currentOrder.opened_ = common::Date::getCurrentTime();

  return currentOrder;
}

common::MarketOrder BittrexQuery::buyOrder(common::Currency::Enum fromCurrency,
                                           common::Currency::Enum toCurrency, double quantity,
                                           double rate) {
  using namespace Poco;

  auto nonce = std::to_string(common::getCurrentMSEpoch());
  std::string request_str =
      resources::bittrex::BITTREX_URL + resources::bittrex::BITTREX_MARKET_URL_API +
      resources::bittrex::BITTREX_BUY_LIMIT + resources::symbols::QUESTION +
      resources::words::API_KEY + resources::symbols::EQUAL + api_key_ + resources::symbols::AND +
      resources::words::MARKET + resources::symbols::EQUAL +
      bittrexCurrency_.getBittrexPair(fromCurrency, toCurrency) + resources::symbols::AND +
      resources::words::QUANTITY + resources::symbols::EQUAL +
      common::MarketOrder::convertCoinToString(quantity) + resources::symbols::AND +
      resources::words::RATE + resources::symbols::EQUAL +
      common::MarketOrder::convertCoinToString(rate) + resources::symbols::AND +
      resources::words::NONCE + resources::symbols::EQUAL + nonce;

  Poco::HMACEngine<common::EncryptionSHA512Engine> hmac{secret_key_};
  hmac.update(request_str);
  std::string signature = DigestEngine::digestToHex(hmac.digest());

  Poco::URI uri(request_str);
  request_str +=
      resources::symbols::AND + resources::words::HASH + resources::symbols::EQUAL + signature;

  auto path = uri.getPathAndQuery();
  path = path.empty() ? resources::symbols::SLASH : path;
  Net::HTTPRequest request(Net::HTTPRequest::HTTP_GET, path, Net::HTTPMessage::HTTP_1_1);

  ConnectionAttributes attributes;
  attributes.host_ = uri.getHost();
  attributes.port_ = uri.getPort();
  std::vector<HTTP_HEADERS> headers = {
      std::make_pair(resources::bittrex::BITTREX_HEADER_APISIGN, signature)};

  auto response = processHttpRequest(attributes, request, headers);

  JSON::Parser parser;
  JSON::Object::Ptr ret = parser.parse(response).extract<JSON::Object::Ptr>();
  checkBittrexResponseMessage(ret);

  auto result = ret->get(resources::words::RESULT);
  JSON::Object::Ptr object = result.extract<Poco::JSON::Object::Ptr>();

  std::string uuid = object->get(resources::words::UUID);

  common::MarketOrder currentOrder;
  currentOrder.fromCurrency_ = fromCurrency;
  currentOrder.toCurrency_ = toCurrency;
  currentOrder.uuid_ = uuid;
  currentOrder.isCanceled_ = false;
  currentOrder.orderType_ = common::OrderType::BUY;
  currentOrder.quantity_ = quantity;
  currentOrder.price_ = rate;
  currentOrder.stockExchangeType_ = common::StockExchangeType::Bittrex;
  currentOrder.opened_ = common::Date::getCurrentTime();

  return currentOrder;
}

common::MarketHistoryPtr BittrexQuery::parseMarketHistory(const std::string& response) const {
  using namespace Poco;

  JSON::Parser parser;
  JSON::Object::Ptr ret = parser.parse(response).extract<JSON::Object::Ptr>();
  checkBittrexResponseMessage(ret);

  auto result = ret->get(resources::words::RESULT);
  auto marketHistory = std::make_unique<common::MarketHistory>();
  Poco::JSON::Array::Ptr objects = result.extract<Poco::JSON::Array::Ptr>();

  for (unsigned int index = 0; index < objects->size(); ++index) {
    JSON::Object::Ptr object = objects->get(index).extract<Poco::JSON::Object::Ptr>();

    common::Date date =
        common::Date::parseDate(object->get(resources::bittrex::MARKET_HISTORY_TIMESTAMP));
    std::string closePrice = object->get(resources::bittrex::MARKET_HISTORY_CLOSE_POSITION);
    std::string openPrice = object->get(resources::bittrex::MARKET_HISTORY_OPEN_POSITION);
    std::string highPrice = object->get(resources::bittrex::MARKET_HISTORY_HIGH_POSITION);
    std::string lowPrice = object->get(resources::bittrex::MARKET_HISTORY_LOW_POSITION);
    std::string volume = object->get(resources::bittrex::MARKET_HISTORY_VOLUME);

    common::MarketData marketData;
    std::string::size_type sizeType;
    marketData.date_ = date;
    marketData.closePrice_ = std::stod(closePrice, &sizeType);
    marketData.openPrice_ = std::stod(openPrice);
    marketData.highPrice_ = std::stod(highPrice);
    marketData.lowPrice_ = std::stod(lowPrice);
    marketData.volume_ = std::stod(volume);

    marketHistory->marketData_.push_back(marketData);
  }

  return marketHistory;
}

bool BittrexQuery::cancelOrder(common::Currency::Enum fromCurrency,
                               common::Currency::Enum toCurrency, const std::string& uuid) {
  using namespace Poco;

  auto nonce = std::to_string(common::getCurrentMSEpoch());
  std::string request_str =
      resources::bittrex::BITTREX_URL + resources::bittrex::BITTREX_MARKET_URL_API +
      resources::bittrex::BITTREX_CANCEL + resources::symbols::QUESTION +
      resources::words::API_KEY + resources::symbols::EQUAL + api_key_ + resources::symbols::AND +
      resources::words::UUID + resources::symbols::EQUAL + uuid + resources::symbols::AND +
      resources::words::NONCE + resources::symbols::EQUAL + nonce;

  Poco::HMACEngine<common::EncryptionSHA512Engine> hmac{secret_key_};
  hmac.update(request_str);
  std::string signature = DigestEngine::digestToHex(hmac.digest());

  Poco::URI uri(request_str);
  request_str +=
      resources::symbols::AND + resources::words::HASH + resources::symbols::EQUAL + signature;

  auto path = uri.getPathAndQuery();
  path = path.empty() ? resources::symbols::SLASH : path;
  Net::HTTPRequest request(Net::HTTPRequest::HTTP_GET, path, Net::HTTPMessage::HTTP_1_1);

  ConnectionAttributes attributes;
  attributes.host_ = uri.getHost();
  attributes.port_ = uri.getPort();
  std::vector<HTTP_HEADERS> headers = {
      std::make_pair(resources::bittrex::BITTREX_HEADER_APISIGN, signature)};

  auto response = processHttpRequest(attributes, request, headers);

  JSON::Parser parser;
  JSON::Object::Ptr ret = parser.parse(response).extract<JSON::Object::Ptr>();
  checkBittrexResponseMessage(ret);

  bool result = ret->get(resources::words::SUCCESS);

  return result;
}

common::CurrencyTick BittrexQuery::getCurrencyTick(common::Currency::Enum fromCurrency,
                                                   common::Currency::Enum toCurrency) {
  using namespace Poco;

  std::string request_str =
      resources::bittrex::BITTREX_URL + resources::bittrex::BITTREX_PUBLIC_URL_API +
      resources::bittrex::BITTREX_TICKER + resources::symbols::QUESTION + resources::words::MARKET +
      resources::symbols::EQUAL + bittrexCurrency_.getBittrexPair(fromCurrency, toCurrency);

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
  JSON::Object::Ptr ret = parser.parse(response).extract<JSON::Object::Ptr>();
  checkBittrexResponseMessage(ret);

  JSON::Object::Ptr result = ret->getObject(resources::words::RESULT);

  common::CurrencyTick currencyTick;
  currencyTick.ask_ = result->get(resources::bittrex::BITTREX_ASK_TICK);
  currencyTick.bid_ = result->get(resources::bittrex::BITTREX_BID_TICK);

  currencyTick.fromCurrency_ = fromCurrency;
  currencyTick.toCurrency_ = toCurrency;

  return currencyTick;
}

std::vector<common::MarketOrder> BittrexQuery::getAccountOpenOrders(
    common::Currency::Enum fromCurrency, common::Currency::Enum toCurrency) {
  using namespace Poco;

  auto nonce = std::to_string(common::getCurrentMSEpoch());
  std::string request_str = resources::bittrex::BITTREX_URL +
                            resources::bittrex::BITTREX_MARKET_OPEN_ORDERS +
                            resources::symbols::QUESTION + resources::words::API_KEY +
                            resources::symbols::EQUAL + api_key_ + resources::symbols::AND +
                            resources::words::NONCE + resources::symbols::EQUAL + nonce;

  Poco::HMACEngine<common::EncryptionSHA512Engine> hmac{secret_key_};
  hmac.update(request_str);
  std::string signature = DigestEngine::digestToHex(hmac.digest());

  Poco::URI uri(request_str);
  request_str +=
      resources::symbols::AND + resources::words::HASH + resources::symbols::EQUAL + signature;

  auto path = uri.getPathAndQuery();
  path = path.empty() ? resources::symbols::SLASH : path;
  Net::HTTPRequest request(Net::HTTPRequest::HTTP_GET, path, Net::HTTPMessage::HTTP_1_1);

  ConnectionAttributes attributes;
  attributes.host_ = uri.getHost();
  attributes.port_ = uri.getPort();
  std::vector<HTTP_HEADERS> headers = {
      std::make_pair(resources::bittrex::BITTREX_HEADER_APISIGN, signature)};

  auto response = processHttpRequest(attributes, request, headers);

  JSON::Parser parser;
  JSON::Object::Ptr ret = parser.parse(response).extract<JSON::Object::Ptr>();
  checkBittrexResponseMessage(ret);

  auto result = ret->getArray(resources::words::RESULT);
  std::vector<common::MarketOrder> openOrders;

  for (int index = 0; index < result->size(); ++index) {
    auto object = result->getObject(index);

    common::MarketOrder currentOrder;
    const std::string exchange = object->get(resources::bittrex::BITTREX_EXCHANGE_KEYWORD);
    std::pair<std::string, std::string> bittrexExchange = parseBittrexExchangeType(exchange);
    currentOrder.fromCurrency_ = common::Currency::fromString(bittrexExchange.first);
    currentOrder.toCurrency_ = common::Currency::fromString(bittrexExchange.second);

    if (currentOrder.fromCurrency_ != fromCurrency || currentOrder.toCurrency_ != toCurrency) {
      continue;
    }

    currentOrder.uuid_ = object->get(resources::bittrex::BITTREX_ORDER_UUID).toString();
    currentOrder.price_ = object->get(resources::bittrex::BITTREX_PRICE_KEYWORD);
    currentOrder.quantity_ = object->get(resources::bittrex::BITTREX_QUANTITY_KEYWORD);

    currentOrder.stockExchangeType_ = common::StockExchangeType::Bittrex;
    const std::string orderType = object->get(resources::bittrex::BITTREX_ORDER_TYPE_KEYWORD);
    currentOrder.orderType_ = common::parseBittrexOrderType(orderType);
    currentOrder.isCanceled_ = false;

    std::string opened = object->get(resources::bittrex::BITTREX_DATE_OPENED_KEYWORD).toString();
    currentOrder.opened_ = common::Date::parseDate(opened);

    openOrders.push_back(currentOrder);
  }

  return openOrders;
}

common::MarketHistoryPtr BittrexQuery::getMarketHistory(common::Currency::Enum fromCurrency,
                                                        common::Currency::Enum toCurrency,
                                                        common::TickInterval::Enum interval) {
  using namespace Poco;

  auto nonce = std::to_string(common::getCurrentMSEpoch());
  std::string request_str =
      resources::bittrex::BITTREX_ADDRESS + resources::bittrex::BITTREX_API_V_0_MARKET +
      resources::bittrex::BITTREX_GET_TICKS + resources::symbols::UNDER_LINE + nonce +
      resources::symbols::AND + resources::bittrex::BITTREX_MARKET_NAME +
      resources::symbols::EQUAL + bittrexCurrency_.getBittrexPair(fromCurrency, toCurrency) +
      resources::symbols::AND + resources::bittrex::BITTREX_TICK_INTERVAL +
      resources::symbols::EQUAL +
      common::convertTickInterval(interval, common::StockExchangeType::Bittrex);

  std::string response;
  std::vector<HTTP_HEADERS> headers;
  try {
    Poco::URI uri(request_str);
    auto path = uri.getPathAndQuery();
    path = path.empty() ? resources::symbols::SLASH : path;
    Net::HTTPRequest request(Net::HTTPRequest::HTTP_GET, path, Net::HTTPMessage::HTTP_1_1);

    ConnectionAttributes attributes;
    attributes.host_ = uri.getHost();
    attributes.port_ = uri.getPort();

    response = processHttpRequest(attributes, request, headers);
  } catch (common::exceptions::RedirectHttpsException& exception) {
    const std::string updatedRequest = exception.what();
    Poco::URI uri(updatedRequest);
    auto path = uri.getPathAndQuery();
    path = path.empty() ? resources::symbols::SLASH : path;
    Net::HTTPRequest request(Net::HTTPRequest::HTTP_GET, path, Net::HTTPMessage::HTTP_1_1);

    ConnectionAttributes attributes;
    attributes.host_ = uri.getHost();
    attributes.port_ = uri.getPort();
    response = processHttpRequest(attributes, request, headers);
  }

  auto marketHistoryData = parseMarketHistory(response);
  marketHistoryData->toBuy_ = toCurrency;
  marketHistoryData->toSell_ = fromCurrency;

  return marketHistoryData;
}

std::vector<common::MarketOrder> BittrexQuery::getMarketOpenOrders(
    common::Currency::Enum fromCurrency, common::Currency::Enum toCurrency) {
  using namespace Poco;

  std::string request_str =
      resources::bittrex::BITTREX_URL + resources::bittrex::BITTREX_PUBLIC_URL_API +
      resources::bittrex::BITTREX_ORDER_BOOK + resources::symbols::QUESTION +
      resources::words::MARKET + resources::symbols::EQUAL +
      bittrexCurrency_.getBittrexPair(fromCurrency, toCurrency) + resources::symbols::AND +
      resources::words::TYPE + resources::symbols::EQUAL + resources::words::BOTH;

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
  JSON::Object::Ptr ret = parser.parse(response).extract<JSON::Object::Ptr>();
  checkBittrexResponseMessage(ret);

  auto result = ret->getObject(resources::words::RESULT);
  auto buyArray = result->getArray(resources::bittrex::BITTREX_OPEN_ORDERS_BUY_KEYWORD);
  auto sellArray = result->getArray(resources::bittrex::BITTREX_OPEN_ORDERS_SELL_KEYWORD);

  std::vector<common::MarketOrder> openOrders;
  fillMarketOpenOrdersInfo(buyArray, openOrders, common::OrderType::BUY);
  fillMarketOpenOrdersInfo(sellArray, openOrders, common::OrderType::SELL);

  for (auto& order : openOrders) {
    order.fromCurrency_ = fromCurrency;
    order.toCurrency_ = toCurrency;
  }

  return openOrders;
}

common::MarketOrder BittrexQuery::getAccountOrder(common::Currency::Enum fromCurrency,
                                                  common::Currency::Enum toCurrency,
                                                  const std::string& uuid) {
  using namespace Poco;

  auto nonce = std::to_string(common::getCurrentMSEpoch());
  std::string request_str =
      resources::bittrex::BITTREX_URL + resources::bittrex::BITTREX_ACCOUNT_API +
      resources::symbols::SLASH + resources::bittrex::BITTREX_GET_ORDER +
      resources::symbols::QUESTION + resources::words::UUID + resources::symbols::EQUAL + uuid +
      resources::symbols::AND + resources::words::API_KEY + resources::symbols::EQUAL + api_key_ +
      resources::symbols::AND + resources::words::NONCE + resources::symbols::EQUAL + nonce;

  Poco::HMACEngine<common::EncryptionSHA512Engine> hmac{secret_key_};
  hmac.update(request_str);
  std::string signature = DigestEngine::digestToHex(hmac.digest());

  Poco::URI uri(request_str);
  request_str +=
      resources::symbols::AND + resources::words::HASH + resources::symbols::EQUAL + signature;

  auto path = uri.getPathAndQuery();
  path = path.empty() ? resources::symbols::SLASH : path;
  Net::HTTPRequest request(Net::HTTPRequest::HTTP_GET, path, Net::HTTPMessage::HTTP_1_1);

  ConnectionAttributes attributes;
  attributes.host_ = uri.getHost();
  attributes.port_ = uri.getPort();
  std::vector<HTTP_HEADERS> headers = {
      std::make_pair(resources::bittrex::BITTREX_HEADER_APISIGN, signature)};

  auto response = processHttpRequest(attributes, request, headers);

  JSON::Parser parser;
  JSON::Object::Ptr ret = parser.parse(response).extract<JSON::Object::Ptr>();
  checkBittrexResponseMessage(ret);

  common::MarketOrder currentOrder;
  auto resultInfoObject = ret->getObject(resources::words::RESULT);
  if (!resultInfoObject) {
    throw common::exceptions::NoDataFoundException("Out of gate order");
  }

  currentOrder.uuid_ = resultInfoObject->get(resources::bittrex::BITTREX_ORDER_UUID).toString();
  currentOrder.price_ = resultInfoObject->get(resources::bittrex::BITTREX_PRICE_KEYWORD);
  currentOrder.quantity_ = resultInfoObject->get(resources::bittrex::BITTREX_QUANTITY_KEYWORD);
  currentOrder.opened_ = common::Date::parseDate(
      resultInfoObject->get(resources::bittrex::BITTREX_DATE_OPENED_KEYWORD));

  const std::string exchange = resultInfoObject->get(resources::bittrex::BITTREX_EXCHANGE_KEYWORD);
  std::pair<std::string, std::string> bittrexExchange = common::parseBittrexExchangeType(exchange);
  currentOrder.fromCurrency_ = common::Currency::fromString(bittrexExchange.first);
  currentOrder.toCurrency_ = common::Currency::fromString(bittrexExchange.second);

  currentOrder.stockExchangeType_ = common::StockExchangeType::Bittrex;
  const std::string orderType = resultInfoObject->get(resources::bittrex::BITTREX_TYPE_KEYWORD);
  currentOrder.orderType_ = common::parseBittrexOrderType(orderType);
  currentOrder.isCanceled_ =
      resultInfoObject->get(resources::bittrex::BITTREX_IS_CANCEL_ORDER_KEYWORD);

  return currentOrder;
}

double BittrexQuery::getBalance(common::Currency::Enum currency) {
  using namespace Poco;

  auto nonce = std::to_string(common::getCurrentMSEpoch());
  std::string request_str = resources::bittrex::BITTREX_URL +
                            resources::bittrex::BITTREX_ACCOUNT_API + resources::symbols::SLASH +
                            resources::bittrex::BITTREX_GET_BALANCE + resources::symbols::QUESTION +
                            resources::words::API_KEY + resources::symbols::EQUAL + api_key_ +
                            resources::symbols::AND + resources::words::NONCE +
                            resources::symbols::EQUAL + nonce;

  Poco::HMACEngine<common::EncryptionSHA512Engine> hmac{secret_key_};
  hmac.update(request_str);
  std::string signature = DigestEngine::digestToHex(hmac.digest());

  Poco::URI uri(request_str);
  request_str +=
      resources::symbols::AND + resources::words::HASH + resources::symbols::EQUAL + signature;

  auto path = uri.getPathAndQuery();
  path = path.empty() ? resources::symbols::SLASH : path;
  Net::HTTPRequest request(Net::HTTPRequest::HTTP_GET, path, Net::HTTPMessage::HTTP_1_1);

  ConnectionAttributes attributes;
  attributes.host_ = uri.getHost();
  attributes.port_ = uri.getPort();
  std::vector<HTTP_HEADERS> headers = {
      std::make_pair(resources::bittrex::BITTREX_HEADER_APISIGN, signature)};

  auto response = processHttpRequest(attributes, request, headers);

  JSON::Parser parser;
  JSON::Object::Ptr ret = parser.parse(response).extract<JSON::Object::Ptr>();

  double balance = 0.0;
  auto resultInfoArray = ret->getArray(resources::words::RESULT);
  for (int i = 0; i < resultInfoArray->size(); ++i) {
    auto currencyInfoObject = resultInfoArray->getObject(i);
    std::string currencyName =
        currencyInfoObject->get(resources::bittrex::BITTREX_BALANCE_CURRENCY).toString();
    if (currencyName == common::Currency::toString(currency)) {
      std::string balance_str =
          currencyInfoObject->get(resources::bittrex::BITTREX_BALANCE_VALUE).toString();
      balance = std::stod(balance_str);

      break;
    }
  }

  return balance;
}

CurrencyLotsHolder BittrexQuery::getCurrencyLotsHolder() {
  CurrencyLotsHolder lotsSizes;
  return lotsSizes;
}

}  // namespace stock_exchange
}  // namespace auto_trader