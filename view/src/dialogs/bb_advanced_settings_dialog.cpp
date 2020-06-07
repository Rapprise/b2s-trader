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

#include "include/dialogs/bb_advanced_settings_dialog.h"

#include <QtWidgets/QPushButton>

#include "include/validators/integer_validator.h"

namespace auto_trader {
namespace view {
namespace dialogs {

BBAdvancedSettingsDialog::BBAdvancedSettingsDialog(model::BollingerBandsAdvancedSettings &settings,
                                                   common::AppListener &appListener,
                                                   common::GuiListener &guiListener,
                                                   DialogType dialogType, QWidget *parent)
    : QDialog(parent),
      bbSettings_(settings),
      appListener_(appListener),
      guiListener_(guiListener),
      dialogType_(dialogType) {
  uiDialog_.setupUi(this);
  initTickIntervals();
  initBBInputType();

  QValidator *intValidator = new validators::IntegerValidator(1, 100, this);

  const std::string &standardDeviationsStr = std::to_string(settings.standardDeviations_);
  uiDialog_.lineEdit->setText(QString::fromStdString(standardDeviationsStr));
  uiDialog_.lineEdit->setValidator(intValidator);

  const std::string &periodStr = std::to_string(settings.period_);
  uiDialog_.lineEdit_2->setText(QString::fromStdString(periodStr));
  uiDialog_.lineEdit_2->setValidator(intValidator);

  const std::string &topLinePercentageStr = std::to_string(settings.topLinePercentage_);
  uiDialog_.lineEdit_3->setText(QString::fromStdString(topLinePercentageStr));
  uiDialog_.lineEdit_3->setValidator(intValidator);

  const std::string &bottomLinePercentageStr = std::to_string(settings.bottomLinePercentage_);
  uiDialog_.lineEdit_4->setText(QString::fromStdString(bottomLinePercentageStr));
  uiDialog_.lineEdit_4->setValidator(intValidator);

  QValidator *crossingIntervalValidator = new validators::IntegerValidator(0, 10, this);
  const std::string &crossingIntervalStr = std::to_string(settings.crossingInterval_);
  uiDialog_.lineEdit_5->setText(QString::fromStdString(crossingIntervalStr));
  uiDialog_.lineEdit_5->setValidator(crossingIntervalValidator);

  checkOkButtonStatus(QString());

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
  connect(uiDialog_.lineEdit_5, SIGNAL(textChanged(const QString &)), this,
          SLOT(checkOkButtonStatus(const QString &)));
}

void BBAdvancedSettingsDialog::initTickIntervals() {
  QStringList items;
  auto lastElement = (unsigned short)common::TickInterval::UNKNOWN;
  for (unsigned short index = 0; index < lastElement; ++index) {
    const std::string interval = common::TickInterval::toString((common::TickInterval::Enum)index);
    items.append(QString::fromStdString(interval));
  }

  uiDialog_.comboBox->addItems(items);
  const std::string &currentTickIntervalStr =
      common::TickInterval::toString(bbSettings_.tickInterval_);
  uiDialog_.comboBox->setCurrentText(QString::fromStdString(currentTickIntervalStr));
}

void BBAdvancedSettingsDialog::initBBInputType() {
  QStringList items;
  auto lastElement = (unsigned short)common::BollingerInputType::unknown;
  for (unsigned short index = 0; index < lastElement; ++index) {
    const std::string type = common::convertBBInputTypeToString((common::BollingerInputType)index);
    items.append(QString::fromStdString(type));
  }
  uiDialog_.comboBox_3->addItems(items);
  const std::string &bbInputTypeStr = common::convertBBInputTypeToString(bbSettings_.bbInputType_);
  uiDialog_.comboBox_3->setCurrentText(QString::fromStdString(bbInputTypeStr));
}

void BBAdvancedSettingsDialog::closeDialog() {
  const std::string standardDeviations = uiDialog_.lineEdit->text().toStdString();
  const std::string period = uiDialog_.lineEdit_2->text().toStdString();

  bbSettings_.name_ = convertStrategyTypeToString(common::StrategiesType::BOLLINGER_BANDS_ADVANCED);
  bbSettings_.period_ = stoi(period);
  bbSettings_.standardDeviations_ = stoi(standardDeviations);
  bbSettings_.strategiesType_ = common::StrategiesType::BOLLINGER_BANDS_ADVANCED;

  const std::string tickInterval = uiDialog_.comboBox->currentText().toStdString();
  bbSettings_.tickInterval_ = common::TickInterval::fromString(tickInterval);

  const std::string bbInput = uiDialog_.comboBox_3->currentText().toStdString();
  bbSettings_.bbInputType_ = common::convertBBInputTypeFromString(bbInput);

  auto topLinePercentStr = uiDialog_.lineEdit_3->text().toStdString();
  auto topLinePercent = std::stoi(topLinePercentStr);

  bbSettings_.topLinePercentage_ = topLinePercent;

  auto bottomLinePercentStr = uiDialog_.lineEdit_4->text().toStdString();
  auto bottomLinePercent = std::stoi(bottomLinePercentStr);
  bbSettings_.bottomLinePercentage_ = bottomLinePercent;

  const std::string crossingInterval = uiDialog_.lineEdit_3->text().toStdString();
  bbSettings_.crossingInterval_ = std::stoi(crossingInterval);

  accept();
}

void BBAdvancedSettingsDialog::checkOkButtonStatus(const QString &text) {
  bool isPeriodEmpty = uiDialog_.lineEdit_2->text().isEmpty();
  bool isStandardDeviationsEmpty = uiDialog_.lineEdit->text().isEmpty();
  bool isTopLinePercentageEmpty = uiDialog_.lineEdit_3->text().isEmpty();
  bool isBottomLinePercentageEmpty = uiDialog_.lineEdit_4->text().isEmpty();
  bool isCrossingIntervalEmpty = uiDialog_.lineEdit_5->text().isEmpty();

  bool isButtonOkDisabled = isPeriodEmpty || isStandardDeviationsEmpty ||
                            isTopLinePercentageEmpty || isBottomLinePercentageEmpty ||
                            isCrossingIntervalEmpty;

  if (!isButtonOkDisabled) {
    auto topLinePercent = std::stoi(uiDialog_.lineEdit_3->text().toStdString());
    auto bottomLinePercent = std::stoi(uiDialog_.lineEdit_4->text().toStdString());
    bool buttonEnabled = (topLinePercent <= 100) && (bottomLinePercent <= 100) &&
                         (topLinePercent > 0) && (bottomLinePercent > 0);

    uiDialog_.buttonBox->button(QDialogButtonBox::StandardButton::Ok)->setEnabled(buttonEnabled);

  } else {
    uiDialog_.buttonBox->button(QDialogButtonBox::StandardButton::Ok)
        ->setDisabled(isButtonOkDisabled);
  }
}

}  // namespace dialogs
}  // namespace view
}  // namespace auto_trader