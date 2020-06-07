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

#ifndef AUTO_TRADER_CUSTOM_CHART_VIEW_H
#define AUTO_TRADER_CUSTOM_CHART_VIEW_H

#include <QtCharts/QAreaSeries>
#include <QtCharts/QCandlestickSeries>
#include <QtCharts/QChartView>
#include <QtCharts/QDateTimeAxis>
#include <QtCharts/QLineSeries>
#include <QtCharts/QValueAxis>
#include <QtCore/QDateTime>
#include <deque>

#include "common/enumerations/charts/chart_display_type.h"
#include "common/enumerations/charts/chart_period_type.h"
#include "common/enumerations/stock_exchange_type.h"
#include "common/market_history.h"

namespace auto_trader {
namespace view {

class CustomChartView : public QtCharts::QChartView {
  Q_OBJECT

 public:
  explicit CustomChartView(QWidget *parent = 0);
  ~CustomChartView() final;

  void refreshChartView(common::MarketHistoryPtr marketHistory,
                        common::StockExchangeType stockExchangeType,
                        common::charts::ChartPeriodType::Enum periodType,
                        common::charts::ChartDisplayType::Enum displayType);

  void resetChart();
  void zoomChart(int delta);
  void zoomIndex(int index);

 protected:
  void mouseMoveEvent(QMouseEvent *event) override;
  void mousePressEvent(QMouseEvent *event) override;
  void mouseReleaseEvent(QMouseEvent *event) override;
  void wheelEvent(QWheelEvent *event) override;

 private:
  void initialize();
  void initializeSeries();
  void setupChart();

  void refreshMountainView(common::MarketHistoryPtr marketHistory,
                           common::charts::ChartPeriodType::Enum periodType);

  void refreshCandleView(common::MarketHistoryPtr marketHistory,
                         common::charts::ChartPeriodType::Enum periodType);

  common::Date getLastTick(common::charts::ChartPeriodType::Enum);

  void displayMarketData(const QDateTime &dateTime, double price);
  void updateVerticalLine(QPointF mousePos);

 private:
  QtCharts::QChart *mainChart_;
  QtCharts::QLineSeries *lineSeries_;
  QtCharts::QLineSeries *verticalSeries_;
  QtCharts::QLineSeries *bottomSeries_;
  QtCharts::QAreaSeries *mountainSeries_;
  QtCharts::QCandlestickSeries *candleSeries_;
  QtCharts::QDateTimeAxis *axisX_;
  QtCharts::QValueAxis *axisY_;
  QGraphicsSimpleTextItem *currentPriceDisplayItem_;
  QGraphicsSimpleTextItem *currentDateDisplayItem_;

  QPointF lastMousePosition_;

  std::deque<QDateTime> marketDates_;
  std::map<QDateTime, double> marketDatesPrice_;

  std::pair<int, bool> zoomIndex_;
};

}  // namespace view
}  // namespace auto_trader

#endif  // AUTO_TRADER_CUSTOM_CHART_VIEW_H
