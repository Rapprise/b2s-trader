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

#include "include/view_model/trading_view_model.h"

#include <sstream>

#include "common/market_order.h"

namespace auto_trader {
namespace view {

TradingViewModel::TradingViewModel(
    const std::vector<model::TradeConfiguration const *> &configurations, QObject *parent)
    : BaseItemModel(parent) {
  std::vector<QVariant> rootData{"Name", "Description"};
  rootNode_ = std::make_unique<GuiTreeNode>(rootData, 0, GuiTreeNodeType::ROOT);
  initializeModel(configurations, rootNode_.get());
}

void TradingViewModel::initializeModel(
    std::vector<model::TradeConfiguration const *> configurations, GuiTreeNode *parent) {
  size_t configurationsCount = configurations.size();
  for (int index = 0; index < configurationsCount; ++index) {
    const model::TradeConfiguration &configuration = *configurations[index];
    GuiTreeNode *configNode =
        createGuiNode(configuration.getName(), "", GuiTreeNodeType::TRADE_CONFIG_NODE, parent, 0);
    parent->addNode(configNode);

    auto buySettings = configuration.getBuySettings();
    GuiTreeNode *buySettingsNode =
        createGuiNode("Buy Settings", "", GuiTreeNodeType::CONFIG_NODE, configNode, 0);
    configNode->addNode(buySettingsNode);

    buySettingsNode->addNode(createGuiNode(
        "Funded amount", common::MarketOrder::convertCoinToString(buySettings.maxCoinAmount_),
        GuiTreeNodeType::CONFIG_NODE, buySettingsNode, 0));
    buySettingsNode->addNode(createGuiNode("Max opened orders",
                                           std::to_string(buySettings.maxOpenOrders_),
                                           GuiTreeNodeType::CONFIG_NODE, buySettingsNode, 1));
    buySettingsNode->addNode(createGuiNode("Max opened order time (min)",
                                           std::to_string(buySettings.maxOpenTime_),
                                           GuiTreeNodeType::CONFIG_NODE, buySettingsNode, 2));
    buySettingsNode->addNode(createGuiNode(
        "Min order price", common::MarketOrder::convertCoinToString(buySettings.minOrderPrice_),
        GuiTreeNodeType::CONFIG_NODE, buySettingsNode, 3));
    buySettingsNode->addNode(createGuiNode("Max opened positions for market",
                                           std::to_string(buySettings.openPositionAmountPerCoins_),
                                           GuiTreeNodeType::CONFIG_NODE, buySettingsNode, 4));
    buySettingsNode->addNode(createGuiNode("Order price (% of funded amount)",
                                           doubleToString(buySettings.percentageBuyAmount_),
                                           GuiTreeNodeType::CONFIG_NODE, buySettingsNode, 5));

    auto sellSettings = configuration.getSellSettings();
    GuiTreeNode *sellSettingsNode =
        createGuiNode("Sell settings", "", GuiTreeNodeType::CONFIG_NODE, configNode, 1);
    configNode->addNode(sellSettingsNode);

    sellSettingsNode->addNode(createGuiNode("Profit (%)",
                                            doubleToString(sellSettings.profitPercentage_),
                                            GuiTreeNodeType::CONFIG_NODE, sellSettingsNode, 0));
    sellSettingsNode->addNode(createGuiNode("Max opened order time",
                                            std::to_string(sellSettings.openOrderTime_),
                                            GuiTreeNodeType::CONFIG_NODE, sellSettingsNode, 1));

    auto coinSettings = configuration.getCoinSettings();
    GuiTreeNode *coinSettingsNode =
        createGuiNode("Coin settings", "", GuiTreeNodeType::CONFIG_NODE, configNode, 2);
    configNode->addNode(coinSettingsNode);

    coinSettingsNode->addNode(createGuiNode("Base currency",
                                            common::Currency::toString(coinSettings.baseCurrency_),
                                            GuiTreeNodeType::CONFIG_NODE, coinSettingsNode, 0));

    auto tradingCurrencies = coinSettings.tradedCurrencies_;
    for (int index = 0; index < tradingCurrencies.size(); ++index) {
      sellSettingsNode->addNode(
          createGuiNode("Trading currencies", common::Currency::toString(tradingCurrencies[index]),
                        GuiTreeNodeType::CONFIG_NODE, coinSettingsNode, index + 1));
    }

    auto stockExchangeSettings = configuration.getStockExchangeSettings();
    GuiTreeNode *stockExchangeSettingsNode =
        createGuiNode("Stock exchange settings", "", GuiTreeNodeType::CONFIG_NODE, configNode, 3);
    configNode->addNode(stockExchangeSettingsNode);

    stockExchangeSettingsNode->addNode(
        createGuiNode("Stock exchange name",
                      convertStockExchangeTypeToString(stockExchangeSettings.stockExchangeType_),
                      GuiTreeNodeType::CONFIG_NODE, stockExchangeSettingsNode, 0));

    stockExchangeSettingsNode->addNode(createGuiNode("Api key", stockExchangeSettings.apiKey_,
                                                     GuiTreeNodeType::CONFIG_NODE,
                                                     stockExchangeSettingsNode, 1));
    stockExchangeSettingsNode->addNode(createGuiNode("Secret key", stockExchangeSettings.secretKey_,
                                                     GuiTreeNodeType::CONFIG_NODE,
                                                     stockExchangeSettingsNode, 2));

    GuiTreeNode *strategiesSettingsNode =
        createGuiNode("Strategies", "", GuiTreeNodeType::CONFIG_NODE, configNode, 4);
    configNode->addNode(strategiesSettingsNode);

    strategiesSettingsNode->addNode(createGuiNode("Strategy name", configuration.getStrategyName(),
                                                  GuiTreeNodeType::CONFIG_NODE,
                                                  strategiesSettingsNode, 0));
  }
}

QVariant TradingViewModel::data(const QModelIndex &index, int role) const {
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

std::string TradingViewModel::doubleToString(double value) {
  std::ostringstream stream;
  stream << value;
  const std::string &maxCoinAmountStr = stream.str();
  return maxCoinAmountStr;
}

}  // namespace view
}  // namespace auto_trader