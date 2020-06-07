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

#include "include/dialogs/telegram_dialog.h"

#include <common/exceptions/telegram_exception/telegram_rest_exception.h>

#include <QValidator>
#include <QtWidgets/QPushButton>

#include "common/exceptions/telegram_exception/unitialized_telegram_option_exception.h"
#include "common/loggers/file_logger.h"
#include "features/include/telegram_announcer.h"
#include "include/validators/integer_validator.h"

namespace auto_trader {
namespace view {
namespace dialogs {

TelegramDialog::TelegramDialog(common::AppListener &appListener, QWidget *parent)
    : QDialog(parent), appListener_(appListener) {
  uiDialog_.setupUi(this);

  auto &telegram = features::telegram_announcer::TelegramAnnouncer::instance();
  uiDialog_.lineEdit_2->setText(QString::fromStdString(telegram.getToken()));

  checkOkButtonStatus("");

  connect(uiDialog_.buttonBox, SIGNAL(accepted()), this, SLOT(closeDialog()));
  connect(uiDialog_.buttonBox, SIGNAL(rejected()), this, SLOT(reject()));

  connect(uiDialog_.lineEdit_2, SIGNAL(textChanged(const QString &)), this,
          SLOT(checkOkButtonStatus(const QString &)));
}

void TelegramDialog::closeDialog() {
  const std::string &tokenId = uiDialog_.lineEdit_2->text().toStdString();

  if (tokenId.empty()) {
    return;
  }

  auto &telegramAnnouncer = features::telegram_announcer::TelegramAnnouncer::instance();
  try {
    telegramAnnouncer.createSession();
    telegramAnnouncer.init(tokenId);
    telegramAnnouncer.setToken(tokenId);
  } catch (const common::exceptions::TelegramException &ex) {
    common::loggers::FileLogger::getLogger() << ex.what();
  }

  appListener_.saveFeaturesSettings();

  accept();
}

bool TelegramDialog::tokenIdChanged() {
  std::string text = uiDialog_.lineEdit_2->text().toStdString();
  return !text.empty();
}

void TelegramDialog::checkOkButtonStatus(const QString &value) {
  bool result = tokenIdChanged();
  if (result) {
    uiDialog_.buttonBox->button(QDialogButtonBox::StandardButton::Ok)->setEnabled(true);
  } else {
    uiDialog_.buttonBox->button(QDialogButtonBox::StandardButton::Ok)->setDisabled(true);
  }
}

}  // namespace dialogs
}  // namespace view
}  // namespace auto_trader