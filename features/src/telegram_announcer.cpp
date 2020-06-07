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

#include "include/telegram_announcer.h"

#include <Poco/Net/Context.h>
#include <Poco/Net/HTTPRequest.h>
#include <Poco/Net/HTTPResponse.h>
#include <Poco/Path.h>
#include <Poco/StreamCopier.h>
#include <Poco/URI.h>

#include "common/exceptions/telegram_exception/telegram_rest_exception.h"
#include "common/exceptions/telegram_exception/unitialized_telegram_option_exception.h"
#include "common/loggers/file_logger.h"
#include "resources/feature_utils.h"

namespace auto_trader {
namespace features {
namespace telegram_announcer {

TelegramAnnouncer& TelegramAnnouncer::instance() {
  static TelegramAnnouncer announcer;
  return announcer;
}

void TelegramAnnouncer::init(const std::string& token) {
  using namespace Poco;

  if (token_ != token) {
    setToken(token);
    JSON::Object::Ptr object;

    try {
      object = getUpdates();
    } catch (const common::exceptions::TelegramRestException& ex) {
      common::loggers::FileLogger::getLogger() << ex.what();
      throw common::exceptions::UnitializedTelegramOptionException("Bad token. Can't get updates.");
    }

    auto resultJsonBlock = object->getArray(resources::TELEGRAM_JSON_BLOCK_RESULT);
    auto blockSize = resultJsonBlock->size();
    if (!blockSize) {
      throw common::exceptions::UnitializedTelegramOptionException(
          "Telegram chat need at least one message.");
    }

    auto messageInfo = resultJsonBlock->getObject(blockSize - 1);
    JSON::Object::Ptr messageJsonBlock =
        messageInfo->getObject(resources::TELEGRAM_JSON_BLOCK_MESSAGE);

    uint64_t message_id = messageJsonBlock->get(resources::TELEGRAM_JSON_FIELD_MESSAGE_ID);
    setLastMessageId(message_id);

    auto chatJsonBlock = messageJsonBlock->getObject(resources::TELEGRAM_JSON_BLOCK_CHAT);

    uint64_t chat_id = chatJsonBlock->get(resources::TELEGRAM_JSON_FIELD_ID);
    chatId_ = chat_id;
  }
}

void TelegramAnnouncer::createSession() {
  using namespace Poco;

  Net::Context::Ptr ctx =
      new Net::Context(Net::Context::CLIENT_USE, resources::EMPTY_STR, resources::EMPTY_STR,
                       resources::EMPTY_STR, Net::Context::VerificationMode::VERIFY_NONE);

  session_.reset(new Poco::Net::HTTPSClientSession(resources::TELEGRAM_HOST,
                                                   Poco::Net::HTTPSClientSession::HTTPS_PORT, ctx));
}

void TelegramAnnouncer::sendMessage(const std::string& message) {
  using namespace Poco;

  std::string request = resources::TELEGRAM_URL + token_ + resources::TELEGRAM_SEND_MESSAGE +
                        resources::TELEGRAM_CHAT_ID + resources::EQUAL + std::to_string(chatId_) +
                        resources::AND + resources::TELEGRAM_TEXT + resources::EQUAL + message;

  Poco::URI uri(request);

  std::istream& response = processHttpRequest(uri, Net::HTTPRequest::HTTP_POST);

  JSON::Parser parser;
  JSON::Object::Ptr object = parser.parse(response).extract<JSON::Object::Ptr>();

  bool result = object->get(resources::TELEGRAM_JSON_FIELD_IS_OK);
  if (!result) {
    std::string errorDescription = object->get(resources::TELEGRAM_ERROR_DESCRIPTION);
    throw common::exceptions::TelegramRestException(errorDescription);
  }
}

void TelegramAnnouncer::setToken(const std::string& token) { token_ = token; }

void TelegramAnnouncer::setChatId(const uint64_t chatId) { chatId_ = chatId; }

void TelegramAnnouncer::setLoggingEnabled(bool enabled) { loggingEnabled_ = enabled; }

const std::string& TelegramAnnouncer::getToken() const { return token_; }

const uint64_t TelegramAnnouncer::getChatId() const { return chatId_; }

const uint64_t TelegramAnnouncer::getLastMessageId() const { return lastMessageId_; }

bool TelegramAnnouncer::getLoggingEnabled() const { return loggingEnabled_; }

bool TelegramAnnouncer::isLoggingEnabled() const { return loggingEnabled_; }

void TelegramAnnouncer::setLastMessageId(const uint64_t& lastMessageId) {
  lastMessageId_ = lastMessageId;
}

std::vector<std::string> TelegramAnnouncer::getLastMessages() {
  using namespace Poco;
  std::vector<std::string> newMessages;

  if (token_ == resources::EMPTY_STR && chatId_ == 0) {
    return newMessages;
  }

  JSON::Object::Ptr object;
  try {
    object = getUpdates();
  } catch (const common::exceptions::TelegramRestException& ex) {
    common::loggers::FileLogger::getLogger() << ex.what();
    throw common::exceptions::UnitializedTelegramOptionException(
        "Can't get last messages. Get updates fail.");
  }

  auto resultJsonBlock = object->getArray(resources::TELEGRAM_JSON_BLOCK_RESULT);
  auto blockSize = resultJsonBlock->size();
  if (blockSize) {
    auto arraySize = resultJsonBlock->size();
    uint64_t last_message = 0;
    for (int arrayIndex = 0; arrayIndex < arraySize; ++arrayIndex) {
      auto messageInfo = resultJsonBlock->getObject(arrayIndex);
      JSON::Object::Ptr messageJsonBlock =
          messageInfo->getObject(resources::TELEGRAM_JSON_BLOCK_MESSAGE);

      unsigned int message_id = messageJsonBlock->get(resources::TELEGRAM_JSON_FIELD_MESSAGE_ID);

      if (lastMessageId_ < message_id) {
        std::string text = messageJsonBlock->get(resources::TELEGRAM_TEXT);

        auto textInfo = feature_utils::compareMessageWithTelegramCommands(text);
        if (textInfo.first) {
          newMessages.emplace_back(textInfo.second);
        } else {
          newMessages.emplace_back(text);
        }

        last_message = message_id;
      }
    }

    if (last_message > 0) {
      lastMessageId_ = last_message;
    }
  }

  return newMessages;
}

Poco::JSON::Object::Ptr TelegramAnnouncer::getUpdates() {
  using namespace Poco;

  std::string request = resources::TELEGRAM_URL + token_ + resources::TELEGRAM_GET_UPDATES;

  Poco::URI uri(request);

  std::istream& response = processHttpRequest(uri, Net::HTTPRequest::HTTP_GET);

  JSON::Parser parser;
  JSON::Object::Ptr object = parser.parse(response).extract<JSON::Object::Ptr>();

  bool result = object->get(resources::TELEGRAM_JSON_FIELD_IS_OK);
  if (!result) {
    std::string errorDescription = object->get(resources::TELEGRAM_ERROR_DESCRIPTION);
    throw common::exceptions::TelegramRestException(errorDescription);
  }

  return object;
}

std::istream& TelegramAnnouncer::processHttpRequest(Poco::URI const& uri,
                                                    const std::string& http_kind) const {
  using namespace Poco;

  std::string path(uri.getPathAndQuery());

  if (path.empty()) {
    path = resources::SLASH;
  }

  Net::HTTPRequest request(http_kind, path, Net::HTTPMessage::HTTP_1_1);
  session_->sendRequest(request);

  Net::HTTPResponse response;
  auto& stream = session_->receiveResponse(response);

  return stream;
}

bool TelegramAnnouncer::isInitialized() const {
  return (token_ != resources::EMPTY_STR) && (chatId_ != 0);
}

}  // namespace telegram_announcer
}  // namespace features
}  // namespace auto_trader