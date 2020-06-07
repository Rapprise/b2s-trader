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

#ifndef AUTO_TRADER_CREATE_TRADE_CONFIGURATION_DIALOG_H
#define AUTO_TRADER_CREATE_TRADE_CONFIGURATION_DIALOG_H

#include <QtCore/QStringListModel>
#include <QtWidgets/QDialog>

#include "common/listeners/app_listener.h"
#include "common/listeners/gui_listener.h"
#include "ui_create_trade_configuration_dialog.h"

namespace auto_trader {

namespace trader {
class AppController;
}

namespace view {
namespace dialogs {

class CreateTradeConfigurationDialog : public QDialog {
  Q_OBJECT
 public:
  enum DialogType {
    CREATE,
    EDIT,
  };

  CreateTradeConfigurationDialog(common::AppListener &appListener, common::GuiListener &listener,
                                 DialogType type, QWidget *parent = nullptr);

  ~CreateTradeConfigurationDialog();

  void setupDefaultParameters(const model::TradeConfiguration &config);

 private:
  void initStrategies();
  void initStockExchangeSettings();
  void refreshTradeConfigurations(std::unique_ptr<model::TradeConfiguration> configuration);

  void checkOkButtonState() const;

  std::vector<common::Currency::Enum> getBaseStockExchangeCurrencies(
      common::StockExchangeType type);
  std::vector<common::Currency::Enum> getTradedStockExchangeCurrencies(
      common::StockExchangeType type);

  static std::string doubleToString(double value);

 private slots:
  void addCoinToTrading();
  void removeCoinFromTrading();

  void editTextChanged(const QString &text);
  void stockExchangedChanged(const QString &text);
  void tradedCoinChanged(const QString &text);

  void closeDialog();

 signals:
  void coinNumberChanged(const QString &text);

 private:
  common::AppListener &appListener_;
  common::GuiListener &guiListener_;

  Ui::CreateTradeConfigDialog uiDialog_;

  QStringList *selectedCoinsList_;
  QStringListModel *selectedCoinsTreeModel_;

  std::string currentConfigName_;
  bool isActive_;

  DialogType dialogType_;
};

}  // namespace dialogs
}  // namespace view
}  // namespace auto_trader
#endif  // AUTO_TRADER_CREATE_TRADE_CONFIGURATION_DIALOG_H
