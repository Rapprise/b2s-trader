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

#include "include/dialogs/rsi_settings_dialog.h"

#include <QtWidgets/QPushButton>

#include "include/validators/integer_validator.h"

namespace auto_trader {
namespace view {
namespace dialogs {

RsiSettingsDialog::RsiSettingsDialog(model::RsiSettings &settings, common::AppListener &appListener,
                                     common::GuiListener &guiListener, DialogType dialogType,
                                     QWidget *parent)
    : QDialog(parent),
      rsiSettings_(settings),
      appListener_(appListener),
      guiListener_(guiListener),
      dialogType_(dialogType) {
  uiDialog_.setupUi(this);

  initTickIntervals();

  QValidator *intValidator = new validators::IntegerValidator(1, 100, this);

  const std::string &periodStr = std::to_string(rsiSettings_.period_);
  uiDialog_.lineEdit->setText(QString::fromStdString(periodStr));

  const std::string &topLevelLine = std::to_string(rsiSettings_.topLevel_);
  uiDialog_.lineEdit_2->setText(QString::fromStdString(topLevelLine));

  const std::string &bottomLevelLine = std::to_string(rsiSettings_.bottomLevel_);
  uiDialog_.lineEdit_3->setText(QString::fromStdString(bottomLevelLine));

  QIntValidator *crossingIntervalValidator = new validators::IntegerValidator(0, 10, this);
  const std::string &candlesAfterCrossing = std::to_string(rsiSettings_.crossingInterval_);
  uiDialog_.lineEdit_4->setText(QString::fromStdString(candlesAfterCrossing));
  uiDialog_.lineEdit_4->setValidator(crossingIntervalValidator);

  checkOkButtonStatus(QString());

  uiDialog_.lineEdit->setValidator(intValidator);
  uiDialog_.lineEdit_2->setValidator(intValidator);
  uiDialog_.lineEdit_3->setValidator(intValidator);
  uiDialog_.lineEdit_3->setValidator(intValidator);

  connect(uiDialog_.buttonBox, SIGNAL(accepted()), this, SLOT(closeDialog()));
  connect(uiDialog_.buttonBox, SIGNAL(rejected()), this, SLOT(reject()));

  connect(uiDialog_.lineEdit, SIGNAL(textChanged(const QString &)), this,
          SLOT(checkOkButtonStatus(const QString &)));
  connect(uiDialog_.lineEdit_2, SIGNAL(textChanged(const QString &)), this,
          SLOT(checkOkButtonStatus(const QString &)));
  connect(uiDialog_.lineEdit_3, SIGNAL(textChanged(const QString &)), this,
          SLOT(checkOkButtonStatus(const QString &)));
  connect(uiDialog_.lineEdit_4, SIGNAL(textChanged(const QString &)), this,
          SLOT(checkOkButtonStatus(const QString &)));
}

void RsiSettingsDialog::initTickIntervals() {
  QStringList items;
  unsigned short lastElement = (unsigned short)common::TickInterval::UNKNOWN;
  for (unsigned short index = 0; index < lastElement; ++index) {
    const std::string interval = common::TickInterval::toString((common::TickInterval::Enum)index);
    items.append(QString::fromStdString(interval));
  }

  uiDialog_.comboBox->addItems(items);

  const std::string &currentTickInterval =
      common::TickInterval::toString(rsiSettings_.tickInterval_);
  uiDialog_.comboBox->setCurrentText(QString::fromStdString(currentTickInterval));
}

void RsiSettingsDialog::closeDialog() {
  const std::string period = uiDialog_.lineEdit->text().toStdString();

  rsiSettings_.name_ = convertStrategyTypeToString(common::StrategiesType::RSI);
  rsiSettings_.period_ = stoi(period);

  const std::string topLevelLineStr = uiDialog_.lineEdit_2->text().toStdString();
  rsiSettings_.topLevel_ = stoi(topLevelLineStr);
  const std::string bottomLevelStr = uiDialog_.lineEdit_3->text().toStdString();
  rsiSettings_.bottomLevel_ = stoi(bottomLevelStr);

  const std::string candlesAfterCrossing = uiDialog_.lineEdit_4->text().toStdString();
  rsiSettings_.crossingInterval_ = stoi(candlesAfterCrossing);

  rsiSettings_.strategiesType_ = common::StrategiesType::RSI;

  auto currentTickIntervalStr = uiDialog_.comboBox->currentText();
  std::string tickInterval = currentTickIntervalStr.toStdString();
  rsiSettings_.tickInterval_ = common::TickInterval::fromString(tickInterval);

  accept();
}

void RsiSettingsDialog::checkOkButtonStatus(const QString &text) {
  bool isPeriodEmpty = uiDialog_.lineEdit->text().isEmpty();
  bool isTopLevelEmpty = uiDialog_.lineEdit_2->text().isEmpty();
  bool isBottomEmpty = uiDialog_.lineEdit_3->text().isEmpty();
  bool isCandlesAfterCrossingsEmpty = uiDialog_.lineEdit_4->text().isEmpty();

  bool isButtonOkDisabled =
      isPeriodEmpty || isTopLevelEmpty || isBottomEmpty || isCandlesAfterCrossingsEmpty;
  if (!isButtonOkDisabled) {
    auto topLevel = std::stoi(uiDialog_.lineEdit_2->text().toStdString());
    auto bottomLevel = std::stoi(uiDialog_.lineEdit_3->text().toStdString());
    bool buttonEnabled = (topLevel > bottomLevel) && (topLevel < 100) && (bottomLevel < 100) &&
                         (topLevel > 0) && (bottomLevel > 0);

    uiDialog_.buttonBox->button(QDialogButtonBox::StandardButton::Ok)->setEnabled(buttonEnabled);

  } else {
    uiDialog_.buttonBox->button(QDialogButtonBox::StandardButton::Ok)
        ->setDisabled(isButtonOkDisabled);
  }
}

}  // namespace dialogs
}  // namespace view
}  // namespace auto_trader