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

#include "include/dialogs/create_trade_configuration_dialog.h"

#include "common/binance_currency.h"
#include "common/bittrex_currency.h"
#include "common/currency.h"
#include "common/enumerations/stock_exchange_type.h"
#include "common/huobi_currency.h"
#include "common/kraken_currency.h"
#include "common/poloniex_currency.h"
#include "include/validators/float_validator.h"
#include "include/validators/integer_validator.h"

namespace auto_trader {
namespace view {
namespace dialogs {

CreateTradeConfigurationDialog::CreateTradeConfigurationDialog(common::AppListener &appListener,
                                                               common::GuiListener &guiListener,
                                                               DialogType type, QWidget *parent)
    : QDialog(parent),
      appListener_(appListener),
      guiListener_(guiListener),
      dialogType_(type),
      isActive_(false) {
  uiDialog_.setupUi(this);

#ifdef WIN32
  setMinimumSize(750, 725);
#elif __APPLE__
  setMinimumSize(750, 870);
#elif __unix__
  setMinimumSize(750, 800);
#endif

  QValidator *doubleValidator = new validators::FloatValidator(0, INT_MAX, 8, this);
  QLocale locale(QLocale::English);
  doubleValidator->setLocale(locale);

  QValidator *intValidator = new validators::IntegerValidator(1, INT_MAX, this);

  uiDialog_.lineEdit->setValidator(intValidator);
  uiDialog_.lineEdit_2->setValidator(intValidator);

  uiDialog_.lineEdit_3->setValidator(doubleValidator);

  uiDialog_.lineEdit_5->setValidator(intValidator);

  QValidator *percentageBuyAmountValidator = new validators::FloatValidator(0, 100, 2, this);
  percentageBuyAmountValidator->setLocale(locale);
  uiDialog_.lineEdit_6->setValidator(percentageBuyAmountValidator);
  uiDialog_.lineEdit_7->setValidator(percentageBuyAmountValidator);

  uiDialog_.lineEdit_4->setValidator(doubleValidator);

  uiDialog_.lineEdit_8->setValidator(intValidator);

  selectedCoinsList_ = new QStringList();
  selectedCoinsTreeModel_ = new QStringListModel(*selectedCoinsList_, NULL);

  uiDialog_.listView->setModel(selectedCoinsTreeModel_);

  sizeHint();
  initStrategies();
  initStockExchangeSettings();
  stockExchangedChanged(QString());
  checkOkButtonState();

  QIcon baseConfigIcon = QIcon(":/b2s_images/cogwheel.png");
  QPixmap iconCurrencyPixmap = baseConfigIcon.pixmap(QSize(16, 16));
  uiDialog_.label_21->setPixmap(iconCurrencyPixmap);
  uiDialog_.label_22->setPixmap(iconCurrencyPixmap);
  uiDialog_.label_23->setPixmap(iconCurrencyPixmap);
  uiDialog_.label->setPixmap(iconCurrencyPixmap);

  QIcon addCurrencyIcon = QIcon(":/b2s_images/add.png");
  QPixmap addCurrencyPixmap = addCurrencyIcon.pixmap(QSize(24, 24));
  uiDialog_.pushButton->setIcon(addCurrencyIcon);
  uiDialog_.pushButton->setIconSize(QSize(24, 24));

  QIcon removeCurrencyIcon = QIcon(":/b2s_images/remove.png");
  QPixmap removeCurrencyPixmap = addCurrencyIcon.pixmap(QSize(24, 24));
  uiDialog_.pushButton_2->setIcon(removeCurrencyIcon);
  uiDialog_.pushButton_2->setIconSize(QSize(24, 24));

  connect(uiDialog_.pushButton, SIGNAL(clicked()), this, SLOT(addCoinToTrading()));
  connect(uiDialog_.pushButton_2, SIGNAL(clicked()), this, SLOT(removeCoinFromTrading()));
  connect(uiDialog_.buttonBox, SIGNAL(accepted()), this, SLOT(closeDialog()));
  connect(uiDialog_.buttonBox, SIGNAL(rejected()), this, SLOT(reject()));

  connect(uiDialog_.lineEdit_10, SIGNAL(textChanged(const QString &)), this,
          SLOT(editTextChanged(const QString &)));
  connect(uiDialog_.lineEdit, SIGNAL(textChanged(const QString &)), this,
          SLOT(editTextChanged(const QString &)));
  connect(uiDialog_.lineEdit_2, SIGNAL(textChanged(const QString &)), this,
          SLOT(editTextChanged(const QString &)));
  connect(uiDialog_.lineEdit_3, SIGNAL(textChanged(const QString &)), this,
          SLOT(editTextChanged(const QString &)));
  connect(uiDialog_.lineEdit_5, SIGNAL(textChanged(const QString &)), this,
          SLOT(editTextChanged(const QString &)));
  connect(uiDialog_.lineEdit_6, SIGNAL(textChanged(const QString &)), this,
          SLOT(editTextChanged(const QString &)));
  connect(uiDialog_.lineEdit_4, SIGNAL(textChanged(const QString &)), this,
          SLOT(editTextChanged(const QString &)));
  connect(uiDialog_.lineEdit_7, SIGNAL(textChanged(const QString &)), this,
          SLOT(editTextChanged(const QString &)));
  connect(uiDialog_.lineEdit_8, SIGNAL(textChanged(const QString &)), this,
          SLOT(editTextChanged(const QString &)));
  connect(uiDialog_.lineEdit_9, SIGNAL(textChanged(const QString &)), this,
          SLOT(editTextChanged(const QString &)));
  connect(uiDialog_.lineEdit_11, SIGNAL(textChanged(const QString &)), this,
          SLOT(editTextChanged(const QString &)));

  connect(uiDialog_.comboBox, SIGNAL(currentIndexChanged(const QString &)), this,
          SLOT(tradedCoinChanged(const QString &)));
  connect(uiDialog_.comboBox_3, SIGNAL(currentIndexChanged(const QString &)), this,
          SLOT(editTextChanged(const QString &)));
  connect(uiDialog_.comboBox_4, SIGNAL(currentIndexChanged(const QString &)), this,
          SLOT(stockExchangedChanged(const QString &)));

  connect(this, SIGNAL(coinNumberChanged(const QString &)), this,
          SLOT(editTextChanged(const QString &)));
}

CreateTradeConfigurationDialog::~CreateTradeConfigurationDialog() {
  delete selectedCoinsList_;
  delete selectedCoinsTreeModel_;
}

void CreateTradeConfigurationDialog::setupDefaultParameters(
    const model::TradeConfiguration &config) {
  currentConfigName_ = config.getName();
  isActive_ = config.isActive();
  uiDialog_.lineEdit_10->setText(QString::fromStdString(currentConfigName_));
  uiDialog_.textEdit->insertPlainText(QString::fromStdString(config.getDescription()));

  uiDialog_.comboBox_3->setCurrentText(QString::fromStdString(config.getStrategyName()));
  const std::string &maxOrderTime = std::to_string(config.getBuySettings().maxOpenTime_);

  uiDialog_.lineEdit->setText(QString::fromStdString(maxOrderTime));
  const std::string &maxOpenOrders = std::to_string(config.getBuySettings().maxOpenOrders_);

  uiDialog_.lineEdit_2->setText(QString::fromStdString(maxOpenOrders));
  const std::string &maxCoinAmount =
      common::MarketOrder::convertCoinToString(config.getBuySettings().maxCoinAmount_);

  uiDialog_.lineEdit_3->setText(QString::fromStdString(maxCoinAmount));
  const std::string &openPositionAmountPerCoin =
      std::to_string(config.getBuySettings().openPositionAmountPerCoins_);

  uiDialog_.lineEdit_5->setText(QString::fromStdString(openPositionAmountPerCoin));
  const std::string &percentageBuyPerCoin =
      doubleToString(config.getBuySettings().percentageBuyAmount_);

  uiDialog_.lineEdit_6->setText(QString::fromStdString(percentageBuyPerCoin));
  const std::string &minOrderPrice =
      common::MarketOrder::convertCoinToString(config.getBuySettings().minOrderPrice_);

  uiDialog_.lineEdit_4->setText(QString::fromStdString(minOrderPrice));
  uiDialog_.checkBox->setChecked(config.getBuySettings().openOrderWhenAnyIndicatorIsTriggered_);

  const std::string &profitPercentage = doubleToString(config.getSellSettings().profitPercentage_);
  uiDialog_.lineEdit_7->setText(QString::fromStdString(profitPercentage));

  const std::string &openOrderTime = std::to_string(config.getSellSettings().openOrderTime_);
  uiDialog_.lineEdit_8->setText(QString::fromStdString(openOrderTime));

  const std::string &apiKey = config.getStockExchangeSettings().apiKey_;
  uiDialog_.lineEdit_9->setText(QString::fromStdString(apiKey));

  const std::string &secretKey = config.getStockExchangeSettings().secretKey_;
  uiDialog_.lineEdit_11->setText(QString::fromStdString(secretKey));

  const std::string &stockExchangeType = common::convertStockExchangeTypeToString(
      config.getStockExchangeSettings().stockExchangeType_);
  uiDialog_.comboBox_4->setCurrentText(QString::fromStdString(stockExchangeType));

  const std::string &baseCurrency =
      common::Currency::toString(config.getCoinSettings().baseCurrency_);
  uiDialog_.comboBox->setCurrentText(QString::fromStdString(baseCurrency));

  for (int index = 0; index < config.getCoinSettings().tradedCurrencies_.size(); ++index) {
    common::Currency::Enum tradedCurrency = config.getCoinSettings().tradedCurrencies_[index];
    std::string tradedCurrencyStr = common::Currency::toString(tradedCurrency);
    selectedCoinsList_->append(QString::fromStdString(tradedCurrencyStr));
  }

  selectedCoinsTreeModel_->setStringList(*selectedCoinsList_);

  checkOkButtonState();
}

void CreateTradeConfigurationDialog::initStrategies() {
  QStringList items;
  auto &strategiesHolder = appListener_.getStrategySettingsHolder();
  strategiesHolder.forEachStrategy([&](model::CustomStrategySettings &setting) {
    items.append(QString::fromStdString(setting.name_));
  });

  uiDialog_.comboBox_3->addItems(items);
}

void CreateTradeConfigurationDialog::initStockExchangeSettings() {
  QStringList items;
  auto lastElement = (unsigned short)common::StockExchangeType::UNKNOWN;
  for (short index = 0; index < lastElement; ++index) {
    const std::string stockExchange =
        common::convertStockExchangeTypeToString((common::StockExchangeType)index);
    items.append(QString::fromStdString(stockExchange));
  }

  uiDialog_.comboBox_4->addItems(items);
}

void CreateTradeConfigurationDialog::addCoinToTrading() {
  auto data = uiDialog_.comboBox_2->currentText();
  if (!selectedCoinsList_->contains(data)) {
    selectedCoinsList_->append(data);
    selectedCoinsTreeModel_->setStringList(*selectedCoinsList_);

    emit coinNumberChanged(QString());
  }
}

void CreateTradeConfigurationDialog::removeCoinFromTrading() {
  QModelIndex index = uiDialog_.listView->currentIndex();
  QString itemText = index.data(Qt::DisplayRole).toString();

  selectedCoinsList_->removeAll(itemText);
  selectedCoinsTreeModel_->setStringList(*selectedCoinsList_);

  emit coinNumberChanged(QString());
}

void CreateTradeConfigurationDialog::closeDialog() {
  const std::string strategyName = uiDialog_.comboBox_3->currentText().toStdString();
  const std::string description = uiDialog_.textEdit->toPlainText().toStdString();
  auto configuration = std::make_unique<model::TradeConfiguration>();
  configuration->setActive(isActive_);
  configuration->setStrategyName(strategyName);
  configuration->setDescription(description);
  const std::string configurationName = uiDialog_.lineEdit_10->text().toStdString();
  configuration->setName(configurationName);
  auto &buySettings = configuration->takeBuySettings();
  const std::string maxOpenOrderTime = uiDialog_.lineEdit->text().toStdString();
  buySettings.maxOpenTime_ = std::stoi(maxOpenOrderTime);
  const std::string maxOpenOrders = uiDialog_.lineEdit_2->text().toStdString();
  buySettings.maxOpenOrders_ = std::stoi(maxOpenOrders);
  const std::string maxCoinAmount = uiDialog_.lineEdit_3->text().toStdString();
  buySettings.maxCoinAmount_ = std::stod(maxCoinAmount);
  const std::string positionAmountForEachCoin = uiDialog_.lineEdit_5->text().toStdString();
  buySettings.openPositionAmountPerCoins_ = std::stoi(positionAmountForEachCoin);
  const std::string percentageBuyPerCoin = uiDialog_.lineEdit_6->text().toStdString();
  buySettings.percentageBuyAmount_ = std::stod(percentageBuyPerCoin);
  const std::string minOrderPrice = uiDialog_.lineEdit_4->text().toStdString();
  buySettings.minOrderPrice_ = std::stod(minOrderPrice);
  buySettings.openOrderWhenAnyIndicatorIsTriggered_ = uiDialog_.checkBox->isChecked();

  auto &sellSettings = configuration->takeSellSettings();
  const std::string profitPercentage = uiDialog_.lineEdit_7->text().toStdString();
  sellSettings.profitPercentage_ = std::stod(profitPercentage);

  const std::string openOrderTime_ = uiDialog_.lineEdit_8->text().toStdString();
  sellSettings.openOrderTime_ = std::stoi(openOrderTime_);

  auto &coinSettings = configuration->takeCoinSettings();
  const std::string baseCurrency = uiDialog_.comboBox->currentText().toStdString();
  coinSettings.baseCurrency_ = common::Currency::fromString(baseCurrency);

  int rowCount = uiDialog_.listView->model()->rowCount();
  for (int index = 0; index < rowCount; ++index) {
    auto modelIndex = uiDialog_.listView->model()->index(index, 0);
    auto tradedCurrencyStr = selectedCoinsTreeModel_->data(modelIndex).toString();
    coinSettings.tradedCurrencies_.push_back(
        common::Currency::fromString(tradedCurrencyStr.toStdString()));
  }

  auto &stockExchangeSettings = configuration->takeStockExchangeSettings();
  const std::string apiKey = uiDialog_.lineEdit_9->text().toStdString();
  stockExchangeSettings.apiKey_ = apiKey;
  const std::string secretKey = uiDialog_.lineEdit_11->text().toStdString();
  stockExchangeSettings.secretKey_ = secretKey;

  const std::string stockExchangeType = uiDialog_.comboBox_4->currentText().toStdString();
  stockExchangeSettings.stockExchangeType_ =
      common::convertStockExchangeTypeFromString(stockExchangeType);

  refreshTradeConfigurations(std::move(configuration));

  accept();
}

void CreateTradeConfigurationDialog::refreshTradeConfigurations(
    std::unique_ptr<model::TradeConfiguration> configuration) {
  switch (dialogType_) {
    case DialogType::CREATE:
      guiListener_.createTradeConfiguration(std::move(configuration));
      break;
    case DialogType::EDIT:
      guiListener_.editTradeConfiguration(std::move(configuration), currentConfigName_);
    default:
      break;
  }
}

void CreateTradeConfigurationDialog::checkOkButtonState() const {
  bool isStrategyEmpty = appListener_.getStrategySettingsHolder().isEmpty();
  bool isConfigEmpty = uiDialog_.lineEdit_10->text().isEmpty();

  bool isOpenOrderTimeValid = !uiDialog_.lineEdit->text().isEmpty();
  bool isMaxOpenOrdersValid = !uiDialog_.lineEdit_2->text().isEmpty();
  bool isMaxCoinAmountValid = !uiDialog_.lineEdit_3->text().isEmpty();
  ;
  bool isPositionAmountPerCoinValid = !uiDialog_.lineEdit_5->text().isEmpty();
  bool isPercentageBuyAmountValid = !uiDialog_.lineEdit_6->text().isEmpty();
  bool isMinOrderPriceValid = !uiDialog_.lineEdit_4->text().isEmpty();
  bool isProfitPercentageValid = !uiDialog_.lineEdit_7->text().isEmpty();
  bool isOpenSellOrderTimeValid = !uiDialog_.lineEdit_8->text().isEmpty();

  bool apiKeyEmpty = uiDialog_.lineEdit_9->text().isEmpty();
  bool secretKeyEmpty = uiDialog_.lineEdit_11->text().isEmpty();
  bool tradedCurrenciesEmpty = selectedCoinsList_->isEmpty();

  bool isButtonOkDisabled =
      isConfigEmpty || isStrategyEmpty || !isOpenOrderTimeValid || !isMaxOpenOrdersValid ||
      !isMaxCoinAmountValid || !isPositionAmountPerCoinValid || !isPercentageBuyAmountValid ||
      !isMinOrderPriceValid || !isProfitPercentageValid || !isOpenSellOrderTimeValid ||
      apiKeyEmpty || secretKeyEmpty || tradedCurrenciesEmpty;

  if (isButtonOkDisabled) {
    uiDialog_.buttonBox->button(QDialogButtonBox::StandardButton::Ok)
        ->setDisabled(isButtonOkDisabled);
    return;
  }

  const std::string maxCoinAmount = uiDialog_.lineEdit_3->text().toStdString();
  const std::string minOrderPrice = uiDialog_.lineEdit_4->text().toStdString();
  const std::string orderPrice = uiDialog_.lineEdit_6->text().toStdString();
  const std::string profit = uiDialog_.lineEdit_7->text().toStdString();

  if (std::stod(maxCoinAmount) == 0) {
    uiDialog_.buttonBox->button(QDialogButtonBox::StandardButton::Ok)->setDisabled(true);
    return;
  }

  if (std::stod(orderPrice) == 0) {
    uiDialog_.buttonBox->button(QDialogButtonBox::StandardButton::Ok)->setDisabled(true);
    return;
  }

  if (std::stod(profit) == 0) {
    uiDialog_.buttonBox->button(QDialogButtonBox::StandardButton::Ok)->setDisabled(true);
    return;
  }

  if (std::stod(minOrderPrice) == 0) {
    uiDialog_.buttonBox->button(QDialogButtonBox::StandardButton::Ok)->setDisabled(true);
    return;
  }

  uiDialog_.buttonBox->button(QDialogButtonBox::StandardButton::Ok)->setDisabled(false);
}

void CreateTradeConfigurationDialog::editTextChanged(const QString &text) { checkOkButtonState(); }

void CreateTradeConfigurationDialog::stockExchangedChanged(const QString &text) {
  disconnect(uiDialog_.comboBox, SIGNAL(currentIndexChanged(const QString &)), this,
             SLOT(tradedCoinChanged(const QString &)));

  selectedCoinsList_->clear();
  selectedCoinsTreeModel_->setStringList(*selectedCoinsList_);

  const std::string stockExchangeTypeStr = uiDialog_.comboBox_4->currentText().toStdString();
  auto stockExchangeType = common::convertStockExchangeTypeFromString(stockExchangeTypeStr);

  QStringList baseItems;
  auto baseCurrencies = getBaseStockExchangeCurrencies(stockExchangeType);
  for (auto currency : baseCurrencies) {
    const std::string currencyStr = common::Currency::toString(currency);
    baseItems.append(QString::fromStdString(currencyStr));
  }
  uiDialog_.comboBox->clear();
  uiDialog_.comboBox->addItems(baseItems);

  QStringList tradedItems;
  auto tradedCurrencies = getTradedStockExchangeCurrencies(stockExchangeType);
  for (auto tradedCurrency : tradedCurrencies) {
    const std::string currencyStr = common::Currency::toString(tradedCurrency);
    tradedItems.append(QString::fromStdString(currencyStr));
  }

  uiDialog_.comboBox_2->clear();
  uiDialog_.comboBox_2->addItems(tradedItems);

  checkOkButtonState();

  connect(uiDialog_.comboBox, SIGNAL(currentIndexChanged(const QString &)), this,
          SLOT(tradedCoinChanged(const QString &)));
}

void CreateTradeConfigurationDialog::tradedCoinChanged(const QString &text) {
  QStringList tradedItems;
  const std::string stockExchangeTypeStr = uiDialog_.comboBox_4->currentText().toStdString();
  auto stockExchangeType = common::convertStockExchangeTypeFromString(stockExchangeTypeStr);
  auto tradedCurrencies = getTradedStockExchangeCurrencies(stockExchangeType);
  for (auto tradedCurrency : tradedCurrencies) {
    const std::string currencyStr = common::Currency::toString(tradedCurrency);
    tradedItems.append(QString::fromStdString(currencyStr));
  }

  uiDialog_.comboBox_2->clear();
  uiDialog_.comboBox_2->addItems(tradedItems);

  selectedCoinsList_->clear();
  selectedCoinsTreeModel_->setStringList(*selectedCoinsList_);

  checkOkButtonState();
}

std::vector<common::Currency::Enum> CreateTradeConfigurationDialog::getBaseStockExchangeCurrencies(
    common::StockExchangeType type) {
  switch (type) {
    case common::StockExchangeType::Bittrex: {
      common::BittrexCurrency bittrexCurrency;
      return bittrexCurrency.getBaseCurrencies();
    }
    case common::StockExchangeType::Binance: {
      common::BinanceCurrency binanceCurrency;
      return binanceCurrency.getBaseCurrencies();
    }
    case common::StockExchangeType::Kraken: {
      common::KrakenCurrency krakenCurrency;
      return krakenCurrency.getBaseCurrencies();
    }
    case common::StockExchangeType::Poloniex: {
      common::PoloniexCurrency poloniexCurrency;
      return poloniexCurrency.getBaseCurrencies();
    }
    case common::StockExchangeType::Huobi: {
      common::HuobiCurrency huobiCurrency;
      return huobiCurrency.getBaseCurrencies();
    }

    default:
      return std::vector<common::Currency::Enum>();
  }
}

std::vector<common::Currency::Enum>
CreateTradeConfigurationDialog::getTradedStockExchangeCurrencies(common::StockExchangeType type) {
  const std::string baseCurrencyStr = uiDialog_.comboBox->currentText().toStdString();
  common::Currency::Enum baseCurrency = common::Currency::fromString(baseCurrencyStr);

  switch (type) {
    case common::StockExchangeType::Bittrex: {
      common::BittrexCurrency bittrexCurrency;
      return bittrexCurrency.getTradedCurrencies(baseCurrency);
    }
    case common::StockExchangeType::Binance: {
      common::BinanceCurrency binanceCurrency;
      return binanceCurrency.getTradedCurrencies(baseCurrency);
    }
    case common::StockExchangeType::Kraken: {
      common::KrakenCurrency krakenCurrency;
      return krakenCurrency.getTradedCurrencies(baseCurrency);
    }
    case common::StockExchangeType::Poloniex: {
      common::PoloniexCurrency poloniexCurrency;
      return poloniexCurrency.getTradedCurrencies(baseCurrency);
    }
    case common::StockExchangeType::Huobi: {
      common::HuobiCurrency huobiCurrency;
      return huobiCurrency.getTradedCurrencies(baseCurrency);
    }

    default:
      return std::vector<common::Currency::Enum>();
  }
}

std::string CreateTradeConfigurationDialog::doubleToString(double value) {
  std::ostringstream stream;
  stream << value;
  const std::string &maxCoinAmountStr = stream.str();
  return maxCoinAmountStr;
}

}  // namespace dialogs
}  // namespace view
}  // namespace auto_trader
