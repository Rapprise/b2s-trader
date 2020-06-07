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

#include "include/context_menu/strategy_context_menu_handler.h"

#include <QtWidgets/QMenu>
#include <QtWidgets/QMessageBox>

#include "include/dialogs/bb_settings_dialog.h"
#include "include/dialogs/create_strategy_dialog.h"
#include "include/dialogs/ema_settings_dialog.h"
#include "include/dialogs/moving_average_crossing_settings_dialog.h"
#include "include/dialogs/rsi_settings_dialog.h"
#include "include/dialogs/sma_settings_dialog.h"
#include "model/include/settings/strategies_settings/bollinger_bands_settings.h"
#include "model/include/settings/strategies_settings/custom_strategy_settings.h"
#include "model/include/settings/strategies_settings/ema_settings.h"
#include "model/include/settings/strategies_settings/ma_crossing_settings.h"
#include "model/include/settings/strategies_settings/rsi_settings.h"
#include "model/include/settings/strategies_settings/sma_settings.h"
#include "view/include/gui_processor.h"

namespace auto_trader {
namespace view {

StrategyContextMenuHandler::StrategyContextMenuHandler(QTreeView &strategiesView,
                                                       common::AppListener &appListener,
                                                       GuiProcessor &guiListener)
    : strategiesView_(strategiesView), appListener_(appListener), guiListener_(guiListener) {
  const QIcon editIcon = QIcon(":/b2s_images/strategy_edit.png");
  const QIcon closeIcon = QIcon(":/b2s_images/strategy_close.png");
  const QIcon removeIcon = QIcon(":/b2s_images/strategy_remove.png");
  const QIcon expandIcon = QIcon(":/b2s_images/expand.png");
  const QIcon collapseIcon = QIcon(":/b2s_images/collapse.png");

  closeAction_ = new QAction(closeIcon, tr("&Close"), this);
  editAction_ = new QAction(editIcon, tr("&Edit"), this);
  removeAction_ = new QAction(removeIcon, tr("Remove"), this);
  expandAction_ = new QAction(expandIcon, tr("&Expand All"), this);
  collapseAction_ = new QAction(collapseIcon, tr("&Collapse All"), this);

  connect(editAction_, &QAction::triggered, this, &StrategyContextMenuHandler::editStrategy);
  connect(closeAction_, &QAction::triggered, this, &StrategyContextMenuHandler::closeStrategy);
  connect(removeAction_, &QAction::triggered, this, &StrategyContextMenuHandler::removeStrategy);
  connect(expandAction_, &QAction::triggered, this, &StrategyContextMenuHandler::expandAll);
  connect(collapseAction_, &QAction::triggered, this, &StrategyContextMenuHandler::collapseAll);

  connect(&strategiesView_, SIGNAL(customContextMenuRequested(const QPoint &)), this,
          SLOT(onCustomStrategiesContextMenu(const QPoint &)));
}

void StrategyContextMenuHandler::editStrategy() {
  if (currentNode_) {
    if (currentNode_->getNodeType() == GuiTreeNodeType::STRATEGY_NODE) {
      editSimpleStrategy();
    } else if (currentNode_->getNodeType() == GuiTreeNodeType::CUSTOM_STRATEGY_NODE) {
      editCustomStrategy();
    }
  }
}

void StrategyContextMenuHandler::closeStrategy() {
  assert(currentNode_);

  if (currentNode_->getNodeType() == GuiTreeNodeType::CUSTOM_STRATEGY_NODE) {
    if (isStrategyUsingInRunningConfiguration(currentNode_, "Strategy cannot be removed")) {
      return;
    }
    QMessageBox::StandardButton button = QMessageBox::question(
        &guiListener_, "Close custom strategy", "Are you sure you want to close custom strategy?");

    if (button != QMessageBox::StandardButton::Yes) return;

    const std::string &customStrategyName = currentNode_->data(0).toString().toStdString();
    auto &strategyHolder = appListener_.getStrategySettingsHolder();
    auto &tradeConfigsHolder = appListener_.getTradeConfigsHolder();
    bool isStrategyUsing = false;
    tradeConfigsHolder.forEachTradeConfiguration([&](model::TradeConfiguration &configuration) {
      if (isStrategyUsing) return;

      if (configuration.getStrategyName() == customStrategyName) {
        QMessageBox::information(&guiListener_, tr("Custom strategy cannot be closed"),
                                 tr("The strategy \'") +
                                     QString::fromStdString(customStrategyName) +
                                     "\' is using in trading configuration \'" +
                                     QString::fromStdString(configuration.getName()) + "\'");

        isStrategyUsing = true;
      }
    });

    if (!isStrategyUsing) {
      strategyHolder.removeCustomStrategy(customStrategyName);
      appListener_.saveStrategiesSettingsFiles();
      guiListener_.refreshStrategiesView();
    }
  }
  currentNode_ = nullptr;
}

void StrategyContextMenuHandler::editSimpleStrategy() {
  if (!currentNode_) {
    return;
  }

  auto parentNode = currentNode_->getParentNode();
  if (!parentNode) {
    return;
  }

  if (isStrategyUsingInRunningConfiguration(parentNode, "Strategy cannot be edit")) {
    return;
  }

  const std::string &customStrategyName = parentNode->data(0).toString().toStdString();
  auto &strategyHolder = appListener_.getStrategySettingsHolder();
  auto &customStrategy = strategyHolder.takeCustomStrategy(customStrategyName);

  const std::string &strategyName = currentNode_->data(0).toString().toStdString();
  common::StrategiesType strategyType = common::convertStrategyTypeFromString(strategyName);

  model::StrategySettings *currentSettings = nullptr;
  for (int index = 0; index < customStrategy.strategies_.size(); ++index) {
    if (customStrategy.strategies_[index]->name_ == strategyName) {
      currentSettings = customStrategy.strategies_[index].get();
    }
  }
  switch (strategyType) {
    case common::StrategiesType::BOLLINGER_BANDS: {
      auto bbSettings = dynamic_cast<model::BollingerBandsSettings *>(currentSettings);
      auto dialog =
          new dialogs::BBSettingsDialog(*bbSettings, appListener_, guiListener_,
                                        dialogs::BBSettingsDialog::EDIT, &strategiesView_);
      dialog->setAttribute(Qt::WA_DeleteOnClose);
      dialog->show();
    } break;
    case common::StrategiesType::SMA: {
      auto smaSettings = dynamic_cast<model::SmaSettings *>(currentSettings);
      auto dialog =
          new dialogs::SmaSettingsDialog(*smaSettings, appListener_, guiListener_,
                                         dialogs::SmaSettingsDialog::EDIT, &strategiesView_);
      dialog->setAttribute(Qt::WA_DeleteOnClose);
      dialog->show();
    } break;
    case common::StrategiesType::EMA: {
      auto emaSettings = dynamic_cast<model::EmaSettings *>(currentSettings);
      auto dialog =
          new dialogs::EmaSettingsDialog(*emaSettings, appListener_, guiListener_,
                                         dialogs::EmaSettingsDialog::EDIT, &strategiesView_);
      dialog->setAttribute(Qt::WA_DeleteOnClose);
      dialog->show();
    } break;
    case common::StrategiesType::RSI: {
      auto rsiSettings = dynamic_cast<model::RsiSettings *>(currentSettings);
      auto dialog =
          new dialogs::RsiSettingsDialog(*rsiSettings, appListener_, guiListener_,
                                         dialogs::RsiSettingsDialog::EDIT, &strategiesView_);
      dialog->setAttribute(Qt::WA_DeleteOnClose);
      dialog->show();
    } break;
    case common::StrategiesType::MA_CROSSING: {
      auto maCrossings = dynamic_cast<model::MovingAveragesCrossingSettings *>(currentSettings);
      auto dialog = new dialogs::MovingAverageCrossingSettingsDialog(
          *maCrossings, appListener_, guiListener_,
          dialogs::MovingAverageCrossingSettingsDialog::EDIT, &strategiesView_);
      dialog->setAttribute(Qt::WA_DeleteOnClose);
      dialog->show();
    }
    default:
      break;
  }
}

void StrategyContextMenuHandler::editCustomStrategy() {
  assert(currentNode_);

  const std::string &customStrategyName = currentNode_->data(0).toString().toStdString();
  auto &strategyHolder = appListener_.getStrategySettingsHolder();
  const auto &customStrategy = strategyHolder.getCustomStrategy(customStrategyName);

  if (isStrategyUsingInRunningConfiguration(currentNode_, "Strategy cannot be edit")) {
    return;
  }

  auto customStrategyDialog = new dialogs::CreateStrategyDialog(
      appListener_, guiListener_, dialogs::CreateStrategyDialog::DialogType::EDIT,
      &strategiesView_);

  customStrategyDialog->setAttribute(Qt::WA_DeleteOnClose);
  customStrategyDialog->setupDefaultParameters(customStrategy);
  customStrategyDialog->show();
}

void StrategyContextMenuHandler::removeStrategy() {
  assert(currentNode_);

  if (currentNode_->getNodeType() == GuiTreeNodeType::STRATEGY_NODE) {
    removeSimpleStrategy();
  } else if (currentNode_->getNodeType() == GuiTreeNodeType::CUSTOM_STRATEGY_NODE) {
    removeCustomStrategy();
  }
  currentNode_ = nullptr;
}

void StrategyContextMenuHandler::removeSimpleStrategy() {
  const std::string &strategyName = currentNode_->data(0).toString().toStdString();
  auto parentNode = currentNode_->getParentNode();
  if (parentNode) {
    if (isStrategyUsingInRunningConfiguration(parentNode, "Strategy cannot be removed")) {
      return;
    }

    QMessageBox::StandardButton button = QMessageBox::question(
        &guiListener_, "Remove strategy", "Are you sure you want to remove this indicator?");

    if (button != QMessageBox::StandardButton::Yes) return;

    const std::string &customStrategyName = parentNode->data(0).toString().toStdString();
    auto &strategyHolder = appListener_.getStrategySettingsHolder();
    auto &customStrategy = strategyHolder.takeCustomStrategy(customStrategyName);
    if (customStrategy.strategies_.size() == 1) {
      QMessageBox::information(&guiListener_, tr("Strategy cannot be removed"),
                               tr("The strategy ") + QString::fromStdString(customStrategyName) +
                                   "has only one indicator and cannot be removed.");

      return;
    }

    std::remove_if(customStrategy.strategies_.begin(), customStrategy.strategies_.end(),
                   [&](std::unique_ptr<model::StrategySettings> &settings) {
                     return strategyName == settings->name_;
                   });

    appListener_.saveStrategiesSettingsFiles();
    guiListener_.refreshStrategiesView();
  }
}

void StrategyContextMenuHandler::removeCustomStrategy() {
  if (currentNode_) {
    if (isStrategyUsingInRunningConfiguration(currentNode_, "Strategy cannot be removed")) {
      return;
    }
    QMessageBox::StandardButton button =
        QMessageBox::question(&guiListener_, "Remove custom strategy",
                              "Are you sure you want to remove custom strategy?");

    if (button != QMessageBox::StandardButton::Yes) return;

    const std::string &customStrategyName = currentNode_->data(0).toString().toStdString();
    auto &strategyHolder = appListener_.getStrategySettingsHolder();
    auto &tradeConfigsHolder = appListener_.getTradeConfigsHolder();
    bool isStrategyUsing = false;
    tradeConfigsHolder.forEachTradeConfiguration([&](model::TradeConfiguration &configuration) {
      if (isStrategyUsing) return;

      if (configuration.getStrategyName() == customStrategyName) {
        QMessageBox::information(&guiListener_, tr("Custom strategy cannot be removed"),
                                 tr("The strategy \'") +
                                     QString::fromStdString(customStrategyName) +
                                     "\' is using in trading configuration \'" +
                                     QString::fromStdString(configuration.getName()) + "\'");

        isStrategyUsing = true;
      }
    });

    if (!isStrategyUsing) {
      strategyHolder.removeCustomStrategy(customStrategyName);
      appListener_.saveStrategiesSettingsFiles();
      guiListener_.refreshStrategiesView();

      auto applicationDir = QApplication::applicationDirPath();
      const std::string &customStrategyPath = applicationDir.toStdString() + std::string("/") +
                                              "config/strategies" + std::string("/") +
                                              customStrategyName + ".json";

      remove(customStrategyPath.c_str());
    }
  }
}

void StrategyContextMenuHandler::expandAll() {
  strategiesView_.expandAll();
  currentNode_ = nullptr;
}

void StrategyContextMenuHandler::collapseAll() {
  strategiesView_.collapseAll();
  currentNode_ = nullptr;
}

void StrategyContextMenuHandler::onCustomStrategiesContextMenu(const QPoint &point) {
  QModelIndex indexPoint = strategiesView_.indexAt(point);
  auto guiTreeNode = static_cast<GuiTreeNode *>(indexPoint.internalPointer());
  contextMenu_ = new QMenu();

  if (guiTreeNode) {
    if (guiTreeNode->getNodeType() == GuiTreeNodeType::CUSTOM_STRATEGY_NODE) {
      contextMenu_->addAction(editAction_);
      contextMenu_->addAction(removeAction_);
      contextMenu_->addAction(closeAction_);
      contextMenu_->addSeparator();
    }

    currentNode_ = guiTreeNode;
  }

  contextMenu_->addAction(expandAction_);
  contextMenu_->addAction(collapseAction_);

  contextMenu_->exec(strategiesView_.viewport()->mapToGlobal(point));
}

bool StrategyContextMenuHandler::isStrategyUsingInRunningConfiguration(
    GuiTreeNode *node, const std::string &title) const {
  if (!node) {
    return false;
  }

  const std::string &customStrategyName = node->data(0).toString().toStdString();

  bool isStrategyUsing = false;
  appListener_.getTradeConfigsHolder().forEachTradeConfiguration(
      [&](model::TradeConfiguration &configuration) {
        if (isStrategyUsing) return;

        if (configuration.getStrategyName() == customStrategyName && configuration.isRunning()) {
          QMessageBox::information(&guiListener_, tr(title.c_str()),
                                   tr("The strategy \'") +
                                       QString::fromStdString(customStrategyName) +
                                       "\' is using in running trading configuration \'" +
                                       QString::fromStdString(configuration.getName()) + "\'");

          isStrategyUsing = true;
        }
      });

  return isStrategyUsing;
}

}  // namespace view
}  // namespace auto_trader