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

#include "include/dialogs/intervals_dialog.h"

#include <QValidator>
#include <QtWidgets/QPushButton>
#include <sstream>

#include "include/validators/integer_validator.h"

namespace auto_trader {
namespace view {
namespace dialogs {

IntervalsDialog::IntervalsDialog(common::AppListener &appListener, QWidget *parent)
    : QDialog(parent), appListener_(appListener) {
  uiDialog_.setupUi(this);

  QValidator *validator = new validators::IntegerValidator(1, 10000, this);
  QLocale locale(QLocale::English);
  validator->setLocale(locale);

  uiDialog_.lineEdit->setValidator(validator);
  uiDialog_.lineEdit_2->setValidator(validator);

  auto &appSettings = appListener_.getAppSettings();
  std::string appStatsTimeout = std::to_string(appSettings.appStatsTimeout_);
  std::string tradingTimeout = std::to_string(appSettings.tradingTimeout_);

  uiDialog_.lineEdit->setText(QString::fromStdString(appStatsTimeout));
  uiDialog_.lineEdit_2->setText(QString::fromStdString(tradingTimeout));

  intervalChanged(QString());

  connect(uiDialog_.buttonBox, SIGNAL(accepted()), this, SLOT(closeDialog()));
  connect(uiDialog_.buttonBox, SIGNAL(rejected()), this, SLOT(reject()));

  connect(uiDialog_.lineEdit, SIGNAL(textChanged(const QString &)), this,
          SLOT(intervalChanged(const QString &)));
  connect(uiDialog_.lineEdit_2, SIGNAL(textChanged(const QString &)), this,
          SLOT(intervalChanged(const QString &)));
}

void IntervalsDialog::closeDialog() {
  auto statsUpdateInterval = uiDialog_.lineEdit->text().toStdString();
  auto tradingUpdateInterval = uiDialog_.lineEdit_2->text().toStdString();

  int statsUpdateIntervalInt = std::stoi(statsUpdateInterval);
  int tradingUpdateIntervalInt = std::stoi(tradingUpdateInterval);

  auto &appSettings = appListener_.getAppSettings();
  appSettings.appStatsTimeout_ = statsUpdateIntervalInt;
  appSettings.tradingTimeout_ = tradingUpdateIntervalInt;

  appListener_.saveAppSettings();

  accept();
}

void IntervalsDialog::intervalChanged(const QString &value) {
  auto statsUpdateInterval = uiDialog_.lineEdit->text().toStdString();
  auto tradingUpdateInterval = uiDialog_.lineEdit_2->text().toStdString();
  if (statsUpdateInterval.empty() || tradingUpdateInterval.empty()) {
    uiDialog_.buttonBox->button(QDialogButtonBox::StandardButton::Ok)->setDisabled(true);
    return;
  }

  uiDialog_.buttonBox->button(QDialogButtonBox::StandardButton::Ok)->setEnabled(true);
}

}  // namespace dialogs
}  // namespace view
}  // namespace auto_trader