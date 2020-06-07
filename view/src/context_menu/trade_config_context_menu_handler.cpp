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

#include "include/context_menu/trade_config_context_menu_handler.h"

#include <QtWidgets/QMenu>
#include <QtWidgets/QMessageBox>

#include "include/dialogs/create_trade_configuration_dialog.h"
#include "include/gui_nodes/gui_tree_node.h"
#include "view/include/gui_processor.h"

namespace auto_trader {
namespace view {

TradeConfigContextMenuHandler::TradeConfigContextMenuHandler(QTreeView &tradeConfigView,
                                                             common::AppListener &appListener,
                                                             GuiProcessor &guiListener)
    : tradeConfigView_(tradeConfigView), appListener_(appListener), guiListener_(guiListener) {
  const QIcon editIcon = QIcon(":/b2s_images/edit_currect_config.png");
  const QIcon closeIcon = QIcon(":/b2s_images/close_config.png");
  const QIcon removeIcon = QIcon(":/b2s_images/remove_config.png");
  const QIcon expandIcon = QIcon(":/b2s_images/expand.png");
  const QIcon collapseIcon = QIcon(":/b2s_images/collapse.png");
  const QIcon activeConfigIcon = QIcon(":/b2s_images/set_active_config.png");

  editAction_ = new QAction(editIcon, tr("&Edit"), this);
  closeAction_ = new QAction(closeIcon, tr("&Close"), this);
  removeAction_ = new QAction(removeIcon, tr("Remove"), this);
  expandAction_ = new QAction(expandIcon, tr("&Expand All"), this);
  collapseAction_ = new QAction(collapseIcon, tr("&Collapse All"), this);
  activeConfig_ = new QAction(activeConfigIcon, tr("&Set Active"), this);

  connect(editAction_, &QAction::triggered, this,
          &TradeConfigContextMenuHandler::editConfiguration);
  connect(closeAction_, &QAction::triggered, this,
          &TradeConfigContextMenuHandler::closeConfiguration);
  connect(removeAction_, &QAction::triggered, this,
          &TradeConfigContextMenuHandler::removeConfiguration);
  connect(activeConfig_, &QAction::triggered, this,
          &TradeConfigContextMenuHandler::setActiveConfig);
  connect(expandAction_, &QAction::triggered, this, &TradeConfigContextMenuHandler::expandAll);
  connect(collapseAction_, &QAction::triggered, this, &TradeConfigContextMenuHandler::collapseAll);

  connect(&tradeConfigView_, SIGNAL(customContextMenuRequested(const QPoint &)), this,
          SLOT(onCustomConfigurationContextMenu(const QPoint &)));
}

void TradeConfigContextMenuHandler::editConfiguration() {
  if (currentNode_) {
    currentNode_ = getTradeConfigNode(currentNode_);
    const std::string &configName = currentNode_->data(0).toString().toStdString();
    const auto &tradeConfigsHolder = appListener_.getTradeConfigsHolder();
    const auto &tradeConfiguration = tradeConfigsHolder.getTradeConfiguration(configName);

    if (tradeConfiguration.isActive() && tradeConfiguration.isRunning()) {
      QMessageBox::information(&guiListener_, tr("Trade configuration cannot be edited."),
                               tr("The trade configuration \'") +
                                   QString::fromStdString(configName) +
                                   tr("\' is running and cannot be edited."));

      currentNode_ = nullptr;
      return;
    }

    auto uiLock = guiListener_.acquireUILock();
    if (!uiLock.try_lock() || guiListener_.isUIUpdating()) {
      QMessageBox::information(
          &guiListener_, tr("Trade configuration cannot be edited."),
          tr("The UI is updating right now. Please wait until process is finished."));

      currentNode_ = nullptr;
      return;
    }

    auto tradeConfigurationDialog = new dialogs::CreateTradeConfigurationDialog(
        appListener_, guiListener_, dialogs::CreateTradeConfigurationDialog::DialogType::EDIT,
        &tradeConfigView_);

    tradeConfigurationDialog->setAttribute(Qt::WA_DeleteOnClose);
    tradeConfigurationDialog->setupDefaultParameters(tradeConfiguration);
    tradeConfigurationDialog->exec();
  }

  currentNode_ = nullptr;
}

void TradeConfigContextMenuHandler::closeConfiguration() {
  if (!currentNode_) {
    return;
  }

  const std::string &configName = currentNode_->data(0).toString().toStdString();
  auto &tradeConfigsHolder = appListener_.getTradeConfigsHolder();
  auto &tradeConfiguration = tradeConfigsHolder.getTradeConfiguration(configName);

  auto uiLock = guiListener_.acquireUILock();
  bool uiUpdating = (!uiLock.try_lock() || guiListener_.isUIUpdating());
  if (uiUpdating && tradeConfiguration.isActive()) {
    QMessageBox::information(
        &guiListener_, tr("Trade configuration is active and UI is updating."),
        tr("The UI is updating right now. Please wait until process is finished."));
    return;
  }

  if (tradeConfiguration.isActive()) {
    if (tradeConfiguration.isRunning()) {
      QMessageBox::information(
          &guiListener_, tr("Trading configuration cannot be closed."),
          tr("The current trading configuration is running. Please stop trading before removing."));

      return;
    }

    QMessageBox::StandardButton reply;
    if (tradeConfigsHolder.getConfigurationsCount() > 1) {
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

    if (reply != QMessageBox::Yes) {
      return;
    }
  } else {
    const std::string message = "Are you sure you want to close configuration " + configName + "?";
    QMessageBox::StandardButton reply =
        QMessageBox::question(&guiListener_, "Close configuration", tr(message.c_str()),
                              QMessageBox::Yes | QMessageBox::No);

    if (reply != QMessageBox::Yes) {
      return;
    }
  }

  bool isClosedConfigActive = tradeConfiguration.isActive();
  tradeConfigsHolder.removeTradeConfig(configName);

  if (tradeConfigsHolder.isEmpty()) {
    guiListener_.resetChart();
    guiListener_.disableChart();
    guiListener_.refreshConfigurationStatusBar();
    appListener_.refreshStockExchangeView();
  } else if (isClosedConfigActive) {
    tradeConfigsHolder.setDefaultActiveConfiguration();
    guiListener_.refreshConfigurationStatusBar();
    guiListener_.refreshChartViewStart();
    appListener_.refreshStockExchangeView();
  }

  appListener_.saveTradeConfigurationsFiles();
  guiListener_.refreshTradeConfigurationView();

  currentNode_ = nullptr;
}

void TradeConfigContextMenuHandler::removeConfiguration() {
  if (!currentNode_) {
    return;
  }

  const std::string &configName = currentNode_->data(0).toString().toStdString();
  auto &tradeConfigsHolder = appListener_.getTradeConfigsHolder();
  auto &tradeConfiguration = tradeConfigsHolder.getTradeConfiguration(configName);

  auto uiLock = guiListener_.acquireUILock();
  bool uiUpdating = (!uiLock.try_lock() || guiListener_.isUIUpdating());
  if (uiUpdating && tradeConfiguration.isActive()) {
    QMessageBox::information(
        &guiListener_, tr("Trade configuration is active and UI is updating"),
        tr("The UI is updating right now. Please wait until process is finished."));
    return;
  }

  if (tradeConfiguration.isActive()) {
    if (tradeConfiguration.isRunning()) {
      QMessageBox::information(
          &guiListener_, tr("Trading configuration cannot be closed."),
          tr("The current trading configuration is running. Please stop trading before removing."));

      return;
    }
    QMessageBox::StandardButton reply;
    if (tradeConfigsHolder.getConfigurationsCount() > 1) {
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

    if (reply != QMessageBox::Yes) {
      return;
    }
  } else {
    const std::string message = "Are you sure you want to remove configuration " + configName + "?";
    QMessageBox::StandardButton reply =
        QMessageBox::question(&guiListener_, "Remove configuration", tr(message.c_str()),
                              QMessageBox::Yes | QMessageBox::No);

    if (reply != QMessageBox::Yes) {
      return;
    }
  }

  bool isRemovedConfigActive = tradeConfiguration.isActive();
  tradeConfigsHolder.removeTradeConfig(configName);
  auto applicationDir = QApplication::applicationDirPath();
  const std::string &tradeConfigPath = applicationDir.toStdString() + std::string("/") + "config" +
                                       std::string("/") + configName + ".json";
  remove(tradeConfigPath.c_str());

  if (tradeConfigsHolder.isEmpty()) {
    guiListener_.refreshConfigurationStatusBar();
    guiListener_.resetChart();
    guiListener_.disableChart();
    appListener_.refreshStockExchangeView();
  } else if (isRemovedConfigActive) {
    tradeConfigsHolder.setDefaultActiveConfiguration();
    guiListener_.refreshConfigurationStatusBar();
    guiListener_.refreshChartViewStart();
    appListener_.refreshStockExchangeView();
  }

  appListener_.saveTradeConfigurationsFiles();
  guiListener_.refreshTradeConfigurationView();

  currentNode_ = nullptr;
}

void TradeConfigContextMenuHandler::expandAll() {
  tradeConfigView_.expandAll();
  currentNode_ = nullptr;
}

void TradeConfigContextMenuHandler::collapseAll() {
  tradeConfigView_.collapseAll();
  currentNode_ = nullptr;
}

void TradeConfigContextMenuHandler::onCustomConfigurationContextMenu(const QPoint &point) {
  QModelIndex indexPoint = tradeConfigView_.indexAt(point);
  auto guiTreeNode = static_cast<GuiTreeNode *>(indexPoint.internalPointer());
  contextMenu_ = new QMenu();

  if (guiTreeNode) {
    if (guiTreeNode->getNodeType() == GuiTreeNodeType::TRADE_CONFIG_NODE) {
      contextMenu_->addAction(editAction_);
      contextMenu_->addAction(removeAction_);
      contextMenu_->addAction(closeAction_);
      contextMenu_->addSeparator();
      contextMenu_->addAction(activeConfig_);
      activeConfig_->setEnabled(true);

      const std::string &configName = guiTreeNode->data(0).toString().toStdString();
      auto &tradeConfigsHolder = appListener_.getTradeConfigsHolder();
      auto &tradeConfiguration = tradeConfigsHolder.takeTradeConfiguration(configName);
      if (tradeConfiguration.isActive()) {
        activeConfig_->setDisabled(true);
      }
    } else if (guiTreeNode->getNodeType() == GuiTreeNodeType::CONFIG_NODE) {
      contextMenu_->addAction(editAction_);
    }

    contextMenu_->addSeparator();

    currentNode_ = guiTreeNode;
  }

  contextMenu_->addAction(expandAction_);
  contextMenu_->addAction(collapseAction_);

  contextMenu_->exec(tradeConfigView_.viewport()->mapToGlobal(point));
}

void TradeConfigContextMenuHandler::setActiveConfig() {
  if (!currentNode_) return;

  QMessageBox::StandardButton button =
      QMessageBox::question(&guiListener_, "Set Active configuration",
                            "Are you sure you want to set new active configuration?");

  if (button != QMessageBox::StandardButton::Yes) return;

  auto uiLock = guiListener_.acquireUILock();
  if (!uiLock.try_lock() || guiListener_.isUIUpdating()) {
    QMessageBox::information(
        &guiListener_, tr("Trade configuration cannot be edited."),
        tr("The UI is updating right now. Please wait until process is finished."));

    currentNode_ = nullptr;
    return;
  }

  const std::string &configName = currentNode_->data(0).toString().toStdString();
  auto &tradeConfigsHolder = appListener_.getTradeConfigsHolder();
  auto &tradeConfiguration = tradeConfigsHolder.takeTradeConfiguration(configName);
  auto &currentTradeConfiguration = tradeConfigsHolder.takeCurrentTradeConfiguration();

  if (currentTradeConfiguration.isRunning()) {
    QMessageBox::information(&guiListener_, tr("Current configuration is running."),
                             tr("The current trading configuration is running. Please stop trading "
                                "before changing active configuration."));

    return;
  }

  if (currentTradeConfiguration.getName() != tradeConfiguration.getName()) {
    currentTradeConfiguration.setActive(false);
    tradeConfiguration.setActive(true);

    appListener_.saveTradeConfigurationsFiles();
    appListener_.refreshApiKeys(tradeConfiguration);
    guiListener_.refreshTradeConfigurationView();
    guiListener_.refreshStockExchangeChartInterval();
    guiListener_.refreshStockExchangeChartMarket();
    guiListener_.refreshConfigurationStatusBar();
    guiListener_.refreshChartViewStart();
    appListener_.refreshStockExchangeView();
  }

  currentNode_ = nullptr;
}

GuiTreeNode *TradeConfigContextMenuHandler::getTradeConfigNode(GuiTreeNode *currentNode) {
  if (!currentNode) return nullptr;

  GuiTreeNode *toReturnNode = currentNode;
  while (currentNode->getNodeType() != GuiTreeNodeType::TRADE_CONFIG_NODE) {
    currentNode = currentNode->getParentNode();
  }

  toReturnNode = currentNode;

  return toReturnNode;
}

}  // namespace view
}  // namespace auto_trader