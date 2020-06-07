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

#ifndef AUTO_TRADER_COMMON_HUOBI_CURRENCY_H
#define AUTO_TRADER_COMMON_HUOBI_CURRENCY_H

#include <exception>
#include <map>
#include <vector>

#include "currency.h"

namespace auto_trader {
namespace common {

class HuobiCurrency {
 public:
  inline std::vector<Currency::Enum> getTradedCurrencies(Currency::Enum baseCurrency) {
    return huobiCurrencies_.at(baseCurrency);
  }

  inline std::vector<Currency::Enum> getBaseCurrencies() {
    std::vector<Currency::Enum> baseCurencies;
    for (const auto& currency : huobiCurrencies_) {
      baseCurencies.emplace_back(currency.first);
    }

    return baseCurencies;
  }

  inline std::string getHuobiPair(Currency::Enum fromCurrency, Currency::Enum toCurrency) {
    if (huobiCurrencies_.find(fromCurrency) != huobiCurrencies_.end()) {
      return Currency::toString(toCurrency) + Currency::toString(fromCurrency);
    } else {
      return Currency::toString(fromCurrency) + Currency::toString(toCurrency);
    }
  }

  inline std::pair<std::string, std::string> parseHuobiExchangeType(
      const std::string& currencyPair) {
    for (const auto& huobiPair : huobiCurrencies_) {
      std::string huobuCurrencyStr = common::Currency::toString(huobiPair.first);

      std::transform(huobuCurrencyStr.begin(), huobuCurrencyStr.end(), huobuCurrencyStr.begin(),
                     [](unsigned char symbol) { return std::tolower(symbol); });

      std::size_t found = currencyPair.rfind(huobuCurrencyStr);
      if (found != std::string::npos && found != 0) {
        std::string baseCurrency = currencyPair.substr(found, std::string::npos);
        std::string tradedCurrency = currencyPair.substr(0, found);

        std::transform(baseCurrency.begin(), baseCurrency.end(), baseCurrency.begin(),
                       [](unsigned char symbol) { return std::toupper(symbol); });

        std::transform(tradedCurrency.begin(), tradedCurrency.end(), tradedCurrency.begin(),
                       [](unsigned char symbol) { return std::toupper(symbol); });

        return std::make_pair(baseCurrency, tradedCurrency);
      }
    }

    throw std::invalid_argument("Unknown huobi pair");
  }

 private:
  const std::map<const Currency::Enum, std::vector<Currency::Enum>> huobiCurrencies_ = {
      {Currency::USDT,
       {Currency::BTC,   Currency::ETH,   Currency::HT,    Currency::XRP,   Currency::LTC,
        Currency::EOS,   Currency::ETC,   Currency::TRX,   Currency::BSV,   Currency::ZEC,
        Currency::ADA,   Currency::DASH,  Currency::XMR,   Currency::IOTA,  Currency::SEELE,
        Currency::EKT,   Currency::LAMB,  Currency::ATOM,  Currency::FSN,   Currency::ALGO,
        Currency::IOST,  Currency::LINK,  Currency::CKB,   Currency::BHD,   Currency::ONE,
        Currency::LXT,   Currency::HPT,   Currency::ONT,   Currency::BTM,   Currency::EM,
        Currency::QTUM,  Currency::BTT,   Currency::PVT,   Currency::NODE,  Currency::CTXC,
        Currency::VET,   Currency::TT,    Currency::ZIL,   Currency::ITC,   Currency::XTZ,
        Currency::XLM,   Currency::NEW,   Currency::AKRO,  Currency::VIDY,  Currency::EGT,
        Currency::FOR,   Currency::LOL,   Currency::HC,    Currency::STEEM, Currency::NEO,
        Currency::MX,    Currency::WAVES, Currency::SKM,   Currency::GXC,   Currency::IRIS,
        Currency::PAI,   Currency::LET,   Currency::THETA, Currency::OGO,   Currency::NAS,
        Currency::DOGE,  Currency::OMG,   Currency::WXT,   Currency::TOP,   Currency::DTA,
        Currency::VSYS,  Currency::OCN,   Currency::WICC,  Currency::LBA,   Currency::UIP,
        Currency::RSR,   Currency::WTC,   Currency::ARPA,  Currency::CRE,   Currency::MANA,
        Currency::CMT,   Currency::FTT,   Currency::KAN,   Currency::ELF,   Currency::ELA,
        Currency::CNNS,  Currency::BAT,   Currency::BTS,   Currency::ACT,   Currency::ATP,
        Currency::RUFF,  Currency::SOC,   Currency::DOCK,  Currency::CVC,   Currency::BHT,
        Currency::DCR,   Currency::NKN,   Currency::HIT,   Currency::MDS,   Currency::REN,
        Currency::STORJ, Currency::SMT,   Currency::GT,    Currency::XEM,   Currency::NULS,
        Currency::BIX,   Currency::CRO,   Currency::XZC,   Currency::SNT,   Currency::ZRX,
        Currency::NANO,  Currency::GNT}},
      {Currency::HUSD,
       {Currency::PAX, Currency::USDC, Currency::TUSD, Currency::USDT, Currency::BTC, Currency::HT,
        Currency::ETH, Currency::XRP, Currency::LTC, Currency::EOS, Currency::BCH, Currency::BSV}},
      {Currency::BTC,
       {Currency::ETH,   Currency::SEELE, Currency::EOS,    Currency::BCH,   Currency::TRX,
        Currency::XRP,   Currency::LTC,   Currency::BSV,    Currency::FTT,   Currency::HT,
        Currency::XMR,   Currency::NEXO,  Currency::ZEC,    Currency::LAMB,  Currency::ETC,
        Currency::ATOM,  Currency::NEO,   Currency::DASH,   Currency::EKT,   Currency::WAVES,
        Currency::IOTA,  Currency::CTXC,  Currency::LINK,   Currency::SHE,   Currency::CKB,
        Currency::EKO,   Currency::UTK,   Currency::HPT,    Currency::CNN,   Currency::CRO,
        Currency::XTZ,   Currency::THETA, Currency::ADA,    Currency::CHAT,  Currency::SNC,
        Currency::DAC,   Currency::KNC,   Currency::UIP,    Currency::KCASH, Currency::XLM,
        Currency::VET,   Currency::DOGE,  Currency::IOST,   Currency::QUN,   Currency::DCR,
        Currency::VSYS,  Currency::PAI,   Currency::QTUM,   Currency::AIDOC, Currency::UGAS,
        Currency::ITC,   Currency::LXT,   Currency::FSN,    Currency::ONT,   Currency::ALGO,
        Currency::BAT,   Currency::DATX,  Currency::KAN,    Currency::ZEN,   Currency::NEW,
        Currency::MANA,  Currency::NULS,  Currency::HC,     Currency::BTT,   Currency::WTC,
        Currency::STEEM, Currency::MDS,   Currency::LBA,    Currency::ONE,   Currency::FTI,
        Currency::MX,    Currency::GXC,   Currency::BTM,    Currency::PROPY, Currency::MT,
        Currency::MUSK,  Currency::VIDY,  Currency::ZIL,    Currency::BTG,   Currency::NODE,
        Currency::WXT,   Currency::RCCC,  Currency::TRIO,   Currency::FAIR,  Currency::OMG,
        Currency::SWFTC, Currency::ZRX,   Currency::PORTAL, Currency::LET,   Currency::COVA,
        Currency::OGO,   Currency::EGT,   Currency::GNX,    Currency::STORJ, Currency::WICC,
        Currency::CMT,   Currency::ELA,   Currency::OCN,    Currency::AKRO,  Currency::DTA,
        Currency::EM,    Currency::ACT,   Currency::ATP,    Currency::XZC,   Currency::BHD,
        Currency::IRIS,  Currency::SNT,   Currency::DGB,    Currency::XEM,   Currency::QASH,
        Currency::MEET,  Currency::TOP}},
      {Currency::ETH,
       {Currency::EOS,   Currency::HT,    Currency::TRX,   Currency::ALGO,  Currency::ADA,
        Currency::XMR,   Currency::IOTA,  Currency::EKT,   Currency::RCCC,  Currency::SEELE,
        Currency::KCASH, Currency::DAC,   Currency::FTI,   Currency::IOST,  Currency::PORTAL,
        Currency::SHE,   Currency::ATOM,  Currency::MEET,  Currency::LAMB,  Currency::LINK,
        Currency::KNC,   Currency::VET,   Currency::ZIL,   Currency::LXT,   Currency::QTUM,
        Currency::BTT,   Currency::OCN,   Currency::WAVES, Currency::SWFTC, Currency::EKO,
        Currency::XVG,   Currency::XTZ,   Currency::CTXC,  Currency::BTM,   Currency::MUSK,
        Currency::ONT,   Currency::UGAS,  Currency::STEEM, Currency::XLM,   Currency::AIDOC,
        Currency::MANA,  Currency::FAIR,  Currency::DATX,  Currency::IRIS,  Currency::LET,
        Currency::ITC,   Currency::MT,    Currency::ZEN,   Currency::PAI,   Currency::DTA,
        Currency::QUN,   Currency::DOGE,  Currency::WICC,  Currency::DOCK,  Currency::DGD,
        Currency::OMG,   Currency::TNB,   Currency::THETA, Currency::NEXO,  Currency::ICX,
        Currency::CHAT,  Currency::KAN,   Currency::LBA,   Currency::ELA,   Currency::BAT,
        Currency::GXC,   Currency::COVA,  Currency::WTC,   Currency::NAS,   Currency::SNC,
        Currency::UTK,   Currency::REN,   Currency::NULS,  Currency::GNX,   Currency::CNN,
        Currency::UIP,   Currency::ELF,   Currency::HC,    Currency::RUFF,  Currency::HOT,
        Currency::CMT,   Currency::ACT,   Currency::CVC,   Currency::MDS,   Currency::KMD,
        Currency::SRN,   Currency::NANO,  Currency::SOC,   Currency::NPXS,  Currency::DGB,
        Currency::BIX,   Currency::QASH,  Currency::SMT,   Currency::ZRX,   Currency::TRIO,
        Currency::HIT,   Currency::PROPY, Currency::XZC,   Currency::DCR,   Currency::SC,
        Currency::GNT}},
      {Currency::HT,
       {Currency::XRP,  Currency::LTC,  Currency::BCH,   Currency::EOS,  Currency::ETC,
        Currency::DASH, Currency::HPT,  Currency::KCASH, Currency::IOST, Currency::BHD,
        Currency::NODE, Currency::NEW,  Currency::LOL,   Currency::CKB,  Currency::ONE,
        Currency::MT,   Currency::TT,   Currency::PVT,   Currency::AKRO, Currency::FOR,
        Currency::EGT,  Currency::SKM,  Currency::EM,    Currency::ARPA, Currency::MX,
        Currency::VIDY, Currency::VSYS, Currency::NKN,   Currency::FTT,  Currency::CRE,
        Currency::CNNS, Currency::TOP,  Currency::WXT,   Currency::OGO,  Currency::BHT,
        Currency::ATP,  Currency::GT,   Currency::CRO}}};
};

}  // namespace common
}  // namespace auto_trader

#endif  // AUTO_TRADER_COMMON_BINANCE_CURRENCY_H
