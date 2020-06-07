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

#include <QIntValidator>
#include <QtWidgets/QPushButton>

#include "common/enumerations/stochastic_oscillator_type.h"
#include "include/dialogs/stochastic_oscillator_dialog.h"
#include "include/validators/integer_validator.h"

namespace auto_trader {
namespace view {
namespace dialogs {

StochasticOscillatorSettingsDialog::StochasticOscillatorSettingsDialog(
    model::StochasticOscillatorSettings &settings, common::AppListener &appListener,
    common::GuiListener &guiListener, DialogType dialogType, QWidget *parent)
    : QDialog(parent),
      stochasticOscillatorSettings_(settings),
      appListener_(appListener),
      guiListener_(guiListener),
      dialogType_(dialogType) {
  uiDialog_.setupUi(this);

  initTickIntervals();
  initStochasticType();

  auto classicLineValidator = new validators::IntegerValidator(1, 30, this);
  const std::string &classicLinePeriodStr =
      std::to_string(stochasticOscillatorSettings_.periodsForClassicLine_);
  uiDialog_.lineEdit->setText(QString::fromStdString(classicLinePeriodStr));
  uiDialog_.lineEdit->setValidator(classicLineValidator);

  auto crossingIntervalValidator = new validators::IntegerValidator(0, 10, this);
  const std::string &candlesAfterCrossingStr =
      std::to_string(stochasticOscillatorSettings_.crossingInterval_);
  uiDialog_.lineEdit_2->setText(QString::fromStdString(candlesAfterCrossingStr));
  uiDialog_.lineEdit_2->setValidator(crossingIntervalValidator);

  auto smoothPeriodValidator = new validators::IntegerValidator(1, 7, this);
  const std::string &smoothFastPeriod =
      std::to_string(stochasticOscillatorSettings_.smoothFastPeriod_);
  uiDialog_.lineEdit_3->setText(QString::fromStdString(smoothFastPeriod));
  uiDialog_.lineEdit_3->setValidator(smoothPeriodValidator);

  const std::string &smoothSlowPeriod =
      std::to_string(stochasticOscillatorSettings_.smoothSlowPeriod_);
  uiDialog_.lineEdit_4->setText(QString::fromStdString(smoothSlowPeriod));
  uiDialog_.lineEdit_4->setValidator(smoothPeriodValidator);

  auto edgesValidator = new validators::IntegerValidator(1, 100, this);
  const std::string &topLevel = std::to_string(stochasticOscillatorSettings_.topLevel);
  uiDialog_.lineEdit_5->setText(QString::fromStdString(topLevel));
  uiDialog_.lineEdit_5->setValidator(edgesValidator);

  const std::string &bottomLevel = std::to_string(stochasticOscillatorSettings_.bottomLevel);
  uiDialog_.lineEdit_6->setText(QString::fromStdString(bottomLevel));
  uiDialog_.lineEdit_6->setValidator(edgesValidator);

  enableSmoothEdit(QString());

  connect(uiDialog_.lineEdit, SIGNAL(textChanged(QString)), this, SLOT(enableOkButton(QString)));
  connect(uiDialog_.lineEdit_2, SIGNAL(textChanged(QString)), this, SLOT(enableOkButton(QString)));
  connect(uiDialog_.lineEdit_3, SIGNAL(textChanged(QString)), this, SLOT(enableOkButton(QString)));
  connect(uiDialog_.lineEdit_4, SIGNAL(textChanged(QString)), this, SLOT(enableOkButton(QString)));
  connect(uiDialog_.lineEdit_5, SIGNAL(textChanged(QString)), this, SLOT(enableOkButton(QString)));
  connect(uiDialog_.lineEdit_6, SIGNAL(textChanged(QString)), this, SLOT(enableOkButton(QString)));

  connect(uiDialog_.comboBox_2, SIGNAL(currentIndexChanged(const QString &)), this,
          SLOT(enableSmoothEdit(const QString &)));

  connect(uiDialog_.buttonBox, SIGNAL(accepted()), this, SLOT(closeDialog()));
  connect(uiDialog_.buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
}

void StochasticOscillatorSettingsDialog::initTickIntervals() {
  QStringList items;
  auto lastElement = (unsigned short)common::TickInterval::UNKNOWN;
  for (unsigned short index = 0; index < lastElement; ++index) {
    const std::string interval = common::TickInterval::toString((common::TickInterval::Enum)index);
    items.append(QString::fromStdString(interval));
  }

  uiDialog_.comboBox->addItems(items);
  const std::string &currentTickIntervalStr =
      common::TickInterval::toString(stochasticOscillatorSettings_.tickInterval_);
  uiDialog_.comboBox->setCurrentText(QString::fromStdString(currentTickIntervalStr));
}

void StochasticOscillatorSettingsDialog::initStochasticType() {
  QStringList items;
  auto lastElement = (unsigned short)common::StochasticOscillatorType::Unknown;
  for (unsigned short index = 0; index < lastElement; ++index) {
    const std::string interval =
        common::convertStochasticOscillatorToString((common::StochasticOscillatorType)index);
    items.append(QString::fromStdString(interval));
  }

  uiDialog_.comboBox_2->addItems(items);
  const std::string &currentStochasticType =
      common::convertStochasticOscillatorToString(stochasticOscillatorSettings_.stochasticType_);
  uiDialog_.comboBox_2->setCurrentText(QString::fromStdString(currentStochasticType));
}

void StochasticOscillatorSettingsDialog::closeDialog() {
  stochasticOscillatorSettings_.name_ =
      convertStrategyTypeToString(common::StrategiesType::STOCHASTIC_OSCILLATOR);

  const std::string tickInterval = uiDialog_.comboBox->currentText().toStdString();
  stochasticOscillatorSettings_.tickInterval_ = common::TickInterval::fromString(tickInterval);

  const std::string stochasticOscillatorTypeStr = uiDialog_.comboBox_2->currentText().toStdString();
  stochasticOscillatorSettings_.stochasticType_ =
      common::convertStochasticOscillatorFromString(stochasticOscillatorTypeStr);

  const std::string &classicLinePeriodStr = uiDialog_.lineEdit->text().toStdString();
  stochasticOscillatorSettings_.periodsForClassicLine_ = std::stoi(classicLinePeriodStr);

  const std::string &candlesAfterCrossingStr = uiDialog_.lineEdit_2->text().toStdString();
  stochasticOscillatorSettings_.crossingInterval_ = std::stoi(candlesAfterCrossingStr);

  const std::string &smoothFastPeriodStr = uiDialog_.lineEdit_3->text().toStdString();
  if (!smoothFastPeriodStr.empty()) {
    stochasticOscillatorSettings_.smoothFastPeriod_ = std::stoi(smoothFastPeriodStr);
  }

  const std::string smoothSlowPeriodStr = uiDialog_.lineEdit_4->text().toStdString();
  if (!smoothSlowPeriodStr.empty()) {
    stochasticOscillatorSettings_.smoothSlowPeriod_ = std::stoi(smoothSlowPeriodStr);
  }

  const std::string &topLevel = uiDialog_.lineEdit_5->text().toStdString();
  stochasticOscillatorSettings_.topLevel = std::stoi(topLevel);

  const std::string &bottomLevel = uiDialog_.lineEdit_6->text().toStdString();
  stochasticOscillatorSettings_.bottomLevel = std::stoi(bottomLevel);

  accept();
}

void StochasticOscillatorSettingsDialog::enableOkButton(const QString &editText) {
  const std::string stochasticOscillatorTypeStr = uiDialog_.comboBox_2->currentText().toStdString();
  auto stochasticType = common::convertStochasticOscillatorFromString(stochasticOscillatorTypeStr);
  if (stochasticType == common::StochasticOscillatorType::Slow) {
    if (uiDialog_.lineEdit->text().isEmpty() || uiDialog_.lineEdit_2->text().isEmpty()) {
      uiDialog_.buttonBox->button(QDialogButtonBox::StandardButton::Ok)->setDisabled(true);
      return;
    }
  } else if (stochasticType == common::StochasticOscillatorType::Quick) {
    if (uiDialog_.lineEdit->text().isEmpty() || uiDialog_.lineEdit_2->text().isEmpty()) {
      uiDialog_.buttonBox->button(QDialogButtonBox::StandardButton::Ok)->setDisabled(true);
      return;
    }
  } else if (stochasticType == common::StochasticOscillatorType::Full) {
    if (uiDialog_.lineEdit->text().isEmpty() || uiDialog_.lineEdit_2->text().isEmpty() ||
        uiDialog_.lineEdit_3->text().isEmpty() || uiDialog_.lineEdit_4->text().isEmpty()) {
      uiDialog_.buttonBox->button(QDialogButtonBox::StandardButton::Ok)->setDisabled(true);
      return;
    }
  }

  uiDialog_.buttonBox->button(QDialogButtonBox::StandardButton::Ok)->setEnabled(true);
}

void StochasticOscillatorSettingsDialog::enableSmoothEdit(const QString &editText) {
  const std::string stochasticOscillatorTypeStr = uiDialog_.comboBox_2->currentText().toStdString();
  auto stochasticType = common::convertStochasticOscillatorFromString(stochasticOscillatorTypeStr);

  switch (stochasticType) {
    case common::StochasticOscillatorType::Slow: {
      uiDialog_.lineEdit_3->setText(QString("3"));
      uiDialog_.lineEdit_4->setText(QString("3"));
      uiDialog_.lineEdit_3->setDisabled(true);
      uiDialog_.lineEdit_4->setDisabled(true);
    } break;

    case common::StochasticOscillatorType::Quick: {
      uiDialog_.lineEdit_3->setDisabled(true);
      uiDialog_.lineEdit_4->setDisabled(true);
      uiDialog_.lineEdit_3->setText(QString(""));
      uiDialog_.lineEdit_4->setText(QString(""));
    } break;
    case common::StochasticOscillatorType::Full: {
      uiDialog_.lineEdit_3->setEnabled(true);
      uiDialog_.lineEdit_4->setEnabled(true);
    } break;

    default:
      break;
  }
}

}  // namespace dialogs
}  // namespace view
}  // namespace auto_trader