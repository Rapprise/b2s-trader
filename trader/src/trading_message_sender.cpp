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

#include "include/trading_message_sender.h"

#include "common/loggers/file_logger.h"
#include "features/include/telegram_announcer.h"

namespace auto_trader {
namespace trader {

constexpr char TRADING_DEFAULT_PREFIX[] = "[TRADING]: ";
constexpr char TRADING_BUYING_PREFIX[] = "[TRADING BUYING]: ";
constexpr char TRADING_SELLING_PREFIX[] = "[TRADING SELLING]: ";

TradingMessageSender::TradingMessageSender(common::GuiListener& guiListener,
                                           model::AppSettings& appSettings)
    : guiListener_(guiListener), appSettings_(appSettings) {
  prefix_ = TRADING_DEFAULT_PREFIX;
}

void TradingMessageSender::sendMessage(const std::string& message) {
  message_.clear();
  message_ = prefix_ + message;
  if (appSettings_.uiLoggingEnabled_) {
    emit uiMessageSent(QString::fromStdString(message_));
  }

  auto& telegramAnnouncer = features::telegram_announcer::TelegramAnnouncer::instance();
  if (telegramAnnouncer.isLoggingEnabled()) {
    telegramAnnouncer.sendMessage(message_);
  }

  common::loggers::TradingFileLogger::getLogger() << message_;
}

void TradingMessageSender::setBuyingPrefix() { prefix_ = TRADING_BUYING_PREFIX; }

void TradingMessageSender::setSellingPrefix() { prefix_ = TRADING_SELLING_PREFIX; }

void TradingMessageSender::setDefaultPrefix() { prefix_ = TRADING_DEFAULT_PREFIX; }

}  // namespace trader
}  // namespace auto_trader