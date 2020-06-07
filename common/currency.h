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

#ifndef COMMON_CURRENCY_H_
#define COMMON_CURRENCY_H_

#include <map>
#include <string>

#include "common/exceptions/undefined_type_exception.h"

namespace auto_trader {
namespace common {

class Currency {
 public:
  enum Enum {
    USD,
    BTC,
    XBT,
    ETH,
    LTC,
    XRP,
    USDT,
    ADA,
    BCH,
    BAT,
    ZEN,
    ETC,
    BSV,
    ZEC,
    TRX,
    DGB,
    TUSD,
    USDC,
    EDR,
    SPND,
    ZRX,
    USDS,
    KMD,
    SC,
    PAX,

    SOLVE,
    ORBS,
    RVN,
    CMCT,
    VBK,
    META,
    IOP,
    AERGO,
    NGC,
    IHT,
    XVG,
    ENJ,
    WAX,
    MONA,

    ANT,
    XEM,
    XLM,
    REP,
    STRAT,
    OMG,
    ANKR,
    WAVES,
    SRN,

    NEO,
    DASH,
    DOGE,
    XMR,

    ATOM,
    EOS,
    CAD,
    EUR,
    GBP,
    JPY,
    GNO,
    MLN,
    QTUM,
    XTZ,

    XDG,

    FET,
    BTT,
    ICX,
    VET,
    NANO,
    ONT,
    ZIL,
    WTC,
    MITH,
    HOT,

    BNB,
    BCHABC,
    DGD,
    CELR,
    QKC,
    NAS,
    IOTX,
    AST,
    TNB,

    FUEL,
    ELF,
    NPXS,

    LINK,

    BCHSV,
    STR,
    GRIN,
    FCT,
    MAID,
    BTS,
    KNC,
    NXT,

    // Huobi
    IOTA,
    SEELE,
    HT,
    EKT,
    LAMB,
    FSN,
    ALGO,
    IOST,
    CKB,
    BHD,
    ONE,
    LXT,
    HPT,
    BTM,
    EM,
    PVT,
    CTXC,
    NODE,
    TT,
    ITC,
    NEW,
    AKRO,
    VIDY,
    EGT,
    FOR,
    LOL,
    STEEM,
    IRIS,
    HC,
    MX,
    SKM,
    GXC,
    PAI,
    LET,
    THETA,
    OGO,
    WXT,
    TOP,
    DTA,
    VSYS,
    OCN,
    WICC,
    LBA,
    UIP,
    RSR,
    ARPA,
    MANA,
    CRE,
    CMT,
    FTT,
    KAN,
    ELA,
    CNNS,
    ACT,
    ATP,
    RUFF,
    SOC,
    DOCK,
    CVC,
    BHT,
    DCR,
    NKN,
    NULS,
    STORJ,
    REN,
    SMT,
    MDS,
    SNT,
    XZC,
    CRO,
    GT,
    HIT,
    BIX,
    GNT,
    KCASH,
    MT,
    HUSD,
    NEXO,
    SHE,
    EKO,
    UTK,
    CHAT,
    AIDOC,
    UGAS,
    DATX,
    PROPY,
    MUSK,
    SWFTC,
    FTI,
    FAIR,
    PORTAL,
    MEET,
    TRIO,
    QUN,
    CNN,
    DAC,
    BTG,
    COVA,
    RCCC,
    SNC,
    QASH,
    GNX,

    UNKNOWN
  };

  static std::string toString(Currency::Enum currency) {
    switch (currency) {
      case Currency::BTC:
        return "BTC";
      case Currency::XBT:
        return "XBT";
      case Currency::USD:
        return "USD";
      case Currency::LTC:
        return "LTC";
      case Currency::USDT:
        return "USDT";
      case Currency::USDC:
        return "USDC";
      case Currency::DOGE:
        return "DOGE";
      case Currency::DASH:
        return "DASH";
      case Currency::MONA:
        return "MONA";
      case Currency::XMR:
        return "XMR";
      case Currency::DGB:
        return "DGB";
      case Currency::XRP:
        return "XRP";
      case Currency::XEM:
        return "XEM";
      case Currency::ETH:
        return "ETH";
      case Currency::ADA:
        return "ADA";
      case Currency::BCH:
        return "BCH";
      case Currency::BAT:
        return "BAT";
      case Currency::ZEN:
        return "ZEN";
      case Currency::ETC:
        return "ETC";
      case Currency::BSV:
        return "BSV";
      case Currency::ZEC:
        return "ZEC";
      case Currency::TRX:
        return "TRX";
      case Currency::TUSD:
        return "TUSD";
      case Currency::EDR:
        return "EDR";
      case Currency::SPND:
        return "SPND";
      case Currency::ZRX:
        return "ZRX";
      case Currency::USDS:
        return "USDS";
      case Currency::KMD:
        return "KMD";
      case Currency::SC:
        return "SC";
      case Currency::PAX:
        return "PAX";
      case Currency::SOLVE:
        return "SOLVE";
      case Currency::ORBS:
        return "ORBS";
      case Currency::RVN:
        return "RVN";
      case Currency::CMCT:
        return "CMCT";
      case Currency::VBK:
        return "VBK";
      case Currency::META:
        return "META";
      case Currency::IOP:
        return "IOP";
      case Currency::AERGO:
        return "AERGO";
      case Currency::NGC:
        return "NGC";
      case Currency::IHT:
        return "IHT";
      case Currency::XVG:
        return "XVG";
      case Currency::ENJ:
        return "ENJ";
      case Currency::WAX:
        return "WAX";
      case Currency::ANT:
        return "ANT";
      case Currency::XLM:
        return "XLM";
      case Currency::REP:
        return "REP";
      case Currency::STRAT:
        return "STRAT";
      case Currency::OMG:
        return "OMG";
      case Currency::ANKR:
        return "ANKR";
      case Currency::WAVES:
        return "WAVES";
      case Currency::SRN:
        return "SRN";
      case Currency::NEO:
        return "NEO";
      case Currency::ATOM:
        return "ATOM";
      case Currency::EOS:
        return "EOS";
      case Currency::CAD:
        return "CAD";
      case Currency::EUR:
        return "EUR";
      case Currency::GBP:
        return "GBP";
      case Currency::JPY:
        return "JPY";
      case Currency::GNO:
        return "GNO";
      case Currency::MLN:
        return "MLN";
      case Currency::QTUM:
        return "QTUM";
      case Currency::XTZ:
        return "XTZ";
      case Currency::XDG:
        return "XDG";
      case Currency::FET:
        return "FET";
      case Currency::BTT:
        return "BTT";
      case Currency::ICX:
        return "ICX";
      case Currency::VET:
        return "VET";
      case Currency::NANO:
        return "NANO";
      case Currency::ONT:
        return "ONT";
      case Currency::ZIL:
        return "ZIL";
      case Currency::WTC:
        return "WTC";
      case Currency::MITH:
        return "MITH";
      case Currency::HOT:
        return "HOT";
      case Currency::BNB:
        return "BNB";
      case Currency::BCHABC:
        return "BCHABC";
      case Currency::DGD:
        return "DGD";
      case Currency::CELR:
        return "CELR";
      case Currency::QKC:
        return "QKC";
      case Currency::NAS:
        return "NAS";
      case Currency::IOTX:
        return "IOTX";
      case Currency::AST:
        return "AST";
      case Currency::TNB:
        return "TNB";
      case Currency::FUEL:
        return "FUEL";
      case Currency::ELF:
        return "ELF";
      case Currency::NPXS:
        return "NPXS";
      case Currency::LINK:
        return "LINK";
      case Currency::BCHSV:
        return "BCHSV";
      case Currency::STR:
        return "STR";
      case Currency::GRIN:
        return "GRIN";
      case Currency::FCT:
        return "FCT";
      case Currency::MAID:
        return "MAID";
      case Currency::BTS:
        return "BTS";
      case Currency::KNC:
        return "KNC";
      case Currency::NXT:
        return "NXT";
      case Currency::IOTA:
        return "IOTA";
      case Currency::SEELE:
        return "SEELE";
      case Currency::HT:
        return "HT";
      case Currency::EKT:
        return "EKT";
      case Currency::LAMB:
        return "LAMB";
      case Currency::FSN:
        return "FSN";
      case Currency::ALGO:
        return "ALGO";
      case Currency::IOST:
        return "IOST";
      case Currency::CKB:
        return "CKB";
      case Currency::BHD:
        return "BHD";
      case Currency::ONE:
        return "ONE";
      case Currency::LXT:
        return "LXT";
      case Currency::HPT:
        return "HPT";
      case Currency::BTM:
        return "BTM";
      case Currency::EM:
        return "EM";
      case Currency::PVT:
        return "PVT";
      case Currency::CTXC:
        return "CTXC";
      case Currency::NODE:
        return "NODE";
      case Currency::TT:
        return "TT";
      case Currency::ITC:
        return "ITC";
      case Currency::NEW:
        return "NEW";
      case Currency::AKRO:
        return "AKRO";
      case Currency::VIDY:
        return "VIDY";
      case Currency::EGT:
        return "EGT";
      case Currency::FOR:
        return "FOR";
      case Currency::LOL:
        return "LOL";
      case Currency::STEEM:
        return "STEEM";
      case Currency::IRIS:
        return "IRIS";
      case Currency::HC:
        return "HC";
      case Currency::MX:
        return "MX";
      case Currency::SKM:
        return "SKM";
      case Currency::GXC:
        return "GXC";
      case Currency::PAI:
        return "PAI";
      case Currency::LET:
        return "LET";
      case Currency::THETA:
        return "THETA";
      case Currency::OGO:
        return "OGO";
      case Currency::WXT:
        return "WXT";
      case Currency::TOP:
        return "TOP";
      case Currency::DTA:
        return "DTA";
      case Currency::VSYS:
        return "VSYS";
      case Currency::OCN:
        return "OCN";
      case Currency::WICC:
        return "WICC";
      case Currency::LBA:
        return "LBA";
      case Currency::UIP:
        return "UIP";
      case Currency::RSR:
        return "RSR";
      case Currency::ARPA:
        return "ARPA";
      case Currency::MANA:
        return "MANA";
      case Currency::CRE:
        return "CRE";
      case Currency::CMT:
        return "CMT";
      case Currency::FTT:
        return "FTT";
      case Currency::KAN:
        return "KAN";
      case Currency::ELA:
        return "ELA";
      case Currency::CNNS:
        return "CNNS";
      case Currency::ACT:
        return "ACT";
      case Currency::ATP:
        return "ATP";
      case Currency::RUFF:
        return "RUFF";
      case Currency::SOC:
        return "SOC";
      case Currency::DOCK:
        return "DOCK";
      case Currency::CVC:
        return "CVC";
      case Currency::BHT:
        return "BHT";
      case Currency::DCR:
        return "DCR";
      case Currency::NKN:
        return "NKN";
      case Currency::NULS:
        return "NULS";
      case Currency::STORJ:
        return "STORJ";
      case Currency::REN:
        return "REN";
      case Currency::SMT:
        return "SMT";
      case Currency::MDS:
        return "MDS";
      case Currency::SNT:
        return "SNT";
      case Currency::XZC:
        return "XZC";
      case Currency::CRO:
        return "CRO";
      case Currency::GT:
        return "GT";
      case Currency::HIT:
        return "HIT";
      case Currency::BIX:
        return "BIX";
      case Currency::GNT:
        return "GNT";
      case Currency::KCASH:
        return "KCASH";
      case Currency::MT:
        return "MT";
      case Currency::HUSD:
        return "HUSD";
      case Currency::NEXO:
        return "NEXO";
      case Currency::SHE:
        return "SHE";
      case Currency::EKO:
        return "EKO";
      case Currency::UTK:
        return "UTK";
      case Currency::CHAT:
        return "CHAT";
      case Currency::AIDOC:
        return "AIDOC";
      case Currency::UGAS:
        return "UGAS";
      case Currency::DATX:
        return "DATX";
      case Currency::PROPY:
        return "PROPY";
      case Currency::MUSK:
        return "MUSK";
      case Currency::SWFTC:
        return "SWFTC";
      case Currency::FTI:
        return "FTI";
      case Currency::FAIR:
        return "FAIR";
      case Currency::PORTAL:
        return "PORTAL";
      case Currency::MEET:
        return "MEET";
      case Currency::TRIO:
        return "TRIO";
      case Currency::QUN:
        return "QUN";
      case Currency::CNN:
        return "CNN";
      case Currency::DAC:
        return "DAC";
      case Currency::BTG:
        return "BTG";
      case Currency::COVA:
        return "COVA";
      case Currency::RCCC:
        return "RCCC";
      case Currency::SNC:
        return "SNC";
      case Currency::QASH:
        return "QASH";
      case Currency::GNX:
        return "GNX";

      default:
        return "UNKNOWN";
    }
  }

  static Enum fromString(const std::string& currency) {
    static std::map<const std::string, Currency::Enum> currencies_str_ = {
        {"USD", USD},     {"BTC", BTC},       {"ETH", ETH},       {"LTC", LTC},
        {"XRP", XRP},     {"USDT", USDT},     {"USDC", USDC},     {"ADA", ADA},
        {"BCH", BCH},     {"BAT", BAT},       {"ZEN", ZEN},       {"ETC", ETC},
        {"BSV", BSV},     {"ZEC", ZEC},       {"TRX", TRX},       {"DGB", DGB},
        {"TUSD", TUSD},   {"EDR", EDR},       {"SPND", SPND},     {"ZRX", ZRX},
        {"USDS", USDS},   {"KMD", KMD},       {"SC", SC},         {"PAX", PAX},
        {"SOLVE", SOLVE}, {"ORBS", ORBS},     {"RVN", RVN},       {"CMCT", CMCT},
        {"VBK", VBK},     {"META", META},     {"IOP", IOP},       {"AERGO", AERGO},
        {"NGC", NGC},     {"IHT", IHT},       {"XVG", XVG},       {"ENJ", ENJ},
        {"WAX", WAX},     {"MONA", MONA},     {"ANT", ANT},       {"XEM", XEM},
        {"XLM", XLM},     {"REP", REP},       {"STRAT", STRAT},   {"OMG", OMG},
        {"ANKR", ANKR},   {"WAVES", WAVES},   {"SRN", SRN},       {"NEO", NEO},
        {"DASH", DASH},   {"DOGE", DOGE},     {"XMR", XMR},       {"ATOM", ATOM},
        {"EOS", EOS},     {"CAD", CAD},       {"EUR", EUR},       {"GBP", GBP},
        {"JPY", JPY},     {"GNO", GNO},       {"MLN", MLN},       {"QTUM", QTUM},
        {"XTZ", XTZ},     {"XDG", XDG},       {"FET", FET},       {"BTT", BTT},
        {"ICX", ICX},     {"VET", VET},       {"NANO", NANO},     {"ONT", ONT},
        {"ZIL", ZIL},     {"WTC", WTC},       {"MITH", MITH},     {"HOT", HOT},
        {"BNB", BNB},     {"BCHABC", BCHABC}, {"DGD", DGD},       {"CELR", CELR},
        {"QKC", QKC},     {"NAS", NAS},       {"IOTX", IOTX},     {"AST", AST},
        {"TNB", TNB},     {"FUEL", FUEL},     {"ELF", ELF},       {"NPXS", NPXS},
        {"LINK", LINK},   {"BCHSV", BCHSV},   {"STR", STR},       {"GRIN", GRIN},
        {"FCT", FCT},     {"MAID", MAID},     {"BTS", BTS},       {"KNC", KNC},
        {"NXT", NXT},     {"IOTA", IOTA},     {"SEELE", SEELE},   {"HT", HT},
        {"EKT", EKT},     {"LAMB", LAMB},     {"FSN", FSN},       {"ALGO", ALGO},
        {"IOST", IOST},   {"CKB", CKB},       {"BHD", BHD},       {"ONE", ONE},
        {"LXT", LXT},     {"HPT", HPT},       {"BTM", BTM},       {"EM", EM},
        {"PVT", PVT},     {"CTXC", CTXC},     {"NODE", NODE},     {"TT", TT},
        {"ITC", ITC},     {"NEW", NEW},       {"AKRP", AKRO},     {"VIDY", VIDY},
        {"EGT", EGT},     {"FOR", FOR},       {"LOL", LOL},       {"STEEM", STEEM},
        {"IRIS", IRIS},   {"HC", HC},         {"MX", MX},         {"SKM", SKM},
        {"GXC", GXC},     {"PAI", PAI},       {"LET", LET},       {"THETA", THETA},
        {"OGO", OGO},     {"WXT", WXT},       {"TOP", TOP},       {"DTA", DTA},
        {"VSYS", VSYS},   {"OCN", OCN},       {"WICC", WICC},     {"LBA", LBA},
        {"UIP", UIP},     {"RSR", RSR},       {"ARPA", ARPA},     {"MANA", MANA},
        {"CRE", CRE},     {"CMT", CMT},       {"FTT", FTT},       {"KAN", KAN},
        {"ELA", ELA},     {"CNNS", CNNS},     {"ACT", ACT},       {"ATP", ATP},
        {"RUFF", RUFF},   {"SOC", SOC},       {"DOCK", DOCK},     {"CVC", CVC},
        {"BHT", BHT},     {"DCR", DCR},       {"NKN", NKN},       {"STORJ", STORJ},
        {"REN", REN},     {"SMT", SMT},       {"MDS", MDS},       {"SNT", SNT},
        {"XZC", XZC},     {"CRO", CRO},       {"GT", GT},         {"HIT", HIT},
        {"BIX", BIX},     {"GNT", GNT},       {"KCASH", KCASH},   {"MT", MT},
        {"HUSD", HUSD},   {"NEXO", NEXO},     {"SHE", SHE},       {"EKO", EKO},
        {"UTK", UTK},     {"CHAT", CHAT},     {"AIDOC", AIDOC},   {"UGAS", UGAS},
        {"DATX", DATX},   {"PROPY", PROPY},   {"MUSK", MUSK},     {"SWFTC", SWFTC},
        {"FTI", FTI},     {"FAIR", FAIR},     {"PORTAL", PORTAL}, {"MEET", MEET},
        {"TRIO", TRIO},   {"QUN", QUN},       {"CNN", CNN},       {"DAC", DAC},
        {"NULS", NULS},   {"BTG", BTG},       {"COVA", COVA},     {"RCCC", RCCC},
        {"SNC", SNC},     {"QASH", QASH},     {"GNX", GNX}};

    auto it = currencies_str_.find(currency);
    if (it == currencies_str_.end()) {
      return UNKNOWN;
    }
    return it->second;
  }
};

struct CurrencyTick {
  double ask_;
  double bid_;

  common::Currency::Enum fromCurrency_;
  common::Currency::Enum toCurrency_;

  bool operator==(const CurrencyTick& tick) {
    return ask_ == tick.ask_ && bid_ == tick.bid_ && fromCurrency_ == tick.fromCurrency_ &&
           toCurrency_ == tick.toCurrency_;
  }
};

}  // namespace common
}  // namespace auto_trader

#endif /* DATA_CURRENCY_H_ */
