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

#include "include/poloniex_query.h"

#include "common/encryption_sha512_engine.h"
#include "common/exceptions/stock_exchange_exception/incorrect_json_exception.h"
#include "common/exceptions/stock_exchange_exception/invalid_stock_exchange_response_exception.h"
#include "common/loggers/file_logger.h"
#include "common/utils.h"
#include "include/stock_exchange_utils.h"
#include "resources/resources.h"

namespace auto_trader {
namespace stock_exchange {

common::MarketOrder PoloniexQuery::sellOrder(common::Currency::Enum fromCurrency,
                                             common::Currency::Enum toCurrency, double quantity,
                                             double rate) {
  using namespace Poco;

  CURL* curl = curl_easy_init();
  stock_exchange_utils::checkCurlPointer(curl);

  auto uri = resources::poloniex::POLONIEX_PRIVATE_ENDPOINT;

  auto timestamp = common::getCurrentMSEpoch();
  std::string parameters =
      resources::words::COMMAND + resources::symbols::EQUAL +
      resources::poloniex::POLONIEX_SELL_KEYWORD + resources::symbols::AND +
      resources::poloniex::POLONIEX_CURRENCY_PAIR_KEYWORD + resources::symbols::EQUAL +
      poloniexCurrency_.getPoloniexPair(fromCurrency, toCurrency) + resources::symbols::AND +
      resources::words::RATE + resources::symbols::EQUAL + std::to_string(rate) +
      resources::symbols::AND + resources::words::AMOUNT + resources::symbols::EQUAL +
      std::to_string(quantity) + resources::symbols::AND +
      resources::poloniex::POLONIEX_TIMESTAMP_KEYWORD + resources::symbols::EQUAL +
      std::to_string(timestamp);

  auto fullUrlWithParameters = uri + resources::symbols::QUESTION + parameters;
  curl_easy_setopt(curl, CURLOPT_URL, fullUrlWithParameters.data());
  curl_easy_setopt(curl, CURLOPT_POST, 1L);
  curl_easy_setopt(curl, CURLOPT_POSTFIELDS, parameters.c_str());

  Poco::HMACEngine<common::EncryptionSHA512Engine> hmac{secret_key_};
  hmac.update(parameters);
  std::string signature = DigestEngine::digestToHex(hmac.digest());

  std::string api_key_header =
      resources::poloniex::POLONIEX_HEADER_API_KEY_KEYWORD + ":" + api_key_;
  std::string signature_header =
      resources::poloniex::POLONIEX_HEADER_SIGNATURE_KEYWORD + ":" + signature;

  curl_slist* chunk = nullptr;
  chunk = curl_slist_append(chunk, api_key_header.c_str());
  chunk = curl_slist_append(chunk, signature_header.c_str());

  curl_easy_setopt(curl, CURLOPT_HTTPHEADER, chunk);

  std::string response = sendRequest(curl);

  auto jsonObject = getJsonObjectAndCheckOnIncorrectJson(response, chunk);
  checkPoloniexResponseMessage(jsonObject, chunk);

  common::MarketOrder currentOrder;

  currentOrder.stockExchangeType_ = common::StockExchangeType::Poloniex;
  currentOrder.fromCurrency_ = fromCurrency;
  currentOrder.toCurrency_ = toCurrency;
  currentOrder.opened_ = common::Date::getCurrentTime();
  currentOrder.isCanceled_ = false;
  currentOrder.quantity_ = quantity;
  currentOrder.price_ = rate;
  currentOrder.orderType_ = common::OrderType::SELL;
  currentOrder.uuid_ = jsonObject->get(resources::poloniex::POLONIEX_ORDER_NUMBER_ID).toString();

  return currentOrder;
}

common::MarketOrder PoloniexQuery::buyOrder(common::Currency::Enum fromCurrency,
                                            common::Currency::Enum toCurrency, double quantity,
                                            double rate) {
  using namespace Poco;

  CURL* curl = curl_easy_init();
  stock_exchange_utils::checkCurlPointer(curl);

  auto uri = resources::poloniex::POLONIEX_PRIVATE_ENDPOINT;

  auto timestamp = common::getCurrentMSEpoch();
  std::string parameters =
      resources::words::COMMAND + resources::symbols::EQUAL +
      resources::poloniex::POLONIEX_BUY_KEYWORD + resources::symbols::AND +
      resources::poloniex::POLONIEX_CURRENCY_PAIR_KEYWORD + resources::symbols::EQUAL +
      poloniexCurrency_.getPoloniexPair(fromCurrency, toCurrency) + resources::symbols::AND +
      resources::words::RATE + resources::symbols::EQUAL + std::to_string(rate) +
      resources::symbols::AND + resources::words::AMOUNT + resources::symbols::EQUAL +
      std::to_string(quantity) + resources::symbols::AND +
      resources::poloniex::POLONIEX_TIMESTAMP_KEYWORD + resources::symbols::EQUAL +
      std::to_string(timestamp);

  auto fullUrlWithParameters = uri + resources::symbols::QUESTION + parameters;
  curl_easy_setopt(curl, CURLOPT_URL, fullUrlWithParameters.data());
  curl_easy_setopt(curl, CURLOPT_POST, 1L);
  curl_easy_setopt(curl, CURLOPT_POSTFIELDS, parameters.c_str());

  Poco::HMACEngine<common::EncryptionSHA512Engine> hmac{secret_key_};
  hmac.update(parameters);
  std::string signature = DigestEngine::digestToHex(hmac.digest());

  std::string api_key_header =
      resources::poloniex::POLONIEX_HEADER_API_KEY_KEYWORD + ":" + api_key_;
  std::string signature_header =
      resources::poloniex::POLONIEX_HEADER_SIGNATURE_KEYWORD + ":" + signature;

  curl_slist* chunk = nullptr;
  chunk = curl_slist_append(chunk, api_key_header.c_str());
  chunk = curl_slist_append(chunk, signature_header.c_str());

  curl_easy_setopt(curl, CURLOPT_HTTPHEADER, chunk);

  std::string response = sendRequest(curl);

  auto jsonObject = getJsonObjectAndCheckOnIncorrectJson(response, chunk);
  checkPoloniexResponseMessage(jsonObject, chunk);

  common::MarketOrder currentOrder;

  currentOrder.stockExchangeType_ = common::StockExchangeType::Poloniex;
  currentOrder.fromCurrency_ = fromCurrency;
  currentOrder.toCurrency_ = toCurrency;
  currentOrder.opened_ = common::Date::getCurrentTime();
  currentOrder.isCanceled_ = false;
  currentOrder.quantity_ = quantity;
  currentOrder.price_ = rate;
  currentOrder.orderType_ = common::OrderType::BUY;
  currentOrder.uuid_ = jsonObject->get(resources::poloniex::POLONIEX_ORDER_NUMBER_ID).toString();

  return currentOrder;
}

bool PoloniexQuery::cancelOrder(common::Currency::Enum fromCurrency,
                                common::Currency::Enum toCurrency, const std::string& uuid) {
  using namespace Poco;

  CURL* curl = curl_easy_init();
  stock_exchange_utils::checkCurlPointer(curl);

  auto uri = resources::poloniex::POLONIEX_PRIVATE_ENDPOINT;

  auto timestamp = common::getCurrentMSEpoch();
  std::string parameters = resources::words::COMMAND + resources::symbols::EQUAL +
                           resources::poloniex::POLONIEX_CANCEL_ORDER_KEYWORD +
                           resources::symbols::AND + resources::poloniex::POLONIEX_ORDER_NUMBER_ID +
                           resources::symbols::EQUAL + uuid + resources::symbols::AND +
                           resources::poloniex::POLONIEX_TIMESTAMP_KEYWORD +
                           resources::symbols::EQUAL + std::to_string(timestamp);

  auto fullUrlWithParameters = uri + resources::symbols::QUESTION + parameters;
  curl_easy_setopt(curl, CURLOPT_URL, fullUrlWithParameters.data());
  curl_easy_setopt(curl, CURLOPT_POST, 1L);
  curl_easy_setopt(curl, CURLOPT_POSTFIELDS, parameters.c_str());

  Poco::HMACEngine<common::EncryptionSHA512Engine> hmac{secret_key_};
  hmac.update(parameters);
  std::string signature = DigestEngine::digestToHex(hmac.digest());

  std::string api_key_header =
      resources::poloniex::POLONIEX_HEADER_API_KEY_KEYWORD + ":" + api_key_;
  std::string signature_header =
      resources::poloniex::POLONIEX_HEADER_SIGNATURE_KEYWORD + ":" + signature;

  curl_slist* chunk = nullptr;
  chunk = curl_slist_append(chunk, api_key_header.c_str());
  chunk = curl_slist_append(chunk, signature_header.c_str());

  curl_easy_setopt(curl, CURLOPT_HTTPHEADER, chunk);

  std::string response = sendRequest(curl);

  auto jsonObject = getJsonObjectAndCheckOnIncorrectJson(response, chunk);
  checkPoloniexResponseMessage(jsonObject, chunk);

  auto success = jsonObject->get(resources::words::SUCCESS);

  return success;
}

std::vector<common::MarketOrder> PoloniexQuery::getAccountOpenOrders(
    common::Currency::Enum fromCurrency, common::Currency::Enum toCurrency) {
  using namespace Poco;

  CURL* curl = curl_easy_init();
  stock_exchange_utils::checkCurlPointer(curl);

  auto uri = resources::poloniex::POLONIEX_PRIVATE_ENDPOINT;

  auto timestamp = common::getCurrentMSEpoch();
  std::string parameters =
      resources::words::COMMAND + resources::symbols::EQUAL +
      resources::poloniex::POLONIEX_OPEN_ORDERS_KEYWORD + resources::symbols::AND +
      resources::poloniex::POLONIEX_CURRENCY_PAIR_KEYWORD + resources::symbols::EQUAL +
      resources::poloniex::POLONIEX_ALL_MARKETS + resources::symbols::AND +
      resources::poloniex::POLONIEX_TIMESTAMP_KEYWORD + resources::symbols::EQUAL +
      std::to_string(timestamp);

  auto fullUrlWithParameters = uri + resources::symbols::QUESTION + parameters;
  curl_easy_setopt(curl, CURLOPT_URL, fullUrlWithParameters.data());
  curl_easy_setopt(curl, CURLOPT_POST, 1L);
  curl_easy_setopt(curl, CURLOPT_POSTFIELDS, parameters.c_str());

  Poco::HMACEngine<common::EncryptionSHA512Engine> hmac{secret_key_};
  hmac.update(parameters);
  std::string signature = DigestEngine::digestToHex(hmac.digest());

  std::string api_key_header =
      resources::poloniex::POLONIEX_HEADER_API_KEY_KEYWORD + ":" + api_key_;
  std::string signature_header =
      resources::poloniex::POLONIEX_HEADER_SIGNATURE_KEYWORD + ":" + signature;

  curl_slist* chunk = nullptr;
  chunk = curl_slist_append(chunk, api_key_header.c_str());
  chunk = curl_slist_append(chunk, signature_header.c_str());

  curl_easy_setopt(curl, CURLOPT_HTTPHEADER, chunk);

  std::string response = sendRequest(curl);

  auto jsonObject = getJsonObjectAndCheckOnIncorrectJson(response, chunk);
  checkPoloniexResponseMessage(jsonObject, chunk);

  std::vector<common::MarketOrder> orders;
  auto mainObjectNamesList = jsonObject->getNames();
  for (const auto& pair_name : mainObjectNamesList) {
    auto openOrdersArray = jsonObject->getArray(pair_name);
    auto arraySize = openOrdersArray->size();
    for (int index = resources::numbers::FIRST_ARRAY_INDEX; index < arraySize; ++index) {
      auto openOrderJsonObject = openOrdersArray->getObject(index);
      common::MarketOrder orderInfo;

      std::string non_const_currency_pair = pair_name;
      std::transform(non_const_currency_pair.begin(), non_const_currency_pair.end(),
                     non_const_currency_pair.begin(), [](char ch) {
                       return ch == *resources::symbols::UNDER_LINE.data()
                                  ? *resources::symbols::SPACE.data()
                                  : ch;
                     });
      std::istringstream iss(non_const_currency_pair);
      std::vector<std::string> results((std::istream_iterator<std::string>(iss)),
                                       std::istream_iterator<std::string>());

      orderInfo.fromCurrency_ =
          common::Currency::fromString(results.at(resources::numbers::FIRST_ARRAY_INDEX));
      orderInfo.toCurrency_ =
          common::Currency::fromString(results.at(resources::numbers::SECOND_ARRAY_INDEX));

      if (orderInfo.fromCurrency_ != fromCurrency || orderInfo.toCurrency_ != toCurrency) {
        continue;
      }

      auto orderType_str = openOrderJsonObject->get(resources::words::TYPE).toString();
      orderInfo.orderType_ = orderType_str == resources::poloniex::POLONIEX_BUY_KEYWORD
                                 ? common::OrderType::BUY
                                 : common::OrderType::SELL;

      auto date_str = openOrderJsonObject->get(resources::poloniex::POLONIEX_DATE).toString();
      std::transform(date_str.begin(), date_str.end(), date_str.begin(), [](char ch) {
        return ch == *resources::symbols::SPACE.data() ? *resources::words::TIME_T_SYMBOL.data()
                                                       : ch;
      });
      orderInfo.opened_ = common::Date::parseDate(date_str);

      orderInfo.quantity_ = openOrderJsonObject->get(resources::words::AMOUNT);
      orderInfo.price_ = openOrderJsonObject->get(resources::words::RATE);
      orderInfo.uuid_ =
          openOrderJsonObject->get(resources::poloniex::POLONIEX_ORDER_NUMBER_ID).toString();

      orderInfo.stockExchangeType_ = common::StockExchangeType::Poloniex;
      orderInfo.isCanceled_ = false;

      orders.emplace_back(orderInfo);
    }
  }

  return orders;
}

std::vector<common::MarketOrder> PoloniexQuery::getMarketOpenOrders(
    common::Currency::Enum fromCurrency, common::Currency::Enum toCurrency) {
  using namespace Poco;

  CURL* curl = curl_easy_init();
  stock_exchange_utils::checkCurlPointer(curl);

  auto uri = resources::poloniex::POLONIEX_PUBLIC_ENDPOINT;

  std::string parameters =
      resources::words::COMMAND + resources::symbols::EQUAL +
      resources::poloniex::POLONIEX_TRADE_HISTORY_KEYWORD + resources::symbols::AND +
      resources::poloniex::POLONIEX_CURRENCY_PAIR_KEYWORD + resources::symbols::EQUAL +
      poloniexCurrency_.getPoloniexPair(fromCurrency, toCurrency);

  auto fullUrlWithParameters = uri + resources::symbols::QUESTION + parameters;
  curl_easy_setopt(curl, CURLOPT_URL, fullUrlWithParameters.data());
  curl_easy_setopt(curl, CURLOPT_HTTPHEADER, nullptr);

  std::string response = sendRequest(curl);

  if (response.at(resources::numbers::FIRST_ARRAY_INDEX) !=
      resources::symbols::LEFT_SQUARE_BRACKET) {
    auto jsonObject = getJsonObjectAndCheckOnIncorrectJson(response, nullptr);
    checkPoloniexResponseMessage(jsonObject, nullptr);
  }

  JSON::Parser parser;
  JSON::Array::Ptr objects = parser.parse(response).extract<JSON::Array::Ptr>();

  auto arraySize = objects->size();
  arraySize = arraySize > resources::numbers::MAX_MARKET_ORDERS_COUNT
                  ? resources::numbers::MAX_MARKET_ORDERS_COUNT
                  : arraySize;

  std::vector<common::MarketOrder> orders;
  for (unsigned int index = resources::numbers::FIRST_ARRAY_INDEX; index < arraySize; ++index) {
    auto currentOrder = objects->getObject(index);
    common::MarketOrder orderInfo;

    auto orderType_str = currentOrder->get(resources::words::TYPE).toString();
    orderInfo.orderType_ = orderType_str == resources::poloniex::POLONIEX_BUY_KEYWORD
                               ? common::OrderType::BUY
                               : common::OrderType::SELL;

    auto date_str = currentOrder->get(resources::poloniex::POLONIEX_DATE).toString();
    std::transform(date_str.begin(), date_str.end(), date_str.begin(), [](char ch) {
      return ch == *resources::symbols::SPACE.data() ? *resources::words::TIME_T_SYMBOL.data() : ch;
    });
    orderInfo.opened_ = common::Date::parseDate(date_str);

    orderInfo.quantity_ = currentOrder->get(resources::words::AMOUNT);
    orderInfo.price_ = currentOrder->get(resources::words::RATE);
    orderInfo.uuid_ = currentOrder->get(resources::poloniex::POLONIEX_GLOBAL_TRADE_ID).toString();

    orderInfo.stockExchangeType_ = common::StockExchangeType::Poloniex;
    orderInfo.fromCurrency_ = fromCurrency;
    orderInfo.toCurrency_ = toCurrency;

    orders.emplace_back(orderInfo);
  }

  return orders;
}

common::MarketOrder PoloniexQuery::getAccountOrder(common::Currency::Enum fromCurrency,
                                                   common::Currency::Enum toCurrency,
                                                   const std::string& uuid) {
  common::MarketOrder orderInfo;
  try {
    orderInfo = getOrderStatusParameters(uuid);
  } catch (const common::exceptions::InvalidStockExchangeResponse& ex) {
    try {
      orderInfo = getOrderTradesParameters(uuid);
    } catch (const common::exceptions::InvalidStockExchangeResponse& ex) {
      orderInfo.fromCurrency_ = fromCurrency;
      orderInfo.toCurrency_ = toCurrency;
      orderInfo.uuid_ = uuid;
      orderInfo.stockExchangeType_ = common::StockExchangeType::Poloniex;
      orderInfo.isCanceled_ = true;
    }
  }

  return orderInfo;
}

common::MarketHistoryPtr PoloniexQuery::getMarketHistory(common::Currency::Enum fromCurrency,
                                                         common::Currency::Enum toCurrency,
                                                         common::TickInterval::Enum interval) {
  using namespace Poco;

  CURL* curl = curl_easy_init();
  stock_exchange_utils::checkCurlPointer(curl);

  auto uri = resources::poloniex::POLONIEX_PUBLIC_ENDPOINT;

  auto timestamp = stock_exchange_utils::getTimestampMiliseconds();
  std::string nonce_start = std::to_string(timestamp - resources::poloniex::THREE_MONTH_IN_SECOND);
  std::string nonce_end = std::to_string(timestamp);

  std::string parameters =
      resources::words::COMMAND + resources::symbols::EQUAL +
      resources::poloniex::POLONIEX_CHART_DATA_KEYWORD + resources::symbols::AND +
      resources::poloniex::POLONIEX_CURRENCY_PAIR_KEYWORD + resources::symbols::EQUAL +
      poloniexCurrency_.getPoloniexPair(fromCurrency, toCurrency) + resources::symbols::AND +
      resources::poloniex::POLONIEX_START_TIME_KEYWORD + resources::symbols::EQUAL + nonce_start +
      resources::symbols::AND + resources::poloniex::POLONIEX_END_TIME_KEYWORD +
      resources::symbols::EQUAL + nonce_end + resources::symbols::AND +
      resources::poloniex::POLONIEX_TIME_INTERVAL_KEYWORD + resources::symbols::EQUAL +
      common::convertTickInterval(interval, common::StockExchangeType::Poloniex);

  auto fullUrlWithParameters = uri + resources::symbols::QUESTION + parameters;
  curl_easy_setopt(curl, CURLOPT_URL, fullUrlWithParameters.data());
  curl_easy_setopt(curl, CURLOPT_HTTPHEADER, nullptr);

  std::string response = sendRequest(curl);

  if (response.at(resources::numbers::FIRST_ARRAY_INDEX) !=
      resources::symbols::LEFT_SQUARE_BRACKET) {
    auto jsonObject = getJsonObjectAndCheckOnIncorrectJson(response, nullptr);
    checkPoloniexResponseMessage(jsonObject, nullptr);
  }

  auto marketHistoryData = parseMarketHistory(response);
  marketHistoryData->toBuy_ = toCurrency;
  marketHistoryData->toSell_ = fromCurrency;

  return marketHistoryData;
}

common::CurrencyTick PoloniexQuery::getCurrencyTick(common::Currency::Enum fromCurrency,
                                                    common::Currency::Enum toCurrency) {
  using namespace Poco;

  CURL* curl = curl_easy_init();
  stock_exchange_utils::checkCurlPointer(curl);

  auto uri = resources::poloniex::POLONIEX_PUBLIC_ENDPOINT;

  std::string parameters = resources::words::COMMAND + resources::symbols::EQUAL +
                           resources::poloniex::POLONIEX_TICKER_KEYWORD + resources::symbols::AND +
                           resources::poloniex::POLONIEX_CURRENCY_PAIR_KEYWORD +
                           resources::symbols::EQUAL +
                           poloniexCurrency_.getPoloniexPair(fromCurrency, toCurrency) +
                           resources::symbols::AND + resources::poloniex::POLONIEX_DEPTH_KEYWORD +
                           resources::symbols::EQUAL + std::to_string(resources::numbers::ONE);

  auto fullUrlWithParameters = uri + resources::symbols::QUESTION + parameters;
  curl_easy_setopt(curl, CURLOPT_URL, fullUrlWithParameters.data());
  curl_easy_setopt(curl, CURLOPT_HTTPHEADER, nullptr);

  std::string response = sendRequest(curl);

  auto jsonObject = getJsonObjectAndCheckOnIncorrectJson(response, nullptr);
  checkPoloniexResponseMessage(jsonObject, nullptr);

  common::CurrencyTick tick;
  auto asksArray = jsonObject->getArray(resources::words::ASKS);
  auto asksValue = asksArray->getArray(resources::numbers::FIRST_ARRAY_INDEX);
  tick.ask_ = asksValue->get(resources::numbers::FIRST_ARRAY_INDEX);

  auto bidsArray = jsonObject->getArray(resources::words::BIDS);
  auto bidsValue = bidsArray->getArray(resources::numbers::FIRST_ARRAY_INDEX);
  tick.bid_ = bidsValue->get(resources::numbers::FIRST_ARRAY_INDEX);

  tick.fromCurrency_ = fromCurrency;
  tick.toCurrency_ = toCurrency;

  return tick;
}

double PoloniexQuery::getBalance(common::Currency::Enum currency) {
  using namespace Poco;

  CURL* curl = curl_easy_init();
  stock_exchange_utils::checkCurlPointer(curl);

  auto uri = resources::poloniex::POLONIEX_PRIVATE_ENDPOINT;

  auto timestamp = common::getCurrentMSEpoch();
  std::string parameters = resources::words::COMMAND + resources::symbols::EQUAL +
                           resources::poloniex::POLONIEX_GET_BALANCE_KEYWORD +
                           resources::symbols::AND +
                           resources::poloniex::POLONIEX_TIMESTAMP_KEYWORD +
                           resources::symbols::EQUAL + std::to_string(timestamp);

  auto fullUrlWithParameters = uri + resources::symbols::QUESTION + parameters;
  curl_easy_setopt(curl, CURLOPT_URL, fullUrlWithParameters.data());
  curl_easy_setopt(curl, CURLOPT_POST, 1L);
  curl_easy_setopt(curl, CURLOPT_POSTFIELDS, parameters.c_str());

  Poco::HMACEngine<common::EncryptionSHA512Engine> hmac{secret_key_};
  hmac.update(parameters);
  std::string signature = DigestEngine::digestToHex(hmac.digest());

  std::string api_key_header =
      resources::poloniex::POLONIEX_HEADER_API_KEY_KEYWORD + ":" + api_key_;
  std::string signature_header =
      resources::poloniex::POLONIEX_HEADER_SIGNATURE_KEYWORD + ":" + signature;

  curl_slist* chunk = nullptr;
  chunk = curl_slist_append(chunk, api_key_header.c_str());
  chunk = curl_slist_append(chunk, signature_header.c_str());

  curl_easy_setopt(curl, CURLOPT_HTTPHEADER, chunk);

  std::string response = sendRequest(curl);

  auto jsonObject = getJsonObjectAndCheckOnIncorrectJson(response, chunk);
  checkPoloniexResponseMessage(jsonObject, chunk);

  auto balance = jsonObject->optValue(common::Currency::toString(currency),
                                      common::Currency::toString(common::Currency::UNKNOWN));
  if (balance == common::Currency::toString(common::Currency::UNKNOWN)) {
    throw common::exceptions::InvalidStockExchangeResponse("Unknown currency: " +
                                                           common::Currency::toString(currency));
  }

  return std::stod(balance);
}

CurrencyLotsHolder PoloniexQuery::getCurrencyLotsHolder() {
  CurrencyLotsHolder holder;
  return holder;
}

std::string PoloniexQuery::sendRequest(CURL* curl) {
  std::string response;
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, static_cast<void*>(&response));

  CURLcode responseResult = curl_easy_perform(curl);

  return response;
}

common::MarketHistoryPtr PoloniexQuery::parseMarketHistory(const std::string& response) const {
  using namespace Poco;

  JSON::Parser parser;
  JSON::Array::Ptr objects = parser.parse(response).extract<JSON::Array::Ptr>();

  auto marketHistory = std::make_unique<common::MarketHistory>();
  for (unsigned int index = 0; index < objects->size(); ++index) {
    auto object = objects->getObject(index);

    common::MarketData marketData;
    auto timestamp = object->get(resources::poloniex::POLONIEX_DATE);
    marketData.date_ = stock_exchange_utils::getDataFromTimestamp(timestamp, false);
    marketData.openPrice_ = object->get(resources::poloniex::POLONIEX_OPEN_PRICE);
    marketData.closePrice_ = object->get(resources::poloniex::POLONIEX_CLOSE_PRICE);
    marketData.lowPrice_ = object->get(resources::poloniex::POLONIEX_LOW_PRICE);
    marketData.highPrice_ = object->get(resources::poloniex::POLONIEX_HIGH_PRICE);
    marketData.volume_ = object->get(resources::poloniex::POLONIEX_VOLUME_PRICE);

    marketHistory->marketData_.push_back(marketData);
  }

  return marketHistory;
}

Poco::JSON::Object::Ptr PoloniexQuery::getJsonObjectAndCheckOnIncorrectJson(
    const std::string& response, curl_slist* chunk) {
  if (chunk != nullptr) {
    curl_slist_free_all(chunk);
  }

  Poco::JSON::Parser parser;
  if (response.at(0) != resources::symbols::LEFT_CURLY_BRACE || response.empty()) {
    throw common::exceptions::IncorrectJsonException("Incorrect json");
  }

  return parser.parse(response).extract<Poco::JSON::Object::Ptr>();
}

void PoloniexQuery::checkPoloniexResponseMessage(Poco::JSON::Object::Ptr& object,
                                                 curl_slist* chunk) {
  auto error = object->get(resources::words::ERROR_WORD);
  if (!error.isEmpty()) {
    common::loggers::FileLogger::getLogger()
        << resources::messages::FAILED_TO_UPLOAD_MESSAGE << error;
    throw common::exceptions::InvalidStockExchangeResponse(error.toString());
  }
}

common::MarketOrder PoloniexQuery::getOrderStatusParameters(const std::string& uuid) {
  using namespace Poco;

  CURL* curl = curl_easy_init();
  stock_exchange_utils::checkCurlPointer(curl);

  auto uri = resources::poloniex::POLONIEX_PRIVATE_ENDPOINT;

  auto timestamp = common::getCurrentMSEpoch();
  std::string parameters = resources::words::COMMAND + resources::symbols::EQUAL +
                           resources::poloniex::POLONIEX_GET_ORDER_STATUS_KEYWORD +
                           resources::symbols::AND + resources::poloniex::POLONIEX_ORDER_NUMBER_ID +
                           resources::symbols::EQUAL + uuid + resources::symbols::AND +
                           resources::poloniex::POLONIEX_TIMESTAMP_KEYWORD +
                           resources::symbols::EQUAL + std::to_string(timestamp);

  auto fullUrlWithParameters = uri + resources::symbols::QUESTION + parameters;
  curl_easy_setopt(curl, CURLOPT_URL, fullUrlWithParameters.data());
  curl_easy_setopt(curl, CURLOPT_POST, 1L);
  curl_easy_setopt(curl, CURLOPT_POSTFIELDS, parameters.c_str());

  Poco::HMACEngine<common::EncryptionSHA512Engine> hmac{secret_key_};
  hmac.update(parameters);
  std::string orderStatusSignature = DigestEngine::digestToHex(hmac.digest());

  std::string api_key_header =
      resources::poloniex::POLONIEX_HEADER_API_KEY_KEYWORD + ":" + api_key_;
  std::string signature_header =
      resources::poloniex::POLONIEX_HEADER_SIGNATURE_KEYWORD + ":" + orderStatusSignature;

  curl_slist* chunk = nullptr;
  chunk = curl_slist_append(chunk, api_key_header.c_str());
  chunk = curl_slist_append(chunk, signature_header.c_str());

  curl_easy_setopt(curl, CURLOPT_HTTPHEADER, chunk);

  std::string response = sendRequest(curl);

  auto jsonObject = getJsonObjectAndCheckOnIncorrectJson(response, chunk);
  auto resultJsonObject = jsonObject->getObject(resources::words::RESULT);
  checkPoloniexResponseMessage(resultJsonObject, chunk);

  common::MarketOrder orderInfo;
  auto infoJsonObject = resultJsonObject->getObject(uuid);

  auto orderType_str = infoJsonObject->get(resources::words::TYPE).toString();
  orderInfo.orderType_ = orderType_str == resources::poloniex::POLONIEX_BUY_KEYWORD
                             ? common::OrderType::BUY
                             : common::OrderType::SELL;

  auto date_str = infoJsonObject->get(resources::poloniex::POLONIEX_DATE).toString();
  std::transform(date_str.begin(), date_str.end(), date_str.begin(), [](char ch) {
    return ch == *resources::symbols::SPACE.data() ? *resources::words::TIME_T_SYMBOL.data() : ch;
  });
  orderInfo.opened_ = common::Date::parseDate(date_str);

  orderInfo.quantity_ = infoJsonObject->get(resources::words::AMOUNT);
  orderInfo.price_ = infoJsonObject->get(resources::words::RATE);
  orderInfo.uuid_ = uuid;

  orderInfo.stockExchangeType_ = common::StockExchangeType::Poloniex;
  orderInfo.isCanceled_ = false;

  std::string non_const_currency_pair =
      infoJsonObject->get(resources::poloniex::POLONIEX_CURRENCY_PAIR_KEYWORD);
  std::transform(non_const_currency_pair.begin(), non_const_currency_pair.end(),
                 non_const_currency_pair.begin(), [](char ch) {
                   return ch == *resources::symbols::UNDER_LINE.data()
                              ? *resources::symbols::SPACE.data()
                              : ch;
                 });
  std::istringstream iss(non_const_currency_pair);
  std::vector<std::string> results((std::istream_iterator<std::string>(iss)),
                                   std::istream_iterator<std::string>());

  orderInfo.fromCurrency_ =
      common::Currency::fromString(results.at(resources::numbers::FIRST_ARRAY_INDEX));
  orderInfo.toCurrency_ =
      common::Currency::fromString(results.at(resources::numbers::SECOND_ARRAY_INDEX));

  return orderInfo;
}

common::MarketOrder PoloniexQuery::getOrderTradesParameters(const std::string& uuid) {
  using namespace Poco;

  CURL* curl = curl_easy_init();
  stock_exchange_utils::checkCurlPointer(curl);

  auto uri = resources::poloniex::POLONIEX_PRIVATE_ENDPOINT;

  auto timestamp = common::getCurrentMSEpoch();
  std::string parameters = resources::words::COMMAND + resources::symbols::EQUAL +
                           resources::poloniex::POLONIEX_GET_ORDER_TRADES_KEYWORD +
                           resources::symbols::AND + resources::poloniex::POLONIEX_ORDER_NUMBER_ID +
                           resources::symbols::EQUAL + uuid + resources::symbols::AND +
                           resources::poloniex::POLONIEX_TIMESTAMP_KEYWORD +
                           resources::symbols::EQUAL + std::to_string(timestamp);

  auto fullUrlWithParameters = uri + resources::symbols::QUESTION + parameters;
  curl_easy_setopt(curl, CURLOPT_URL, fullUrlWithParameters.data());
  curl_easy_setopt(curl, CURLOPT_POST, 1L);
  curl_easy_setopt(curl, CURLOPT_POSTFIELDS, parameters.c_str());

  Poco::HMACEngine<common::EncryptionSHA512Engine> hmac{secret_key_};
  hmac.update(parameters);
  std::string orderStatusSignature = DigestEngine::digestToHex(hmac.digest());

  std::string api_key_header =
      resources::poloniex::POLONIEX_HEADER_API_KEY_KEYWORD + ":" + api_key_;
  std::string signature_header =
      resources::poloniex::POLONIEX_HEADER_SIGNATURE_KEYWORD + ":" + orderStatusSignature;

  curl_slist* chunk = nullptr;
  chunk = curl_slist_append(chunk, api_key_header.c_str());
  chunk = curl_slist_append(chunk, signature_header.c_str());

  curl_easy_setopt(curl, CURLOPT_HTTPHEADER, chunk);

  std::string response = sendRequest(curl);

  if (response.at(resources::numbers::FIRST_ARRAY_INDEX) !=
      resources::symbols::LEFT_SQUARE_BRACKET) {
    auto jsonObject = getJsonObjectAndCheckOnIncorrectJson(response, nullptr);
    checkPoloniexResponseMessage(jsonObject, nullptr);
  }

  JSON::Parser parser;
  JSON::Array::Ptr objects = parser.parse(response).extract<JSON::Array::Ptr>();

  common::MarketOrder orderInfo;
  auto currentOrder = objects->getObject(resources::numbers::FIRST_ARRAY_INDEX);

  auto orderType_str = currentOrder->get(resources::words::TYPE).toString();
  orderInfo.orderType_ = orderType_str == resources::poloniex::POLONIEX_BUY_KEYWORD
                             ? common::OrderType::BUY
                             : common::OrderType::SELL;

  auto date_str = currentOrder->get(resources::poloniex::POLONIEX_DATE).toString();
  std::transform(date_str.begin(), date_str.end(), date_str.begin(), [](char ch) {
    return ch == *resources::symbols::SPACE.data() ? *resources::words::TIME_T_SYMBOL.data() : ch;
  });
  orderInfo.opened_ = common::Date::parseDate(date_str);

  orderInfo.quantity_ = currentOrder->get(resources::words::AMOUNT);
  orderInfo.price_ = currentOrder->get(resources::words::RATE);
  orderInfo.uuid_ = currentOrder->get(resources::poloniex::POLONIEX_GLOBAL_TRADE_ID).toString();

  orderInfo.stockExchangeType_ = common::StockExchangeType::Poloniex;

  std::string non_const_currency_pair =
      currentOrder->get(resources::poloniex::POLONIEX_CURRENCY_PAIR_KEYWORD);
  std::transform(non_const_currency_pair.begin(), non_const_currency_pair.end(),
                 non_const_currency_pair.begin(), [](char ch) {
                   return ch == *resources::symbols::UNDER_LINE.data()
                              ? *resources::symbols::SPACE.data()
                              : ch;
                 });
  std::istringstream iss(non_const_currency_pair);
  std::vector<std::string> results((std::istream_iterator<std::string>(iss)),
                                   std::istream_iterator<std::string>());

  orderInfo.fromCurrency_ =
      common::Currency::fromString(results.at(resources::numbers::FIRST_ARRAY_INDEX));
  orderInfo.toCurrency_ =
      common::Currency::fromString(results.at(resources::numbers::SECOND_ARRAY_INDEX));

  return orderInfo;
}

}  // namespace stock_exchange
}  // namespace auto_trader