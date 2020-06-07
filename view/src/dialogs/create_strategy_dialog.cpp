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

#include "include/dialogs/create_strategy_dialog.h"

#include "include/dialogs/bb_advanced_settings_dialog.h"
#include "include/dialogs/bb_settings_dialog.h"
#include "include/dialogs/ema_settings_dialog.h"
#include "include/dialogs/moving_average_crossing_settings_dialog.h"
#include "include/dialogs/rsi_settings_dialog.h"
#include "include/dialogs/sma_settings_dialog.h"
#include "include/dialogs/stochastic_oscillator_dialog.h"

namespace auto_trader {
namespace view {
namespace dialogs {

CreateStrategyDialog::CreateStrategyDialog(common::AppListener &appListener,
                                           common::GuiListener &guiListener, DialogType dialogType,
                                           QWidget *parent)
    : QDialog(parent),
      appListener_(appListener),
      guiListener_(guiListener),
      dialogType_(dialogType) {
  dialog_.setupUi(this);
  dialog_.listView->setSelectionMode(QAbstractItemView::ExtendedSelection);
  dialog_.listView_2->setSelectionMode(QAbstractItemView::SingleSelection);

  dialog_.listView->setEditTriggers(QAbstractItemView::NoEditTriggers);
  dialog_.listView_2->setEditTriggers(QAbstractItemView::NoEditTriggers);

  customStrategySettings_ = std::make_unique<model::CustomStrategySettings>();
  strategySettingsFactory_ = std::make_unique<model::StrategySettingsFactory>();
  initStrategiesList();
  sizeHint();
  checkOkButtonStatus(QString());

  connect(dialog_.pushButton, SIGNAL(clicked()), this, SLOT(selectStrategy()));
  connect(dialog_.pushButton_2, SIGNAL(clicked()), this, SLOT(removeStrategy()));
  connect(dialog_.pushButton_3, SIGNAL(clicked()), this, SLOT(editStrategy()));
  connect(dialog_.buttonBox, SIGNAL(accepted()), this, SLOT(closeDialog()));
  connect(dialog_.buttonBox, SIGNAL(rejected()), this, SLOT(reject()));

  connect(dialog_.lineEdit, SIGNAL(textChanged(const QString &)), this,
          SLOT(checkOkButtonStatus(const QString &)));
  connect(this, SIGNAL(strategiesNumberChanged(const QString &)), this,
          SLOT(checkOkButtonStatus(const QString &)));
}

void CreateStrategyDialog::setupDefaultParameters(const model::CustomStrategySettings &settings) {
  customStrategyName_ = settings.name_;
  dialog_.textEdit->insertPlainText(QString::fromStdString(settings.description_));
  dialog_.lineEdit->setText(QString::fromStdString(customStrategyName_));
  for (int index = 0; index < settings.strategies_.size(); index++) {
    auto strategy = settings.strategies_.at(index).get();
    selectedStrategiesList_->append(QString::fromStdString(strategy->name_));
    strategySettings_.insert(std::make_pair<>(strategy->strategiesType_, strategy->clone()));
  }

  selectedStrategiesModel_->setStringList(*selectedStrategiesList_);

  checkOkButtonStatus(QString());
}

void CreateStrategyDialog::selectStrategy() {
  for (const QModelIndex &index : dialog_.listView->selectionModel()->selectedIndexes()) {
    auto selectedStrategy = currentStrategiesModel_->data(index).toString();
    if (selectedStrategiesList_->indexOf(selectedStrategy) == -1) {
      selectedStrategiesList_->append(selectedStrategy);
      common::StrategiesType type =
          common::convertStrategyTypeFromString(selectedStrategy.toStdString());
      const std::string strategyName = selectedStrategy.toStdString();
      auto strategySettings = strategySettingsFactory_->createStrategySettings(type);
      strategySettings->strategiesType_ = type;
      strategySettings->name_ = strategyName;
      strategySettings_.insert(std::make_pair<>(type, std::move(strategySettings)));
    }
  }

  selectedStrategiesModel_->setStringList(*selectedStrategiesList_);

  emit strategiesNumberChanged(QString());
}

void CreateStrategyDialog::removeStrategy() {
  QModelIndex modelIndex = dialog_.listView_2->currentIndex();
  QString itemText = modelIndex.data(Qt::DisplayRole).toString();
  common::StrategiesType type = common::convertStrategyTypeFromString(itemText.toStdString());
  strategySettings_.erase(type);

  int indexToRemove = selectedStrategiesList_->indexOf(itemText);
  selectedStrategiesList_->removeAt(indexToRemove);
  selectedStrategiesModel_->setStringList(*selectedStrategiesList_);

  emit strategiesNumberChanged(QString());
}

void CreateStrategyDialog::editStrategy() {
  QModelIndex modelIndex = dialog_.listView_2->currentIndex();
  QString itemText = modelIndex.data(Qt::DisplayRole).toString();
  common::StrategiesType type = common::convertStrategyTypeFromString(itemText.toStdString());
  openEditStrategySettingsDialog(type);
}

void CreateStrategyDialog::initStrategiesList() {
  currentStrategiesList_ = new QStringList();
  selectedStrategiesList_ = new QStringList();

  auto lastElement = (unsigned short)common::StrategiesType::UNKNOWN;
  for (unsigned short index = 0; index < lastElement; ++index) {
    auto type = (common::StrategiesType)index;
    // TODO: Add MACD indicator to UI
    if (type != common::StrategiesType::CUSTOM && type != common::StrategiesType::MACD) {
      const std::string strategyStr = common::convertStrategyTypeToString(type);
      currentStrategiesList_->append(QString::fromStdString(strategyStr));
    }
  }

  currentStrategiesModel_ = new QStringListModel(*currentStrategiesList_, this);
  selectedStrategiesModel_ = new QStringListModel(*selectedStrategiesList_, this);

  dialog_.listView->setModel(currentStrategiesModel_);
  dialog_.listView_2->setModel(selectedStrategiesModel_);
}

void CreateStrategyDialog::openEditStrategySettingsDialog(common::StrategiesType type) {
  auto &strategySettings = strategySettings_[type];
  switch (type) {
    case common::StrategiesType::BOLLINGER_BANDS: {
      auto bbSettings = dynamic_cast<model::BollingerBandsSettings *>(strategySettings.get());
      auto dialog = new BBSettingsDialog(*bbSettings, appListener_, guiListener_,
                                         BBSettingsDialog::CREATE, this);
      dialog->setAttribute(Qt::WA_DeleteOnClose);
      dialog->show();
    } break;
    case common::StrategiesType::BOLLINGER_BANDS_ADVANCED: {
      auto bbAdvancedSettings =
          dynamic_cast<model::BollingerBandsAdvancedSettings *>(strategySettings.get());
      auto dialog = new BBAdvancedSettingsDialog(*bbAdvancedSettings, appListener_, guiListener_,
                                                 BBAdvancedSettingsDialog::CREATE, this);
      dialog->setAttribute(Qt::WA_DeleteOnClose);
      dialog->show();
    } break;
    case common::StrategiesType::SMA: {
      auto smaSettings = dynamic_cast<model::SmaSettings *>(strategySettings.get());
      auto dialog = new SmaSettingsDialog(*smaSettings, appListener_, guiListener_,
                                          SmaSettingsDialog::CREATE, this);
      dialog->setAttribute(Qt::WA_DeleteOnClose);
      dialog->show();
    } break;
    case common::StrategiesType::EMA: {
      auto emaSettings = dynamic_cast<model::EmaSettings *>(strategySettings.get());
      auto dialog = new EmaSettingsDialog(*emaSettings, appListener_, guiListener_,
                                          EmaSettingsDialog::CREATE, this);
      dialog->setAttribute(Qt::WA_DeleteOnClose);
      dialog->show();
    } break;
    case common::StrategiesType::RSI: {
      auto rsiSettings = dynamic_cast<model::RsiSettings *>(strategySettings.get());
      auto dialog = new RsiSettingsDialog(*rsiSettings, appListener_, guiListener_,
                                          RsiSettingsDialog::CREATE, this);
      dialog->setAttribute(Qt::WA_DeleteOnClose);
      dialog->show();
    } break;
    case common::StrategiesType::MA_CROSSING: {
      auto maCrossings =
          dynamic_cast<model::MovingAveragesCrossingSettings *>(strategySettings.get());
      auto dialog = new MovingAverageCrossingSettingsDialog(
          *maCrossings, appListener_, guiListener_, MovingAverageCrossingSettingsDialog::CREATE,
          this);
      dialog->setAttribute(Qt::WA_DeleteOnClose);
      dialog->show();
    } break;
    case common::StrategiesType::STOCHASTIC_OSCILLATOR: {
      auto stochasticOscillator =
          dynamic_cast<model::StochasticOscillatorSettings *>(strategySettings.get());
      auto dialog =
          new StochasticOscillatorSettingsDialog(*stochasticOscillator, appListener_, guiListener_,
                                                 StochasticOscillatorSettingsDialog::CREATE, this);
      dialog->setAttribute(Qt::WA_DeleteOnClose);
      dialog->show();
    } break;
    default:
      break;
  }
}

void CreateStrategyDialog::closeDialog() {
  customStrategySettings_->name_ = dialog_.lineEdit->text().toStdString();
  customStrategySettings_->description_ = dialog_.textEdit->toPlainText().toStdString();
  customStrategySettings_->strategiesType_ = common::StrategiesType::CUSTOM;
  for (auto &strategy : strategySettings_) {
    auto strategySetting = std::move(strategy.second);
    customStrategySettings_->strategies_.push_back(std::move(strategySetting));
  }

  refreshStrategySettings();

  accept();
}

void CreateStrategyDialog::refreshStrategySettings() {
  switch (dialogType_) {
    case DialogType::CREATE: {
      guiListener_.createCustomStrategy(std::move(customStrategySettings_));
    } break;
    case DialogType::EDIT: {
      guiListener_.editCustomStrategy(std::move(customStrategySettings_), customStrategyName_);
    } break;

    default:
      break;
  }
}

void CreateStrategyDialog::checkOkButtonStatus(const QString &text) {
  bool isStrategiesEmpty = selectedStrategiesList_->isEmpty();
  bool isStrategyNameEmpty = dialog_.lineEdit->text().isEmpty();

  bool isButtonDisabled = isStrategiesEmpty || isStrategyNameEmpty;
  dialog_.buttonBox->button(QDialogButtonBox::StandardButton::Ok)->setDisabled(isButtonDisabled);
}

}  // namespace dialogs
}  // namespace view
}  // namespace auto_trader
