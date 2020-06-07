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

#include "include/dialogs/moving_average_crossing_settings_dialog.h"

#include <QtWidgets/QPushButton>

#include "common/enumerations/moving_average_type.h"
#include "include/validators/integer_validator.h"

namespace auto_trader {
namespace view {
namespace dialogs {

MovingAverageCrossingSettingsDialog::MovingAverageCrossingSettingsDialog(
    model::MovingAveragesCrossingSettings &settings, common::AppListener &appListener,
    common::GuiListener &guiListener, DialogType dialogType, QWidget *parent)
    : QDialog(parent),
      movingAveragesCrossingSettings_(settings),
      appListener_(appListener),
      guiListener_(guiListener),
      dialogType_(dialogType) {
  uiDialog_.setupUi(this);

  initTickIntervals();
  initMovingAverages();

  QValidator *periodValidator = new validators::IntegerValidator(1, 100, this);

  auto smallerPeriodStr = std::to_string(movingAveragesCrossingSettings_.smallerPeriod_);
  uiDialog_.lineEdit->setText(QString::fromStdString(smallerPeriodStr));
  uiDialog_.lineEdit->setValidator(periodValidator);

  auto biggerPeriodStr = std::to_string(movingAveragesCrossingSettings_.biggerPeriod_);
  uiDialog_.lineEdit_2->setText(QString::fromStdString(biggerPeriodStr));
  uiDialog_.lineEdit_2->setValidator(periodValidator);

  QValidator *crossingIntervalValidator = new validators::IntegerValidator(0, 10, this);
  auto crossingInterval = std::to_string(movingAveragesCrossingSettings_.crossingInterval_);
  uiDialog_.lineEdit_3->setText(QString::fromStdString(crossingInterval));
  uiDialog_.lineEdit_3->setValidator(crossingIntervalValidator);

  connect(uiDialog_.buttonBox, SIGNAL(accepted()), this, SLOT(closeDialog()));
  connect(uiDialog_.buttonBox, SIGNAL(rejected()), this, SLOT(reject()));

  connect(uiDialog_.lineEdit, SIGNAL(textChanged(const QString &)), this,
          SLOT(checkOkButtonStatus(const QString &)));
  connect(uiDialog_.lineEdit_2, SIGNAL(textChanged(const QString &)), this,
          SLOT(checkOkButtonStatus(const QString &)));
  connect(uiDialog_.lineEdit_3, SIGNAL(textChanged(const QString &)), this,
          SLOT(checkOkButtonStatus(const QString &)));
}

void MovingAverageCrossingSettingsDialog::initTickIntervals() {
  QStringList items;
  auto lastElement = (unsigned short)common::TickInterval::UNKNOWN;
  for (unsigned short index = 0; index < lastElement; ++index) {
    const std::string interval = common::TickInterval::toString((common::TickInterval::Enum)index);
    items.append(QString::fromStdString(interval));
  }

  uiDialog_.comboBox->addItems(items);

  const std::string &currentTickInterval =
      common::TickInterval::toString(movingAveragesCrossingSettings_.tickInterval_);
  uiDialog_.comboBox->setCurrentText(QString::fromStdString(currentTickInterval));
}

void MovingAverageCrossingSettingsDialog::initMovingAverages() {
  QStringList items;
  auto lastElement = (unsigned short)common::MovingAverageType::UNKNOWN;
  for (unsigned short index = 0; index < lastElement; ++index) {
    const std::string typeStr =
        common::convertMovingAverageTypeToString((common::MovingAverageType)index);
    items.append(QString::fromStdString(typeStr));
  }
  uiDialog_.comboBox_2->addItems(items);

  const std::string &currentMovingAverageType =
      common::convertMovingAverageTypeToString(movingAveragesCrossingSettings_.movingAverageType_);
  uiDialog_.comboBox_2->setCurrentText(QString::fromStdString(currentMovingAverageType));
}

void MovingAverageCrossingSettingsDialog::closeDialog() {
  movingAveragesCrossingSettings_.name_ =
      common::convertStrategyTypeToString(common::StrategiesType::MA_CROSSING);
  auto currentBoxIndex = uiDialog_.comboBox->currentText();
  std::string tickInterval = currentBoxIndex.toStdString();
  movingAveragesCrossingSettings_.tickInterval_ = common::TickInterval::fromString(tickInterval);

  auto movingAverageType = uiDialog_.comboBox_2->currentText();
  std::string movingAverageTypeStr = movingAverageType.toStdString();
  movingAveragesCrossingSettings_.movingAverageType_ =
      common::convertMovingAverageTypeFromString(movingAverageTypeStr);

  auto smallerPeriodStr = uiDialog_.lineEdit->text().toStdString();
  movingAveragesCrossingSettings_.smallerPeriod_ = std::stoi(smallerPeriodStr);

  auto biggerPeriodStr = uiDialog_.lineEdit_2->text().toStdString();
  movingAveragesCrossingSettings_.biggerPeriod_ = std::stoi(biggerPeriodStr);

  auto crossingInterval = uiDialog_.lineEdit_3->text().toStdString();
  movingAveragesCrossingSettings_.crossingInterval_ = std::stoi(crossingInterval);

  movingAveragesCrossingSettings_.strategiesType_ = common::StrategiesType::MA_CROSSING;

  accept();
}

void MovingAverageCrossingSettingsDialog::checkOkButtonStatus(const QString &text) {
  bool isSmallerPeriodEmpty = uiDialog_.lineEdit->text().isEmpty();
  bool isBiggerPeriodEmpty = uiDialog_.lineEdit_2->text().isEmpty();
  bool isCrossingIntervalEmpty = uiDialog_.lineEdit_3->text().isEmpty();

  bool isButtonOkDisabled = isSmallerPeriodEmpty || isBiggerPeriodEmpty || isCrossingIntervalEmpty;

  uiDialog_.buttonBox->button(QDialogButtonBox::StandardButton::Ok)
      ->setDisabled(isButtonOkDisabled);
}

}  // namespace dialogs
}  // namespace view
}  // namespace auto_trader