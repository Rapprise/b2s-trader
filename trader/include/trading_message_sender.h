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

#ifndef AUTO_TRADER_TRADING_MESSAGE_SENDER_H
#define AUTO_TRADER_TRADING_MESSAGE_SENDER_H

#include <QObject>

#include "common/listeners/app_listener.h"
#include "common/listeners/gui_listener.h"
#include "model/include/settings/app_settings.h"

namespace auto_trader {
namespace trader {

class TradingMessageSender : public QObject {
  Q_OBJECT
 public:
  TradingMessageSender(common::GuiListener& guiListener, model::AppSettings& appSettings);

  void setBuyingPrefix();
  void setSellingPrefix();
  void setDefaultPrefix();

  void sendMessage(const std::string& message);

 signals:
  void uiMessageSent(const QString& message);

 private:
  common::GuiListener& guiListener_;
  model::AppSettings& appSettings_;

  std::string message_;
  std::string prefix_;
};

}  // namespace trader
}  // namespace auto_trader

#endif  // AUTO_TRADER_TRADING_MESSAGE_SENDER_H
