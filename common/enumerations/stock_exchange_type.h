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

#ifndef DATA_STOCK_EXCHANGE_TYPE_H_
#define DATA_STOCK_EXCHANGE_TYPE_H_

#include <string>

#include "common/exceptions/undefined_type_exception.h"

namespace auto_trader {
namespace common {

enum class StockExchangeType { Bittrex, Binance, Kraken, Poloniex, Huobi, UNKNOWN };

static std::string convertStockExchangeTypeToString(StockExchangeType type) {
  switch (type) {
    case StockExchangeType::Bittrex:
      return "Bittrex";
    case StockExchangeType::Binance:
      return "Binance";
    case StockExchangeType::Kraken:
      return "Kraken";
    case StockExchangeType::Poloniex:
      return "Poloniex";
    case StockExchangeType::Huobi:
      return "Huobi";
    default:
      return "UNKNOWN";
  }
}

static StockExchangeType convertStockExchangeTypeFromString(const std::string& type) {
  if (type == "Bittrex") {
    return StockExchangeType::Bittrex;
  } else if (type == "Binance") {
    return StockExchangeType::Binance;
  } else if (type == "Kraken") {
    return StockExchangeType::Kraken;
  } else if (type == "Poloniex") {
    return StockExchangeType::Poloniex;
  } else if (type == "Huobi") {
    return StockExchangeType::Huobi;
  } else {
    return StockExchangeType::UNKNOWN;
  }
}

static std::pair<std::string, std::string> parseBittrexExchangeType(const std::string& type) {
  auto dashTockenPosition = type.find('-');
  const std::string& fromTypeStr = type.substr(0, dashTockenPosition);
  const std::string& toTypeStr = type.substr(dashTockenPosition + 1);
  return std::make_pair<>(fromTypeStr, toTypeStr);
}

static std::pair<std::string, std::string> parseBinanceExchangeType(const std::string& type) {
  if (type == "ETHBTC")
    return std::make_pair("ETH", "BTC");
  else if (type == "LTCBTC")
    return std::make_pair("LTC", "BTC");
  else if (type == "XRPBNB")
    return std::make_pair("XRP", "BNB");
  else if (type == "BNBBTC")
    return std::make_pair("BNB", "BTC");
  else if (type == "NEOBTC")
    return std::make_pair("NEO", "BTC");
  else if (type == "QTUMETH")
    return std::make_pair("QTUM", "ETH");
  else if (type == "EOSETH")
    return std::make_pair("EOS", "ETH");
  else if (type == "SNTETH")
    return std::make_pair("SNT", "ETH");
  else if (type == "BNTETH")
    return std::make_pair("BNT", "ETH");
  else if (type == "BCCBTC")
    return std::make_pair("BCC", "BTC");
  else if (type == "GASBTC")
    return std::make_pair("GAS", "BTC");
  else if (type == "BNBETH")
    return std::make_pair("BNB", "ETH");
  else if (type == "BTCUSDT")
    return std::make_pair("BTC", "USDT");
  else if (type == "ETHUSDT")
    return std::make_pair("ETH", "USDT");
  else if (type == "HSRBTC")
    return std::make_pair("HSR", "BTC");
  else if (type == "OAXETH")
    return std::make_pair("OAX", "ETH");
  else if (type == "DNTETH")
    return std::make_pair("DNT", "ETH");
  else if (type == "MCOETH")
    return std::make_pair("MCO", "ETH");
  else if (type == "ICNETH")
    return std::make_pair("ICN", "ETH");
  else if (type == "MCOBTC")
    return std::make_pair("MCO", "BTC");
  else if (type == "WTCBTC")
    return std::make_pair("WTC", "BTC");
  else if (type == "WTCETH")
    return std::make_pair("WTC", "ETH");
  else if (type == "LRCBTC")
    return std::make_pair("LRC", "BTC");
  else if (type == "LRCETH")
    return std::make_pair("LRC", "ETH");
  else if (type == "QTUMBTC")
    return std::make_pair("QTUM", "BTC");
  else if (type == "YOYOBTC")
    return std::make_pair("YOYO", "BTC");
  else if (type == "OMGBTC")
    return std::make_pair("OMG", "BTC");
  else if (type == "OMGETH")
    return std::make_pair("OMG", "ETH");
  else if (type == "ZRXBTC")
    return std::make_pair("ZRX", "BTC");
  else if (type == "ZRXETH")
    return std::make_pair("ZRX", "ETH");
  else if (type == "STRATBTC")
    return std::make_pair("STRAT", "BTC");
  else if (type == "STRATETH")
    return std::make_pair("STRAT", "ETH");
  else if (type == "SNGLSBTC")
    return std::make_pair("SNGLS", "BTC");
  else if (type == "SNGLSETH")
    return std::make_pair("SNGLS", "ETH");
  else if (type == "BQXBTC")
    return std::make_pair("BQX", "BTC");
  else if (type == "BQXETH")
    return std::make_pair("BQX", "ETH");
  else if (type == "KNCBTC")
    return std::make_pair("KNC", "BTC");
  else if (type == "KNCETH")
    return std::make_pair("KNC", "ETH");
  else if (type == "FUNBTC")
    return std::make_pair("FUN", "BTC");
  else if (type == "FUNETH")
    return std::make_pair("FUN", "ETH");
  else if (type == "SNMBTC")
    return std::make_pair("SNM", "BTC");
  else if (type == "SNMETH")
    return std::make_pair("SNM", "ETH");
  else if (type == "NEOETH")
    return std::make_pair("NEO", "ETH");
  else if (type == "IOTABTC")
    return std::make_pair("IOTA", "BTC");
  else if (type == "IOTAETH")
    return std::make_pair("IOTA", "ETH");
  else if (type == "LINKBTC")
    return std::make_pair("LINK", "BTC");
  else if (type == "LINKETH")
    return std::make_pair("LINK", "ETH");
  else if (type == "XVGBTC")
    return std::make_pair("XVG", "BTC");
  else if (type == "XVGETH")
    return std::make_pair("XVG", "ETH");
  else if (type == "SALTBTC")
    return std::make_pair("SALT", "BTC");
  else if (type == "SALTETH")
    return std::make_pair("SALT", "ETH");
  else if (type == "MDABTC")
    return std::make_pair("MDA", "BTC");

  throw exceptions::UndefinedTypeException("Binance exchange pair");
}

}  // namespace common
}  // namespace auto_trader

#endif /* DATA_STOCK_EXCHANGE_TYPE_H_ */
