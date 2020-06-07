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

#include "include/app_settings_json_serializer.h"

namespace auto_trader {
namespace serializer {

void AppSettingsJsonSerializer::serialize(const model::AppSettings &appSettings,
                                          std::ostream &stream) {
  Poco::JSON::PrintHandler printHandler(stream, 1);
  printHandler.startObject();

  printHandler.key("ui_logging_enabled");
  printHandler.value(appSettings.uiLoggingEnabled_);

  printHandler.key("trading_timeout");
  printHandler.value(appSettings.tradingTimeout_);

  printHandler.key("app_stats_timeout");
  printHandler.value(appSettings.appStatsTimeout_);

  printHandler.key("ui_theme");
  printHandler.value(static_cast<unsigned int>(appSettings.theme_));

  printHandler.endObject();
}

void AppSettingsJsonSerializer::deserialize(model::AppSettings &appSettings, std::istream &stream) {
  Poco::JSON::Parser parser;
  auto jsonObject = parser.parse(stream);
  auto object = jsonObject.extract<Poco::JSON::Object::Ptr>();
  appSettings.uiLoggingEnabled_ = object->getValue<bool>("ui_logging_enabled");
  auto tradingTimeout = object->getValue<unsigned int>("trading_timeout");
  auto appStatsTimeout = object->getValue<unsigned int>("app_stats_timeout");

  if (tradingTimeout <= 0) {
    tradingTimeout = 1;
  }

  if (appStatsTimeout <= 0) {
    appStatsTimeout = 1;
  }

  appSettings.tradingTimeout_ = tradingTimeout;
  appSettings.appStatsTimeout_ = appStatsTimeout;

  auto theme = object->getValue<unsigned int>("ui_theme");
  appSettings.theme_ = static_cast<common::ApplicationThemeType>(theme);
}

}  // namespace serializer
}  // namespace auto_trader