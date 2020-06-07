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

#ifndef AUTO_TRADER_TRADE_VIEW_CONFIG_CONTEXT_MENU_HANDLER_H
#define AUTO_TRADER_TRADE_VIEW_CONFIG_CONTEXT_MENU_HANDLER_H

#include <QtWidgets/QTreeView>

#include "common/listeners/app_listener.h"

namespace auto_trader {
namespace view {

namespace dialogs {
class CreateTradeConfigurationDialog;
}

class GuiProcessor;
class GuiTreeNode;

class TradeConfigContextMenuHandler : public QObject {
  Q_OBJECT
 public:
  TradeConfigContextMenuHandler(QTreeView &tradeConfigView, common::AppListener &appListener,
                                GuiProcessor &guiListener);

  void editConfiguration();
  void closeConfiguration();
  void removeConfiguration();
  void setActiveConfig();

  void expandAll();
  void collapseAll();

 private slots:
  void onCustomConfigurationContextMenu(const QPoint &point);

 private:
  GuiTreeNode *getTradeConfigNode(GuiTreeNode *currentNode);

 private:
  QTreeView &tradeConfigView_;
  common::AppListener &appListener_;
  GuiProcessor &guiListener_;

  QMenu *contextMenu_;

  QAction *editAction_;
  QAction *closeAction_;
  QAction *removeAction_;
  QAction *activeConfig_;
  QAction *expandAction_;
  QAction *collapseAction_;

  GuiTreeNode *currentNode_;
};

}  // namespace view
}  // namespace auto_trader

#endif  // AUTO_TRADER_TRADE_VIEW_CONFIG_CONTEXT_MENU_HANDLER_H
