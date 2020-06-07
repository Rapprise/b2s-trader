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

#ifndef AUTO_TRADER_VIEW_STRATEGIES_VIEW_MODEL_H
#define AUTO_TRADER_VIEW_STRATEGIES_VIEW_MODEL_H

#include "model/include/trade_configuration.h"
#include "view/include/gui_nodes/gui_tree_node.h"
#include "view/include/view_model/base_item_model.h"

namespace auto_trader {

namespace model {
struct StrategySettings;
struct BollingerBandsSettings;
}  // namespace model

namespace view {

class GuiTreeNode;

class StrategiesViewModel : public BaseItemModel, private model::StrategySettingsVisitor {
  Q_OBJECT

 public:
  QVariant data(const QModelIndex &index, int role) const override;

  explicit StrategiesViewModel(const std::vector<model::StrategySettings const *> &strategies,
                               QObject *parent = nullptr);

 private:
  void visit(const model::BollingerBandsSettings &bandsSettings) override;
  void visit(const model::BollingerBandsAdvancedSettings &bollingerBandsAdvancedSettings) override;
  void visit(const model::RsiSettings &rsiSettings) override;
  void visit(const model::EmaSettings &emaSettings) override;
  void visit(const model::SmaSettings &smaSettings) override;
  void visit(const model::MovingAveragesCrossingSettings &movingAveragesCrossingSettings) override;
  void visit(const model::StochasticOscillatorSettings &stochasticOscillatorSettings) override;
  void visit(const model::CustomStrategySettings &customStrategySettings) override;

  void initializeModel(const std::vector<model::StrategySettings const *> &strategies,
                       GuiTreeNode *parent);

 private:
  GuiTreeNode *currentParentNode;
};

}  // namespace view
}  // namespace auto_trader

#endif  // AUTO_TRADER_VIEW_STRATEGIES_VIEW_MODEL_H
