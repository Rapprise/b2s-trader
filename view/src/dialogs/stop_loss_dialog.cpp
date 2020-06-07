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

#include "include/dialogs/stop_loss_dialog.h"

#include <QValidator>
#include <QtWidgets/QPushButton>
#include <sstream>

#include "features/include/stop_loss_announcer.h"
#include "include/validators/float_validator.h"

namespace auto_trader {
namespace view {
namespace dialogs {

StopLossDialog::StopLossDialog(common::AppListener &appListener, QWidget *parent)
    : QDialog(parent), appListener_(appListener) {
  uiDialog_.setupUi(this);

  QValidator *validator = new validators::FloatValidator(1, 100, 4, this);
  QLocale locale(QLocale::English);
  validator->setLocale(locale);

  uiDialog_.lineEdit->setValidator(validator);

  auto &stopLossAnnouncer = features::stop_loss_announcer::StopLossAnnouncer::instance();
  double currentValue = stopLossAnnouncer.getValue();
  std::ostringstream stream;
  stream << currentValue;

  const std::string &currentValueStr = stream.str();
  stopLossChanged(QString::fromStdString(currentValueStr));
  uiDialog_.lineEdit->setText(QString::fromStdString(currentValueStr));

  connect(uiDialog_.buttonBox, SIGNAL(accepted()), this, SLOT(closeDialog()));
  connect(uiDialog_.buttonBox, SIGNAL(rejected()), this, SLOT(reject()));

  connect(uiDialog_.lineEdit, SIGNAL(textChanged(const QString &)), this,
          SLOT(stopLossChanged(const QString &)));
}

void StopLossDialog::closeDialog() {
  auto stopLossValueStr = uiDialog_.lineEdit->text().toStdString();
  if (stopLossValueStr.empty()) {
    return;
  }
  double stopLossValue = std::stod(stopLossValueStr);

  auto &stopLossAnnouncer = features::stop_loss_announcer::StopLossAnnouncer::instance();
  stopLossAnnouncer.setValue(stopLossValue);

  appListener_.saveFeaturesSettings();

  accept();
}

void StopLossDialog::stopLossChanged(const QString &value) {
  std::string text = value.toStdString();
  if (text.empty()) {
    uiDialog_.buttonBox->button(QDialogButtonBox::StandardButton::Ok)->setDisabled(true);
    return;
  }

  double stopLossValue = std::stod(text);
  if (stopLossValue > 100) {
    uiDialog_.buttonBox->button(QDialogButtonBox::StandardButton::Ok)->setDisabled(true);
    return;
  } else if (stopLossValue <= 0) {
    uiDialog_.buttonBox->button(QDialogButtonBox::StandardButton::Ok)->setDisabled(true);
    return;
  }

  uiDialog_.buttonBox->button(QDialogButtonBox::StandardButton::Ok)->setEnabled(true);
}

}  // namespace dialogs
}  // namespace view
}  // namespace auto_trader