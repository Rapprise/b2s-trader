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

#include "include/feature_json_serializer.h"

#include "features/include/stop_loss_announcer.h"
#include "features/include/telegram_announcer.h"

namespace auto_trader {
namespace serializer {

void FeatureJsonSerializer::serializeTelegramSettings(std::ostream& stream) {
  Poco::JSON::PrintHandler printHandler(stream, 1);
  printHandler.startObject();

  auto& telegramAnnouncer = features::telegram_announcer::TelegramAnnouncer::instance();

  printHandler.key("token");
  printHandler.value(telegramAnnouncer.getToken());

  printHandler.key("chatId");
  printHandler.value((int)telegramAnnouncer.getChatId());

  printHandler.key("lastMessageId");
  printHandler.value((int)telegramAnnouncer.getLastMessageId());

  printHandler.key("loggingEnabled");
  printHandler.value(telegramAnnouncer.getLoggingEnabled());
  printHandler.endObject();
}

void FeatureJsonSerializer::serializeStopLossSettings(std::ostream& stream) {
  Poco::JSON::PrintHandler printHandler(stream, 1);
  printHandler.startObject();

  auto& stopLossAnnouncer = features::stop_loss_announcer::StopLossAnnouncer::instance();
  printHandler.key("stop_loss");
  printHandler.value(stopLossAnnouncer.getValue());

  printHandler.endObject();
}

void FeatureJsonSerializer::deserializeTelegramSettings(std::istream& stream) {
  Poco::JSON::Parser parser;
  auto jsonObject = parser.parse(stream);
  auto object = jsonObject.extract<Poco::JSON::Object::Ptr>();
  const std::string& token = object->getValue<std::string>("token");
  auto chatId = object->getValue<uint64_t>("chatId");
  auto lastMessageId = object->getValue<uint64_t>("lastMessageId");
  auto loggingEnabled = object->getValue<bool>("loggingEnabled");

  auto& telegramAnnouncer = features::telegram_announcer::TelegramAnnouncer::instance();
  telegramAnnouncer.setToken(token);
  telegramAnnouncer.setChatId(chatId);
  telegramAnnouncer.setLastMessageId(lastMessageId);
  telegramAnnouncer.setLoggingEnabled(loggingEnabled);

  telegramAnnouncer.createSession();
}

void FeatureJsonSerializer::deserializeStopLossSettings(std::istream& stream) {
  Poco::JSON::Parser parser;
  auto jsonObject = parser.parse(stream);
  auto object = jsonObject.extract<Poco::JSON::Object::Ptr>();
  double stop_loss = object->getValue<double>("stop_loss");

  auto& stopLossAnnouncer = features::stop_loss_announcer::StopLossAnnouncer::instance();
  stopLossAnnouncer.setValue(stop_loss);
}

}  // namespace serializer
}  // namespace auto_trader