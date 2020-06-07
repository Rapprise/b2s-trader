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

#include "include/view_model/strategies_view_model.h"

#include "common/enumerations/stochastic_oscillator_type.h"
#include "model/include/settings/strategies_settings/bollinger_bands_advanced_settings.h"
#include "model/include/settings/strategies_settings/bollinger_bands_settings.h"
#include "model/include/settings/strategies_settings/custom_strategy_settings.h"
#include "model/include/settings/strategies_settings/ema_settings.h"
#include "model/include/settings/strategies_settings/ma_crossing_settings.h"
#include "model/include/settings/strategies_settings/rsi_settings.h"
#include "model/include/settings/strategies_settings/sma_settings.h"
#include "model/include/settings/strategies_settings/stochastic_oscillator_settings.h"

namespace auto_trader {
namespace view {

StrategiesViewModel::StrategiesViewModel(
    const std::vector<model::StrategySettings const *> &strategies, QObject *parent)
    : BaseItemModel(parent) {
  std::vector<QVariant> rootData{"Strategy name", "Settings"};
  rootNode_ = std::make_unique<GuiTreeNode>(rootData, 0, GuiTreeNodeType::ROOT);
  initializeModel(strategies, rootNode_.get());
}

void StrategiesViewModel::initializeModel(
    const std::vector<model::StrategySettings const *> &strategies, GuiTreeNode *parent) {
  size_t strategiesCount = strategies.size();
  for (int index = 0; index < strategiesCount; ++index) {
    const model::StrategySettings *settings = strategies[index];

    currentParentNode = parent;

    settings->accept(*this);
  }
}

QVariant StrategiesViewModel::data(const QModelIndex &index, int role) const {
  if (!index.isValid()) {
    return QVariant();
  }

  if (role == Qt::TextAlignmentRole) {
    if (index.column() == 1) {
      return Qt::AlignCenter;
    }
  }

  return BaseItemModel::data(index, role);
}

void StrategiesViewModel::visit(const model::BollingerBandsSettings &bandsSettings) {
  GuiTreeNode *parent = createGuiNode(bandsSettings.name_, "", GuiTreeNodeType::STRATEGY_NODE,
                                      currentParentNode, currentParentNode->getChildrenCounts());

  currentParentNode->addNode(parent);

  GuiTreeNode *periodNode =
      createGuiNode("Period", std::to_string(bandsSettings.period_),
                    GuiTreeNodeType::STRATEGY_INFO_NODE, parent, parent->getChildrenCounts());

  parent->addNode(periodNode);

  GuiTreeNode *tickIntervalNode =
      createGuiNode("Tick interval", common::TickInterval::toString(bandsSettings.tickInterval_),
                    GuiTreeNodeType::STRATEGY_INFO_NODE, parent, parent->getChildrenCounts());
  parent->addNode(tickIntervalNode);

  GuiTreeNode *bbInputTypeNode =
      createGuiNode("BB input type", common::convertBBInputTypeToString(bandsSettings.bbInputType_),
                    GuiTreeNodeType::STRATEGY_INFO_NODE, parent, parent->getChildrenCounts());

  parent->addNode(bbInputTypeNode);

  GuiTreeNode *deviationsNode =
      createGuiNode("Standard deviations", std::to_string(bandsSettings.standardDeviations_),
                    GuiTreeNodeType::STRATEGY_INFO_NODE, parent, parent->getChildrenCounts());

  parent->addNode(deviationsNode);

  GuiTreeNode *candlesAfterCrossing =
      createGuiNode("Crossing interval", std::to_string(bandsSettings.crossingInterval_),
                    GuiTreeNodeType::STRATEGY_INFO_NODE, parent, parent->getChildrenCounts());

  parent->addNode(candlesAfterCrossing);
}

void StrategiesViewModel::visit(const model::RsiSettings &rsiSettings) {
  GuiTreeNode *parent = createGuiNode(rsiSettings.name_, "", GuiTreeNodeType::STRATEGY_NODE,
                                      currentParentNode, currentParentNode->getChildrenCounts());

  currentParentNode->addNode(parent);

  GuiTreeNode *periodNode =
      createGuiNode("Period", std::to_string(rsiSettings.period_),
                    GuiTreeNodeType::STRATEGY_INFO_NODE, parent, parent->getChildrenCounts());

  parent->addNode(periodNode);

  GuiTreeNode *tickIntervalNode =
      createGuiNode("Tick interval", common::TickInterval::toString(rsiSettings.tickInterval_),
                    GuiTreeNodeType::STRATEGY_INFO_NODE, parent, parent->getChildrenCounts());

  parent->addNode(tickIntervalNode);

  GuiTreeNode *topLevelNode =
      createGuiNode("Top line", std::to_string(rsiSettings.topLevel_),
                    GuiTreeNodeType::STRATEGY_INFO_NODE, parent, parent->getChildrenCounts());

  parent->addNode(topLevelNode);

  GuiTreeNode *bottomLevelNode =
      createGuiNode("Bottom line", std::to_string(rsiSettings.bottomLevel_),
                    GuiTreeNodeType::STRATEGY_INFO_NODE, parent, parent->getChildrenCounts());

  parent->addNode(bottomLevelNode);

  GuiTreeNode *candlesAfterCrossing =
      createGuiNode("Crossing interval", std::to_string(rsiSettings.crossingInterval_),
                    GuiTreeNodeType::STRATEGY_INFO_NODE, parent, parent->getChildrenCounts());

  parent->addNode(candlesAfterCrossing);
}

void StrategiesViewModel::visit(const model::EmaSettings &emaSettings) {
  GuiTreeNode *parent = createGuiNode(emaSettings.name_, "", GuiTreeNodeType::STRATEGY_NODE,
                                      currentParentNode, currentParentNode->getChildrenCounts());

  currentParentNode->addNode(parent);

  GuiTreeNode *periodNode =
      createGuiNode("Period", std::to_string(emaSettings.period_),
                    GuiTreeNodeType::STRATEGY_INFO_NODE, parent, parent->getChildrenCounts());

  parent->addNode(periodNode);

  GuiTreeNode *tickIntervalNode =
      createGuiNode("Tick interval", common::TickInterval::toString(emaSettings.tickInterval_),
                    GuiTreeNodeType::STRATEGY_INFO_NODE, parent, parent->getChildrenCounts());

  parent->addNode(tickIntervalNode);

  GuiTreeNode *candlesAfterCrossing =
      createGuiNode("Crossing interval", std::to_string(emaSettings.crossingInterval_),
                    GuiTreeNodeType::STRATEGY_INFO_NODE, parent, parent->getChildrenCounts());

  parent->addNode(candlesAfterCrossing);
}

void StrategiesViewModel::visit(const model::SmaSettings &smaSettings) {
  GuiTreeNode *parent = createGuiNode(smaSettings.name_, "", GuiTreeNodeType::STRATEGY_NODE,
                                      currentParentNode, currentParentNode->getChildrenCounts());

  currentParentNode->addNode(parent);

  GuiTreeNode *periodNode =
      createGuiNode("Period", std::to_string(smaSettings.period_),
                    GuiTreeNodeType::STRATEGY_INFO_NODE, parent, parent->getChildrenCounts());

  parent->addNode(periodNode);

  GuiTreeNode *tickIntervalNode =
      createGuiNode("Tick interval", common::TickInterval::toString(smaSettings.tickInterval_),
                    GuiTreeNodeType::STRATEGY_INFO_NODE, parent, parent->getChildrenCounts());

  parent->addNode(tickIntervalNode);

  GuiTreeNode *candlesAfterCrossing =
      createGuiNode("Crossing interval", std::to_string(smaSettings.crossingInterval_),
                    GuiTreeNodeType::STRATEGY_INFO_NODE, parent, parent->getChildrenCounts());

  parent->addNode(candlesAfterCrossing);
}

void StrategiesViewModel::visit(
    const model::MovingAveragesCrossingSettings &movingAveragesCrossingSettings) {
  GuiTreeNode *parent =
      createGuiNode(movingAveragesCrossingSettings.name_, "", GuiTreeNodeType::STRATEGY_NODE,
                    currentParentNode, currentParentNode->getChildrenCounts());

  currentParentNode->addNode(parent);

  GuiTreeNode *tickIntervalNode = createGuiNode(
      "Tick interval", common::TickInterval::toString(movingAveragesCrossingSettings.tickInterval_),
      GuiTreeNodeType::STRATEGY_INFO_NODE, parent, parent->getChildrenCounts());

  parent->addNode(tickIntervalNode);

  GuiTreeNode *movingAverageTypeNode = createGuiNode(
      "Moving average type",
      common::convertMovingAverageTypeToString(movingAveragesCrossingSettings.movingAverageType_),
      GuiTreeNodeType::STRATEGY_INFO_NODE, parent, parent->getChildrenCounts());

  parent->addNode(movingAverageTypeNode);

  GuiTreeNode *smallPeriodNode =
      createGuiNode("Smaller period", std::to_string(movingAveragesCrossingSettings.smallerPeriod_),
                    GuiTreeNodeType::STRATEGY_INFO_NODE, parent, parent->getChildrenCounts());

  parent->addNode(smallPeriodNode);

  GuiTreeNode *biggerPeriod =
      createGuiNode("Bigger period", std::to_string(movingAveragesCrossingSettings.biggerPeriod_),
                    GuiTreeNodeType::STRATEGY_INFO_NODE, parent, parent->getChildrenCounts());

  parent->addNode(biggerPeriod);

  GuiTreeNode *crossingInterval = createGuiNode(
      "Crossing interval", std::to_string(movingAveragesCrossingSettings.crossingInterval_),
      GuiTreeNodeType::STRATEGY_INFO_NODE, parent, parent->getChildrenCounts());

  parent->addNode(crossingInterval);
}

void StrategiesViewModel::visit(
    const auto_trader::model::BollingerBandsAdvancedSettings &bollingerBandsAdvancedSettings) {
  GuiTreeNode *parent =
      createGuiNode(bollingerBandsAdvancedSettings.name_, "", GuiTreeNodeType::STRATEGY_NODE,
                    currentParentNode, currentParentNode->getChildrenCounts());

  currentParentNode->addNode(parent);

  GuiTreeNode *periodNode =
      createGuiNode("Period", std::to_string(bollingerBandsAdvancedSettings.period_),
                    GuiTreeNodeType::STRATEGY_INFO_NODE, parent, parent->getChildrenCounts());

  parent->addNode(periodNode);

  GuiTreeNode *tickIntervalNode = createGuiNode(
      "Tick interval", common::TickInterval::toString(bollingerBandsAdvancedSettings.tickInterval_),
      GuiTreeNodeType::STRATEGY_INFO_NODE, parent, parent->getChildrenCounts());
  parent->addNode(tickIntervalNode);

  GuiTreeNode *bbInputTypeNode =
      createGuiNode("BB input type",
                    common::convertBBInputTypeToString(bollingerBandsAdvancedSettings.bbInputType_),
                    GuiTreeNodeType::STRATEGY_INFO_NODE, parent, parent->getChildrenCounts());

  parent->addNode(bbInputTypeNode);

  GuiTreeNode *crossingIntervalNode = createGuiNode(
      "Crossing interval", std::to_string(bollingerBandsAdvancedSettings.crossingInterval_),
      GuiTreeNodeType::STRATEGY_INFO_NODE, parent, parent->getChildrenCounts());

  parent->addNode(crossingIntervalNode);

  GuiTreeNode *deviationsNode = createGuiNode(
      "Standard deviations", std::to_string(bollingerBandsAdvancedSettings.standardDeviations_),
      GuiTreeNodeType::STRATEGY_INFO_NODE, parent, parent->getChildrenCounts());

  parent->addNode(deviationsNode);

  GuiTreeNode *topLinePercentNode = createGuiNode(
      "Top line percentage", std::to_string(bollingerBandsAdvancedSettings.topLinePercentage_),
      GuiTreeNodeType::STRATEGY_INFO_NODE, parent, parent->getChildrenCounts());

  parent->addNode(topLinePercentNode);

  GuiTreeNode *bottomLinePercentage =
      createGuiNode("Bottom line percentage",
                    std::to_string(bollingerBandsAdvancedSettings.bottomLinePercentage_),
                    GuiTreeNodeType::STRATEGY_INFO_NODE, parent, parent->getChildrenCounts());

  parent->addNode(bottomLinePercentage);
}

void StrategiesViewModel::visit(
    const auto_trader::model::StochasticOscillatorSettings &stochasticOscillatorSettings) {
  GuiTreeNode *parent =
      createGuiNode(stochasticOscillatorSettings.name_, "", GuiTreeNodeType::STRATEGY_NODE,
                    currentParentNode, currentParentNode->getChildrenCounts());

  currentParentNode->addNode(parent);

  GuiTreeNode *tickIntervalNode = createGuiNode(
      "Tick interval", common::TickInterval::toString(stochasticOscillatorSettings.tickInterval_),
      GuiTreeNodeType::STRATEGY_INFO_NODE, parent, parent->getChildrenCounts());

  parent->addNode(tickIntervalNode);

  GuiTreeNode *periodNode =
      createGuiNode("Period for classic line",
                    std::to_string(stochasticOscillatorSettings.periodsForClassicLine_),
                    GuiTreeNodeType::STRATEGY_INFO_NODE, parent, parent->getChildrenCounts());

  parent->addNode(periodNode);

  GuiTreeNode *smoothSlowPeriod = createGuiNode(
      "Smooth slow period", std::to_string(stochasticOscillatorSettings.smoothSlowPeriod_),
      GuiTreeNodeType::STRATEGY_INFO_NODE, parent, parent->getChildrenCounts());

  parent->addNode(smoothSlowPeriod);

  GuiTreeNode *smoothFastPeriod = createGuiNode(
      "Smooth fast period", std::to_string(stochasticOscillatorSettings.smoothFastPeriod_),
      GuiTreeNodeType::STRATEGY_INFO_NODE, parent, parent->getChildrenCounts());

  parent->addNode(smoothFastPeriod);

  GuiTreeNode *stochasticType = createGuiNode(
      "Stochastic type",
      common::convertStochasticOscillatorToString(stochasticOscillatorSettings.stochasticType_),
      GuiTreeNodeType::STRATEGY_INFO_NODE, parent, parent->getChildrenCounts());

  parent->addNode(stochasticType);

  GuiTreeNode *crossingIntervalNode = createGuiNode(
      "Crossing interval", std::to_string(stochasticOscillatorSettings.crossingInterval_),
      GuiTreeNodeType::STRATEGY_INFO_NODE, parent, parent->getChildrenCounts());

  parent->addNode(crossingIntervalNode);
}

void StrategiesViewModel::visit(const model::CustomStrategySettings &customStrategySettings) {
  GuiTreeNode *parent =
      createGuiNode(customStrategySettings.name_,
                    common::convertStrategyTypeToString(customStrategySettings.strategiesType_),
                    GuiTreeNodeType::CUSTOM_STRATEGY_NODE, currentParentNode,
                    currentParentNode->getChildrenCounts());

  currentParentNode->addNode(parent);
  currentParentNode = parent;

  for (int index = 0; index < customStrategySettings.getStrategiesCount(); index++) {
    customStrategySettings.getStrategy(index)->accept(*this);
  }
}

}  // namespace view
}  // namespace auto_trader
