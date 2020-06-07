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

#ifndef AUTO_TRADER_VIEW_CREATE_STRATEGY_DIALOG_H
#define AUTO_TRADER_VIEW_CREATE_STRATEGY_DIALOG_H

#include <QtCore/QStringListModel>
#include <memory>

#include "common/enumerations/strategies_type.h"
#include "common/listeners/app_listener.h"
#include "common/listeners/gui_listener.h"
#include "model/include/settings/strategies_settings/custom_strategy_settings.h"
#include "model/include/settings/strategies_settings/strategy_settings_factory.h"
#include "ui_create_strategy_dialog.h"

namespace auto_trader {

namespace trader {
class AppController;
}

namespace view {
namespace dialogs {

class CreateStrategyDialog : public QDialog {
  Q_OBJECT
 public:
  enum DialogType {
    CREATE,
    EDIT,
  };

  CreateStrategyDialog(common::AppListener &appListener, common::GuiListener &guiListener,
                       DialogType dialogType, QWidget *parent = nullptr);

  void setupDefaultParameters(const model::CustomStrategySettings &settings);

 private slots:
  void selectStrategy();
  void removeStrategy();
  void editStrategy();

  void closeDialog();
  void checkOkButtonStatus(const QString &text);

 signals:
  void strategiesNumberChanged(const QString &text);

 private:
  void initStrategiesList();
  void openEditStrategySettingsDialog(common::StrategiesType type);

  void refreshStrategySettings();

 private:
  common::AppListener &appListener_;
  common::GuiListener &guiListener_;

  Ui::CreateStrategyDialog dialog_;

  QStringList *currentStrategiesList_;
  QStringList *selectedStrategiesList_;

  QStringListModel *currentStrategiesModel_;
  QStringListModel *selectedStrategiesModel_;

  std::unique_ptr<model::CustomStrategySettings> customStrategySettings_;
  std::unique_ptr<model::StrategySettingsFactory> strategySettingsFactory_;

  std::map<common::StrategiesType, std::unique_ptr<model::StrategySettings>> strategySettings_;

  std::string customStrategyName_;

  DialogType dialogType_;
};

}  // namespace dialogs
}  // namespace view
}  // namespace auto_trader

#endif  // AUTO_TRADER_VIEW_CREATE_STRATEGY_DIALOG_H
