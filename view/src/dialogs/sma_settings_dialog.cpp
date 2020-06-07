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

#include "include/dialogs/sma_settings_dialog.h"

#include <QtWidgets/QPushButton>

#include "include/validators/integer_validator.h"

namespace auto_trader {
namespace view {
namespace dialogs {

SmaSettingsDialog::SmaSettingsDialog(model::SmaSettings &settings, common::AppListener &appListener,
                                     common::GuiListener &guiListener, DialogType dialogType,
                                     QWidget *parent)
    : QDialog(parent),
      smaSettings_(settings),
      appListener_(appListener),
      guiListener_(guiListener),
      dialogType_(dialogType) {
  uiDialog_.setupUi(this);

  initTickIntervals();

  QValidator *smaPeriodValidator = new validators::IntegerValidator(1, 100, this);

  const std::string &periodStr = std::to_string(smaSettings_.period_);
  uiDialog_.lineEdit->setText(QString::fromStdString(periodStr));
  uiDialog_.lineEdit->setValidator(smaPeriodValidator);

  QValidator *crossingIntervalValidator = new validators::IntegerValidator(0, 10, this);
  const std::string &candlesAfterCrossing = std::to_string(smaSettings_.crossingInterval_);
  uiDialog_.lineEdit_2->setText(QString::fromStdString(candlesAfterCrossing));
  uiDialog_.lineEdit_2->setValidator(crossingIntervalValidator);

  checkOkButtonStatus(QString());

  connect(uiDialog_.buttonBox, SIGNAL(accepted()), this, SLOT(closeDialog()));
  connect(uiDialog_.buttonBox, SIGNAL(rejected()), this, SLOT(reject()));

  connect(uiDialog_.lineEdit, SIGNAL(textChanged(const QString &)), this,
          SLOT(checkOkButtonStatus(const QString &)));
  connect(uiDialog_.lineEdit_2, SIGNAL(textChanged(const QString &)), this,
          SLOT(checkOkButtonStatus(const QString &)));
}

void SmaSettingsDialog::initTickIntervals() {
  QStringList items;
  auto lastElement = (unsigned short)common::TickInterval::UNKNOWN;
  for (unsigned short index = 0; index < lastElement; ++index) {
    const std::string interval = common::TickInterval::toString((common::TickInterval::Enum)index);
    items.append(QString::fromStdString(interval));
  }

  uiDialog_.comboBox->addItems(items);

  const std::string &currentTickInterval =
      common::TickInterval::toString(smaSettings_.tickInterval_);
  uiDialog_.comboBox->setCurrentText(QString::fromStdString(currentTickInterval));
}

void SmaSettingsDialog::closeDialog() {
  const std::string period = uiDialog_.lineEdit->text().toStdString();
  const std::string candlesAfterCrossing = uiDialog_.lineEdit_2->text().toStdString();

  smaSettings_.name_ = convertStrategyTypeToString(common::StrategiesType::SMA);
  smaSettings_.period_ = stoi(period);
  smaSettings_.crossingInterval_ = stoi(candlesAfterCrossing);
  smaSettings_.strategiesType_ = common::StrategiesType::SMA;

  auto currentBoxIndex = uiDialog_.comboBox->currentText();
  std::string tickInterval = currentBoxIndex.toStdString();
  smaSettings_.tickInterval_ = common::TickInterval::fromString(tickInterval);

  accept();
}

void SmaSettingsDialog::checkOkButtonStatus(const QString &textEdit) const {
  bool isPeriodEmpty = uiDialog_.lineEdit->text().isEmpty();
  bool isCandlesAfterCrossingEmpty = uiDialog_.lineEdit_2->text().isEmpty();
  bool isOkButtonDisabled = isPeriodEmpty || isCandlesAfterCrossingEmpty;

  uiDialog_.buttonBox->button(QDialogButtonBox::StandardButton::Ok)
      ->setDisabled(isOkButtonDisabled);
}

}  // namespace dialogs
}  // namespace view
}  // namespace auto_trader