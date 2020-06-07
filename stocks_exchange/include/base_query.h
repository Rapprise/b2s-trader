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


#ifndef AUTO_TRADER_STOCK_EXCHANGE_BASE_QUERY_H
#define AUTO_TRADER_STOCK_EXCHANGE_BASE_QUERY_H

#include <Poco/JSON/Object.h>
#include <Poco/JSON/Parser.h>
#include <Poco/Net/Context.h>
#include <Poco/Net/HTTPRequest.h>
#include <Poco/Net/HTTPResponse.h>
#include <Poco/Net/HTTPSClientSession.h>
#include <Poco/URI.h>

#include <string>

#include "common/exceptions/stock_exchange_exception/redirect_http_exception.h"
#include "resources/resources.h"

namespace auto_trader {
namespace stock_exchange {

template <typename BaseClass>
class BaseQuery : public BaseClass {
 public:
  struct ConnectionAttributes {
    std::string host_;
    unsigned short port_;
  };

 public:
  inline virtual void updateApiKey(const std::string& api_key) { api_key_ = api_key; }
  inline virtual void updateSecretKey(const std::string& secret_key) { secret_key_ = secret_key; }

 public:
  typedef std::pair<std::string, std::string> HTTP_HEADERS;
  virtual const std::string processHttpRequest(const ConnectionAttributes& host_and_port,
                                               Poco::Net::HTTPRequest& request,
                                               const std::vector<HTTP_HEADERS>& headers) const;

 protected:
  std::string api_key_;
  std::string secret_key_;
};

template <typename BaseClass>
const std::string BaseQuery<BaseClass>::processHttpRequest(
    const ConnectionAttributes& host_and_port, Poco::Net::HTTPRequest& request,
    const std::vector<HTTP_HEADERS>& headers) const {
  using namespace Poco;

  Net::Context::Ptr ctx = new Net::Context(
      Net::Context::CLIENT_USE, resources::symbols::EMPTY_STR, resources::symbols::EMPTY_STR,
      resources::symbols::EMPTY_STR, Net::Context::VerificationMode::VERIFY_NONE);
  Poco::Net::HTTPSClientSession session(host_and_port.host_, host_and_port.port_, ctx);

  for (auto header : headers) {
    request.set(header.first, header.second);
  }

  session.sendRequest(request);

  Net::HTTPResponse response;
  auto& stream = session.receiveResponse(response);

  bool moved = (response.getStatus() == Net::HTTPResponse::HTTP_MOVED_PERMANENTLY ||
                response.getStatus() == Net::HTTPResponse::HTTP_FOUND ||
                response.getStatus() == Net::HTTPResponse::HTTP_SEE_OTHER ||
                response.getStatus() == Net::HTTPResponse::HTTP_TEMPORARY_REDIRECT);

  if (moved) {
    const std::string& location = response.get("Location");
    throw common::exceptions::RedirectHttpsException(location);
  }

  std::istreambuf_iterator<char> iterator;
  std::string output(std::istreambuf_iterator<char>(stream), iterator);

  return output;
}

}  // namespace stock_exchange
}  // namespace auto_trader

#endif  // AUTO_TRADER_STOCK_EXCHANGE_BASE_QUERY_H
