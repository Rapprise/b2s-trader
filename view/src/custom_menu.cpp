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

#include "include/custom_menu.h"

#include <QProgressBar>
#include <QtWidgets/QAction>
#include <QtWidgets/QFileDialog>
#include <QtWidgets/QMessageBox>
#include <QtWidgets/QWidgetAction>
#include <fstream>

#include "common/exceptions/model_exception/no_active_trading_config_exception.h"
#include "common/exceptions/model_exception/unknown_strategy_name_exception.h"
#include "common/exceptions/strategy_exception/wrong_strategy_type_in_json_file.h"
#include "common/listeners/app_listener.h"
#include "common/loggers/file_logger.h"
#include "include/dialogs/about_dialog.h"
#include "include/dialogs/bb_settings_dialog.h"
#include "include/dialogs/create_strategy_dialog.h"
#include "include/dialogs/create_trade_configuration_dialog.h"
#include "include/dialogs/ema_settings_dialog.h"
#include "include/dialogs/intervals_dialog.h"
#include "include/dialogs/moving_average_crossing_settings_dialog.h"
#include "include/dialogs/rsi_settings_dialog.h"
#include "include/dialogs/sma_settings_dialog.h"
#include "include/dialogs/stop_loss_dialog.h"
#include "include/dialogs/telegram_dialog.h"
#include "include/gui_processor.h"
#include "model/include/holders/strategies_settings_holder.h"
#include "model/include/settings/strategies_settings/custom_strategy_settings.h"
#include "model/include/settings/strategies_settings/strategy_settings.h"
#include "serializer/include/strategy_json_serializer.h"
#include "serializer/include/trade_config_json_serializer.h"
#include "ui_bb_settings_dialog.h"
#include "ui_create_trade_configuration_dialog.h"
#include "ui_sma_settings_dialog.h"

namespace auto_trader {
namespace view {

CustomMenu::CustomMenu(common::AppListener& appListener, GuiProcessor& guiProcessor)
    : appListener_(appListener), guiListener_(guiProcessor) {
  setupUi(&guiProcessor);

  const QIcon runTradingIcon = QIcon(":/b2s_images/run_trading.png");
  const QIcon stopTradingIcon = QIcon(":/b2s_images/stop_trading.png");
  const QIcon startLoggingIcon = QIcon(":/b2s_images/logging_on.png");
  const QIcon stopLoggingIcon = QIcon(":/b2s_images/logging_off.png");
  const QIcon clearLoggingIcon = QIcon(":/b2s_images/logging_clear.png");
  const QIcon newIcon = QIcon(":/b2s_images/new_config.png");
  const QIcon openIcon = QIcon(":/b2s_images/open_existing_config.png");
  const QIcon removeIcon = QIcon(":/b2s_images/remove_config.png");
  const QIcon closeIcon = QIcon(":/b2s_images/close_config.png");
  const QIcon telegramIcon = QIcon(":/b2s_images/telegram_icon.png");
  const QIcon stopLossIcon = QIcon(":/b2s_images/stop_loss.png");
  const QIcon intervalsIcon = QIcon(":/b2s_images/intervals.png");
  const QIcon themesIcon = QIcon(":/b2s_images/themes.png");
  const QIcon whiteThemeIcon = QIcon(":/b2s_images/white_theme.png");
  const QIcon darkThemeIcon = QIcon(":/b2s_images/dark_theme.png");
  const QIcon aboutIcon = QIcon(":/b2s_images/about.png");
  const QIcon statisticStartUpdateIcon = QIcon(":/b2s_images/update_statistic.png");
  const QIcon statisticStopUpdateIcon = QIcon(":/b2s_images/stop_updating.png");

  /** Trading toolbar */
  runTradingAction_ = new QAction(runTradingIcon, tr("&Run Trading"), this);
  stopTradingAction_ = new QAction(stopTradingIcon, tr("&Stop Trading"), this);

  /** Logging toolbar */
  loggingStartAction_ = new QAction(startLoggingIcon, tr("&Start Logging"), this);
  loggingStopAction_ = new QAction(stopLoggingIcon, tr("&Stop Logging"), this);
  loggingClearAction_ = new QAction(clearLoggingIcon, tr("&Clear Logging"), this);

  loggingStopAction_->setDisabled(true);

  /** Statistic toolbar */
  statisticStartUpdateAction_ =
      new QAction(statisticStartUpdateIcon, tr("&Start statistic update"), this);
  statisticStopUpdateAction_ =
      new QAction(statisticStopUpdateIcon, tr("&Stop statistic update"), this);

  actionNew->setIcon(newIcon);
  actionLoad->setIcon(openIcon);
  actionClose->setIcon(closeIcon);
  actionRemove->setIcon(removeIcon);

  actionNew->setShortcut(QKeySequence("Ctrl+N"));
  actionLoad->setShortcut(QKeySequence("Ctrl+O"));
  actionClose->setShortcut(QKeySequence("Ctrl+Q"));

  initToolBar(&guiProcessor);

  runTradingAction_->setEnabled(false);
  stopTradingAction_->setDisabled(true);

  runTradingAction_->setShortcut(QKeySequence(Qt::Key_F5));
  stopTradingAction_->setShortcut(QKeySequence(Qt::Key_F6));

  loggingStartAction_->setShortcut(QKeySequence(Qt::Key_F10));
  loggingStopAction_->setShortcut(QKeySequence(Qt::Key_F11));

  /** File menu **/

  connect(actionNew, &QAction::triggered, this, &CustomMenu::createConfiguration);
  connect(actionLoad, &QAction::triggered, this, &CustomMenu::loadConfiguration);
  connect(actionRemove, &QAction::triggered, this, &CustomMenu::removeCurrentConfiguration);
  connect(actionClose, &QAction::triggered, this, &CustomMenu::closeCurrentConfiguration);

  /** Features menu **/
  connect(actionTelegram, &QAction::triggered, this, &CustomMenu::openTelegramSettings);
  connect(actionStop_loss, &QAction::triggered, this, &CustomMenu::openStopLossSettings);
  connect(actionIntervals, &QAction::triggered, this, &CustomMenu::openIntervalsSettings);

  actionTelegram->setIcon(telegramIcon);
  actionStop_loss->setIcon(stopLossIcon);
  actionIntervals->setIcon(intervalsIcon);

  /** Strategies menu **/
  connect(actionStrategyAdd, &QAction::triggered, this, &CustomMenu::createCustomStrategy);
  connect(actionStrategyLoad, &QAction::triggered, this, &CustomMenu::loadCustomStrategy);

  const QIcon addStrategyIcon = QIcon(":/b2s_images/add_custom_strategy.png");
  const QIcon loadStrategyIcon = QIcon(":/b2s_images/load_custom_strategy.png");
  actionStrategyAdd->setIcon(addStrategyIcon);
  actionStrategyAdd->setShortcut(QKeySequence("Ctrl+Shift+N"));
  actionStrategyLoad->setIcon(loadStrategyIcon);
  actionStrategyLoad->setShortcut(QKeySequence("Ctrl+Shift+O"));

  const QIcon customStrategiesIcon = QIcon(":/b2s_images/custom_strategies.png");
  menuCustom->setIcon(customStrategiesIcon);

  /** View menu **/
  connect(actionViewStrategies, &QAction::triggered, &guiProcessor,
          &GuiProcessor::showStrategiesView);
  connect(actionViewConfigurations, &QAction::triggered, &guiProcessor,
          &GuiProcessor::showConfigurationsView);
  connect(actionViewTrading_currencies, &QAction::triggered, &guiProcessor,
          &GuiProcessor::showTradingCurrenciesView);
  connect(actionViewAllCurrencies, &QAction::triggered, &guiProcessor,
          &GuiProcessor::showAllCurrenciesView);
  connect(actionViewLogging_console, &QAction::triggered, &guiProcessor,
          &GuiProcessor::showLoggingConsoleView);
  connect(actionViewOrders, &QAction::triggered, &guiProcessor, &GuiProcessor::showOpenOrdersView);
  connect(actionAll_Orders, &QAction::triggered, &guiProcessor, &GuiProcessor::showAllOrdersView);
  connect(actionAccount_Balance, &QAction::triggered, &guiProcessor,
          &GuiProcessor::showAccountBalanceView);

  actionViewStrategies->setCheckable(true);
  actionViewConfigurations->setCheckable(true);
  actionViewTrading_currencies->setCheckable(true);
  actionViewAllCurrencies->setCheckable(true);
  actionViewLogging_console->setCheckable(true);
  actionViewOrders->setCheckable(true);
  actionAll_Orders->setCheckable(true);
  actionAccount_Balance->setCheckable(true);

  /*Theme menu*/

  menuTheme->setIcon(themesIcon);

  actionWhite->setIcon(whiteThemeIcon);
  actionDark->setIcon(darkThemeIcon);

  connect(actionWhite, &QAction::triggered, &guiProcessor, &GuiProcessor::setWhiteTheme);
  connect(actionDark, &QAction::triggered, &guiProcessor, &GuiProcessor::setDarkTheme);

  /** Trading menu **/

  connect(runTradingAction_, &QAction::triggered, &guiProcessor, &GuiProcessor::runTrading);
  connect(stopTradingAction_, &QAction::triggered, &guiProcessor, &GuiProcessor::stopTrading);

  /** Logging menu **/
  connect(loggingStartAction_, &QAction::triggered, this, &CustomMenu::startLogging);
  connect(loggingStopAction_, &QAction::triggered, this, &CustomMenu::stopLogging);
  connect(loggingClearAction_, &QAction::triggered, this, &CustomMenu::clearLogging);

  /** Statistic menu **/
  connect(statisticStartUpdateAction_, &QAction::triggered, this,
          &CustomMenu::startStatisticUpdate);
  connect(statisticStopUpdateAction_, &QAction::triggered, this, &CustomMenu::stopStatisticUpdate);

  /** Help menu **/

  actionAbout->setIcon(aboutIcon);

  connect(actionAbout, &QAction::triggered, this, &CustomMenu::openAbout);
}

void CustomMenu::initToolBar(QMainWindow* window) {
  toolBar_ = new QToolBar(window);
  toolBar_->setObjectName("toolbar_object");
  toolBar_->setIconSize(QSize(24, 24));

  toolBar_->addAction(actionNew);
  toolBar_->addAction(actionLoad);
  toolBar_->addAction(actionRemove);
  toolBar_->addAction(actionClose);
  toolBar_->addSeparator();
  toolBar_->addAction(runTradingAction_);
  toolBar_->addAction(stopTradingAction_);
  toolBar_->addSeparator();
  toolBar_->addAction(loggingStartAction_);
  toolBar_->addAction(loggingStopAction_);
  toolBar_->addSeparator();
  toolBar_->addAction(loggingClearAction_);
  toolBar_->addSeparator();
  toolBar_->addAction(statisticStartUpdateAction_);

  initTradingStatus(window);
  window->addToolBar(toolBar_);
}

void CustomMenu::initTradingStatus(QMainWindow* window) {
  QWidget* spacer = new QWidget(window);
  spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  toolBar_->addWidget(spacer);

  auto& tradeConfigsHolder = appListener_.getTradeConfigsHolder();

  try {
    auto& tradeConfig = appListener_.getTradeConfigsHolder().getCurrentTradeConfiguration();

    auto currentConfigLabel = new QLabel(window);
    QIcon configIcon = QIcon(":/b2s_images/current_config.png");
    QPixmap configPixmap = configIcon.pixmap(QSize(24, 24));
    currentConfigLabel->setPixmap(configPixmap);

    toolBar_->addWidget(currentConfigLabel);

    const std::string& configTitleStr = " Current configuration : " + tradeConfig.getName();
    currentConfigLabel_ = new QLabel(QString::fromStdString(configTitleStr));
    toolBar_->addWidget(currentConfigLabel_);

    QWidget* configPlaceHolder = new QWidget(window);
    toolBar_->addWidget(configPlaceHolder);
    configPlaceHolder->setMinimumWidth(20);

    auto baseCurrencyLabel = new QLabel(window);
    QIcon baseConfigIcon = QIcon(":/b2s_images/base_currency.png");
    QPixmap iconCurrencyPixmap = baseConfigIcon.pixmap(QSize(24, 24));
    baseCurrencyLabel->setPixmap(iconCurrencyPixmap);

    toolBar_->addWidget(baseCurrencyLabel);

    auto baseCurrency = tradeConfig.getCoinSettings().baseCurrency_;
    const std::string baseCurrencyStr = common::Currency::toString(baseCurrency);

    const std::string& currencyTitleStr = " Base Currency : " + baseCurrencyStr;
    baseCurrencyTitleLabel_ = new QLabel(QString::fromStdString(currencyTitleStr));
    toolBar_->addWidget(baseCurrencyTitleLabel_);

    QWidget* currencyPlaceHolder = new QWidget(window);
    toolBar_->addWidget(currencyPlaceHolder);
    currencyPlaceHolder->setMinimumWidth(20);
  } catch (common::exceptions::NoActiveTradingConfigException& exception) {
    auto currentConfigLabel = new QLabel(window);
    QIcon configIcon = QIcon(":/b2s_images/current_config.png");
    QPixmap configPixmap = configIcon.pixmap(QSize(24, 24));
    currentConfigLabel->setPixmap(configPixmap);

    toolBar_->addWidget(currentConfigLabel);

    currentConfigLabel_ = new QLabel(" Current configuration : None");
    toolBar_->addWidget(currentConfigLabel_);

    QWidget* configPlaceHolder = new QWidget(window);
    toolBar_->addWidget(configPlaceHolder);
    configPlaceHolder->setMinimumWidth(20);

    auto baseCurrencyLabel = new QLabel(window);
    QIcon baseConfigIcon = QIcon(":/b2s_images/base_currency.png");
    QPixmap iconCurrencyPixmap = baseConfigIcon.pixmap(QSize(24, 24));
    baseCurrencyLabel->setPixmap(iconCurrencyPixmap);

    toolBar_->addWidget(baseCurrencyLabel);

    baseCurrencyTitleLabel_ = new QLabel(" Base Currency : None");
    toolBar_->addWidget(baseCurrencyTitleLabel_);

    QWidget* currencyPlaceHolder = new QWidget(window);
    toolBar_->addWidget(currencyPlaceHolder);
    currencyPlaceHolder->setMinimumWidth(20);
  }

  tradingStatusHeaderLabel_ = new QLabel(" Trading status: ");
  toolBar_->addWidget(tradingStatusHeaderLabel_);

  tradingActivityLabel_ = new QLabel(window);
  QIcon icon = QIcon(":/b2s_images/trading_status_inactive.png");
  QPixmap iconPixmap = icon.pixmap(QSize(24, 24));
  tradingActivityLabel_->setPixmap(iconPixmap);

  toolBar_->addWidget(tradingActivityLabel_);

  QWidget* currencyPlaceHolder = new QWidget(window);
  toolBar_->addWidget(currencyPlaceHolder);
  currencyPlaceHolder->setMinimumWidth(20);

  progressLabel_ = new QLabel("Updating data: ", window);
  progressLabel_->setText("");
  toolBar_->addWidget(progressLabel_);

  actionProgressBar_ = new QWidgetAction(window);
  progressBar_ = new QProgressBar(window);
  progressBar_->setMaximumWidth(150);
  actionProgressBar_->setDefaultWidget(progressBar_);
  toolBar_->addAction(actionProgressBar_);
  toolBar_->addAction(statisticStopUpdateAction_);

  QWidget* currencyPlaceHolder2 = new QWidget(window);
  toolBar_->addWidget(currencyPlaceHolder2);
  currencyPlaceHolder2->setMinimumWidth(20);

  actionProgressBar_->setVisible(false);
  statisticStopUpdateAction_->setVisible(false);
  progressBar_->setVisible(false);
}

void CustomMenu::createConfiguration() {
  configurationDialog_.reset();
  configurationDialog_ = std::make_unique<dialogs::CreateTradeConfigurationDialog>(
      appListener_, guiListener_, dialogs::CreateTradeConfigurationDialog::DialogType::CREATE,
      &guiListener_);

  configurationDialog_->show();
}

void CustomMenu::loadConfiguration() {
  auto fileName = QFileDialog::getOpenFileName(menuVIew, tr("Load Trading Configuration"),
                                               QDir::currentPath(), tr("JSON Files (*.json)"));
  const std::string& fileNameStr = fileName.toStdString();
  if (fileName.isEmpty()) return;

  std::ifstream inputStream(fileNameStr.c_str());
  if (!inputStream.good()) {
    QMessageBox::information(&guiListener_, tr("Broken file"),
                             tr("Selected file is broken or path is invalid."));
    return;
  }
  try {
    serializer::TradeConfigJSONSerializer tradeConfigJSONSerializer;
    auto tradeConfiguration = tradeConfigJSONSerializer.deserialize(inputStream);

    auto& tradeConfigsHolder = appListener_.getTradeConfigsHolder();
    if (tradeConfigsHolder.containsConfiguration(tradeConfiguration->getName())) {
      QMessageBox::information(
          &guiListener_, tr("Trading configuration exists."),
          tr("Trading configuration with current name exists. Please provide another file."));

      return;
    }

    const std::string strategyName = tradeConfiguration->getStrategyName();
    auto& strategySettingsHolder = appListener_.getStrategySettingsHolder();
    if (!strategySettingsHolder.containsStrategy(strategyName)) {
      throw common::exceptions::UnknownStrategyNameException(strategyName);
    }

    bool isConfigsHolderEmpty = tradeConfigsHolder.isEmpty();

    tradeConfiguration->setActive(isConfigsHolderEmpty);
    tradeConfigsHolder.addTradeConfig(std::move(tradeConfiguration));

    appListener_.saveTradeConfigurationsFiles();
    guiListener_.refreshTradeConfigurationView();

    if (isConfigsHolderEmpty) {
      restoreConfigurationStatusBar();
      guiListener_.refreshStockExchangeChartInterval();
      guiListener_.refreshStockExchangeChartMarket();
      guiListener_.refreshChartViewStart();
      appListener_.refreshStockExchangeView();
    }
  } catch (std::exception& exception) {
    QMessageBox::information(&guiListener_, tr("JSON file is broken"),
                             tr("Selected file is broken or strategy name is undefined."));
    common::loggers::FileLogger::getLogger() << exception.what();
  }
}

void CustomMenu::removeCurrentConfiguration() {
  auto& tradeHolders = appListener_.getTradeConfigsHolder();
  try {
    auto& currentConfig = tradeHolders.getCurrentTradeConfiguration();
    if (currentConfig.isRunning()) {
      QMessageBox::information(
          &guiListener_, tr("Trading configuration cannot be removed"),
          tr("The current trading configuration is running. Please stop trading before removing."));
      return;
    }

    auto uiLock = guiListener_.acquireUILock();
    if (!uiLock.try_lock() || guiListener_.isUIUpdating()) {
      QMessageBox::information(
          &guiListener_, tr("Trading configuration cannot be removed"),
          tr("The UI is updating right now. Please wait until process is finished."));
      return;
    }

    QMessageBox::StandardButton reply;

    if (tradeHolders.getConfigurationsCount() > 1) {
      reply = QMessageBox::question(
          &guiListener_, "Remove current active configuration",
          "Are you sure you want to remove current configuration? Since it is active, new active "
          "configuration will be chosen from remaining ones.",
          QMessageBox::Yes | QMessageBox::No);
    } else {
      reply = QMessageBox::question(&guiListener_, "Remove current active configuration",
                                    "Are you sure you want to remove current active configuration?",
                                    QMessageBox::Yes | QMessageBox::No);
    }

    if (reply == QMessageBox::Yes) {
      auto applicationDir = QApplication::applicationDirPath();
      const std::string configurationPath = applicationDir.toStdString() + std::string("/") +
                                            "config" + std::string("/") + currentConfig.getName() +
                                            ".json";
      tradeHolders.removeTradeConfig(currentConfig.getName());
      remove(configurationPath.c_str());

      restoreConfigurationStatusBar();

      if (tradeHolders.isEmpty()) {
        guiListener_.resetChart();
        guiListener_.disableChart();
        guiListener_.refreshTradeConfigurationView();
        appListener_.refreshStockExchangeView();
      } else {
        tradeHolders.setDefaultActiveConfiguration();
        guiListener_.refreshTradeConfigurationView();
        guiListener_.refreshChartViewStart();
        appListener_.refreshStockExchangeView();
      }

      appListener_.saveTradeConfigurationsFiles();
    }
  } catch (common::exceptions::NoActiveTradingConfigException& exception) {
    QMessageBox::information(&guiListener_, tr("No active configurations"),
                             tr("There is no active trading configuration."));
  }
}

void CustomMenu::closeCurrentConfiguration() {
  auto& tradeHolders = appListener_.getTradeConfigsHolder();
  if (tradeHolders.isEmpty()) return;

  try {
    auto uiLock = guiListener_.acquireUILock();
    if (!uiLock.try_lock() || guiListener_.isUIUpdating()) {
      QMessageBox::information(
          &guiListener_, tr("Trading configurations cannot be closed."),
          tr("The UI is updating right now. Please wait until process is finished."));

      return;
    }

    auto& currentConfig = tradeHolders.getCurrentTradeConfiguration();
    if (currentConfig.isRunning()) {
      QMessageBox::information(
          &guiListener_, tr("Trading configuration cannot be closed."),
          tr("The current trading configuration is running. Please stop trading before removing."));

      return;
    }

    QMessageBox::StandardButton reply;
    if (tradeHolders.getConfigurationsCount() > 1) {
      reply = QMessageBox::question(
          &guiListener_, "Close current active configuration",
          "Are you sure you want to remove current configuration? Since it is active, new active "
          "configuration will be chosen from remaining ones.",
          QMessageBox::Yes | QMessageBox::No);
    } else {
      reply = QMessageBox::question(&guiListener_, "Close current active configuration",
                                    "Are you sure you want to close current active configuration?",
                                    QMessageBox::Yes | QMessageBox::No);
    }

    if (reply == QMessageBox::Yes) {
      const std::string currentConfigName = currentConfig.getName();

      tradeHolders.removeTradeConfig(currentConfigName);

      tradeHolders.setDefaultActiveConfiguration();
      appListener_.saveTradeConfigurationsFiles();
      guiListener_.refreshConfigurationStatusBar();
      guiListener_.refreshTradeConfigurationView();

      if (tradeHolders.isEmpty()) {
        guiListener_.resetChart();
        guiListener_.disableChart();
        appListener_.refreshStockExchangeView();
      } else {
        guiListener_.refreshChartViewStart();
        appListener_.refreshStockExchangeView();
      }
    }
  } catch (common::exceptions::NoActiveTradingConfigException& exception) {
    QMessageBox::information(&guiListener_, tr("No configurations found"),
                             tr("There are no trading configurations."));
  }
}

void CustomMenu::createCustomStrategy() {
  customStrategyDialog_.reset();
  customStrategyDialog_ = std::make_unique<dialogs::CreateStrategyDialog>(
      appListener_, guiListener_, dialogs::CreateStrategyDialog::CREATE, &guiListener_);

  customStrategyDialog_->show();
}

void CustomMenu::loadCustomStrategy() {
  auto fileName = QFileDialog::getOpenFileName(&guiListener_, tr("Load Custom Strategy"),
                                               QDir::currentPath(), tr("JSON Files (*.json)"));
  const std::string& fileNameStr = fileName.toStdString();
  if (fileName.isEmpty()) return;

  std::ifstream inputStream(fileNameStr.c_str());
  if (!inputStream.good()) {
    QMessageBox::information(&guiListener_, tr("Broken file"),
                             tr("Selected file is broken or path is invalid."));
    return;
  }
  try {
    serializer::StrategyJSONSerializer strategyJSONSerializer;
    auto strategySettings = strategyJSONSerializer.deserialize(inputStream);

    auto& strategySettingsHolder = appListener_.getStrategySettingsHolder();
    if (strategySettingsHolder.containsStrategy(strategySettings->name_)) {
      QMessageBox::information(
          &guiListener_, tr("Strategy exists."),
          tr("Strategy with current name exists. Please provide another file."));

      return;
    }

    if (strategySettings->strategiesType_ == common::StrategiesType::CUSTOM) {
      std::unique_ptr<model::CustomStrategySettings> customStrategySettings{
          dynamic_cast<model::CustomStrategySettings*>(strategySettings.release())};

      strategySettingsHolder.addCustomStrategySettings(std::move(customStrategySettings));
      appListener_.saveStrategiesSettingsFiles();
      guiListener_.refreshStrategiesView();

    } else {
      throw common::exceptions::WrongStrategyInJsonFile(
          common::convertStrategyTypeToString(strategySettings->strategiesType_));
    }
  } catch (std::exception& exception) {
    QMessageBox::information(&guiListener_, tr("JSON file is broken"),
                             tr("Selected file is broken."));
    common::loggers::FileLogger::getLogger() << exception.what();
  }
}

void CustomMenu::openTelegramSettings() {
  telegramDialog_.reset();
  telegramDialog_ = std::make_unique<dialogs::TelegramDialog>(appListener_, &guiListener_);
  telegramDialog_->show();
}

void CustomMenu::openStopLossSettings() {
  stopLossDialog_.reset();
  stopLossDialog_ = std::make_unique<dialogs::StopLossDialog>(appListener_, &guiListener_);
  stopLossDialog_->show();
}

void CustomMenu::openIntervalsSettings() {
  intervalsDialog_.reset();
  intervalsDialog_ = std::make_unique<dialogs::IntervalsDialog>(appListener_, &guiListener_);
  intervalsDialog_->show();
}

void CustomMenu::openAbout() {
  aboutDialog_.reset();
  aboutDialog_ = std::make_unique<dialogs::AboutDialog>(
      appListener_, guiListener_.getLicenseOwner(), guiListener_.getLicenseExpirationDate(),
      &guiListener_);
  aboutDialog_->show();
}

void CustomMenu::dispatchTradingStartEvent() {
  actionRemove->setDisabled(true);
  actionClose->setDisabled(true);
  runTradingAction_->setDisabled(true);

  QIcon icon = QIcon(":/b2s_images/trading_status_active.png");
  QPixmap iconPixmap = icon.pixmap(QSize(24, 24));
  tradingActivityLabel_->setPixmap(iconPixmap);
}

void CustomMenu::dispatchTradingFinishEvent() {
  actionRemove->setEnabled(true);
  actionClose->setEnabled(true);
  stopTradingAction_->setDisabled(true);

  QIcon icon = QIcon(":/b2s_images/trading_status_inactive.png");
  QPixmap iconPixmap = icon.pixmap(QSize(24, 24));
  tradingActivityLabel_->setPixmap(iconPixmap);
}

void CustomMenu::refreshTradingStartButton(bool value) { runTradingAction_->setEnabled(value); }

void CustomMenu::refreshTradingStopButton(bool value) { stopTradingAction_->setEnabled(value); }

void CustomMenu::startLogging() {
  loggingStartAction_->setDisabled(true);
  loggingStopAction_->setEnabled(true);

  auto& appSettings = appListener_.getAppSettings();
  appSettings.uiLoggingEnabled_ = true;
  appListener_.saveAppSettings();
}

void CustomMenu::stopLogging() {
  loggingStartAction_->setEnabled(true);
  loggingStopAction_->setDisabled(true);

  auto& appSettings = appListener_.getAppSettings();
  appSettings.uiLoggingEnabled_ = false;

  appListener_.saveAppSettings();
}

void CustomMenu::clearLogging() { guiListener_.clearLogging(); }

void CustomMenu::restoreLogging() {
  auto& appSettings = appListener_.getAppSettings();
  loggingStartAction_->setDisabled(appSettings.uiLoggingEnabled_);
  loggingStopAction_->setEnabled(appSettings.uiLoggingEnabled_);
}

void CustomMenu::restoreConfigurationStatusBar() {
  try {
    auto& tradeConfigsHolder = appListener_.getTradeConfigsHolder();
    auto& currentTradeConfiguration = tradeConfigsHolder.getCurrentTradeConfiguration();
    const std::string& configTitleStr =
        " Current configuration : " + currentTradeConfiguration.getName();
    currentConfigLabel_->setText(QString::fromStdString(configTitleStr));

    auto baseCurrency = currentTradeConfiguration.getCoinSettings().baseCurrency_;
    const std::string baseCurrencyStr = common::Currency::toString(baseCurrency);

    const std::string& currencyTitleStr = " Base Currency : " + baseCurrencyStr;
    baseCurrencyTitleLabel_->setText(QString::fromStdString(currencyTitleStr));

    // TODO: Move trading enabling to somewhere else.
    runTradingAction_->setEnabled(true);
    stopTradingAction_->setDisabled(true);

  } catch (common::exceptions::NoActiveTradingConfigException& exception) {
    common::loggers::FileLogger::getLogger() << exception.what();

    const std::string& configTitleStr = " Current configuration : None";
    currentConfigLabel_->setText(QString::fromStdString(configTitleStr));

    const std::string& currencyTitleStr = " Base Currency : None";
    baseCurrencyTitleLabel_->setText(QString::fromStdString(currencyTitleStr));

    // TODO: Move trading enabling to somewhere else.
    runTradingAction_->setEnabled(false);
    stopTradingAction_->setDisabled(false);
  }
}

void CustomMenu::dispatchProgressBarStartEvent(int maximum) {
  guiListener_.setUIUpdating(true);
  progressLabel_->setVisible(true);
  progressLabel_->setText("Updating data: ");
  progressBar_->setVisible(true);
  statisticStopUpdateAction_->setVisible(true);
  statisticStartUpdateAction_->setEnabled(false);
  actionProgressBar_->setVisible(true);

  progressBar_->setRange(0, maximum);
  progressBar_->setValue(0);
}

void CustomMenu::dispatchProgressBarFinishEvent() {
  progressLabel_->setText("");
  progressBar_->setVisible(false);
  actionProgressBar_->setVisible(false);
  statisticStopUpdateAction_->setVisible(false);
  statisticStartUpdateAction_->setEnabled(true);
  guiListener_.setUIUpdating(false);
}

void CustomMenu::startStatisticUpdate() { appListener_.refreshStockExchangeView(); }

void CustomMenu::stopStatisticUpdate() { appListener_.interruptStatsUpdate(); }

void CustomMenu::incrementProgressBarValue(int value) {
  auto currentValue = progressBar_->value();
  if (currentValue < 0) {
    currentValue = 0;
  }
  progressBar_->setValue(currentValue + value);
  if (progressBar_->maximum() <= progressBar_->value()) {
    dispatchProgressBarFinishEvent();
  }
}

}  // namespace view
}  // namespace auto_trader