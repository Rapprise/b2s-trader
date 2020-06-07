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

#include "include/custom_chart_widget.h"

#include "common/enumerations/charts/chart_display_type.h"
#include "common/enumerations/charts/chart_period_type.h"
#include "common/enumerations/tick_interval.h"
#include "common/listeners/app_listener.h"
#include "common/loggers/file_logger.h"
#include "common/utils.h"
#include "ui_chart_manual.h"

namespace auto_trader {
namespace view {

constexpr int ZOOM_IN_TICK_VALUE = 120;
constexpr int ZOOM_OUT_TICK_VALUE = -120;

CustomChartWidget::CustomChartWidget(common::AppListener &appListener, QWidget *parent)
    : QWidget(parent), chartManual_(new Ui::ChartManual), appListener_(appListener) {
  chartManual_->setupUi(this);
  setLayout(chartManual_->gridLayout);

  initPeriods();
  initDisplayTypes();
  initTickIntervals();

  QIcon zoomInIcon = QIcon(":/b2s_images/zoom_in.png");
  QPixmap zoomInPixmap = zoomInIcon.pixmap(QSize(24, 24));
  chartManual_->pushButton->setIcon(zoomInIcon);
  chartManual_->pushButton->setIconSize(zoomInPixmap.rect().size());
  chartManual_->pushButton->setFlat(true);
  chartManual_->pushButton->setToolTip("Zoom In");
  chartManual_->pushButton->setShortcut(QKeySequence(QKeySequence::ZoomIn));

  QIcon zoomOutIcon = QIcon(":/b2s_images/zoom_out.png");
  QPixmap zoomOutPixmap = zoomInIcon.pixmap(QSize(24, 24));
  chartManual_->pushButton_2->setIcon(zoomOutIcon);
  chartManual_->pushButton_2->setIconSize(zoomOutPixmap.rect().size());
  chartManual_->pushButton_2->setFlat(true);
  chartManual_->pushButton_2->setToolTip("Zoom Out");
  chartManual_->pushButton_2->setShortcut(QKeySequence(QKeySequence::ZoomOut));

  QIcon zoomResetIcon = QIcon(":/b2s_images/zoom_reset.png");
  QPixmap zoomResetPixmap = zoomResetIcon.pixmap(QSize(24, 24));
  chartManual_->pushButton_3->setIcon(zoomResetIcon);
  chartManual_->pushButton_3->setIconSize(zoomResetPixmap.rect().size());
  chartManual_->pushButton_3->setFlat(true);
  chartManual_->pushButton_3->setToolTip("Reset Zoom");
  chartManual_->pushButton_3->setShortcut(QKeySequence("Ctrl+R"));

  QIcon updateChartIcon = QIcon(":/b2s_images/update_chart.png");
  QPixmap updateChartPixmap = updateChartIcon.pixmap(32, 32);
  chartManual_->pushButton_4->setIcon(updateChartIcon);
  chartManual_->pushButton_4->setIconSize(updateChartPixmap.rect().size());
  chartManual_->pushButton_4->setFlat(true);
  chartManual_->pushButton_4->setToolTip("Chart Update");
  chartManual_->pushButton_4->setShortcut(QKeySequence("Ctrl+W"));

  QIcon tickIntervalIcon = QIcon(":/b2s_images/tick_interval.png");
  QPixmap tickIntervalPixmap = tickIntervalIcon.pixmap(QSize(24, 24));
  chartManual_->label_4->setPixmap(tickIntervalPixmap);
  chartManual_->label_4->setAlignment(Qt::AlignmentFlag::AlignRight);
  chartManual_->label->setAlignment(Qt::AlignmentFlag::AlignLeft | Qt::AlignVCenter);

  QIcon periodIcon = QIcon(":/b2s_images/period.png");
  QPixmap periodPixmap = periodIcon.pixmap(QSize(24, 24));
  chartManual_->label_5->setPixmap(periodPixmap);
  chartManual_->label_5->setAlignment(Qt::AlignmentFlag::AlignRight);
  chartManual_->label_2->setAlignment(Qt::AlignmentFlag::AlignLeft | Qt::AlignVCenter);

  QIcon viewIcon = QIcon(":/b2s_images/view_of_chart.png");
  QPixmap viewPixmap = viewIcon.pixmap(QSize(24, 24));
  chartManual_->label_6->setPixmap(viewPixmap);
  chartManual_->label_6->setAlignment(Qt::AlignmentFlag::AlignRight);
  chartManual_->label_3->setAlignment(Qt::AlignmentFlag::AlignLeft | Qt::AlignVCenter);

  QIcon marketIcon = QIcon(":b2s_images/market_type.png");
  QPixmap marketPixmap = marketIcon.pixmap(QSize(24, 24));
  chartManual_->label_7->setPixmap(marketPixmap);
  chartManual_->label_7->setAlignment(Qt::AlignmentFlag::AlignRight);
  chartManual_->label_8->setAlignment(Qt::AlignmentFlag::AlignLeft | Qt::AlignVCenter);

  disableChart();

  connect(chartManual_->pushButton, SIGNAL(clicked()), this, SLOT(zoomIn()));
  connect(chartManual_->pushButton_2, SIGNAL(clicked()), this, SLOT(zoomOut()));
  connect(chartManual_->pushButton_3, SIGNAL(clicked()), this, SLOT(zoomReset()));
  connect(chartManual_->pushButton_4, SIGNAL(clicked()), this, SLOT(refreshChart()));

  connectSignals();
}

CustomChartWidget::~CustomChartWidget() { delete chartManual_; }

void CustomChartWidget::initPeriods() {
  QStringList items;
  auto lastElement = (unsigned short)common::charts::ChartPeriodType::UNKNOWN;
  for (unsigned short index = 0; index < lastElement; ++index) {
    const std::string interval =
        common::charts::ChartPeriodType::toString((common::charts::ChartPeriodType::Enum)index);
    items.append(QString::fromStdString(interval));
  }

  chartManual_->comboBox_2->addItems(items);
}

void CustomChartWidget::initDisplayTypes() {
  QStringList items;
  auto lastElement = (unsigned short)common::charts::ChartDisplayType::UNKNOWN;
  for (unsigned short index = 0; index < lastElement; ++index) {
    const std::string interval =
        common::charts::ChartDisplayType::toString((common::charts::ChartDisplayType::Enum)index);
    items.append(QString::fromStdString(interval));
  }

  chartManual_->comboBox_3->addItems(items);
}

void CustomChartWidget::initTickIntervals() {
  QStringList items;
  const std::string oneMinStr = common::TickInterval::toString(common::TickInterval::ONE_MIN);
  const std::string fiveMinStr = common::TickInterval::toString(common::TickInterval::FIVE_MIN);
  const std::string thirtyMinStr = common::TickInterval::toString(common::TickInterval::THIRTY_MIN);
  const std::string oneHourStr = common::TickInterval::toString(common::TickInterval::ONE_HOUR);

  items.append(QString::fromStdString(oneMinStr));
  items.append(QString::fromStdString(fiveMinStr));
  items.append(QString::fromStdString(thirtyMinStr));
  items.append(QString::fromStdString(oneHourStr));

  chartManual_->comboBox->addItems(items);
}

void CustomChartWidget::zoomIn() { chartManual_->graphicsView->zoomChart(ZOOM_IN_TICK_VALUE); }

void CustomChartWidget::zoomOut() { chartManual_->graphicsView->zoomChart(ZOOM_OUT_TICK_VALUE); }

void CustomChartWidget::zoomReset() {
  chartManual_->graphicsView->chart()->zoomReset();
  chartManual_->graphicsView->zoomIndex(0);
  chartManual_->graphicsView->repaint();
}

void CustomChartWidget::refreshChart() { refreshChartViewStart(); }

void CustomChartWidget::changeComboBoxItem(const QString &value) { refreshChartViewStart(); }

void CustomChartWidget::refreshChartViewStart() {
  try {
    disableChart();
    auto currentBoxIndex = chartManual_->comboBox->currentText();
    std::string tickInterval = currentBoxIndex.toStdString();
    common::TickInterval::Enum interval = common::TickInterval::fromString(tickInterval);

    auto currentMarketBoxIndex = chartManual_->comboBox_4->currentText();
    std::string currencyMarket = currentMarketBoxIndex.toStdString();

    auto dashTockenPosition = currencyMarket.find('-');
    const std::string &baseCurrencyStr = currencyMarket.substr(0, dashTockenPosition);
    const std::string &tradedCurrencyStr = currencyMarket.substr(dashTockenPosition + 1);
    common::Currency::Enum baseCurrency = common::Currency::fromString(baseCurrencyStr);
    common::Currency::Enum tradedCurrency = common::Currency::fromString(tradedCurrencyStr);

    appListener_.refreshMarketHistory(baseCurrency, tradedCurrency, interval);
  } catch (std::exception &ex) {
    common::loggers::FileLogger::getLogger() << ex.what();
  }
}

void CustomChartWidget::refreshChartViewFinish(common::MarketHistoryPtr marketHistory,
                                               common::StockExchangeType stockExchangeType) {
  try {
    auto chartPeriod = chartManual_->comboBox_2->currentText();
    auto period = common::charts::ChartPeriodType::fromString(chartPeriod.toStdString());

    auto displayTypeStr = chartManual_->comboBox_3->currentText();
    auto displayType = common::charts::ChartDisplayType::fromString(displayTypeStr.toStdString());

    chartManual_->graphicsView->refreshChartView(std::move(marketHistory), stockExchangeType,
                                                 period, displayType);
    enableChart();

  } catch (std::exception &ex) {
    common::loggers::FileLogger::getLogger() << ex.what();
  }
}

void CustomChartWidget::refreshStockExchangeTickInterval() {
  auto &tradeConfigsHolder = appListener_.getTradeConfigsHolder();
  if (tradeConfigsHolder.isEmpty()) {
    return;
  }

  QStringList items;

  try {
    auto &currentConfiguration = tradeConfigsHolder.getCurrentTradeConfiguration();
    auto stockExchangeType = currentConfiguration.getStockExchangeSettings().stockExchangeType_;
    auto lastElement = (unsigned short)common::TickInterval::UNKNOWN;
    for (unsigned short index = 0; index < lastElement; ++index) {
      auto tickInterval = common::TickInterval::Enum(index);
      if (common::isStockExchangeContainsTickInterval(tickInterval, stockExchangeType)) {
        const std::string interval = common::TickInterval::toString(tickInterval);
        items.append(QString::fromStdString(interval));
      }
    }
  } catch (std::exception &exception) {
    common::loggers::FileLogger::getLogger() << exception.what();
  }

  disconnectionSignals();

  QString previousTickInterval = chartManual_->comboBox->currentText();
  chartManual_->comboBox->clear();
  chartManual_->comboBox->addItems(items);

  if (items.contains(previousTickInterval, Qt::CaseSensitive)) {
    chartManual_->comboBox->setCurrentText(previousTickInterval);
  }

  connectSignals();
}

void CustomChartWidget::refreshStockExchangeMarket() {
  auto &tradeConfigsHolder = appListener_.getTradeConfigsHolder();
  if (tradeConfigsHolder.isEmpty()) {
    return;
  }

  QStringList items;

  try {
    auto &currentConfiguration = tradeConfigsHolder.getCurrentTradeConfiguration();
    auto &coinSettings = currentConfiguration.getCoinSettings();
    size_t tradedCurrenciesSize = coinSettings.tradedCurrencies_.size();
    for (size_t index = 0; index < tradedCurrenciesSize; ++index) {
      auto tradedCurrency = coinSettings.tradedCurrencies_.at(index);
      const std::string tradedCurrencyStr = common::Currency::toString(tradedCurrency);
      const std::string market =
          common::Currency::toString(coinSettings.baseCurrency_) + "-" + tradedCurrencyStr;
      items.append(QString::fromStdString(market));
    }
  } catch (std::exception &exception) {
    common::loggers::FileLogger::getLogger() << exception.what();
  }

  disconnectionSignals();

  QString previousStockExchangeMarket = chartManual_->comboBox_4->currentText();
  chartManual_->comboBox_4->clear();
  chartManual_->comboBox_4->addItems(items);

  if (items.contains(previousStockExchangeMarket, Qt::CaseSensitive)) {
    chartManual_->comboBox_4->setCurrentText(previousStockExchangeMarket);
  }

  connectSignals();
}

void CustomChartWidget::disableChart() {
  chartManual_->graphicsView->setDisabled(true);
  chartManual_->comboBox->setDisabled(true);
  chartManual_->comboBox_2->setDisabled(true);
  chartManual_->comboBox_3->setDisabled(true);
  chartManual_->comboBox_4->setDisabled(true);
  chartManual_->pushButton->setDisabled(true);
  chartManual_->pushButton_2->setDisabled(true);
  chartManual_->pushButton_3->setDisabled(true);
  chartManual_->pushButton_4->setDisabled(true);
}

void CustomChartWidget::enableChart() {
  chartManual_->graphicsView->setEnabled(true);
  chartManual_->comboBox->setEnabled(true);
  chartManual_->comboBox_2->setEnabled(true);
  chartManual_->comboBox_3->setEnabled(true);
  chartManual_->comboBox_4->setEnabled(true);
  chartManual_->pushButton->setEnabled(true);
  chartManual_->pushButton_2->setEnabled(true);
  chartManual_->pushButton_3->setEnabled(true);
  chartManual_->pushButton_4->setEnabled(true);
}

void CustomChartWidget::resetChart() { chartManual_->graphicsView->resetChart(); }

void CustomChartWidget::connectSignals() {
  connect(chartManual_->comboBox, SIGNAL(currentIndexChanged(const QString &)), this,
          SLOT(changeComboBoxItem(const QString &)));
  connect(chartManual_->comboBox_2, SIGNAL(currentIndexChanged(const QString &)), this,
          SLOT(changeComboBoxItem(const QString &)));
  connect(chartManual_->comboBox_3, SIGNAL(currentIndexChanged(const QString &)), this,
          SLOT(changeComboBoxItem(const QString &)));
  connect(chartManual_->comboBox_4, SIGNAL(currentIndexChanged(const QString &)), this,
          SLOT(changeComboBoxItem(const QString &)));
}

void CustomChartWidget::disconnectionSignals() {
  disconnect(chartManual_->comboBox, SIGNAL(currentIndexChanged(const QString &)), this,
             SLOT(changeComboBoxItem(const QString &)));
  disconnect(chartManual_->comboBox_2, SIGNAL(currentIndexChanged(const QString &)), this,
             SLOT(changeComboBoxItem(const QString &)));
  disconnect(chartManual_->comboBox_3, SIGNAL(currentIndexChanged(const QString &)), this,
             SLOT(changeComboBoxItem(const QString &)));
  disconnect(chartManual_->comboBox_4, SIGNAL(currentIndexChanged(const QString &)), this,
             SLOT(changeComboBoxItem(const QString &)));
}

}  // namespace view
}  // namespace auto_trader