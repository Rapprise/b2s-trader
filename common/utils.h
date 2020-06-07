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

#ifndef AUTO_TRADER_COMMON_UTILS_H
#define AUTO_TRADER_COMMON_UTILS_H

#include <assert.h>

#include <iomanip>
#include <sstream>
#include <string>

#ifndef WIN32
#include <sys/time.h>
#endif

#include <openssl/bio.h>
#include <openssl/buffer.h>
#include <openssl/hmac.h>
#include <openssl/sha.h>

#include "common/exceptions/stock_exchange_exception/encoding_query_signature_exception.h"
#include "common/exceptions/undefined_type_exception.h"
#include "date.h"
#include "enumerations/stock_exchange_type.h"
#include "enumerations/tick_interval.h"
#include "market_data.h"
#include "market_order.h"
#include "tick_interval_ratio.h"

#ifdef WIN32
#include <winsock2.h>
struct custom_timeval {
  uint64_t tv_sec;  /* seconds */
  uint64_t tv_usec; /* and microseconds */
};

inline int gettimeofday(struct custom_timeval* tp, struct timezone* tzp) {
  // Note: some broken versions only have 8 trailing zero's, the correct epoch has 9 trailing zero's
  // This magic number is the number of 100 nanosecond intervals since January 1, 1601 (UTC)
  // until 00:00:00 January 1, 1970
  static const uint64_t EPOCH = ((uint64_t)116444736000000000ULL);

  SYSTEMTIME system_time;
  FILETIME file_time;
  uint64_t time;

  GetSystemTime(&system_time);
  SystemTimeToFileTime(&system_time, &file_time);
  time = ((uint64_t)file_time.dwLowDateTime);
  time += ((uint64_t)file_time.dwHighDateTime) << 32;

  tp->tv_sec = (uint64_t)((time - EPOCH) / 10000000L);
  tp->tv_usec = (uint64_t)(system_time.wMilliseconds * 1000);
  return 0;
}
#endif

namespace auto_trader {
namespace common {

inline uint64_t getCurrentMSEpoch() {
#ifdef WIN32
  struct custom_timeval tv;
  gettimeofday(&tv, NULL);
  return (uint64_t)(tv.tv_sec * 1000 + tv.tv_usec / 1000);
#else
  struct timeval tv;
  gettimeofday(&tv, NULL);
  return (uint64_t)(tv.tv_sec * 1000 + tv.tv_usec / 1000);
#endif
}

inline const char* convertTickInterval(common::TickInterval::Enum interval,
                                       StockExchangeType stockExchange) {
  switch (stockExchange) {
    case StockExchangeType::Binance: {
      return BinanceTickInterval.at(interval);
    }
    case StockExchangeType::Bittrex: {
      return BittrexTickInterval.at(interval);
    }
    case StockExchangeType::Kraken: {
      return KrakenickInterval.at(interval);
    }
    case StockExchangeType::Poloniex: {
      return PoloniexInterval.at(interval);
    }
    case StockExchangeType::Huobi: {
      return HuobiInterval.at(interval);
    }
    default: {
      throw exceptions::UndefinedTypeException("Stock Exchange Type");
    }
  };
}

inline bool isStockExchangeContainsTickInterval(common::TickInterval::Enum interval,
                                                StockExchangeType stockExchange) {
  switch (stockExchange) {
    case StockExchangeType::Binance: {
      return (BinanceTickInterval.find(interval) != BinanceTickInterval.end());
    }
    case StockExchangeType::Bittrex: {
      return (BittrexTickInterval.find(interval) != BittrexTickInterval.end());
    }
    case StockExchangeType::Kraken: {
      return (KrakenickInterval.find(interval) != KrakenickInterval.end());
    }
    case StockExchangeType::Poloniex: {
      return (PoloniexInterval.find(interval) != PoloniexInterval.end());
    }
    default: {
      throw exceptions::UndefinedTypeException("Stock Exchange Type");
    }
  };
}

static std::vector<unsigned char> decode_to_sha_256(const std::string& data) {
  std::vector<unsigned char> digest(SHA256_DIGEST_LENGTH);

  SHA256_CTX ctx;
  SHA256_Init(&ctx);
  SHA256_Update(&ctx, data.c_str(), data.length());
  SHA256_Final(digest.data(), &ctx);

  return digest;
}

static std::vector<unsigned char> decode_to_hmac_sha_512(const std::vector<unsigned char>& data,
                                                         const std::vector<unsigned char>& key) {
  unsigned int len = EVP_MAX_MD_SIZE;
  std::vector<unsigned char> digest(len);

  HMAC_CTX* ctx = HMAC_CTX_new();

  HMAC_Init_ex(ctx, key.data(), key.size(), EVP_sha512(), NULL);
  HMAC_Update(ctx, data.data(), data.size());
  HMAC_Final(ctx, digest.data(), &len);

  HMAC_CTX_free(ctx);

  return digest;
}

static std::vector<unsigned char> base_64_decode(const std::string& data) {
  BIO* b64 = BIO_new(BIO_f_base64());
  BIO_set_flags(b64, BIO_FLAGS_BASE64_NO_NL);

  BIO* bmem = BIO_new_mem_buf((void*)data.c_str(), data.length());
  bmem = BIO_push(b64, bmem);

  std::vector<unsigned char> output(data.length());
  int decoded_size = BIO_read(bmem, output.data(), output.size());
  BIO_free_all(bmem);

  if (decoded_size < 0)
    throw exceptions::EncodingQuerySignatureException(
        "Failed while decoding query signature with SHA512.");

  return output;
}

static std::string base_64_encode(const std::vector<unsigned char>& data) {
  BIO* b64 = BIO_new(BIO_f_base64());
  BIO_set_flags(b64, BIO_FLAGS_BASE64_NO_NL);

  BIO* bmem = BIO_new(BIO_s_mem());
  b64 = BIO_push(b64, bmem);

  BIO_write(b64, data.data(), data.size());
  BIO_flush(b64);

  BUF_MEM* bptr = NULL;
  BIO_get_mem_ptr(b64, &bptr);

  std::string output(bptr->data, bptr->length);
  BIO_free_all(b64);

  return output;
}

static std::string generateSignatureSHA512(const std::string& uri_path, const std::string& nonce,
                                           const std::string& post_data,
                                           const std::string& secret_key) {
  std::vector<unsigned char> data(uri_path.begin(), uri_path.end());
  std::vector<unsigned char> nonce_postdata = common::decode_to_sha_256(nonce + post_data);
  data.insert(data.end(), nonce_postdata.begin(), nonce_postdata.end());

  return common::base_64_encode(
      common::decode_to_hmac_sha_512(data, common::base_64_decode(secret_key)));
}

static bool lowPriceCompare(const MarketData& left, const MarketData& right) {
  return left.lowPrice_ < right.lowPrice_;
}

static bool highPriceCompare(const MarketData& left, const MarketData& right) {
  return left.highPrice_ < right.highPrice_;
}

}  // namespace common
}  // namespace auto_trader

#endif  // AUTO_TRADER_COMMON_UTILS_H
