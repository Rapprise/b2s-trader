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

#ifndef AUTO_TRADER_VIEW_CUSTOM_MENU_H
#define AUTO_TRADER_VIEW_CUSTOM_MENU_H

#include <QDialog>
#include <QLabel>
#include <QObject>
#include <QProgressBar>
#include <QToolBar>
#include <QWidgetAction>
#include <memory>

#include "ui_main_menu.h"

namespace Ui {
class MainMenu;
}

namespace auto_trader {
namespace common {
class AppListener;
class GuiListener;
}  // namespace common
namespace trader {
class AppController;
}

namespace view {

class GuiProcessor;

class CustomMenu : public QObject, public Ui::MainMenu {
  Q_OBJECT

 public:
  explicit CustomMenu(common::AppListener& appListener, GuiProcessor& guiProcessor);

  void dispatchTradingStartEvent();
  void dispatchTradingFinishEvent();

  void restoreLogging();
  void restoreConfigurationStatusBar();

  void dispatchProgressBarStartEvent(int maximum);
  void dispatchProgressBarFinishEvent();

  void incrementProgressBarValue(int value);

  void refreshTradingStartButton(bool value);
  void refreshTradingStopButton(bool value);

 private:
  void createConfiguration();
  void loadConfiguration();
  void removeCurrentConfiguration();
  void closeCurrentConfiguration();

  void createCustomStrategy();
  void loadCustomStrategy();

  void openTelegramSettings();
  void openStopLossSettings();
  void openIntervalsSettings();

  void startStatisticUpdate();
  void stopStatisticUpdate();

  void openAbout();

  void initToolBar(QMainWindow* window);
  void initTradingStatus(QMainWindow* window);

  void startLogging();
  void stopLogging();
  void clearLogging();

 private:
  QToolBar* toolBar_;

  QAction* runTradingAction_;
  QAction* stopTradingAction_;
  QAction* loggingStartAction_;
  QAction* loggingStopAction_;
  QAction* loggingClearAction_;
  QAction* statisticStartUpdateAction_;
  QAction* statisticStopUpdateAction_;

  QLabel* tradingActivityLabel_;
  QLabel* progressLabel_;
  QLabel* tradingStatusHeaderLabel_;
  QLabel* currentConfigLabel_;
  QLabel* baseCurrencyTitleLabel_;

  QWidgetAction* actionProgressBar_;
  QProgressBar* progressBar_;

  std::unique_ptr<QDialog> configurationDialog_;
  std::unique_ptr<QDialog> customStrategyDialog_;
  std::unique_ptr<QDialog> telegramDialog_;
  std::unique_ptr<QDialog> stopLossDialog_;
  std::unique_ptr<QDialog> intervalsDialog_;
  std::unique_ptr<QDialog> aboutDialog_;

  common::AppListener& appListener_;
  GuiProcessor& guiListener_;
};

}  // namespace view
}  // namespace auto_trader

#endif  // AUTO_TRADER_VIEW_CUSTOM_MENU_H
