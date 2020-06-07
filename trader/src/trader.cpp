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

#include "include/trader.h"

#include <QMessageBox>
#include <QPixmap>
#include <QSplashScreen>
#include <QtCore/QSharedMemory>
#include <QtCore/QSystemSemaphore>
#include <QtWidgets/QApplication>
#include <iostream>

#include "common/exceptions/base_exception.h"
#include "common/loggers/file_logger.h"

namespace auto_trader {
namespace trader {

Trader::Trader() {}

Trader& Trader::getInstance() {
  static Trader trader;
  return trader;
}

int Trader::init(int argc, char** argv) {
  QApplication application(argc, argv);

  QSystemSemaphore semaphore("4c44c0d6-e5f2-11e9-81b4-2a2ae2dbcce4", 1);
  semaphore.acquire();

#ifndef Q_OS_WIN32
  QSharedMemory sharedMemoryHolder("4c44c342-e5f2-11e9-81b4-2a2ae2dbcce4");
  if (sharedMemoryHolder.attach()) {
    sharedMemoryHolder.detach();
  }
#endif

  QSharedMemory sharedMemory("4c44c342-e5f2-11e9-81b4-2a2ae2dbcce4");

  if (sharedMemory.attach()) {
    semaphore.release();
    return 0;
  }

  sharedMemory.create(1);
  semaphore.release();

  application.setAttribute(Qt::AA_DisableWindowContextHelpButton);
  setlocale(LC_NUMERIC, "C");
  auto appController_ = std::make_unique<AppController>(application);
  auto& guiListener = appController_->getGuiListener();
  try {
    appController_->loadStrategies();
    appController_->loadTradeConfigurations();
    appController_->loadFeaturesSettings();
    appController_->loadAppSettings();

    guiListener.refreshLogging();
    guiListener.refreshConfigurationStatusBar();

    guiListener.showMainWindow();

    if (!guiListener.checkLicense()) {
      return 0;
    }

    appController_->startStatsUpdater();

    guiListener.refreshStrategiesView();
    guiListener.refreshTradeConfigurationView();
    guiListener.refreshStockExchangeChartInterval();
    guiListener.refreshStockExchangeChartMarket();
    guiListener.refreshChartViewStart();

    appController_->refreshStockExchangeView();

  } catch (common::exceptions::BaseException& baseException) {
    common::loggers::FileLogger::getLogger() << baseException.what();
  } catch (std::exception& exception) {
    common::loggers::FileLogger::getLogger() << exception.what();
  } catch (...) {
    common::loggers::FileLogger::getLogger()
        << "Undefined exception type raised. Please contact technical support.";
  }

  return application.exec();
}

}  // namespace trader
}  // namespace auto_trader