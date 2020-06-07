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

#include "include/gui_processor.h"

#include <QDesktopWidget>
#include <QSettings>
#include <QTimer>
#include <QtWidgets/QDockWidget>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QMessageBox>
#include <QtWidgets/QToolBar>

#include "common/binance_currency.h"
#include "common/bittrex_currency.h"
#include "common/enumerations/refresh_ui_type.h"
#include "common/kraken_currency.h"
#include "common/listeners/app_listener.h"
#include "common/loggers/file_logger.h"
#include "features/include/license.h"
#include "include/custom_chart_widget.h"
#include "include/custom_menu.h"
#include "include/dialogs/license_dialog.h"
#include "include/gui_nodes/gui_tree_node_type.h"
#include "include/view_model/currencies_view_model.h"
#include "include/view_model/strategies_view_model.h"
#include "include/view_model/trading_view_model.h"
#include "model/include/trade_configuration.h"

namespace auto_trader {
namespace view {

constexpr int MIN_APP_WIDTH = 1300;
constexpr int MIN_APP_HEIGHT = 700;

constexpr int MIN_CENTRAL_WIDGET_WIDTH = 640;
constexpr int MIN_CENTRAL_WIDGET_HEIGHT = 320;

GuiProcessor::GuiProcessor(common::AppListener &appListener)
    : QMainWindow(),
      appListener_(appListener),
      customMenu_(new CustomMenu(appListener_, *this)),
      customChartWidget_(new CustomChartWidget(appListener_, this)),
      uiUpdating_(false) {
  QPixmap pixmap(":/b2s_images/start_logo.png");

  splashScreen = new QSplashScreen(this, pixmap);
  splashScreen->show();

#ifndef __APPLE__
  menuBar()->setNativeMenuBar(false);
#endif

  setCentralWidget(customChartWidget_);

  auto centrWidget = centralWidget();
  if (centrWidget) {
    centrWidget->setMinimumSize(QSize(MIN_CENTRAL_WIDGET_WIDTH, MIN_CENTRAL_WIDGET_HEIGHT));
  }

  setWindowTitle(tr("B2S Trader 1.0"));

#ifdef __linux__
  QIcon windowIcon = QIcon(":/b2s_images/B2S_icon.png");
  setWindowIcon(windowIcon);
#endif

  setWindowState(Qt::WindowMaximized);

  setSizePolicy(QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Expanding);
  setMinimumSize(MIN_APP_WIDTH, MIN_APP_HEIGHT);

  initDockWindows();

  strategyContextMenuHandler_ =
      std::make_unique<StrategyContextMenuHandler>(*strategiesView_, appListener_, *this);
  tradeConfigContextMenuHanler_ =
      std::make_unique<TradeConfigContextMenuHandler>(*tradingExplorerView_, appListener_, *this);

  readSettings();
}

GuiProcessor::~GuiProcessor() { delete customMenu_; }

void GuiProcessor::initDockWindows() {
  strategiesView_ = new QTreeView(this);
  strategiesView_->setContextMenuPolicy(Qt::CustomContextMenu);
  strategiesView_->setSizePolicy(QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Expanding);
  strategiesView_->header()->setDefaultAlignment(Qt::AlignCenter);
  strategiesView_->header()->setSectionResizeMode(QHeaderView::Stretch);
  strategiesView_->header()->setStretchLastSection(false);

  strategiesDock_ = new QDockWidget(tr("Strategies"), this);
  strategiesDock_->setMinimumWidth(300);
  strategiesDock_->setAllowedAreas(Qt::LeftDockWidgetArea);
  strategiesDock_->setWidget(strategiesView_);
  strategiesDock_->setObjectName(tr("Strategies"));

  addDockWidget(Qt::LeftDockWidgetArea, strategiesDock_);

  tradingExplorerView_ = new QTreeView(this);
  tradingExplorerView_->setItemDelegate(new TradingViewModelStyleDelegate(appListener_, this));
  tradingExplorerView_->setContextMenuPolicy(Qt::CustomContextMenu);
  tradingExplorerView_->setSizePolicy(QSizePolicy::Policy::Expanding,
                                      QSizePolicy::Policy::Expanding);
  tradingExplorerView_->header()->setSectionResizeMode(QHeaderView::Stretch);
  tradingExplorerView_->header()->setStretchLastSection(false);

  tradingConfigDock_ = new QDockWidget(tr("Trading Configurations"), this);
  tradingConfigDock_->setObjectName(tr("Trading Configurations"));
  tradingConfigDock_->setMinimumWidth(300);
  tradingConfigDock_->setAllowedAreas(Qt::LeftDockWidgetArea);
  tradingConfigDock_->setWidget(tradingExplorerView_);
  addDockWidget(Qt::LeftDockWidgetArea, tradingConfigDock_);

  customConsole_ = new QTextEdit(this);
  customConsole_->setText(QString("Welcome to B2S Trader 1.0"));
  customConsole_->setReadOnly(true);
  customConsole_->setSizePolicy(QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Expanding);

  consoleDock_ = new QDockWidget(tr("Logger view"), this);
  consoleDock_->setAllowedAreas(Qt::BottomDockWidgetArea);
  consoleDock_->setWidget(customConsole_);
  consoleDock_->setObjectName(tr("Logger view"));
  consoleDock_->setMinimumHeight(200);
  addDockWidget(Qt::BottomDockWidgetArea, consoleDock_);

  currenciesView_ = new QTableView(this);
  currenciesView_->setSizePolicy(QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Expanding);
  currenciesView_->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
  currenciesView_->setEditTriggers(QAbstractItemView::NoEditTriggers);
  currenciesDock_ = new QDockWidget(tr("Currencies"), this);
  currenciesDock_->setAllowedAreas(Qt::RightDockWidgetArea);
  currenciesDock_->setWidget(currenciesView_);
  currenciesDock_->setMinimumWidth(300);
  currenciesDock_->setObjectName(tr("Currencies"));

  addDockWidget(Qt::RightDockWidgetArea, currenciesDock_);

  tradingCurrenciesView_ = new QTableView(this);
  tradingCurrenciesView_->setSizePolicy(QSizePolicy::Policy::Expanding,
                                        QSizePolicy::Policy::Expanding);
  tradingCurrenciesView_->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
  tradingCurrenciesView_->setEditTriggers(QAbstractItemView::NoEditTriggers);
  tradingCurrenciesDock_ = new QDockWidget(tr("Trading Currencies"), this);
  tradingCurrenciesDock_->setAllowedAreas(Qt::RightDockWidgetArea);
  tradingCurrenciesDock_->setWidget(tradingCurrenciesView_);
  tradingCurrenciesDock_->setMinimumWidth(300);
  tradingCurrenciesDock_->setObjectName(tr("Trading Currencies"));
  addDockWidget(Qt::RightDockWidgetArea, tradingCurrenciesDock_);

  accountBalanceView_ = new QTableView(this);
  accountBalanceView_->setSizePolicy(QSizePolicy::Policy::Expanding,
                                     QSizePolicy::Policy::Expanding);
  accountBalanceView_->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
  accountBalanceView_->setEditTriggers(QAbstractItemView::NoEditTriggers);
  accountBalanceDock_ = new QDockWidget(tr("Account Balance"), this);
  accountBalanceDock_->setAllowedAreas(Qt::RightDockWidgetArea | Qt::LeftDockWidgetArea);
  accountBalanceDock_->setWidget(accountBalanceView_);
  accountBalanceDock_->setObjectName("Account Balance");
  addDockWidget(Qt::RightDockWidgetArea, accountBalanceDock_);

  openOrdersView_ = new QTableView(this);
  openOrdersView_->setSizePolicy(QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Expanding);
  openOrdersView_->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
  openOrdersView_->setEditTriggers(QAbstractItemView::NoEditTriggers);
  openOrdersDock_ = new QDockWidget(tr("Open stock exchange orders"), this);
  openOrdersDock_->setAllowedAreas(Qt::BottomDockWidgetArea);
  openOrdersDock_->setWidget(openOrdersView_);
  openOrdersDock_->setObjectName(tr("Open stock exchange orders"));
  openOrdersDock_->setMinimumHeight(200);
  addDockWidget(Qt::BottomDockWidgetArea, openOrdersDock_);

  allOrdersView_ = new QTableView(this);
  allOrdersView_->setSizePolicy(QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Expanding);
  allOrdersView_->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
  allOrdersView_->setEditTriggers(QAbstractItemView::NoEditTriggers);
  allOrdersDock_ = new QDockWidget(tr("All stock exchange orders"), this);
  allOrdersDock_->setAllowedAreas(Qt::BottomDockWidgetArea);
  allOrdersDock_->setWidget(allOrdersView_);
  allOrdersDock_->setObjectName(tr("All stock exchange orders"));
  allOrdersDock_->setMinimumHeight(200);
  addDockWidget(Qt::BottomDockWidgetArea, allOrdersDock_);

  tabifyDockWidget(openOrdersDock_, allOrdersDock_);
  tabifyDockWidget(currenciesDock_, tradingCurrenciesDock_);

  connect(tradingConfigDock_, &QDockWidget::visibilityChanged, this, [&](bool visible) {
    customMenu_->actionViewConfigurations->setChecked(!tradingConfigDock_->isHidden());
  });

  connect(strategiesDock_, &QDockWidget::visibilityChanged, this, [&](bool visible) {
    customMenu_->actionViewStrategies->setChecked(!strategiesDock_->isHidden());
  });

  connect(consoleDock_, &QDockWidget::visibilityChanged, this, [&](bool visible) {
    customMenu_->actionViewLogging_console->setChecked(!consoleDock_->isHidden());
  });

  connect(currenciesDock_, &QDockWidget::visibilityChanged, this, [&](bool visible) {
    customMenu_->actionViewAllCurrencies->setChecked(!currenciesDock_->isHidden());
  });

  connect(tradingCurrenciesDock_, &QDockWidget::visibilityChanged, this, [&](bool visible) {
    customMenu_->actionViewTrading_currencies->setChecked(!tradingCurrenciesDock_->isHidden());
  });

  connect(accountBalanceDock_, &QDockWidget::visibilityChanged, this, [&](bool visible) {
    customMenu_->actionAccount_Balance->setChecked(!accountBalanceDock_->isHidden());
  });

  connect(allOrdersDock_, &QDockWidget::visibilityChanged, this, [&](bool visible) {
    customMenu_->actionAll_Orders->setChecked(!allOrdersDock_->isHidden());
  });

  connect(openOrdersDock_, &QDockWidget::visibilityChanged, this, [&](bool visible) {
    customMenu_->actionViewOrders->setChecked(!openOrdersDock_->isHidden());
  });
}

void GuiProcessor::showStrategiesView() {
  bool isHidden = strategiesDock_->isHidden();
  customMenu_->actionViewStrategies->setChecked(isHidden);
  isHidden ? strategiesDock_->show() : strategiesDock_->hide();
}

void GuiProcessor::showConfigurationsView() {
  bool isHidden = tradingConfigDock_->isHidden();
  customMenu_->actionViewConfigurations->setChecked(isHidden);
  isHidden ? tradingConfigDock_->show() : tradingConfigDock_->hide();
}

void GuiProcessor::showTradingCurrenciesView() {
  bool isHidden = tradingCurrenciesDock_->isHidden();
  customMenu_->actionViewTrading_currencies->setChecked(isHidden);
  tradingCurrenciesDock_->isHidden() ? tradingCurrenciesDock_->show()
                                     : tradingCurrenciesDock_->hide();
}

void GuiProcessor::showAllCurrenciesView() {
  bool isHidden = currenciesDock_->isHidden();
  customMenu_->actionViewAllCurrencies->setChecked(isHidden);
  isHidden ? currenciesDock_->show() : currenciesDock_->hide();
}

void GuiProcessor::showLoggingConsoleView() {
  bool isHidden = consoleDock_->isHidden();
  customMenu_->actionViewLogging_console->setChecked(isHidden);
  isHidden ? consoleDock_->show() : consoleDock_->hide();
}

void GuiProcessor::showOpenOrdersView() {
  bool isHidden = openOrdersDock_->isHidden();
  customMenu_->actionViewOrders->setChecked(isHidden);
  isHidden ? openOrdersDock_->show() : openOrdersDock_->hide();
}

void GuiProcessor::showAllOrdersView() {
  bool isHidden = allOrdersDock_->isHidden();
  customMenu_->actionAll_Orders->setChecked(isHidden);
  isHidden ? allOrdersDock_->show() : allOrdersDock_->hide();
}

void GuiProcessor::showAccountBalanceView() {
  bool isHidden = accountBalanceDock_->isHidden();
  customMenu_->actionAccount_Balance->setChecked(isHidden);
  isHidden ? accountBalanceDock_->show() : accountBalanceDock_->hide();
}

bool GuiProcessor::checkLicense() {
  features::license::License license;
  if (!license.isLicenseExpired()) {
    licenseOwner_ = license.getName() + " " + license.getSurname();
    licenseExpirationDate_ = license.getExpireDate();
    return true;
  }

  splashScreen->close();
  auto licenseDialog = new dialogs::LicenseDialog(license, this);
  QRect screenGeometry = QApplication::desktop()->screenGeometry();
  int x = (screenGeometry.width() - licenseDialog->width()) / 2;
  int y = (screenGeometry.height() - licenseDialog->height()) / 2;
  licenseDialog->move(x, y);
  licenseDialog->exec();

  licenseOwner_ = license.getName() + " " + license.getSurname();
  licenseExpirationDate_ = license.getExpireDate();

  return licenseDialog->getLicenseStatus();
}

std::string GuiProcessor::getLicenseOwner() const { return licenseOwner_; }

std::string GuiProcessor::getLicenseExpirationDate() const { return licenseExpirationDate_; }

void GuiProcessor::showMainWindow() {
  QTimer::singleShot(1500, splashScreen, SLOT(close()));
  QTimer::singleShot(1500, this, SLOT(show()));
}

void GuiProcessor::refreshStrategiesView() {
  std::vector<model::StrategySettings const *> strategies;
  auto &strategiesHolder = appListener_.getStrategySettingsHolder();
  strategiesHolder.forEachStrategy([&](model::CustomStrategySettings &strategySettings) {
    strategies.push_back(&strategySettings);
  });

  strategiesViewModel_.reset();
  strategiesViewModel_ = std::make_unique<StrategiesViewModel>(strategies);
  strategiesView_->setModel(strategiesViewModel_.get());
}

void GuiProcessor::refreshTradeConfigurationView() {
  std::vector<model::TradeConfiguration const *> tradeConfigurations;
  auto &tradeConfigHolder = appListener_.getTradeConfigsHolder();
  tradeConfigHolder.forEachTradeConfiguration([&](model::TradeConfiguration &tradeConfiguration) {
    tradeConfigurations.push_back(&tradeConfiguration);
  });

  tradingViewModel_.reset();
  tradingViewModel_ = std::make_unique<TradingViewModel>(tradeConfigurations);
  tradingExplorerView_->setModel(tradingViewModel_.get());
}

void GuiProcessor::refreshTradingCurrenciesView(
    const std::vector<common::CurrencyTick> &currencies) {
  tradingCurrenciesViewModel_.reset();
  tradingCurrenciesViewModel_ = std::make_unique<CurrenciesViewModel>(currencies);
  tradingCurrenciesView_->setModel(tradingCurrenciesViewModel_.get());
}

void GuiProcessor::refreshAllOrdersView(const std::vector<common::MarketOrder> &allOrders) {
  allOrdersViewModel_.reset();
  allOrdersViewModel_ = std::make_unique<TradingMarketOrdersViewModel>(allOrders);
  allOrdersView_->setModel(allOrdersViewModel_.get());
}

void GuiProcessor::refreshOpenOrdersView(const std::vector<common::MarketOrder> &openOrders) {
  openOrdersViewModel_.reset();
  openOrdersViewModel_ = std::make_unique<TradingAccountOrdersViewModel>(openOrders);
  openOrdersView_->setModel(openOrdersViewModel_.get());
}

void GuiProcessor::refreshAccountBalanceView(const std::vector<AccountBalance> &accountBalance) {
  balanceViewModel_.reset();
  balanceViewModel_ = std::make_unique<BalanceViewModel>(accountBalance);
  accountBalanceView_->setModel(balanceViewModel_.get());
}

void GuiProcessor::refreshAllCurrenciesView(const std::vector<common::CurrencyTick> &currencies) {
  allCurrenciesViewModel_.reset();
  allCurrenciesViewModel_ = std::make_unique<CurrenciesViewModel>(currencies);
  currenciesView_->setModel(allCurrenciesViewModel_.get());
}

void GuiProcessor::refreshChartViewStart() {
  try {
    customChartWidget_->refreshChartViewStart();
  } catch (std::exception &exception) {
    common::loggers::FileLogger::getLogger() << exception.what();
  }
}

void GuiProcessor::refreshChartViewFinish(common::MarketHistoryPtr marketHistory,
                                          common::StockExchangeType stockExchangeType) {
  try {
    if (marketHistory->marketData_.empty()) {
      printMessage("Market history could not be loaded");
    }
    customChartWidget_->refreshChartViewFinish(std::move(marketHistory), stockExchangeType);
  } catch (std::exception &exception) {
    common::loggers::FileLogger::getLogger() << exception.what();
  }
}

void GuiProcessor::refreshStockExchangeChartInterval() {
  customChartWidget_->refreshStockExchangeTickInterval();
}

void GuiProcessor::refreshStockExchangeChartMarket() {
  customChartWidget_->refreshStockExchangeMarket();
}

void GuiProcessor::refreshLogging() { customMenu_->restoreLogging(); }

void GuiProcessor::refreshConfigurationStatusBar() { customMenu_->restoreConfigurationStatusBar(); }

void GuiProcessor::refreshTradingStartButton(bool value) {
  customMenu_->refreshTradingStartButton(value);
}

void GuiProcessor::refreshTradingStopButton(bool value) {
  customMenu_->refreshTradingStopButton(value);
}

bool GuiProcessor::refreshTradingOutdatedData() {
  QMessageBox::StandardButton reply =
      QMessageBox::question(this, "Saved trading information for current configuration settings",
                            "There are saved data for current configuration settings from previous "
                            "trading in database. Would you like to reset it?",
                            QMessageBox::Yes | QMessageBox::No);

  return (reply == QMessageBox::Yes);
}

void GuiProcessor::runTrading() {
  if (checkTradingInterval()) {
    return;
  }

  appListener_.runTrading();
}

void GuiProcessor::stopTrading() { appListener_.stopTrading(); }

void GuiProcessor::setDarkTheme() { appListener_.changeTheme(common::ApplicationThemeType::DARK); }

void GuiProcessor::setWhiteTheme() {
  appListener_.changeTheme(common::ApplicationThemeType::WHITE);
}

void GuiProcessor::printMessage(const std::string &message) {
  customConsole_->append(QString::fromStdString(message));
}

void GuiProcessor::createTradeConfiguration(
    std::unique_ptr<model::TradeConfiguration> configuration) {
  auto &tradeConfigsHolder = appListener_.getTradeConfigsHolder();

  if (!tradeConfigsHolder.isEmpty()) {
    tradeConfigsHolder.addTradeConfig(std::move(configuration));
    appListener_.saveTradeConfigurationsFiles();
    refreshTradeConfigurationView();
    return;
  }

  bool isActive = false;
  if (tradeConfigsHolder.isEmpty()) {
    configuration->setActive(true);
    isActive = true;
  }

  appListener_.refreshApiKeys(*configuration);
  tradeConfigsHolder.addTradeConfig(std::move(configuration));

  appListener_.saveTradeConfigurationsFiles();
  refreshTradeConfigurationView();

  if (isActive) {
    refreshConfigurationStatusBar();
    refreshStockExchangeChartInterval();
    refreshStockExchangeChartMarket();
    refreshChartViewStart();
    appListener_.refreshStockExchangeView();
  }
}

void GuiProcessor::editTradeConfiguration(
    std::unique_ptr<model::TradeConfiguration> newConfiguration,
    const std::string &currentConfigName) {
  auto &tradeConfigHolder = appListener_.getTradeConfigsHolder();
  tradeConfigHolder.removeTradeConfig(currentConfigName);
  auto applicationDir = QApplication::applicationDirPath();
  const std::string &tradeConfigPath = applicationDir.toStdString() + std::string("/") + "config" +
                                       std::string("/") + currentConfigName + ".json";
  remove(tradeConfigPath.c_str());

  appListener_.refreshApiKeys(*newConfiguration);

  bool isActive = newConfiguration->isActive();

  tradeConfigHolder.addTradeConfig(std::move(newConfiguration));

  refreshTradeConfigurationView();
  appListener_.saveTradeConfigurationsFiles();

  if (isActive) {
    refreshConfigurationStatusBar();
    refreshStockExchangeChartInterval();
    refreshStockExchangeChartMarket();
    refreshChartViewStart();
    appListener_.refreshStockExchangeView();
  }
}

void GuiProcessor::createCustomStrategy(std::unique_ptr<model::CustomStrategySettings> settings) {
  auto &strategiesHolder = appListener_.getStrategySettingsHolder();
  strategiesHolder.addCustomStrategySettings(std::move(settings));

  appListener_.saveStrategiesSettingsFiles();
  refreshStrategiesView();
}

void GuiProcessor::editCustomStrategy(std::unique_ptr<model::CustomStrategySettings> settings,
                                      const std::string &currentCustomStrategyName) {
  auto &strategyHolder = appListener_.getStrategySettingsHolder();
  strategyHolder.removeCustomStrategy(currentCustomStrategyName);
  auto applicationDir = QApplication::applicationDirPath();
  const std::string &customStrategyPath = applicationDir.toStdString() + std::string("/") +
                                          "config/strategies" + std::string("/") +
                                          currentCustomStrategyName + ".json";
  remove(customStrategyPath.c_str());

  strategyHolder.addCustomStrategySettings(std::move(settings));
  appListener_.saveStrategiesSettingsFiles();
  refreshStrategiesView();
}

void GuiProcessor::dispatchTradingStartEvent() { customMenu_->dispatchTradingStartEvent(); }

void GuiProcessor::dispatchTradingFinishEvent() { customMenu_->dispatchTradingFinishEvent(); }

void GuiProcessor::closeEvent(QCloseEvent *event) {
  if (!appListener_.getTradeConfigsHolder().isEmpty()) {
    try {
      auto &currentConfiguration =
          appListener_.getTradeConfigsHolder().getCurrentTradeConfiguration();
      if (currentConfiguration.isRunning()) {
        QMessageBox::StandardButton button =
            QMessageBox::question(this, tr("Close B2S Trader"),
                                  tr("Trading process is active now. It will be stopped after "
                                     "closing app.\n Are you sure you want to quit?"));
        if (button == QMessageBox::Yes) {
          QSettings settings("Rapprise", "B2S");
          settings.setValue("geometry", saveGeometry());
          settings.setValue("windowState", saveState());
          event->accept();
          return;
        } else {
          event->ignore();
          return;
        }
      }
    } catch (std::exception &exception) {
      common::loggers::FileLogger::getLogger() << exception.what();
    }
  }

  QSettings settings("Rapprise", "B2S");
  settings.setValue("geometry", saveGeometry());
  settings.setValue("windowState", saveState());

  event->accept();
}

void GuiProcessor::readSettings() {
  QSettings settings("Rapprise", "B2S");
  restoreGeometry(settings.value("geometry").toByteArray());
  restoreState(settings.value("windowState").toByteArray());
}

void GuiProcessor::dispatchProgressBarStartEvent(int maximum) {
  customMenu_->dispatchProgressBarStartEvent(maximum);
}

void GuiProcessor::dispatchProgressBarFinishEvent() {
  customMenu_->dispatchProgressBarFinishEvent();
}

void GuiProcessor::incrementProgressBarValue(int value) {
  customMenu_->incrementProgressBarValue(value);
}

std::unique_lock<std::mutex> GuiProcessor::acquireUILock() {
  return std::unique_lock<std::mutex>(uiLocker_, std::defer_lock);
}

bool GuiProcessor::isUIUpdating() { return uiUpdating_; }

void GuiProcessor::setUIUpdating(bool value) { uiUpdating_ = value; }

void GuiProcessor::disableChart() { customChartWidget_->disableChart(); }

void GuiProcessor::enableChart() { customChartWidget_->enableChart(); }

void GuiProcessor::resetChart() { customChartWidget_->resetChart(); }

void GuiProcessor::clearLogging() { customConsole_->clear(); }

bool GuiProcessor::checkTradingInterval() {
  try {
    std::map<std::string, std::string> strategiesToRevise;
    auto &currentConfiguration =
        appListener_.getTradeConfigsHolder().getCurrentTradeConfiguration();
    const std::string strategyName = currentConfiguration.getStrategyName();

    auto stockExchangeType = currentConfiguration.getStockExchangeSettings().stockExchangeType_;
    auto stockExchangeIntervals = common::getStockExchangeIntervals(stockExchangeType);
    auto &customStrategy = appListener_.getStrategySettingsHolder().getCustomStrategy(strategyName);
    for (size_t index = 0; index < customStrategy.getStrategiesCount(); ++index) {
      auto &strategy = customStrategy.getStrategy(index);
      if (stockExchangeIntervals.find(strategy->tickInterval_) == stockExchangeIntervals.end()) {
        strategiesToRevise.insert(std::make_pair<>(
            strategy->name_, common::TickInterval::toString(strategy->tickInterval_)));
      }
    }

    if (strategiesToRevise.empty()) {
      return false;
    }

    std::string invalidIntervals;
    std::string allowedIntervals;

    for (auto iterator : strategiesToRevise) {
      invalidIntervals += "[" + iterator.first + " : " + iterator.second + "]";
      invalidIntervals += " , ";
    }
    invalidIntervals.pop_back();
    invalidIntervals.pop_back();

    allowedIntervals += "[";
    for (auto iterator : stockExchangeIntervals) {
      allowedIntervals += common::TickInterval::toString(iterator.first);
      allowedIntervals += ", ";
    }
    allowedIntervals.pop_back();
    allowedIntervals.pop_back();
    allowedIntervals += "]";

    const std::string header =
        "Strategy '" + strategyName + "' has unsuitable tick interval for current stock exchange.";
    const std::string description = "Strategy '" + strategyName + "' tick intervals :\n" +
                                    invalidIntervals +
                                    "\nCannot match to current stock exchange.\n"
                                    "Required tick intervals are :\n" +
                                    allowedIntervals;

    QMessageBox::information(this, tr(header.c_str()), tr(description.c_str()));

    return true;
  } catch (std::exception &exception) {
    common::loggers::FileLogger::getLogger() << exception.what();
  }

  return false;
}

}  // namespace view
}  // namespace auto_trader