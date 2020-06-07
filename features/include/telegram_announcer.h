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

#ifndef AUTO_TRADER_FEATURES_TELEGRAM_ANNOUNCER_H
#define AUTO_TRADER_FEATURES_TELEGRAM_ANNOUNCER_H

#include <Poco/JSON/Parser.h>
#include <Poco/Net/HTTPSClientSession.h>
#include <Poco/URI.h>

#include <memory>
#include <string>
#include <vector>

#include "features/resources/resources.h"

namespace auto_trader {
namespace features {
namespace telegram_announcer {

class TelegramAnnouncer {
 public:
  TelegramAnnouncer(const TelegramAnnouncer& announcer) = delete;
  TelegramAnnouncer(TelegramAnnouncer&&) = delete;
  TelegramAnnouncer& operator=(const TelegramAnnouncer& announcer) = delete;
  TelegramAnnouncer& operator=(TelegramAnnouncer&&) = delete;

  static TelegramAnnouncer& instance();

  void init(const std::string& token);
  void createSession();
  void sendMessage(const std::string& message);

  void setToken(const std::string& token);
  void setChatId(const uint64_t chatId);
  void setLastMessageId(const uint64_t& lastMessageId);

  const std::string& getToken() const;
  const uint64_t getChatId() const;
  const uint64_t getLastMessageId() const;
  bool getLoggingEnabled() const;

  void setLoggingEnabled(bool enabled);
  bool isLoggingEnabled() const;

  bool isInitialized() const;

  std::vector<std::string> getLastMessages();

 protected:
  TelegramAnnouncer() = default;

  Poco::JSON::Object::Ptr getUpdates();
  virtual std::istream& processHttpRequest(Poco::URI const&, const std::string& http_kind) const;

 private:
  std::string token_ {resources::EMPTY_STR};
  uint64_t chatId_ {0};
  uint64_t lastMessageId_ {0};
  bool loggingEnabled_ {false};
  std::unique_ptr<Poco::Net::HTTPSClientSession> session_;
};

}  // namespace telegram_announcer
}  // namespace features
}  // namespace auto_trader

#endif  // AUTO_TRADER_FEATURES_TELEGRAM_ANNOUNCER_H
