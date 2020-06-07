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

#ifndef AUTO_TRADER_STOCK_EXCHANGE_UTILS_H
#define AUTO_TRADER_STOCK_EXCHANGE_UTILS_H

#include <Poco/JSON/Array.h>
#include <curl/curl.h>

#include <QDateTime>
#include <ctime>

#include "common/binance_currency.h"
#include "common/bittrex_currency.h"
#include "common/exceptions/undefined_type_exception.h"
#include "common/kraken_currency.h"
#include "common/market_order.h"
#include "common/poloniex_currency.h"

namespace auto_trader {
namespace stock_exchange_utils {

static void parseOpenedOrdersForType(const Poco::JSON::Array::Ptr objects,
                                     std::vector<common::MarketOrder>& outfillingObject,
                                     common::OrderType type,
                                     common::StockExchangeType shockExchange) {
  int quantityFieldId = 0;
  int priceFieldId = 0;

  switch (shockExchange) {
    case common::StockExchangeType::Binance: {
      quantityFieldId = 0;
      priceFieldId = 1;
      break;
    }

    case common::StockExchangeType::Kraken: {
      quantityFieldId = 1;
      priceFieldId = 0;
      break;
    }

    default:
      break;
  }

  for (int index = 0; index < objects->size(); ++index) {
    auto object = objects->getArray(index);
    auto quantity = object->get(quantityFieldId).toString();
    auto price = object->get(priceFieldId).toString();

    common::MarketOrder order;
    order.quantity_ = std::stod(quantity);
    order.price_ = std::stod(price);
    order.orderType_ = type;

    outfillingObject.emplace_back(order);
  }
}

static std::pair<std::string, std::string> parseBinanceExchangeType(const std::string& type) {
  if (type == "LTCBNB")
    return std::make_pair("BNB", "LTC");
  else if (type == "XRPBNB")
    return std::make_pair("BNB", "XRP");
  else if (type == "EOSBNB")
    return std::make_pair("BNB", "EOS");
  else if (type == "ADABNB")
    return std::make_pair("BNB", "ADA");
  else if (type == "BATBNB")
    return std::make_pair("BNB", "BAT");
  else if (type == "FETBNB")
    return std::make_pair("BNB", "FET");
  else if (type == "TRXBNB")
    return std::make_pair("BNB", "TRX");
  else if (type == "BTTBNB")
    return std::make_pair("BNB", "BTT");
  else if (type == "ICXBNB")
    return std::make_pair("BNB", "ICX");
  else if (type == "ENJBNB")
    return std::make_pair("BNB", "ENJ");
  else if (type == "XLMBNB")
    return std::make_pair("BNB", "XLM");
  else if (type == "NEOBNB")
    return std::make_pair("BNB", "NEO");
  else if (type == "RVNBNB")
    return std::make_pair("BNB", "RVN");
  else if (type == "VETBNB")
    return std::make_pair("BNB", "VET");
  else if (type == "NANOBNB")
    return std::make_pair("BNB", "NANO");
  else if (type == "ONTBNB")
    return std::make_pair("BNB", "ONT");
  else if (type == "ZILBNB")
    return std::make_pair("BNB", "ZIL");
  else if (type == "WTCBNB")
    return std::make_pair("BNB", "WTC");
  else if (type == "MITHBNB")
    return std::make_pair("BNB", "MITH");
  else if (type == "HOTBNB")
    return std::make_pair("BNB", "HOT");

  else if (type == "BNBBTC")
    return std::make_pair("BTC", "BNB");
  else if (type == "ETHBTC")
    return std::make_pair("BTC", "ETH");
  else if (type == "BCHABCBTC")
    return std::make_pair("BTC", "BCHABC");
  else if (type == "DGDBTC")
    return std::make_pair("BTC", "DGD");
  else if (type == "BATBTC")
    return std::make_pair("BTC", "BAT");
  else if (type == "XRPBTC")
    return std::make_pair("BTC", "XRP");
  else if (type == "ADABTC")
    return std::make_pair("BTC", "ADA");
  else if (type == "LTCBTC")
    return std::make_pair("BTC", "LTC");
  else if (type == "EOSBTC")
    return std::make_pair("BTC", "EOS");
  else if (type == "ICXBTC")
    return std::make_pair("BTC", "ICX");
  else if (type == "ETHBTC")
    return std::make_pair("BTC", "ETH");
  else if (type == "CELRBTC")
    return std::make_pair("CELR", "BTC");
  else if (type == "FETBTC")
    return std::make_pair("BTC", "FET");
  else if (type == "ENJBTC")
    return std::make_pair("BTC", "ENJ");
  else if (type == "QKCBTC")
    return std::make_pair("BTC", "QKC");
  else if (type == "TRXBTC")
    return std::make_pair("BTC", "TRX");
  else if (type == "NANOBTC")
    return std::make_pair("BTC", "NANO");
  else if (type == "NASBTC")
    return std::make_pair("BTC", "NAS");
  else if (type == "IOTXBTC")
    return std::make_pair("BTC", "IOTX");
  else if (type == "ASTBTC")
    return std::make_pair("BTC", "AST");
  else if (type == "RVNBTC")
    return std::make_pair("BTC", "RVN");
  else if (type == "WTCBTC")
    return std::make_pair("TNB", "BTC");

  else if (type == "BNBETH")
    return std::make_pair("ETH", "BNB");
  else if (type == "DGDETH")
    return std::make_pair("ETH", "DGD");
  else if (type == "FUELETH")
    return std::make_pair("ETH", "FUEL");
  else if (type == "BATETH")
    return std::make_pair("ETH", "BAT");
  else if (type == "XRPETH")
    return std::make_pair("ETH", "XRP");
  else if (type == "ADAETH")
    return std::make_pair("ETH", "ADA");
  else if (type == "ENJETH")
    return std::make_pair("ETH", "ENJ");
  else if (type == "TRXETH")
    return std::make_pair("ETH", "TRX");
  else if (type == "ICXETH")
    return std::make_pair("ETH", "ICX");
  else if (type == "HOTETH")
    return std::make_pair("ETH", "HOT");
  else if (type == "QKCETH")
    return std::make_pair("ETH", "QKC");
  else if (type == "VETETH")
    return std::make_pair("ETH", "VET");
  else if (type == "DASHETH")
    return std::make_pair("ETH", "DASH");
  else if (type == "LTCETH")
    return std::make_pair("ETH", "LTC");
  else if (type == "ONTETH")
    return std::make_pair("ETH", "ONT");
  else if (type == "ZECETH")
    return std::make_pair("ETH", "ZEC");
  else if (type == "IOTXETH")
    return std::make_pair("ETH", "IOTX");
  else if (type == "NASETH")
    return std::make_pair("ETH", "NAS");

  else if (type == "BTCUSDT")
    return std::make_pair("USDT", "BTC");
  else if (type == "BNBUSDT")
    return std::make_pair("USDT", "BNB");
  else if (type == "ETHUSDT")
    return std::make_pair("USDT", "ETH");
  else if (type == "BTTUSDT")
    return std::make_pair("USDT", "BTT");
  else if (type == "BCHUSDT")
    return std::make_pair("USDT", "BCH");
  else if (type == "BCHABCUSDT")
    return std::make_pair("USDT", "BCHABC");
  else if (type == "LTCUSDT")
    return std::make_pair("USDT", "LTC");
  else if (type == "EOSUSDT")
    return std::make_pair("USDT", "EOS");
  else if (type == "XRPUSDT")
    return std::make_pair("USDT", "XRP");
  else if (type == "ADAUSDT")
    return std::make_pair("USDT", "ADA");
  else if (type == "FETUSDT")
    return std::make_pair("USDT", "FET");
  else if (type == "TRXUSDT")
    return std::make_pair("USDT", "TRX");
  else if (type == "ICXUSDT")
    return std::make_pair("USDT", "ICX");
  else if (type == "NEOUSDT")
    return std::make_pair("USDT", "NEO");
  else if (type == "BATUSDT")
    return std::make_pair("USDT", "BAT");
  else if (type == "TUSDUSDT")
    return std::make_pair("USDT", "TUSD");
  else if (type == "ONTUSDT")
    return std::make_pair("USDT", "ONT");
  else if (type == "PAXUSDT")
    return std::make_pair("USDT", "PAX");
  else if (type == "ETCUSDT")
    return std::make_pair("USDT", "ETC");
  else if (type == "XLMUSDT")
    return std::make_pair("USDT", "XLM");

  else if (type == "BTCTUSD")
    return std::make_pair("TUSD", "BTC");
  else if (type == "BNBTUSD")
    return std::make_pair("TUSD", "BNB");
  else if (type == "ETHTUSD")
    return std::make_pair("TUSD", "ETH");
  else if (type == "BCHABCTUSD")
    return std::make_pair("TUSD", "BCHABC");
  else if (type == "LTCTUSD")
    return std::make_pair("TUSD", "LTC");
  else if (type == "PAXTUSD")
    return std::make_pair("TUSD", "PAX");
  else if (type == "ADATUSD")
    return std::make_pair("TUSD", "ADA");
  else if (type == "EOSTUSD")
    return std::make_pair("TUSD", "EOS");
  else if (type == "USDSTUSD")
    return std::make_pair("TUSD", "USDS");
  else if (type == "XRPTUSD")
    return std::make_pair("TUSD", "XRP");
  else if (type == "TRXTUSD")
    return std::make_pair("TUSD", "TRX");
  else if (type == "NEOTUSD")
    return std::make_pair("TUSD", "NEO");
  else if (type == "WAVESTUSD")
    return std::make_pair("TUSD", "WAVES");
  else if (type == "ZECTUSD")
    return std::make_pair("TUSD", "ZEC");
  else if (type == "LINKTUSD")
    return std::make_pair("TUSD", "LINK");
  else if (type == "BTTTUSD")
    return std::make_pair("TUSD", "BTT");

  throw common::exceptions::UndefinedTypeException("Binance exchange pair");
}

static QDateTime unixtime2datetime(QString strUnixDate, bool isMilliseconds) {
  int denominator = isMilliseconds ? 1000 : 1;
  return QDateTime::fromTime_t(strUnixDate.toLongLong() / denominator);
}

static common::Date getDataFromTimestamp(time_t timestamp, bool isMilliseconds) {
  common::Date currentDate;
  auto timestamp_str = std::to_string(timestamp);
  QDateTime dt = unixtime2datetime(timestamp_str.data(), isMilliseconds);

  auto date = dt.date();
  auto time = dt.time();

  currentDate.second_ = time.second();
  currentDate.minute_ = time.minute();
  currentDate.hour_ = time.hour();

  currentDate.year_ = date.year();
  currentDate.month_ = date.month();
  currentDate.day_ = date.day();

  return currentDate;
}

static std::pair<std::string, std::string> parseKrakenCyrrencyPair(const std::string& pair) {
  if (pair == "ADAUSD")
    return std::make_pair("USD", "ADA");
  else if (pair == "ATOMUSD")
    return std::make_pair("USD", "ATOM");
  else if (pair == "BCHUSD")
    return std::make_pair("USD", "BCH");
  else if (pair == "DASHUSD")
    return std::make_pair("USD", "DASH");
  else if (pair == "EOSUSD")
    return std::make_pair("USD", "EOS");
  else if (pair == "ETCUSD")
    return std::make_pair("USD", "ETC");
  else if (pair == "ETHUSD")
    return std::make_pair("USD", "ETH");
  else if (pair == "GNOUSD")
    return std::make_pair("USD", "GNO");
  else if (pair == "LTCUSD")
    return std::make_pair("USD", "LTC");
  else if (pair == "QTUMUSD")
    return std::make_pair("USD", "QTUMU");
  else if (pair == "REPUSD")
    return std::make_pair("USD", "REP");
  else if (pair == "XLMUSD")
    return std::make_pair("USD", "XLM");
  else if (pair == "XMRUSD")
    return std::make_pair("USD", "XMR");
  else if (pair == "XRPUSD")
    return std::make_pair("USD", "XRP");
  else if (pair == "XTZUSD")
    return std::make_pair("USD", "XTZ");
  else if (pair == "ZECUSD")
    return std::make_pair("USD", "ZEC");

  else if (pair == "ADAETH")
    return std::make_pair("ETH", "ADA");
  else if (pair == "ATOMETH")
    return std::make_pair("ETH", "ATOM");
  else if (pair == "EOSETH")
    return std::make_pair("ETH", "EOS");
  else if (pair == "ETCETH")
    return std::make_pair("ETH", "ETC");
  else if (pair == "ETHCAD")
    return std::make_pair("ETH", "CAD");
  else if (pair == "ETHEUR")
    return std::make_pair("ETH", "EUR");
  else if (pair == "ETHGBP")
    return std::make_pair("ETH", "GBP");
  else if (pair == "ETHJPY")
    return std::make_pair("ETH", "JPY");
  else if (pair == "ETHUSD")
    return std::make_pair("ETH", "USD");
  else if (pair == "GNOETH")
    return std::make_pair("ETH", "GNO");
  else if (pair == "MLNETH")
    return std::make_pair("ETH", "MLN");
  else if (pair == "QTUMETH")
    return std::make_pair("ETH", "QTUM");
  else if (pair == "REPETH")
    return std::make_pair("ETH", "REP");
  else if (pair == "XTZETH")
    return std::make_pair("ETH", "XTZ");

  else if (pair == "ADAEUR")
    return std::make_pair("EUR", "ADA");
  else if (pair == "ATOMEUR")
    return std::make_pair("EUR", "ATOM");
  else if (pair == "BCHEUR")
    return std::make_pair("EUR", "BCH");
  else if (pair == "DASHEUR")
    return std::make_pair("EUR", "DASH");
  else if (pair == "EOSEUR")
    return std::make_pair("EUR", "EOS");
  else if (pair == "ETCEUR")
    return std::make_pair("EUR", "ETC");
  else if (pair == "ETHEUR")
    return std::make_pair("EUR", "ETH");
  else if (pair == "GNOEUR")
    return std::make_pair("EUR", "GNO");
  else if (pair == "LTCEUR")
    return std::make_pair("EUR", "LTC");
  else if (pair == "QTUMEUR")
    return std::make_pair("EUR", "QTUM");
  else if (pair == "REPEUR")
    return std::make_pair("EUR", "REP");
  else if (pair == "XBTEUR")
    return std::make_pair("BTC", "EUR");
  else if (pair == "XLMEUR")
    return std::make_pair("EUR", "XLM");
  else if (pair == "XMREUR")
    return std::make_pair("EUR", "XMR");
  else if (pair == "XRPEUR")
    return std::make_pair("EUR", "XRP");
  else if (pair == "XTZEUR")
    return std::make_pair("EUR", "XTZ");
  else if (pair == "ZECEUR")
    return std::make_pair("EUR", "ZEC");

  else if (pair == "ADAXBT")
    return std::make_pair("BTC", "ADA");
  else if (pair == "ATOMXBT")
    return std::make_pair("BTC", "ATOM");
  else if (pair == "BCHXBT")
    return std::make_pair("BTC", "BCH");
  else if (pair == "DASHXBT")
    return std::make_pair("BTC", "DASH");
  else if (pair == "EOSXBT")
    return std::make_pair("BTC", "EOS");
  else if (pair == "ETCXBT")
    return std::make_pair("BTC", "ETC");
  else if (pair == "ETHXBT")
    return std::make_pair("BTC", "ETH");
  else if (pair == "GNOXBT")
    return std::make_pair("BTC", "GNO");
  else if (pair == "LTCXBT")
    return std::make_pair("BTC", "LTC");
  else if (pair == "MLNXBT")
    return std::make_pair("BTC", "MLN");
  else if (pair == "QTUMXBT")
    return std::make_pair("BTC", "QTUM");
  else if (pair == "REPXBT")
    return std::make_pair("BTC", "REP");
  else if (pair == "XBTUSD")
    return std::make_pair("BTC", "USD");
  else if (pair == "XDGXBT")
    return std::make_pair("BTC", "XDG");
  else if (pair == "XLMXBT")
    return std::make_pair("BTC", "XLM");
  else if (pair == "XMRXBT")
    return std::make_pair("BTC", "XMR");
  else if (pair == "XRPXBT")
    return std::make_pair("BTC", "XRP");
  else if (pair == "XTZXBT")
    return std::make_pair("BTC", "XTZ");
  else if (pair == "ZECXBT")
    return std::make_pair("BTC", "ZEC");

  throw common::exceptions::UndefinedTypeException("Kraken exchange pair");
}

static common::Currency::Enum getKrakenCurrencyFromString(const std::string& currency) {
  if (currency == "ZUSD") return common::Currency::USD;
  if (currency == "ADA") return common::Currency::ADA;
  if (currency == "ATOM") return common::Currency::ATOM;
  if (currency == "BCH") return common::Currency::BCH;
  if (currency == "DASH") return common::Currency::DASH;
  if (currency == "EOS") return common::Currency::EOS;
  if (currency == "XETC") return common::Currency::ETC;
  if (currency == "XETH") return common::Currency::ETH;
  if (currency == "GNO") return common::Currency::GNO;
  if (currency == "XLTC") return common::Currency::LTC;
  if (currency == "QTUM") return common::Currency::QTUM;
  if (currency == "XREP ") return common::Currency::REP;
  if (currency == "XXBT") return common::Currency::BTC;
  if (currency == "XXLM") return common::Currency::XLM;
  if (currency == "XXMR") return common::Currency::XMR;
  if (currency == "XXRP") return common::Currency::XRP;
  if (currency == "XTZ") return common::Currency::XTZ;
  if (currency == "XZEC") return common::Currency::ZEC;
  if (currency == "ZCAD") return common::Currency::CAD;
  if (currency == "ZEUR") return common::Currency::EUR;
  if (currency == "ZGBP") return common::Currency::GBP;
  if (currency == "ZJPY") return common::Currency::JPY;
  if (currency == "XMLN") return common::Currency::MLN;
  if (currency == "XXDG") return common::Currency::XDG;

  throw common::exceptions::UndefinedTypeException("Undefine kraken currency enum");
}

static std::string getKrakenCurrencyStringFromEnum(const common::Currency::Enum currency) {
  if (currency == common::Currency::USD) return "ZUSD";
  if (currency == common::Currency::ADA) return "ADA";
  if (currency == common::Currency::ATOM) return "ATOM";
  if (currency == common::Currency::BCH) return "BCH";
  if (currency == common::Currency::DASH) return "DASH";
  if (currency == common::Currency::EOS) return "EOS";
  if (currency == common::Currency::ETC) return "XETC";
  if (currency == common::Currency::ETH) return "XETH";
  if (currency == common::Currency::GNO) return "GNO";
  if (currency == common::Currency::BTC) return "XXBT";
  if (currency == common::Currency::LTC) return "XLTC";
  if (currency == common::Currency::QTUM) return "QTUM";
  if (currency == common::Currency::REP) return "XREP";
  if (currency == common::Currency::XLM) return "XXLM";
  if (currency == common::Currency::XMR) return "XXMR";
  if (currency == common::Currency::XRP) return "XXRP";
  if (currency == common::Currency::XTZ) return "XTZ";
  if (currency == common::Currency::ZEC) return "XZEC";
  if (currency == common::Currency::CAD) return "ZCAD";
  if (currency == common::Currency::EUR) return "ZEUR";
  if (currency == common::Currency::GBP) return "ZGBP";
  if (currency == common::Currency::JPY) return "ZJPY";
  if (currency == common::Currency::MLN) return "XMLN";
  if (currency == common::Currency::XDG) return "XXDG";

  throw common::exceptions::UndefinedTypeException("Undefine kraken currency string");
}

static std::string getMarketPair(common::StockExchangeType stockExchangeType,
                                 common::Currency::Enum baseCurrency,
                                 common::Currency::Enum tradedCurrency) {
  switch (stockExchangeType) {
    case common::StockExchangeType::Binance: {
      common::BinanceCurrency binanceCurrency;
      return binanceCurrency.getBinancePair(baseCurrency, tradedCurrency);
    }

    case common::StockExchangeType::Bittrex: {
      common::BittrexCurrency bittrexCurrency;
      return bittrexCurrency.getBittrexPair(baseCurrency, tradedCurrency);
    }

    case common::StockExchangeType::Kraken: {
      common::KrakenCurrency krakenCurrency;
      return krakenCurrency.getKrakenPair(baseCurrency, tradedCurrency);
    }

    case common::StockExchangeType::Poloniex: {
      common::PoloniexCurrency poloniexCurrency;
      return poloniexCurrency.getPoloniexPair(baseCurrency, tradedCurrency);
    }

    default:
      return "";
  }
}

static time_t getTimestampMiliseconds() {
  std::time_t result = std::time(nullptr);
  std::asctime(std::localtime(&result));
  return result;
}

static size_t writeCurlSize(char* ptr, size_t size, size_t nmemb, void* userdata) {
  std::string* response = reinterpret_cast<std::string*>(userdata);
  size_t real_size = size * nmemb;

  response->append(ptr, real_size);
  return real_size;
}

static void checkCurlPointer(CURL* curl) {
  if (curl) {
    curl_easy_setopt(curl, CURLOPT_VERBOSE, 0L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 2L);

    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCurlSize);
  } else {
    throw common::exceptions::BaseException("Can't create curl handle");
  }
}

static const std::string base64_chars =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
    "abcdefghijklmnopqrstuvwxyz"
    "0123456789+/";

static std::string HuobiB64Encode(unsigned char const* bytes_to_encode, unsigned int in_len) {
  std::string ret;
  int i = 0;
  int j = 0;
  unsigned char char_array_3[3];
  unsigned char char_array_4[4];

  while (in_len--) {
    char_array_3[i++] = *(bytes_to_encode++);
    if (i == 3) {
      char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
      char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
      char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
      char_array_4[3] = char_array_3[2] & 0x3f;

      for (i = 0; (i < 4); i++) ret += base64_chars[char_array_4[i]];
      i = 0;
    }
  }

  if (i) {
    for (j = i; j < 3; j++) char_array_3[j] = '\0';

    char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
    char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
    char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
    char_array_4[3] = char_array_3[2] & 0x3f;

    for (j = 0; (j < i + 1); j++) ret += base64_chars[char_array_4[j]];

    while ((i++ < 3)) ret += '=';
  }

  return ret;
}

static char dec2hexChar(short int n) {
  if (0 <= n && n <= 9) {
    return char(short('0') + n);
  } else if (10 <= n && n <= 15) {
    return char(short('A') + n - 10);
  } else {
    return char(0);
  }
}

static std::string escapeURL(const std::string& URL) {
  std::string result = "";
  for (unsigned int i = 0; i < URL.size(); i++) {
    char c = URL[i];
    if (('0' <= c && c <= '9') || ('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z') || c == '/' ||
        c == '.') {
      result += c;
    } else {
      int j = (short int)c;
      if (j < 0) {
        j += 256;
      }
      int i1, i0;
      i1 = j / 16;
      i0 = j - i1 * 16;
      result += '%';
      result += dec2hexChar(i1);
      result += dec2hexChar(i0);
    }
  }
  return result;
}

}  // namespace stock_exchange_utils
}  // namespace auto_trader

#endif  // AUTO_TRADER_STOCK_EXCHANGE_UTILS_H
