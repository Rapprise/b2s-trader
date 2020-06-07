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

#include "include/huobi_query.h"

#include <Poco/JSON/Parser.h>
#include <Poco/Net/HTTPRequest.h>
#include <Poco/Path.h>
#include <Poco/URI.h>

#include <QtCore/QJsonArray>
#include <QtCore/QJsonDocument>
#include <QtCore/QJsonObject>

#include "common/encryption_sha256_engine.h"
#include "common/exceptions/stock_exchange_exception/invalid_stock_exchange_response_exception.h"
#include "common/huobi_currency.h"
#include "common/loggers/file_logger.h"
#include "common/utils.h"
#include "include/stock_exchange_utils.h"
#include "resources/resources.h"

namespace auto_trader {
namespace stock_exchange {

constexpr char timeBufferRegexp[] = "%04d-%02d-%02dT%02d%%3A%02d%%3A%02d";

static void verifyHuobiResponse(Poco::JSON::Object::Ptr &object, curl_slist *chunk) {
  if (chunk != nullptr) {
    curl_slist_free_all(chunk);
  }

  auto status = object->get(resources::words::STATUS);
  if (status != resources::huobi::HUOBI_STATUS_OK) {
    const std::string message = object->get(resources::huobi::HUOBI_ERROR_CODE_KEY).toString();
    common::loggers::FileLogger::getLogger()
        << resources::messages::FAILED_TO_UPLOAD_MESSAGE << message;

    throw common::exceptions::InvalidStockExchangeResponse(message);
  }
}

static std::string encodeToSignature(const std::string &secretKey, const std::string &url) {
  const EVP_MD *engine = EVP_sha256();
  unsigned char output[1024] = {0};
  uint32_t len = 1024;
  HMAC_CTX *ctx = HMAC_CTX_new();
  HMAC_Init_ex(ctx, secretKey.c_str(), secretKey.size(), engine, NULL);
  HMAC_Update(ctx, (unsigned char *)url.c_str(), url.size());
  HMAC_Final(ctx, output, &len);
  HMAC_CTX_free(ctx);

  auto b62enc = stock_exchange_utils::HuobiB64Encode(output, 32);
  std::string uriEncodedParams = stock_exchange_utils::escapeURL(b62enc);
  return uriEncodedParams;
}

common::MarketOrder HuobiQuery::sellOrder(common::Currency::Enum fromCurrency,
                                          common::Currency::Enum toCurrency, double quantity,
                                          double rate) {
  CURL *curl = curl_easy_init();
  stock_exchange_utils::checkCurlPointer(curl);

  time_t t = time(NULL);
  struct tm *local = gmtime(&t);
  char timeBuffer[100] = {0};

  sprintf(timeBuffer, timeBufferRegexp, local->tm_year + 1900, local->tm_mon + 1, local->tm_mday,
          local->tm_hour, local->tm_min, local->tm_sec);

  std::string account_identifier = getAccountIdentifier();
  HuobiPrecision huobiPrecision = getHuobiPrecision(fromCurrency, toCurrency);

  std::string additional_data_to_params =
      resources::huobi::HUOBI_POST + resources::symbols::NEW_LINE_SYMBOL +
      resources::huobi::HUOBI_PRO_API_URL + resources::symbols::NEW_LINE_SYMBOL +
      resources::symbols::SLASH + resources::huobi::HUOBI_ORDERS_REQUEST +
      resources::symbols::SLASH + resources::huobi::HUOBI_PLACE_KEYWORD +
      resources::symbols::NEW_LINE_SYMBOL;

  std::string param = resources::huobi::HUOBI_API_KEY + resources::symbols::EQUAL + api_key_ +
                      resources::symbols::AND + resources::huobi::HUOBI_SIGNATURE_METHOD +
                      resources::symbols::AND + resources::huobi::HUOBI_SIGNATURE_VERSION +
                      resources::symbols::AND + resources::huobi::HUOBI_TIMESTAMP +
                      resources::symbols::EQUAL + timeBuffer;

  auto completeUrl = additional_data_to_params + param;
  std::string uriEncodedParams = encodeToSignature(secret_key_, completeUrl);

  common::HuobiCurrency huobiCurrency;
  std::string currencyPair = huobiCurrency.getHuobiPair(fromCurrency, toCurrency);
  std::transform(currencyPair.begin(), currencyPair.end(), currencyPair.begin(),
                 [](unsigned char c) { return std::tolower(c); });

  const std::string postParams =
      resources::symbols::LEFT_CURLY_BRACE + resources::symbols::DOUBLE_QUOTES +
      resources::huobi::HOUBI_ACCOUNT_ID + resources::symbols::DOUBLE_QUOTES +
      resources::symbols::COLON + resources::symbols::SPACE + resources::symbols::DOUBLE_QUOTES +
      account_identifier + resources::symbols::DOUBLE_QUOTES + resources::symbols::COMMA +
      resources::symbols::DOUBLE_QUOTES + resources::words::SYMBOL +
      resources::symbols::DOUBLE_QUOTES + resources::symbols::COLON + resources::symbols::SPACE +
      resources::symbols::DOUBLE_QUOTES + currencyPair + resources::symbols::DOUBLE_QUOTES +
      resources::symbols::COMMA + resources::symbols::DOUBLE_QUOTES + resources::words::TYPE +
      resources::symbols::DOUBLE_QUOTES + resources::symbols::COLON + resources::symbols::SPACE +
      resources::symbols::DOUBLE_QUOTES + resources::huobi::HUOBI_SELL_LIMIT +
      resources::symbols::DOUBLE_QUOTES + resources::symbols::COMMA +
      resources::symbols::DOUBLE_QUOTES + resources::words::AMOUNT +
      resources::symbols::DOUBLE_QUOTES + resources::symbols::COLON + resources::symbols::SPACE +
      resources::symbols::DOUBLE_QUOTES +
      common::MarketOrder::convertCoinWithPrecision(quantity, huobiPrecision.amountPrecision_) +
      resources::symbols::DOUBLE_QUOTES + resources::symbols::COMMA +
      resources::symbols::DOUBLE_QUOTES + resources::words::PRICE +
      resources::symbols::DOUBLE_QUOTES + resources::symbols::COLON + resources::symbols::SPACE +
      resources::symbols::DOUBLE_QUOTES +
      common::MarketOrder::convertCoinWithPrecision(rate, huobiPrecision.pricePrecision_) +
      resources::symbols::DOUBLE_QUOTES + resources::symbols::RIGHT_CURLY_BRACE;

  const std::string fullUrlWithParameters =
      resources::huobi::HUOBI_HTTPS_API_URL + resources::symbols::SLASH +
      resources::huobi::HUOBI_ORDERS_REQUEST + resources::symbols::SLASH +
      resources::huobi::HUOBI_PLACE_KEYWORD + resources::symbols::QUESTION + param +
      resources::symbols::AND + resources::huobi::HUOBI_SIGNATURE + resources::symbols::EQUAL +
      uriEncodedParams;

  curl_easy_setopt(curl, CURLOPT_URL, fullUrlWithParameters.data());
  curl_easy_setopt(curl, CURLOPT_POST, 1L);

  curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postParams.c_str());
  curl_slist *plist = curl_slist_append(NULL, resources::huobi::HUOBI_CONTENT_TYPE);

  curl_easy_setopt(curl, CURLOPT_HTTPHEADER, plist);
  curl_easy_setopt(curl, CURLOPT_TIMEOUT, 20);

  std::string response = sendRequest(curl);

  Poco::JSON::Parser parser;
  Poco::JSON::Object::Ptr jsonMainObject =
      parser.parse(response).extract<Poco::JSON::Object::Ptr>();

  verifyHuobiResponse(jsonMainObject, nullptr);

  std::string orderId = jsonMainObject->get(resources::huobi::HUOBI_ORDERS_DATA_KEYWORD);
  common::MarketOrder order = getAccountOrder(fromCurrency, toCurrency, orderId);
  return order;
}

common::MarketOrder HuobiQuery::buyOrder(common::Currency::Enum fromCurrency,
                                         common::Currency::Enum toCurrency, double quantity,
                                         double rate) {
  CURL *curl = curl_easy_init();
  stock_exchange_utils::checkCurlPointer(curl);

  time_t t = time(NULL);
  struct tm *local = gmtime(&t);
  char timeBuffer[100] = {0};

  sprintf(timeBuffer, timeBufferRegexp, local->tm_year + 1900, local->tm_mon + 1, local->tm_mday,
          local->tm_hour, local->tm_min, local->tm_sec);

  std::string account_identifier = getAccountIdentifier();
  HuobiPrecision huobiPrecision = getHuobiPrecision(fromCurrency, toCurrency);

  std::string additional_data_to_params =
      resources::huobi::HUOBI_POST + resources::symbols::NEW_LINE_SYMBOL +
      resources::huobi::HUOBI_PRO_API_URL + resources::symbols::NEW_LINE_SYMBOL +
      resources::symbols::SLASH + resources::huobi::HUOBI_ORDERS_REQUEST +
      resources::symbols::SLASH + resources::huobi::HUOBI_PLACE_KEYWORD +
      resources::symbols::NEW_LINE_SYMBOL;

  std::string param = resources::huobi::HUOBI_API_KEY + resources::symbols::EQUAL + api_key_ +
                      resources::symbols::AND + resources::huobi::HUOBI_SIGNATURE_METHOD +
                      resources::symbols::AND + resources::huobi::HUOBI_SIGNATURE_VERSION +
                      resources::symbols::AND + resources::huobi::HUOBI_TIMESTAMP +
                      resources::symbols::EQUAL + timeBuffer;

  auto completeUrl = additional_data_to_params + param;
  std::string uriEncodedParams = encodeToSignature(secret_key_, completeUrl);

  common::HuobiCurrency huobiCurrency;
  std::string currencyPair = huobiCurrency.getHuobiPair(fromCurrency, toCurrency);
  std::transform(currencyPair.begin(), currencyPair.end(), currencyPair.begin(),
                 [](unsigned char c) { return std::tolower(c); });

  const std::string postParams =
      resources::symbols::LEFT_CURLY_BRACE + resources::symbols::DOUBLE_QUOTES +
      resources::huobi::HOUBI_ACCOUNT_ID + resources::symbols::DOUBLE_QUOTES +
      resources::symbols::COLON + resources::symbols::SPACE + resources::symbols::DOUBLE_QUOTES +
      account_identifier + resources::symbols::DOUBLE_QUOTES + resources::symbols::COMMA +
      resources::symbols::DOUBLE_QUOTES + resources::words::SYMBOL +
      resources::symbols::DOUBLE_QUOTES + resources::symbols::COLON + resources::symbols::SPACE +
      resources::symbols::DOUBLE_QUOTES + currencyPair + resources::symbols::DOUBLE_QUOTES +
      resources::symbols::COMMA + resources::symbols::DOUBLE_QUOTES + resources::words::TYPE +
      resources::symbols::DOUBLE_QUOTES + resources::symbols::COLON + resources::symbols::SPACE +
      resources::symbols::DOUBLE_QUOTES + resources::huobi::HUOBI_BUY_LIMIT +
      resources::symbols::DOUBLE_QUOTES + resources::symbols::COMMA +
      resources::symbols::DOUBLE_QUOTES + resources::words::AMOUNT +
      resources::symbols::DOUBLE_QUOTES + resources::symbols::COLON + resources::symbols::SPACE +
      resources::symbols::DOUBLE_QUOTES +
      common::MarketOrder::convertCoinWithPrecision(quantity, huobiPrecision.amountPrecision_) +
      resources::symbols::DOUBLE_QUOTES + resources::symbols::COMMA +
      resources::symbols::DOUBLE_QUOTES + resources::words::PRICE +
      resources::symbols::DOUBLE_QUOTES + resources::symbols::COLON + resources::symbols::SPACE +
      resources::symbols::DOUBLE_QUOTES +
      common::MarketOrder::convertCoinWithPrecision(rate, huobiPrecision.pricePrecision_) +
      resources::symbols::DOUBLE_QUOTES + resources::symbols::RIGHT_CURLY_BRACE;

  const std::string fullUrlWithParameters =
      resources::huobi::HUOBI_HTTPS_API_URL + resources::symbols::SLASH +
      resources::huobi::HUOBI_ORDERS_REQUEST + resources::symbols::SLASH +
      resources::huobi::HUOBI_PLACE_KEYWORD + resources::symbols::QUESTION + param +
      resources::symbols::AND + resources::huobi::HUOBI_SIGNATURE + resources::symbols::EQUAL +
      uriEncodedParams;

  curl_easy_setopt(curl, CURLOPT_URL, fullUrlWithParameters.data());
  curl_easy_setopt(curl, CURLOPT_POST, 1L);

  curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postParams.c_str());
  curl_slist *plist = curl_slist_append(NULL, resources::huobi::HUOBI_CONTENT_TYPE);

  curl_easy_setopt(curl, CURLOPT_HTTPHEADER, plist);
  curl_easy_setopt(curl, CURLOPT_TIMEOUT, 20);

  std::string response = sendRequest(curl);

  Poco::JSON::Parser parser;
  Poco::JSON::Object::Ptr jsonMainObject =
      parser.parse(response).extract<Poco::JSON::Object::Ptr>();

  verifyHuobiResponse(jsonMainObject, nullptr);

  std::string orderId = jsonMainObject->get(resources::huobi::HUOBI_ORDERS_DATA_KEYWORD);
  common::MarketOrder order = getAccountOrder(fromCurrency, toCurrency, orderId);
  return order;
}

bool HuobiQuery::cancelOrder(common::Currency::Enum fromCurrency, common::Currency::Enum toCurrency,
                             const std::string &uuid) {
  CURL *curl = curl_easy_init();
  stock_exchange_utils::checkCurlPointer(curl);

  time_t t = time(NULL);
  struct tm *local = gmtime(&t);
  char timeBuffer[100] = {0};

  sprintf(timeBuffer, timeBufferRegexp, local->tm_year + 1900, local->tm_mon + 1, local->tm_mday,
          local->tm_hour, local->tm_min, local->tm_sec);

  std::string additional_data_to_params =
      resources::huobi::HUOBI_POST + resources::symbols::NEW_LINE_SYMBOL +
      resources::huobi::HUOBI_PRO_API_URL + resources::symbols::NEW_LINE_SYMBOL +
      resources::symbols::SLASH + resources::huobi::HUOBI_ORDERS_REQUEST +
      resources::symbols::SLASH + uuid + resources::symbols::SLASH +
      resources::huobi::HUOBI_SUBMIT_CANCEL + resources::symbols::NEW_LINE_SYMBOL;

  std::string param = resources::huobi::HUOBI_API_KEY + resources::symbols::EQUAL + api_key_ +
                      resources::symbols::AND + resources::huobi::HUOBI_SIGNATURE_METHOD +
                      resources::symbols::AND + resources::huobi::HUOBI_SIGNATURE_VERSION +
                      resources::symbols::AND + resources::huobi::HUOBI_TIMESTAMP +
                      resources::symbols::EQUAL + timeBuffer;

  auto completeUrl = additional_data_to_params + param;
  std::string uriEncodedParams = encodeToSignature(secret_key_, completeUrl);

  const std::string postParams = param + resources::symbols::AND +
                                 resources::huobi::HUOBI_SIGNATURE + resources::symbols::EQUAL +
                                 uriEncodedParams;

  const std::string fullUrlWithParameters =
      resources::huobi::HUOBI_HTTPS_API_URL + resources::symbols::SLASH +
      resources::huobi::HUOBI_ORDERS_REQUEST + resources::symbols::SLASH + uuid +
      resources::symbols::SLASH + resources::huobi::HUOBI_SUBMIT_CANCEL +
      resources::symbols::QUESTION + postParams;

  curl_easy_setopt(curl, CURLOPT_URL, fullUrlWithParameters.data());
  curl_easy_setopt(curl, CURLOPT_POST, 1L);

  curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postParams.c_str());
  curl_slist *plist = curl_slist_append(NULL, resources::huobi::HUOBI_CONTENT_TYPE);

  curl_easy_setopt(curl, CURLOPT_HTTPHEADER, plist);
  curl_easy_setopt(curl, CURLOPT_TIMEOUT, 20);

  std::string response = sendRequest(curl);

  Poco::JSON::Parser parser;
  Poco::JSON::Object::Ptr jsonMainObject =
      parser.parse(response).extract<Poco::JSON::Object::Ptr>();

  verifyHuobiResponse(jsonMainObject, nullptr);

  std::string canceledOrderUUID = jsonMainObject->get(resources::huobi::HUOBI_ORDERS_DATA_KEYWORD);
  return canceledOrderUUID == uuid;
}

common::MarketHistoryPtr HuobiQuery::parseMarketHistory(
    const Poco::JSON::Object::Ptr &response) const {
  using namespace Poco;

  auto candlesArray = response->getArray(resources::huobi::HUOBI_ORDERS_DATA_KEYWORD);
  auto arraySize = candlesArray->size();

  auto marketHistory = std::make_unique<common::MarketHistory>();
  for (int index = 0; index < arraySize; ++index) {
    auto candleObjectJson = candlesArray->getObject(index);
    common::MarketData candle;

    candle.closePrice_ = candleObjectJson->get(resources::huobi::HUOBI_CANDLE_CLOSE_PRICE);
    candle.openPrice_ = candleObjectJson->get(resources::huobi::HUOBI_CANDLE_OPEN_PRICE);
    candle.lowPrice_ = candleObjectJson->get(resources::huobi::HUOBI_CANDLE_LOW_PRICE);
    candle.highPrice_ = candleObjectJson->get(resources::huobi::HUOBI_CANDLE_HIGH_PRICE);
    candle.volume_ = candleObjectJson->get(resources::huobi::HUOBI_CANDLE_VOLUME_PRICE);

    auto timestamp = candleObjectJson->get(resources::huobi::HUOBI_CANDLE_TIMESTAMP);
    candle.date_ = stock_exchange_utils::getDataFromTimestamp(timestamp, false);

    marketHistory->marketData_.emplace_back(candle);
  }

  return marketHistory;
}

common::CurrencyTick HuobiQuery::getCurrencyTick(common::Currency::Enum fromCurrency,
                                                 common::Currency::Enum toCurrency) {
  CURL *curl = curl_easy_init();
  stock_exchange_utils::checkCurlPointer(curl);

  common::HuobiCurrency huobiCurrency;
  std::string currencyPair = huobiCurrency.getHuobiPair(fromCurrency, toCurrency);
  std::transform(currencyPair.begin(), currencyPair.end(), currencyPair.begin(),
                 [](unsigned char c) { return std::tolower(c); });

  std::string uri = resources::huobi::HUOBI_PUBLIC_URL + resources::symbols::SLASH +
                    resources::huobi::HUOBI_CURRENCY_TICK_KEYWORD;

  std::string parameters = resources::words::SYMBOL + resources::symbols::EQUAL + currencyPair +
                           resources::symbols::AND + resources::words::TYPE +
                           resources::symbols::EQUAL + resources::huobi::HUOBI_CURRENCY_TICK_STEP;

  auto fullUrlWithParameters = uri + resources::symbols::QUESTION + parameters;
  curl_easy_setopt(curl, CURLOPT_URL, fullUrlWithParameters.data());
  curl_easy_setopt(curl, CURLOPT_HTTPHEADER, nullptr);

  std::string response = sendRequest(curl);

  Poco::JSON::Parser parser;
  Poco::JSON::Object::Ptr jsonMainObject =
      parser.parse(response).extract<Poco::JSON::Object::Ptr>();

  verifyHuobiResponse(jsonMainObject, nullptr);

  common::CurrencyTick currencyTick;

  currencyTick.fromCurrency_ = fromCurrency;
  currencyTick.toCurrency_ = toCurrency;

  auto jsonTickObject = jsonMainObject->getObject(resources::huobi::HUOBI_CURRENCY_TICK_BLOCK);

  auto bidArrayObject = jsonTickObject->getArray(resources::words::BIDS);
  auto firstBidPair = bidArrayObject->getArray(resources::numbers::FIRST_ARRAY_INDEX);
  currencyTick.bid_ = firstBidPair->get(resources::numbers::FIRST_ARRAY_INDEX);

  auto asksArrayObject = jsonTickObject->getArray(resources::words::ASKS);
  auto firstAsksPair = asksArrayObject->getArray(resources::numbers::FIRST_ARRAY_INDEX);
  currencyTick.ask_ = firstAsksPair->get(resources::numbers::FIRST_ARRAY_INDEX);

  return currencyTick;
}

std::vector<common::MarketOrder> HuobiQuery::getAccountOpenOrders(
    common::Currency::Enum fromCurrency, common::Currency::Enum toCurrency) {
  CURL *curl = curl_easy_init();
  stock_exchange_utils::checkCurlPointer(curl);

  time_t t = time(NULL);
  struct tm *local = gmtime(&t);
  char timeBuf[100] = {0};
  sprintf(timeBuf, timeBufferRegexp, local->tm_year + 1900, local->tm_mon + 1, local->tm_mday,
          local->tm_hour, local->tm_min, local->tm_sec);

  common::HuobiCurrency huobiCurrency;
  std::string currencyPair = huobiCurrency.getHuobiPair(fromCurrency, toCurrency);
  std::transform(currencyPair.begin(), currencyPair.end(), currencyPair.begin(),
                 [](unsigned char c) { return std::tolower(c); });

  std::string additional_data_to_params =
      resources::huobi::HUOBI_GET + resources::symbols::NEW_LINE_SYMBOL +
      resources::huobi::HUOBI_PRO_API_URL + resources::symbols::NEW_LINE_SYMBOL +
      resources::symbols::SLASH + resources::huobi::HUOBI_OPEN_ORDERS_REQUEST +
      resources::symbols::NEW_LINE_SYMBOL;

  std::string account_identifier = getAccountIdentifier();
  std::string param = resources::huobi::HUOBI_API_KEY + resources::symbols::EQUAL + api_key_ +
                      resources::symbols::AND + resources::huobi::HUOBI_SIGNATURE_METHOD +
                      resources::symbols::AND + resources::huobi::HUOBI_SIGNATURE_VERSION +
                      resources::symbols::AND + resources::huobi::HUOBI_TIMESTAMP +
                      resources::symbols::EQUAL + timeBuf + resources::symbols::AND +
                      resources::huobi::HOUBI_ACCOUNT_ID + resources::symbols::EQUAL +
                      account_identifier + resources::symbols::AND + resources::words::SYMBOL +
                      resources::symbols::EQUAL + currencyPair;

  auto all_str = additional_data_to_params + param;
  std::string uriEncodedParams = encodeToSignature(secret_key_, all_str);

  const std::string fullUrlWithParameters =
      resources::huobi::HUOBI_HTTPS_API_URL + resources::symbols::SLASH +
      resources::huobi::HUOBI_OPEN_ORDERS_REQUEST + resources::symbols::QUESTION + param +
      resources::symbols::AND + resources::huobi::HUOBI_SIGNATURE + resources::symbols::EQUAL +
      uriEncodedParams;

  curl_easy_setopt(curl, CURLOPT_URL, fullUrlWithParameters.data());
  curl_slist *plist = curl_slist_append(NULL, resources::huobi::HUOBI_CONTENT_TYPE);

  curl_easy_setopt(curl, CURLOPT_HTTPHEADER, plist);
  curl_easy_setopt(curl, CURLOPT_TIMEOUT, 20);

  std::string response = sendRequest(curl);

  Poco::JSON::Parser parser;
  Poco::JSON::Object::Ptr jsonMainObject =
      parser.parse(response).extract<Poco::JSON::Object::Ptr>();

  verifyHuobiResponse(jsonMainObject, nullptr);

  auto jsonOrdersDataArray = jsonMainObject->getArray(resources::huobi::HUOBI_ORDERS_DATA_KEYWORD);
  auto arraySize = jsonOrdersDataArray->size();

  std::vector<common::MarketOrder> openOrders;
  for (size_t index = 0; index < arraySize; ++index) {
    auto orderJsonObject = jsonOrdersDataArray->getObject(index);

    common::MarketOrder orderInfo;

    orderInfo.quantity_ = orderJsonObject->get(resources::words::AMOUNT);
    orderInfo.price_ = orderJsonObject->get(resources::words::PRICE);

    const std::string symbol = orderJsonObject->get(resources::words::SYMBOL);
    auto currencyPair = huobiCurrency.parseHuobiExchangeType(symbol);
    orderInfo.fromCurrency_ = common::Currency::fromString(currencyPair.first);
    orderInfo.toCurrency_ = common::Currency::fromString(currencyPair.second);

    auto timestamp = orderJsonObject->get(resources::huobi::HUOBI_TIMESTAMP_KEYWORD);
    orderInfo.opened_ = stock_exchange_utils::getDataFromTimestamp(timestamp, true);

    const std::string type = jsonMainObject->get(resources::words::TYPE);
    if (type == resources::huobi::HUOBI_BUY_LIMIT) {
      orderInfo.orderType_ = common::OrderType::BUY;
    } else {
      orderInfo.orderType_ = common::OrderType::SELL;
    }

    orderInfo.isCanceled_ = false;
    orderInfo.stockExchangeType_ = common::StockExchangeType::Huobi;

    openOrders.emplace_back(orderInfo);
  }

  return openOrders;
}

common::MarketHistoryPtr HuobiQuery::getMarketHistory(common::Currency::Enum fromCurrency,
                                                      common::Currency::Enum toCurrency,
                                                      common::TickInterval::Enum interval) {
  CURL *curl = curl_easy_init();
  stock_exchange_utils::checkCurlPointer(curl);

  common::HuobiCurrency huobiCurrency;
  std::string currencyPair = huobiCurrency.getHuobiPair(fromCurrency, toCurrency);
  std::transform(currencyPair.begin(), currencyPair.end(), currencyPair.begin(),
                 [](unsigned char c) { return std::tolower(c); });

  std::string uri = resources::huobi::HUOBI_PUBLIC_URL + resources::symbols::SLASH +
                    resources::huobi::HUOBI_GET_MARKET_HISTORY_KEYWORD;

  std::string parameters = resources::words::SYMBOL + resources::symbols::EQUAL + currencyPair +
                           resources::symbols::AND +
                           resources::huobi::HUOBI_MERKET_HISTORY_PERIOD_KEYWORD +
                           resources::symbols::EQUAL +
                           common::convertTickInterval(interval, common::StockExchangeType::Huobi);

  auto fullUrlWithParameters = uri + resources::symbols::QUESTION + parameters;
  curl_easy_setopt(curl, CURLOPT_URL, fullUrlWithParameters.data());
  curl_easy_setopt(curl, CURLOPT_HTTPHEADER, nullptr);

  std::string response = sendRequest(curl);

  Poco::JSON::Parser parser;
  Poco::JSON::Object::Ptr jsonMainObject =
      parser.parse(response).extract<Poco::JSON::Object::Ptr>();

  verifyHuobiResponse(jsonMainObject, nullptr);

  auto marketHistoryData = parseMarketHistory(jsonMainObject);
  marketHistoryData->toSell_ = fromCurrency;
  marketHistoryData->toBuy_ = toCurrency;

  return marketHistoryData;
}

bool replace(std::string &str, const std::string &from, const std::string &to) {
  size_t start_pos = str.find(from);
  if (start_pos == std::string::npos) return false;
  str.replace(start_pos, from.length(), to);
  return true;
}

std::vector<common::MarketOrder> HuobiQuery::getMarketOpenOrders(
    common::Currency::Enum fromCurrency, common::Currency::Enum toCurrency) {
  CURL *curl = curl_easy_init();
  stock_exchange_utils::checkCurlPointer(curl);

  common::HuobiCurrency huobiCurrency;
  std::string currencyPair = huobiCurrency.getHuobiPair(fromCurrency, toCurrency);
  std::transform(currencyPair.begin(), currencyPair.end(), currencyPair.begin(),
                 [](unsigned char c) { return std::tolower(c); });

  std::string uri = resources::huobi::HUOBI_PUBLIC_URL + resources::symbols::SLASH +
                    resources::huobi::HUOBI_MARKET_OPEN_ORDERS_KEYWORD;

  std::string parameters = resources::words::SYMBOL + resources::symbols::EQUAL + currencyPair +
                           resources::symbols::AND + resources::words::SIZE +
                           resources::symbols::EQUAL + std::to_string(2);

  auto fullUrlWithParameters = uri + resources::symbols::QUESTION + parameters;
  curl_easy_setopt(curl, CURLOPT_URL, fullUrlWithParameters.data());
  curl_easy_setopt(curl, CURLOPT_HTTPHEADER, nullptr);

  std::string response = sendRequest(curl);
  QString jsonStr = QString::fromStdString(response);

  QJsonDocument jsonResponse = QJsonDocument::fromJson(jsonStr.toUtf8());
  QJsonObject jsonObject = jsonResponse.object();

  auto status = jsonObject[resources::words::STATUS.c_str()].toString().toStdString();
  if (status != resources::huobi::HUOBI_STATUS_OK) {
    const std::string message =
        jsonObject[resources::huobi::HUOBI_ERROR_CODE_KEY.c_str()].toString().toStdString();
    common::loggers::FileLogger::getLogger()
        << resources::messages::FAILED_TO_UPLOAD_MESSAGE << message;

    throw common::exceptions::InvalidStockExchangeResponse(message);
  }

  auto jsonOrdersDataArray =
      jsonObject[resources::huobi::HUOBI_ORDERS_DATA_KEYWORD.c_str()].toArray();
  auto arraySize = jsonOrdersDataArray.size();

  std::vector<common::MarketOrder> openOrders;
  for (size_t index = 0; index < arraySize; ++index) {
    auto orderJsonObject = jsonOrdersDataArray[index].toObject();
    auto orderDataArray =
        orderJsonObject[resources::huobi::HUOBI_ORDERS_DATA_KEYWORD.c_str()].toArray();
    auto orderJsonMainData = orderDataArray[0].toObject();

    common::MarketOrder orderInfo;

    orderInfo.quantity_ = orderJsonMainData[resources::words::AMOUNT.c_str()].toDouble();
    orderInfo.price_ = orderJsonMainData[resources::words::PRICE.c_str()].toDouble();

    auto timestamp = orderJsonMainData[resources::huobi::HUOBI_TIMESTAMP_KEYWORD.c_str()]
                         .toVariant()
                         .toLongLong();
    orderInfo.opened_ = stock_exchange_utils::getDataFromTimestamp(timestamp, true);

    std::string orderTypeStr = orderJsonMainData[resources::huobi::HUOBI_ORDER_TIME_KEYWORD.c_str()]
                                   .toString()
                                   .toStdString();
    orderInfo.orderType_ = orderTypeStr == resources::words::BUY_SIDE_LOWER_CASE
                               ? common::OrderType::BUY
                               : common::OrderType::SELL;

    orderInfo.fromCurrency_ = fromCurrency;
    orderInfo.toCurrency_ = toCurrency;
    orderInfo.isCanceled_ = false;
    orderInfo.stockExchangeType_ = common::StockExchangeType::Huobi;

    openOrders.emplace_back(orderInfo);
  }

  return openOrders;
}

common::MarketOrder HuobiQuery::getAccountOrder(common::Currency::Enum fromCurrency,
                                                common::Currency::Enum toCurrency,
                                                const std::string &uuid) {
  CURL *curl = curl_easy_init();
  stock_exchange_utils::checkCurlPointer(curl);

  time_t t = time(NULL);
  struct tm *local = gmtime(&t);
  char timeBuf[100] = {0};
  sprintf(timeBuf, timeBufferRegexp, local->tm_year + 1900, local->tm_mon + 1, local->tm_mday,
          local->tm_hour, local->tm_min, local->tm_sec);

  std::string additional_data_to_params =
      resources::huobi::HUOBI_GET + resources::symbols::NEW_LINE_SYMBOL +
      resources::huobi::HUOBI_PRO_API_URL + resources::symbols::NEW_LINE_SYMBOL +
      resources::symbols::SLASH + resources::huobi::HUOBI_ORDERS_REQUEST +
      resources::symbols::SLASH + uuid + resources::symbols::NEW_LINE_SYMBOL;

  std::string param = resources::huobi::HUOBI_API_KEY + resources::symbols::EQUAL + api_key_ +
                      resources::symbols::AND + resources::huobi::HUOBI_SIGNATURE_METHOD +
                      resources::symbols::AND + resources::huobi::HUOBI_SIGNATURE_VERSION +
                      resources::symbols::AND + resources::huobi::HUOBI_TIMESTAMP +
                      resources::symbols::EQUAL + timeBuf;

  auto completeUrl = additional_data_to_params + param;
  std::string uriEncodedParams = encodeToSignature(secret_key_, completeUrl);

  const std::string postParams = param + resources::symbols::AND +
                                 resources::huobi::HUOBI_SIGNATURE + resources::symbols::EQUAL +
                                 uriEncodedParams;

  const std::string fullUrlWithParameters =
      resources::huobi::HUOBI_HTTPS_API_URL + resources::symbols::SLASH +
      resources::huobi::HUOBI_ORDERS_REQUEST + resources::symbols::SLASH + uuid +
      resources::symbols::QUESTION + postParams;

  curl_easy_setopt(curl, CURLOPT_URL, fullUrlWithParameters.data());
  curl_slist *plist = curl_slist_append(NULL, resources::huobi::HUOBI_CONTENT_TYPE);
  curl_easy_setopt(curl, CURLOPT_HTTPHEADER, plist);
  curl_easy_setopt(curl, CURLOPT_TIMEOUT, 20);

  std::string response = sendRequest(curl);

  Poco::JSON::Parser parser;
  Poco::JSON::Object::Ptr jsonMainObject =
      parser.parse(response).extract<Poco::JSON::Object::Ptr>();

  verifyHuobiResponse(jsonMainObject, nullptr);

  auto jsonDataObject = jsonMainObject->getObject(resources::huobi::HUOBI_ORDERS_DATA_KEYWORD);
  common::MarketOrder orderInfo;

  common::HuobiCurrency huobiCurrency;

  orderInfo.quantity_ = jsonDataObject->get(resources::words::AMOUNT);
  orderInfo.price_ = jsonDataObject->get(resources::words::PRICE);

  const std::string symbol = jsonDataObject->get(resources::words::SYMBOL);
  auto currencyPair = huobiCurrency.parseHuobiExchangeType(symbol);
  orderInfo.fromCurrency_ = common::Currency::fromString(currencyPair.first);
  orderInfo.toCurrency_ = common::Currency::fromString(currencyPair.second);

  auto timestamp = jsonDataObject->get(resources::huobi::HUOBI_CREATED_AT);
  orderInfo.opened_ = stock_exchange_utils::getDataFromTimestamp(timestamp, true);

  const std::string type = jsonDataObject->get(resources::words::TYPE);
  if (type == resources::huobi::HUOBI_BUY_LIMIT) {
    orderInfo.orderType_ = common::OrderType::BUY;
  } else {
    orderInfo.orderType_ = common::OrderType::SELL;
  }

  orderInfo.isCanceled_ = false;
  orderInfo.stockExchangeType_ = common::StockExchangeType::Huobi;
  orderInfo.uuid_ = uuid;

  return orderInfo;
}

double HuobiQuery::getBalance(common::Currency::Enum currency) {
  CURL *curl = curl_easy_init();
  stock_exchange_utils::checkCurlPointer(curl);

  time_t t = time(NULL);
  struct tm *local = gmtime(&t);
  char timeBuf[100] = {0};
  sprintf(timeBuf, timeBufferRegexp, local->tm_year + 1900, local->tm_mon + 1, local->tm_mday,
          local->tm_hour, local->tm_min, local->tm_sec);

  std::string account_identifier = getAccountIdentifier();
  std::string additional_data_to_params =
      resources::huobi::HUOBI_GET + resources::symbols::NEW_LINE_SYMBOL +
      resources::huobi::HUOBI_PRO_API_URL + resources::symbols::NEW_LINE_SYMBOL +
      resources::symbols::SLASH + resources::huobi::HUOBI_ACCOUNT_REQUEST +
      resources::symbols::SLASH + account_identifier + resources::symbols::SLASH +
      resources::huobi::HUOBI_GET_BALANCE_SECOND_PART + resources::symbols::NEW_LINE_SYMBOL;

  std::string param = resources::huobi::HUOBI_API_KEY + resources::symbols::EQUAL + api_key_ +
                      resources::symbols::AND + resources::huobi::HUOBI_SIGNATURE_METHOD +
                      resources::symbols::AND + resources::huobi::HUOBI_SIGNATURE_VERSION +
                      resources::symbols::AND + resources::huobi::HUOBI_TIMESTAMP +
                      resources::symbols::EQUAL + timeBuf;

  auto completeUrl = additional_data_to_params + param;
  std::string uriEncodedParams = encodeToSignature(secret_key_, completeUrl);

  const std::string postParams = param + resources::symbols::AND +
                                 resources::huobi::HUOBI_SIGNATURE + resources::symbols::EQUAL +
                                 uriEncodedParams;

  const std::string fullUrlWithParameters =
      resources::huobi::HUOBI_HTTPS_API_URL + resources::symbols::SLASH +
      resources::huobi::HUOBI_ACCOUNT_REQUEST + resources::symbols::SLASH + account_identifier +
      resources::symbols::SLASH + resources::huobi::HUOBI_GET_BALANCE_SECOND_PART +
      resources::symbols::QUESTION + postParams;

  curl_easy_setopt(curl, CURLOPT_URL, fullUrlWithParameters.data());
  curl_slist *plist = curl_slist_append(NULL, resources::huobi::HUOBI_CONTENT_TYPE);
  curl_easy_setopt(curl, CURLOPT_HTTPHEADER, plist);
  curl_easy_setopt(curl, CURLOPT_TIMEOUT, 20);

  std::string response = sendRequest(curl);
  std::string currencyStr = common::Currency::toString(currency);

  std::transform(currencyStr.begin(), currencyStr.end(), currencyStr.begin(),
                 [](unsigned char c) { return std::tolower(c); });

  Poco::JSON::Parser parser;
  Poco::JSON::Object::Ptr jsonMainObject =
      parser.parse(response).extract<Poco::JSON::Object::Ptr>();

  verifyHuobiResponse(jsonMainObject, nullptr);

  auto dataJsonArray = jsonMainObject->getObject(resources::huobi::HUOBI_ORDERS_DATA_KEYWORD);
  auto arrayJsonObject = dataJsonArray->getArray(resources::huobi::HUOBI_LIST_KEYWORD);
  auto arraySize = arrayJsonObject->size();

  for (int index = 0; index < arraySize; ++index) {
    auto listElementObject = arrayJsonObject->getObject(index);
    std::string currentCurrency = listElementObject->get(resources::huobi::HUOBI_CURRENCY_KEYWORD);
    std::string balance = listElementObject->get(resources::huobi::HUOBI_GET_BALANCE_SECOND_PART);

    if (currentCurrency == currencyStr) {
      return std::stod(balance);
    }
  }

  throw common::exceptions::InvalidStockExchangeResponse(
      "The currency " + currencyStr + " has not been found on Huobi stock exchange");
}

std::string HuobiQuery::getAccountIdentifier() const {
  CURL *curl = curl_easy_init();
  stock_exchange_utils::checkCurlPointer(curl);

  time_t t = time(NULL);
  struct tm *local = gmtime(&t);
  char timeBuf[100] = {0};
  sprintf(timeBuf, timeBufferRegexp, local->tm_year + 1900, local->tm_mon + 1, local->tm_mday,
          local->tm_hour, local->tm_min, local->tm_sec);

  std::string additional_data_to_params =
      resources::huobi::HUOBI_GET + resources::symbols::NEW_LINE_SYMBOL +
      resources::huobi::HUOBI_PRO_API_URL + resources::symbols::NEW_LINE_SYMBOL +
      resources::symbols::SLASH + resources::huobi::HUOBI_ACCOUNT_REQUEST +
      resources::symbols::NEW_LINE_SYMBOL;

  std::string param = resources::huobi::HUOBI_API_KEY + resources::symbols::EQUAL + api_key_ +
                      resources::symbols::AND + resources::huobi::HUOBI_SIGNATURE_METHOD +
                      resources::symbols::AND + resources::huobi::HUOBI_SIGNATURE_VERSION +
                      resources::symbols::AND + resources::huobi::HUOBI_TIMESTAMP +
                      resources::symbols::EQUAL + timeBuf;

  auto completeUrl = additional_data_to_params + param;
  std::string uriEncodedParams = encodeToSignature(secret_key_, completeUrl);

  const std::string postParams = param + resources::symbols::AND +
                                 resources::huobi::HUOBI_SIGNATURE + resources::symbols::EQUAL +
                                 uriEncodedParams;

  const std::string fullUrlWithParameters =
      resources::huobi::HUOBI_HTTPS_API_URL + resources::symbols::SLASH +
      resources::huobi::HUOBI_ACCOUNT_REQUEST + resources::symbols::QUESTION + postParams;

  curl_slist *chunk = nullptr;
  chunk = curl_slist_append(NULL, resources::huobi::HUOBI_CONTENT_TYPE);
  curl_easy_setopt(curl, CURLOPT_URL, fullUrlWithParameters.data());
  curl_easy_setopt(curl, CURLOPT_HTTPHEADER, chunk);
  curl_easy_setopt(curl, CURLOPT_TIMEOUT, 20);

  std::string response = sendRequest(curl);
  curl_slist_free_all(chunk);
  curl_easy_cleanup(curl);

  Poco::JSON::Parser parser;
  Poco::JSON::Object::Ptr jsonMainObject =
      parser.parse(response).extract<Poco::JSON::Object::Ptr>();

  verifyHuobiResponse(jsonMainObject, nullptr);

  auto dataJsonArray = jsonMainObject->getArray(resources::huobi::HUOBI_ORDERS_DATA_KEYWORD);
  auto arrayJsonObject = dataJsonArray->getObject(resources::numbers::FIRST_ARRAY_INDEX);

  std::string account_identifier = arrayJsonObject->get(resources::huobi::HUOBI_ID_KEYWORD);
  return account_identifier;
}

uint64_t HuobiQuery::getCurrentServerTime() {
  const std::string request_str = resources::huobi::HUOBI_HTTPS_API_URL +
                                  resources::symbols::SLASH + resources::huobi::HUOBI_TIMESTAMP_URL;

  Poco::URI uri(request_str);
  auto path = uri.getPathAndQuery();
  path = path.empty() ? resources::symbols::SLASH : path;
  Poco::Net::HTTPRequest request(Poco::Net::HTTPRequest::HTTP_GET, path,
                                 Poco::Net::HTTPMessage::HTTP_1_1);

  ConnectionAttributes attributes;
  attributes.host_ = uri.getHost();
  attributes.port_ = uri.getPort();
  std::vector<HTTP_HEADERS> headers;

  auto response = processHttpRequest(attributes, request, headers);

  Poco::JSON::Parser parser;
  Poco::JSON::Object::Ptr object = parser.parse(response).extract<Poco::JSON::Object::Ptr>();

  verifyHuobiResponse(object, nullptr);

  uint64_t serverTime = object->get(resources::huobi::HUOBI_ORDERS_DATA_KEYWORD);
  return serverTime;
}

std::string HuobiQuery::sendRequest(CURL *curl) const {
  std::string response;
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, static_cast<void *>(&response));
  curl_easy_perform(curl);
  return response;
}

CurrencyLotsHolder HuobiQuery::getCurrencyLotsHolder() {
  CurrencyLotsHolder lotsSizes;
  return lotsSizes;
}

HuobiPrecision HuobiQuery::getHuobiPrecision(common::Currency::Enum fromCurrency,
                                             common::Currency::Enum toCurrency) {
  const std::string request_str = resources::huobi::HUOBI_HTTPS_API_URL +
                                  resources::symbols::SLASH +
                                  resources::huobi::HUOBI_COMMON_SYMBOLS;

  Poco::URI uri(request_str);
  auto path = uri.getPathAndQuery();
  path = path.empty() ? resources::symbols::SLASH : path;
  Poco::Net::HTTPRequest request(Poco::Net::HTTPRequest::HTTP_GET, path,
                                 Poco::Net::HTTPMessage::HTTP_1_1);

  ConnectionAttributes attributes;
  attributes.host_ = uri.getHost();
  attributes.port_ = uri.getPort();
  std::vector<HTTP_HEADERS> headers;

  auto response = processHttpRequest(attributes, request, headers);

  Poco::JSON::Parser parser;
  Poco::JSON::Object::Ptr jsonMainObject =
      parser.parse(response).extract<Poco::JSON::Object::Ptr>();

  verifyHuobiResponse(jsonMainObject, nullptr);

  common::HuobiCurrency huobiCurrency;
  std::string currencyPair = huobiCurrency.getHuobiPair(fromCurrency, toCurrency);
  std::transform(currencyPair.begin(), currencyPair.end(), currencyPair.begin(),
                 [](unsigned char c) { return std::tolower(c); });

  auto jsonOrdersDataArray = jsonMainObject->getArray(resources::huobi::HUOBI_ORDERS_DATA_KEYWORD);
  auto arraySize = jsonOrdersDataArray->size();
  for (size_t index = 0; index < arraySize; ++index) {
    auto orderJsonObject = jsonOrdersDataArray->getObject(index);

    const std::string symbol = orderJsonObject->get(resources::words::SYMBOL);
    if (symbol == currencyPair) {
      HuobiPrecision huobiPrecision{0, 0};
      huobiPrecision.amountPrecision_ =
          orderJsonObject->get(resources::huobi::HUOBI_AMOUNT_PRECISION);
      huobiPrecision.pricePrecision_ =
          orderJsonObject->get(resources::huobi::HUOBI_PRICE_PRECISION);
      return huobiPrecision;
    }
  }

  throw common::exceptions::InvalidStockExchangeResponse(
      "The currency " + currencyPair + " has not been found on Huobi stock exchange");
}

}  // namespace stock_exchange
}  // namespace auto_trader