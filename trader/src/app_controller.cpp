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

#include "include/app_controller.h"

#include <sys/stat.h>
#include <sys/types.h>

#include <QMessageBox>
#include <QTextStream>
#include <QtCore/QDir>
#include <QtCore/QFile>
#include <fstream>
#include <queue>
#include <thread>

#include "common/crossplatform_functions.h"
#include "common/exceptions/model_exception/trading_model_exception.h"
#include "common/exceptions/model_exception/unknown_strategy_name_exception.h"
#include "common/loggers/file_logger.h"
#include "common/tick_interval_ratio.h"
#include "features/include/telegram_announcer.h"
#include "model/include/orders/orders_matching.h"
#include "model/include/orders/orders_profit.h"
#include "serializer/include/app_settings_json_serializer.h"
#include "serializer/include/feature_json_serializer.h"
#include "serializer/include/strategy_json_serializer.h"
#include "serializer/include/trade_config_json_serializer.h"
#include "stocks_exchange/include/query_processor.h"
#include "view/include/gui_processor.h"

namespace auto_trader {
namespace trader {

constexpr char STRATEGIES_CONFIG_EXTENSION[] = "*.json";
constexpr char DARK_THEME_PATH[] = ":qdarkstyle/style.qss";

static std::set<common::MarketOrder> getOrdersForType(
    const std::vector<common::MarketOrder> &allOrders, common::OrderType type) {
  std::set<common::MarketOrder> ordersForType;
  for (const auto &order : allOrders) {
    if (order.orderType_ == type) ordersForType.insert(order);
  }

  return ordersForType;
}

AppController::AppController(QApplication &application) : application_(application) {
  strategiesSettingsHolder_ = std::make_unique<model::StrategiesSettingsHolder>();
  tradeConfigurationsHolder_ = std::make_unique<model::TradeConfigsHolder>();
  tradeOrdersHolder_ = std::make_unique<model::TradeOrdersHolder>();
  tradeSignaledStrategyMarketHolder_ = std::make_unique<model::TradeSignaledStrategyMarketHolder>();

  strategyFacade_ = std::make_unique<strategies::StrategyFacade>();
  stockExchangeLibrary_ = std::make_unique<stock_exchange::StockExchangeLibrary>();

  guiListener_ = std::make_unique<view::GuiProcessor>(*this);

  appStatsUpdater_ = std::make_unique<AppStatsUpdater>(*this, *guiListener_);
  appStatsUpdater_->moveToThread(&statsUpdateThread_);
  statsUpdateThread_.start();

  appChartUpdater_ = std::make_unique<AppChartUpdater>(*this, *guiListener_);
  appChartUpdater_->moveToThread(&chartUpdateThread_);
  chartUpdateThread_.start();

  databaseProvider_ = std::make_unique<database::Database>();

  messageSender_ = std::make_unique<TradingMessageSender>(*guiListener_, appSettings_);

  tradingManager_ = std::make_unique<TradingManager>(
      stockExchangeLibrary_->getQueryProcessor(), *strategyFacade_, *databaseProvider_, *this,
      *guiListener_, appSettings_, *messageSender_, *strategiesSettingsHolder_, *tradeOrdersHolder_,
      *tradeConfigurationsHolder_, *tradeSignaledStrategyMarketHolder_);

  tradingManager_->moveToThread(&tradingThread_);
  tradingThread_.start();

  connect(this, SIGNAL(runStatsUpdaterThread()), appStatsUpdater_.get(), SLOT(start()));
  connect(this, SIGNAL(runTradingThread()), tradingManager_.get(), SLOT(startTradingSlot()));

  connect(this, SIGNAL(runChartUpdaterThread(unsigned int, unsigned int, unsigned int)),
          appChartUpdater_.get(),
          SLOT(refreshMarketHistory(unsigned int, unsigned int, unsigned int)));

  connect(tradingManager_.get(), SIGNAL(tradingStarted()), this, SLOT(refreshTradingStopButton()));
  connect(tradingManager_.get(), SIGNAL(tradingStopped()), this, SLOT(refreshTradingStartButton()));
  connect(tradingManager_.get(), SIGNAL(tradingDataOutdated()), this,
          SLOT(refreshTradingOutdatedData()), Qt::BlockingQueuedConnection);

  connect(appStatsUpdater_.get(), SIGNAL(tradingCurrenciesChanged()), this,
          SLOT(refreshTradingCurrenciesUI()));
  connect(appStatsUpdater_.get(), SIGNAL(allCurrenciesChanged()), this,
          SLOT(refreshAllCurrenciesUI()));
  connect(appStatsUpdater_.get(), SIGNAL(accountBalanceChanged()), this,
          SLOT(refreshAccountBalanceUI()));
  connect(appStatsUpdater_.get(), SIGNAL(allOrdersChanged()), this, SLOT(refreshAllOrdersUI()));
  connect(appStatsUpdater_.get(), SIGNAL(openOrdersChanged()), this, SLOT(refreshOpenOrdersUI()));
  connect(appStatsUpdater_.get(), SIGNAL(statsUpdateInterrupted()), this,
          SLOT(refreshStatsInterruptableUI()));

  connect(appStatsUpdater_.get(), SIGNAL(tradingStartedFromTelegram()), this,
          SLOT(tradingStartedFromTelegramUI()));
  connect(appStatsUpdater_.get(), SIGNAL(tradingStoppedFromTelegram()), this,
          SLOT(tradingStoppedFromTelegramUI()));

  connect(appChartUpdater_.get(), SIGNAL(marketHistoryChanged(unsigned int)), this,
          SLOT(refreshMarketHistoryUI(unsigned int)));

  connect(messageSender_.get(), SIGNAL(uiMessageSent(const QString &)), this,
          SLOT(printMessage(const QString &)));

  connect(appStatsUpdater_.get(), SIGNAL(progressStarted(int)), this,
          SLOT(dispatchProgressBarStartEvent(int)));
  connect(appStatsUpdater_.get(), SIGNAL(progressFinished()), this,
          SLOT(dispatchProgressBarFinishEvent()));
  connect(appStatsUpdater_.get(), SIGNAL(incProgress(int)), this,
          SLOT(incrementProgressBarValue(int)));
}

AppController::~AppController() {
  stopTradingThread();
  stopStatsUpdater();
  stopChartUpdater();
}

void AppController::loadStrategies() {
  constexpr char STRATEGIES_CONFIG_PATH[] = "/config/strategies";
  auto applicationDir = QApplication::applicationDirPath();
  QDir directory(applicationDir + STRATEGIES_CONFIG_PATH);
  QStringList files =
      directory.entryList(QStringList() << STRATEGIES_CONFIG_EXTENSION, QDir::Files);
  for (const auto &filename : files) {
    std::string filenameStr =
        directory.path().toStdString() + std::string("/") + filename.toStdString();
    std::ifstream inputStream(filenameStr);
    if (!inputStream.good()) {
      continue;
    }
    try {
      serializer::StrategyJSONSerializer strategyJSONSerializer;
      auto strategySetting = strategyJSONSerializer.deserialize(inputStream);
      if (strategySetting->strategiesType_ == common::StrategiesType::CUSTOM) {
        std::unique_ptr<model::CustomStrategySettings> customStrategySettings{
            dynamic_cast<model::CustomStrategySettings *>(strategySetting.release())};

        strategiesSettingsHolder_->addCustomStrategySettings(std::move(customStrategySettings));
      }
    } catch (std::exception &exception) {
      common::loggers::FileLogger::getLogger() << exception.what();
    }
  }
}

void AppController::loadTradeConfigurations() {
  auto applicationDir = QApplication::applicationDirPath();
  QDir directory(applicationDir + "/config");
  QStringList files = directory.entryList(QStringList() << "*.json", QDir::Files);
  bool foundActiveConfiguration = false;
  for (const auto &filename : files) {
    std::string filenameStr =
        directory.path().toStdString() + std::string("/") + filename.toStdString();
    std::ifstream inputStream(filenameStr);
    if (!inputStream.good()) {
      continue;
    }
    try {
      serializer::TradeConfigJSONSerializer tradeConfigJSONSerializer;
      auto tradeConfiguration = tradeConfigJSONSerializer.deserialize(inputStream);

      const std::string strategyName = tradeConfiguration->getStrategyName();
      if (!strategiesSettingsHolder_->containsStrategy(strategyName)) {
        throw common::exceptions::UnknownStrategyNameException(strategyName);
      }

      /* Only one configuration per time can be active */
      if (tradeConfiguration->isActive()) {
        if (!foundActiveConfiguration) {
          foundActiveConfiguration = true;
        } else {
          tradeConfiguration->setActive(false);
        }
      }

      tradeConfigurationsHolder_->addTradeConfig(std::move(tradeConfiguration));
    } catch (std::exception &exception) {
      common::loggers::FileLogger::getLogger() << exception.what();
    }
  }

  try {
    if (!foundActiveConfiguration) {
      tradeConfigurationsHolder_->setDefaultActiveConfiguration();
    }

    auto &currentTradeConfig = tradeConfigurationsHolder_->getCurrentTradeConfiguration();
    refreshApiKeys(currentTradeConfig);
  } catch (const std::exception &exception) {
    common::loggers::FileLogger::getLogger() << exception.what();
  }
}

void AppController::loadFeaturesSettings() {
  constexpr char FEATURES_CONFIG_PATH[] = "/config/features";
  auto applicationDir = QApplication::applicationDirPath();
  serializer::FeatureJsonSerializer featureJsonSerializer;
  try {
    const std::string telegramSettingsFile = applicationDir.toStdString() + FEATURES_CONFIG_PATH +
                                             std::string("/") + "telegram_settings.json";
    std::ifstream telegramInputStream(telegramSettingsFile);
    if (telegramInputStream.good()) {
      featureJsonSerializer.deserializeTelegramSettings(telegramInputStream);
    }
  } catch (std::exception &exception) {
    common::loggers::FileLogger::getLogger() << exception.what();
  }

  try {
    const std::string stopLossSettingsFile = applicationDir.toStdString() + FEATURES_CONFIG_PATH +
                                             std::string("/") + "stop_loss_settings.json";
    std::ifstream stopLossInputStream(stopLossSettingsFile);
    if (stopLossInputStream.good()) {
      featureJsonSerializer.deserializeStopLossSettings(stopLossInputStream);
    }
  } catch (std::exception &exception) {
    common::loggers::FileLogger::getLogger() << exception.what();
  }
}

void AppController::loadAppSettings() {
  try {
    constexpr char APP_SETTINGS_CONFIG_PATH[] = "/config/app_settings";

    auto applicationDir = QApplication::applicationDirPath();
    serializer::AppSettingsJsonSerializer appSettingsJsonSerializer;
    const std::string appSettingsFile = applicationDir.toStdString() + APP_SETTINGS_CONFIG_PATH +
                                        std::string("/") + "app_settings.json";
    std::ifstream inputStream(appSettingsFile);
    if (inputStream.good()) {
      appSettingsJsonSerializer.deserialize(appSettings_, inputStream);
      guiListener_->refreshLogging();
    }
  } catch (std::exception &exception) {
    common::loggers::FileLogger::getLogger() << exception.what();
  }

  changeTheme(appSettings_.theme_);
}

void AppController::saveStrategiesSettingsFiles() const {
  auto applicationDir = QApplication::applicationDirPath();
  const std::string configDir = applicationDir.toStdString() + "/config";
  const std::string strategiesDir = configDir + "/" + "strategies";

#ifdef WIN32
  if (!common::isDirectoryExists(configDir)) {
    _mkdir(configDir.c_str());
  }
  if (!common::isDirectoryExists(strategiesDir)) {
    _mkdir(strategiesDir.c_str());
  }
#else
  if (!common::isDirectoryExists(configDir)) {
    mkdir(configDir.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
  }
  if (!common::isDirectoryExists(strategiesDir)) {
    mkdir(strategiesDir.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
  }
#endif

  strategiesSettingsHolder_->forEachStrategy([&](model::StrategySettings const &settings) {
    const std::string filename = strategiesDir + std::string("/") + settings.name_ + ".json";
    remove(filename.c_str());
    serializer::StrategyJSONSerializer strategyJSONSerializer;
    std::ofstream fileStream(filename);
    strategyJSONSerializer.serialize(settings, fileStream);
  });
}

void AppController::saveTradeConfigurationsFiles() const {
  auto applicationDir = QApplication::applicationDirPath();
  const std::string configDir = applicationDir.toStdString() + "/config";
#ifdef WIN32
  if (!common::isDirectoryExists(configDir)) {
    _mkdir(configDir.c_str());
  }
#else
  if (!common::isDirectoryExists(configDir)) {
    mkdir(configDir.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
  }
#endif

  tradeConfigurationsHolder_->forEachTradeConfiguration(
      [&](model::TradeConfiguration const &configuration) {
        const std::string filename =
            configDir + std::string("/") + configuration.getName() + ".json";
        remove(filename.c_str());
        serializer::TradeConfigJSONSerializer tradeConfigJSONSerializer;
        std::ofstream fileStream(filename);
        tradeConfigJSONSerializer.serialize(configuration, fileStream);
      });
}

void AppController::saveFeaturesSettings() const {
  auto applicationDir = QApplication::applicationDirPath();
  const std::string configDir = applicationDir.toStdString() + "/config";
  const std::string featuresDir = configDir + "/" + "features";

#ifdef WIN32
  if (!common::isDirectoryExists(configDir)) {
    _mkdir(configDir.c_str());
  }
  if (!common::isDirectoryExists(featuresDir)) {
    _mkdir(featuresDir.c_str());
  }
#else
  if (!common::isDirectoryExists(configDir)) {
    mkdir(configDir.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
  }
  if (!common::isDirectoryExists(featuresDir)) {
    mkdir(featuresDir.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
  }

#endif
  const std::string telegramSettingsFileName =
      featuresDir + std::string("/") + "telegram_settings.json";
  remove(telegramSettingsFileName.c_str());
  serializer::FeatureJsonSerializer featureJsonSerializer;
  std::ofstream telegramFileStream(telegramSettingsFileName);
  featureJsonSerializer.serializeTelegramSettings(telegramFileStream);

  const std::string stopLossSettingsFileName =
      featuresDir + std::string("/") + "stop_loss_settings.json";
  remove(stopLossSettingsFileName.c_str());
  std::ofstream stopLossFileStream(stopLossSettingsFileName);
  featureJsonSerializer.serializeStopLossSettings(stopLossFileStream);
}

void AppController::saveAppSettings() const {
  auto applicationDir = QApplication::applicationDirPath();
  const std::string configDir = applicationDir.toStdString() + "/config";
  const std::string appSettingsDir = configDir + "/" + "app_settings";

#ifdef WIN32
  if (!common::isDirectoryExists(configDir)) {
    _mkdir(configDir.c_str());
  }
  if (!common::isDirectoryExists(appSettingsDir)) {
    _mkdir(appSettingsDir.c_str());
  }

#else
  if (!common::isDirectoryExists(configDir)) {
    mkdir(configDir.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
  }
  if (!common::isDirectoryExists(appSettingsDir)) {
    mkdir(appSettingsDir.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
  }
#endif

  const std::string appSettingsFilename = appSettingsDir + std::string("/") + "app_settings.json";
  remove(appSettingsFilename.c_str());
  serializer::AppSettingsJsonSerializer appSettingsJsonSerializer;
  std::ofstream stream(appSettingsFilename);
  appSettingsJsonSerializer.serialize(appSettings_, stream);
}

common::GuiListener &AppController::getGuiListener() { return *guiListener_; }

model::StrategiesSettingsHolder &AppController::getStrategySettingsHolder() {
  return *strategiesSettingsHolder_;
}

model::TradeConfigsHolder &AppController::getTradeConfigsHolder() {
  return *tradeConfigurationsHolder_;
}

model::AppSettings &AppController::getAppSettings() { return appSettings_; }

stock_exchange::QueryProcessor &AppController::getQueryProcessor() {
  return stockExchangeLibrary_->getQueryProcessor();
}

void AppController::changeTheme(common::ApplicationThemeType themeType) {
  switch (themeType) {
    case common::ApplicationThemeType::WHITE: {
      application_.setStyleSheet("");
      appSettings_.theme_ = common::ApplicationThemeType::WHITE;
      saveAppSettings();
    } break;
    case common::ApplicationThemeType::DARK: {
      QFile darkThemeFile(DARK_THEME_PATH);
      if (!darkThemeFile.exists()) {
        common::loggers::FileLogger::getLogger()
            << "Unable to set dark theme, resource file not found.\n";
      } else {
        darkThemeFile.open(QFile::ReadOnly | QFile::Text);
        QTextStream textStream(&darkThemeFile);
        application_.setStyleSheet(textStream.readAll());
        appSettings_.theme_ = common::ApplicationThemeType::DARK;
        saveAppSettings();
      }
    } break;

    default:
      break;
  }
}

void AppController::refreshUIMessage(common::RefreshUiType refreshUiType) {
  appStatsUpdater_->addMessage(refreshUiType);
}

void AppController::refreshMarketHistory(common::Currency::Enum baseCurrency,
                                         common::Currency::Enum tradedCurrency,
                                         common::TickInterval::Enum interval) {
  auto baseCurrencyInt = static_cast<unsigned int>(baseCurrency);
  auto tradedCurrencyInt = static_cast<unsigned int>(tradedCurrency);
  auto intervalInt = static_cast<unsigned int>(interval);
  emit runChartUpdaterThread(baseCurrencyInt, tradedCurrencyInt, intervalInt);
}

void AppController::refreshStockExchangeView() { appStatsUpdater_->refreshStockExchangeViewData(); }

void AppController::refreshTradingView() { appStatsUpdater_->refreshTradingViewData(); }

void AppController::refreshApiKeys(const model::TradeConfiguration &configuration) {
  auto &stockExchangeSettings = configuration.getStockExchangeSettings();
  auto &queryProcessor = stockExchangeLibrary_->getQueryProcessor();
  auto query = queryProcessor.getQuery(stockExchangeSettings.stockExchangeType_);
  query->updateApiKey(stockExchangeSettings.apiKey_);
  query->updateSecretKey(stockExchangeSettings.secretKey_);
}

void AppController::startStatsUpdater() {
  appStatsUpdater_->initialize();

  emit runStatsUpdaterThread();
}

void AppController::interruptStatsUpdate() { appStatsUpdater_->setUpdateInterruptable(); }

void AppController::stopTradingThread() {
  stopTrading();

  tradingThread_.quit();
  tradingThread_.wait();
}

void AppController::stopStatsUpdater() {
  appStatsUpdater_->stop();

  statsUpdateThread_.quit();
  statsUpdateThread_.wait();
}

void AppController::stopChartUpdater() {
  chartUpdateThread_.quit();
  chartUpdateThread_.wait();
}

void AppController::runTrading() {
  emit runTradingThread();

  guiListener_->dispatchTradingStartEvent();
}

void AppController::stopTrading() {
  tradingManager_->stopTradingSlot();

  guiListener_->dispatchTradingFinishEvent();
}

void AppController::refreshTradingCurrenciesUI() { appStatsUpdater_->refreshTradingCurrenciesUI(); }

void AppController::refreshAllCurrenciesUI() { appStatsUpdater_->refreshAllCurrenciesUI(); }

void AppController::refreshMarketHistoryUI(unsigned int stockExchangeType) {
  appChartUpdater_->refreshMarketHistoryUI(stockExchangeType);
}

void AppController::refreshAccountBalanceUI() { appStatsUpdater_->refreshAccountBalanceUI(); }

void AppController::refreshAllOrdersUI() { appStatsUpdater_->refreshAllOrdersUI(); }

void AppController::refreshOpenOrdersUI() { appStatsUpdater_->refreshOpenOrdersUI(); }

void AppController::refreshStatsInterruptableUI() {
  appStatsUpdater_->refreshStatsInterruptableUI();
}

void AppController::refreshTradingStartButton() { guiListener_->refreshTradingStartButton(true); }

void AppController::refreshTradingStopButton() { guiListener_->refreshTradingStopButton(true); }

void AppController::refreshTradingOutdatedData() {
  bool value = guiListener_->refreshTradingOutdatedData();
  tradingManager_->reset(value);
}

void AppController::printMessage(const QString &message) {
  guiListener_->printMessage(message.toStdString());
}

void AppController::dispatchProgressBarStartEvent(int maximum) {
  guiListener_->dispatchProgressBarStartEvent(maximum);
}

void AppController::dispatchProgressBarFinishEvent() {
  guiListener_->dispatchProgressBarFinishEvent();
}

void AppController::incrementProgressBarValue(int value) {
  guiListener_->incrementProgressBarValue(value);
}

void AppController::tradingStartedFromTelegramUI() { runTrading(); }

void AppController::tradingStoppedFromTelegramUI() { stopTrading(); }

}  // namespace trader
}  // namespace auto_trader