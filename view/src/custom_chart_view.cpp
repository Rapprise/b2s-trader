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

#include "include/custom_chart_view.h"

#include <common/loggers/file_logger.h>

#include <QApplication>
#include <QtCharts/QBarCategoryAxis>
#include <QtCharts/QCandlestickSet>
#include <QtCharts/QDateTimeAxis>
#include <QtCharts/QValueAxis>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <queue>

#include "common/exceptions/undefined_type_exception.h"

namespace auto_trader {
namespace view {

constexpr int MAX_BARS_COUNT = 2000;
constexpr int MAX_TICK_COUNT_PER_TIME = 9;

constexpr int MIN_TICK_RANGE_ON_SCREEN = 10;

CustomChartView::CustomChartView(QWidget *parent)
    : QtCharts::QChartView(parent),
      mainChart_(new QtCharts::QChart()),
      lineSeries_(new QtCharts::QLineSeries(this)),
      verticalSeries_(new QtCharts::QLineSeries(this)),
      bottomSeries_(new QtCharts::QLineSeries(this)),
      mountainSeries_(new QtCharts::QAreaSeries(this)),
      candleSeries_(new QtCharts::QCandlestickSeries(this)),
      currentPriceDisplayItem_(nullptr),
      currentDateDisplayItem_(nullptr),
      zoomIndex_(0, true) {
  initialize();
}

CustomChartView::~CustomChartView() {
  delete currentPriceDisplayItem_;
  delete currentDateDisplayItem_;
  delete mainChart_;
}

void CustomChartView::initialize() {
  marketDates_.resize(MAX_BARS_COUNT);
  mainChart_->createDefaultAxes();

  mainChart_->legend()->hide();
  mainChart_->setTitle("No active stock exchange chosen.");
  mainChart_->setPlotAreaBackgroundBrush(QBrush(QColor("#B7BBC1")));
  mainChart_->setPlotAreaBackgroundVisible(true);
  mainChart_->setBackgroundBrush(QBrush(QColor("#B7BBC1")));

  axisX_ = new QtCharts::QDateTimeAxis;
  axisX_->setTickCount(MAX_TICK_COUNT_PER_TIME);
  axisX_->setFormat("h:mm:ss");
  axisX_->setTitleText("Date");

  auto currentDate = common::Date::getCurrentTime();
  QDateTime todayDate, oneHourAhead;
  todayDate.setDate(QDate(currentDate.year_, currentDate.month_, currentDate.day_));
  todayDate.setTime(QTime(currentDate.hour_, currentDate.minute_, currentDate.second_));
  auto currentHour = currentDate.hour_;
  auto oneHourFromNow = currentHour == 23 ? 1 : currentHour + 1;
  oneHourAhead.setDate(QDate(currentDate.year_, currentDate.month_, currentDate.day_));
  oneHourAhead.setTime(QTime(oneHourFromNow, currentDate.minute_, currentDate.second_));

  marketDates_.push_back(todayDate);
  marketDates_.push_back(oneHourAhead);
  marketDatesPrice_[todayDate] = 0.5;
  marketDatesPrice_[oneHourAhead] = 0.5;

  lineSeries_->append(todayDate.toMSecsSinceEpoch(), 0.5);
  lineSeries_->append(oneHourAhead.toMSecsSinceEpoch(), 0.5);

  bottomSeries_->append(todayDate.toMSecsSinceEpoch(), 0);
  bottomSeries_->append(oneHourAhead.toMSecsSinceEpoch(), 0);

  axisY_ = new QtCharts::QValueAxis;
  axisY_->setLabelFormat("%.6f");
  axisY_->setTickCount(MAX_TICK_COUNT_PER_TIME);
  axisY_->setTitleText("Price");

  axisY_->setRange(0, 1);
  axisX_->setRange(todayDate, oneHourAhead);

  mainChart_->addAxis(axisX_, Qt::AlignBottom);
  mainChart_->addAxis(axisY_, Qt::AlignLeft);

  setupChart();

  setChart(mainChart_);

  currentPriceDisplayItem_ = new QGraphicsSimpleTextItem(mainChart_);
  currentPriceDisplayItem_->setPos(120, 10);
  currentPriceDisplayItem_->setText("closed price: ");

  currentDateDisplayItem_ = new QGraphicsSimpleTextItem(mainChart_);
  currentDateDisplayItem_->setPos(120, 35);
  currentDateDisplayItem_->setText("closed date");

  setMouseTracking(true);
  setDragMode(QGraphicsView::NoDrag);
  setRenderHint(QPainter::Antialiasing);
  setSizePolicy(QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Expanding);
}

void CustomChartView::mouseMoveEvent(QMouseEvent *event) {
  updateVerticalLine(event->pos());

  if (event->buttons() & Qt::LeftButton) {
    auto dPos = event->pos() - lastMousePosition_;
    auto axisXMin = axisX_->min().toMSecsSinceEpoch() + (-dPos.x());
    auto axisXMax = axisX_->max().toMSecsSinceEpoch() + (-dPos.x());

    if (marketDates_.empty()) {
      chart()->scroll(-dPos.x(), 0);
      lastMousePosition_ = event->pos();
      event->accept();
      QChartView::mouseMoveEvent(event);
      return;
    }

    auto lowerBound = std::lower_bound(marketDates_.begin(), marketDates_.end(),
                                       QDateTime::fromMSecsSinceEpoch(axisXMin));
    auto upperBound = std::upper_bound(marketDates_.begin(), marketDates_.end(),
                                       QDateTime::fromMSecsSinceEpoch(axisXMax));

    if (upperBound == marketDates_.end()) {
      if (dPos.x() < 0) {
        lastMousePosition_ = event->pos();
        event->accept();
        QChartView::mouseMoveEvent(event);
        axisX_->setMax(marketDates_.back());
        return;
      }
    }

    if (lowerBound == marketDates_.begin()) {
      if (dPos.x() > 0) {
        lastMousePosition_ = event->pos();
        event->accept();
        QChartView::mouseMoveEvent(event);
        axisX_->setMin(marketDates_.front());
        return;
      }
    }

    std::vector<QDateTime> range(lowerBound, upperBound);
    double minValue = INT32_MAX;
    double maxValue = 0;
    for (auto &date : range) {
      double currentValue = marketDatesPrice_[date];
      minValue = std::min(minValue, currentValue);
      maxValue = std::max(maxValue, currentValue);
    }

    auto deltaY = (maxValue - minValue) / MAX_TICK_COUNT_PER_TIME;
    minValue = minValue - deltaY;
    maxValue = maxValue + deltaY;
    axisY_->setRange(minValue, maxValue);

    chart()->scroll(-dPos.x(), 0);
    lastMousePosition_ = event->pos();
    event->accept();
  }

  QChartView::mouseMoveEvent(event);
}

void CustomChartView::displayMarketData(const QDateTime &dateTime, double price) {
  std::string dayText = (dateTime.date().day() < 10)
                            ? std::string("0") + std::to_string(dateTime.date().day())
                            : std::to_string(dateTime.date().day());
  std::string monthText = (dateTime.date().month() < 10)
                              ? std::string("0") + std::to_string(dateTime.date().month())
                              : std::to_string(dateTime.date().month());
  std::string hourText = (dateTime.time().hour() < 10)
                             ? std::string("0") + std::to_string(dateTime.time().hour())
                             : std::to_string(dateTime.time().hour());
  std::string minuteText = (dateTime.time().minute() < 10)
                               ? std::string("0") + std::to_string(dateTime.time().minute())
                               : std::to_string(dateTime.time().minute());
  std::string secondText = (dateTime.time().second() < 10)
                               ? std::string("0") + std::to_string(dateTime.time().second())
                               : std::to_string(dateTime.time().second());
  std::string dateText = "closed date : " + dayText + ":" + monthText + "-" + hourText + ":" +
                         minuteText + ":" + secondText;

  std::ostringstream stream;
  stream << std::fixed << std::setprecision(8) << price;
  std::string pp = stream.str();
  std::string priceStr = "closed price : " + pp;

  currentPriceDisplayItem_->setText(QString::fromStdString(priceStr));
  currentDateDisplayItem_->setText(QString::fromStdString(dateText));
}

void CustomChartView::mousePressEvent(QMouseEvent *event) {
  if (event->button() == Qt::LeftButton) {
    QApplication::setOverrideCursor(QCursor(Qt::SizeAllCursor));
    lastMousePosition_ = event->pos();
    event->accept();
  }

  QChartView::mousePressEvent(event);
}

void CustomChartView::mouseReleaseEvent(QMouseEvent *event) {
  QApplication::restoreOverrideCursor();
  event->accept();

  QChartView::mouseReleaseEvent(event);
}

void CustomChartView::zoomChart(int delta) {
  if (delta > 0) {
    auto xMin = axisX_->min().toMSecsSinceEpoch();
    auto xMax = axisX_->max().toMSecsSinceEpoch();
    auto lowerLocalBound = std::lower_bound(marketDates_.begin(), marketDates_.end(),
                                            QDateTime::fromMSecsSinceEpoch(xMin));
    auto upperLocalBound = std::upper_bound(marketDates_.begin(), marketDates_.end(),
                                            QDateTime::fromMSecsSinceEpoch(xMax));
    std::vector<QDateTime> localRange(lowerLocalBound, upperLocalBound);
    if (localRange.size() < MIN_TICK_RANGE_ON_SCREEN) return;
  } else {
    auto frontTimestampLocal = marketDates_.front().toMSecsSinceEpoch();
    auto backTimestampLocal = marketDates_.back().toMSecsSinceEpoch();
    if (axisX_->min().toMSecsSinceEpoch() <= frontTimestampLocal &&
        axisX_->max().toMSecsSinceEpoch() >= backTimestampLocal) {
      return;
    }
  }

  double factor = pow((double)2, delta / 240.0);
  chart()->zoom(factor);

  QPointF mousePos = mapFromGlobal(QCursor::pos());

  auto axisXMin = axisX_->min().toMSecsSinceEpoch();
  auto axisXMax = axisX_->max().toMSecsSinceEpoch();

  auto frontTimestamp = marketDates_.front().toMSecsSinceEpoch();
  auto backTimestamp = marketDates_.back().toMSecsSinceEpoch();

  if (axisXMin <= frontTimestamp) {
    axisX_->setMin(QDateTime::fromMSecsSinceEpoch(frontTimestamp));
  }

  if (axisXMax >= backTimestamp) {
    axisX_->setMax(QDateTime::fromMSecsSinceEpoch(backTimestamp));
  }

  auto lowerBound = std::lower_bound(marketDates_.begin(), marketDates_.end(),
                                     QDateTime::fromMSecsSinceEpoch(axisXMin));
  auto upperBound = std::upper_bound(marketDates_.begin(), marketDates_.end(),
                                     QDateTime::fromMSecsSinceEpoch(axisXMax));

  std::vector<QDateTime> range(lowerBound, upperBound);
  double minValue = INT32_MAX;
  double maxValue = 0;
  for (auto &date : range) {
    double currentValue = marketDatesPrice_[date];
    minValue = std::min(minValue, currentValue);
    maxValue = std::max(maxValue, currentValue);
  }

  auto deltaY = (maxValue - minValue) / MAX_TICK_COUNT_PER_TIME;
  minValue = minValue - deltaY;
  maxValue = maxValue + deltaY;
  axisY_->setRange(minValue, maxValue);
  lastMousePosition_ = mousePos;

  updateVerticalLine(mousePos);

  if (zoomIndex_.second) {
    zoomIndex_.first += delta;
  }
}

void CustomChartView::zoomIndex(int index) { zoomIndex_.first = index; }

void CustomChartView::wheelEvent(QWheelEvent *event) {
  int delta = event->delta();
  delta = (delta > 0) ? 120 : -120;
  zoomChart(delta);
  event->accept();
}

void CustomChartView::updateVerticalLine(QPointF mousePos) {
  verticalSeries_->clear();

  if (currentDateDisplayItem_ && currentPriceDisplayItem_) {
    auto msEpoch = mainChart_->mapToValue(mousePos).x();
    QDateTime dateTime = QDateTime::fromMSecsSinceEpoch(msEpoch);
    auto price = mainChart_->mapToValue(mousePos).y();

    if (marketDates_.empty()) {
      verticalSeries_->append(dateTime.toMSecsSinceEpoch(), axisY_->min());
      verticalSeries_->append(dateTime.toMSecsSinceEpoch(), axisY_->max());
      displayMarketData(dateTime, price);
    } else {
      auto lowerBound = std::lower_bound(marketDates_.begin(), marketDates_.end(), dateTime);
      if (lowerBound != marketDates_.end()) {
        if (lowerBound == marketDates_.begin()) {
          auto elem = *marketDates_.begin();
          verticalSeries_->append(elem.toMSecsSinceEpoch(), axisY_->min());
          verticalSeries_->append(elem.toMSecsSinceEpoch(), axisY_->max());
          displayMarketData(elem, marketDatesPrice_[elem]);
        } else {
          uint64_t previousIndex = lowerBound - marketDates_.begin();
          auto prevElement = marketDates_[--previousIndex];
          if (dateTime.toMSecsSinceEpoch() - prevElement.toMSecsSinceEpoch() <
              lowerBound->toMSecsSinceEpoch() - dateTime.toMSecsSinceEpoch()) {
            verticalSeries_->append(prevElement.toMSecsSinceEpoch(), axisY_->min());
            verticalSeries_->append(prevElement.toMSecsSinceEpoch(), axisY_->max());
            displayMarketData(prevElement, marketDatesPrice_[prevElement]);
          } else {
            verticalSeries_->append(lowerBound->toMSecsSinceEpoch(), axisY_->min());
            verticalSeries_->append(lowerBound->toMSecsSinceEpoch(), axisY_->max());
            auto &lowerBoundDate = *lowerBound;
            displayMarketData(lowerBoundDate, marketDatesPrice_[lowerBoundDate]);
          }
        }
      } else {
        auto lastDate = marketDates_.back();
        displayMarketData(lastDate, marketDatesPrice_[lastDate]);
        verticalSeries_->append(lastDate.toMSecsSinceEpoch(), axisY_->min());
        verticalSeries_->append(lastDate.toMSecsSinceEpoch(), axisY_->max());
      }
    }
  }
}

void CustomChartView::refreshChartView(common::MarketHistoryPtr marketHistory,
                                       common::StockExchangeType stockExchangeType,
                                       common::charts::ChartPeriodType::Enum periodType,
                                       common::charts::ChartDisplayType::Enum displayType) {
  switch (displayType) {
    case common::charts::ChartDisplayType::MOUNTAINS:
      refreshMountainView(std::move(marketHistory), periodType);
      break;
    case common::charts::ChartDisplayType::CANDLES:
      refreshCandleView(std::move(marketHistory), periodType);
      break;
    case common::charts::ChartDisplayType::UNKNOWN:
      throw common::exceptions::UndefinedTypeException("Chart display type");
  }

  const std::string &chartTitle = common::convertStockExchangeTypeToString(stockExchangeType);
  mainChart_->setTitle(QString::fromStdString(chartTitle));
}

void CustomChartView::refreshMountainView(common::MarketHistoryPtr marketHistory,
                                          common::charts::ChartPeriodType::Enum periodType) {
  if (marketHistory->marketData_.empty()) return;

  lineSeries_->clear();
  bottomSeries_->clear();
  candleSeries_->clear();
  marketDatesPrice_.clear();
  marketDates_.clear();

  mainChart_->removeAllSeries();

  initializeSeries();

  size_t candlesCount = marketHistory->marketData_.size();

  double minPrice = marketHistory->marketData_.back().closePrice_;
  double maxPrice = marketHistory->marketData_.back().closePrice_;
  auto firstDate = marketHistory->marketData_.back().date_;
  QDateTime firstDateTime;
  firstDateTime.setDate(QDate(firstDate.year_, firstDate.month_, firstDate.day_));
  firstDateTime.setTime(QTime(firstDate.hour_, firstDate.minute_, firstDate.second_));
  QDateTime lastDateTime = firstDateTime;

  int maxTicksCount = 0;
  common::Date lastDateTick = getLastTick(periodType);
  size_t lastCandleIndex = 0;

  if (candlesCount > MAX_BARS_COUNT) {
    lastCandleIndex = candlesCount - MAX_BARS_COUNT;
  }

  for (size_t index = candlesCount - 1; index >= 0; --index) {
    auto marketData = marketHistory->marketData_[index];
    if (marketData.date_ <= lastDateTick) break;

    if (index == lastCandleIndex) break;

    QDateTime todayDate;
    todayDate.setDate(
        QDate(marketData.date_.year_, marketData.date_.month_, marketData.date_.day_));
    todayDate.setTime(
        QTime(marketData.date_.hour_, marketData.date_.minute_, marketData.date_.second_));

    if (maxTicksCount < MAX_TICK_COUNT_PER_TIME) {
      minPrice = std::min(minPrice, marketData.closePrice_);
      maxPrice = std::max(maxPrice, marketData.closePrice_);
      firstDateTime = std::min(todayDate, firstDateTime);
      lastDateTime = std::max(todayDate, lastDateTime);
      maxTicksCount++;
    }

    lineSeries_->append(todayDate.toMSecsSinceEpoch(), marketData.closePrice_);
    bottomSeries_->append(todayDate.toMSecsSinceEpoch(), 0);
    marketDatesPrice_.insert(std::make_pair<>(todayDate, marketData.closePrice_));
    marketDates_.push_front(todayDate);
  }

  setupChart();

  auto deltaY = (maxPrice - minPrice) / MAX_TICK_COUNT_PER_TIME;
  minPrice = minPrice - deltaY;
  maxPrice = maxPrice + deltaY;
  axisY_->setRange(minPrice, maxPrice);
  axisX_->setRange(firstDateTime, lastDateTime);

  zoomIndex_.second = false;
  zoomChart(zoomIndex_.first);
  zoomIndex_.second = true;
}

void CustomChartView::refreshCandleView(common::MarketHistoryPtr marketHistory,
                                        common::charts::ChartPeriodType::Enum periodType) {
  if (marketHistory->marketData_.empty()) return;

  lineSeries_->clear();
  bottomSeries_->clear();
  candleSeries_->clear();
  marketDatesPrice_.clear();
  marketDates_.clear();

  mainChart_->removeAllSeries();

  initializeSeries();

  double minPrice = marketHistory->marketData_.back().closePrice_;
  double maxPrice = marketHistory->marketData_.back().closePrice_;
  auto firstDate = marketHistory->marketData_.back().date_;
  QDateTime firstDateTime;
  firstDateTime.setDate(QDate(firstDate.year_, firstDate.month_, firstDate.day_));
  firstDateTime.setTime(QTime(firstDate.hour_, firstDate.minute_, firstDate.second_));
  QDateTime lastDateTime = firstDateTime;

  size_t candlesCount = marketHistory->marketData_.size();
  common::Date latestDate = getLastTick(periodType);
  size_t lastCandleIndex = 0;

  if (candlesCount > MAX_BARS_COUNT) {
    lastCandleIndex = candlesCount - MAX_BARS_COUNT;
  }

  int maxTicksCount = 0;
  for (size_t index = candlesCount - 1; index >= 0; --index) {
    auto marketData = marketHistory->marketData_[index];
    if (marketData.date_ <= latestDate) break;

    if (index == lastCandleIndex) break;

    QDateTime todayDate;
    todayDate.setDate(
        QDate(marketData.date_.year_, marketData.date_.month_, marketData.date_.day_));
    todayDate.setTime(
        QTime(marketData.date_.hour_, marketData.date_.minute_, marketData.date_.second_));

    if (maxTicksCount < MAX_TICK_COUNT_PER_TIME) {
      minPrice = std::min(minPrice, marketData.closePrice_);
      maxPrice = std::max(maxPrice, marketData.closePrice_);
      firstDateTime = std::min(todayDate, firstDateTime);
      lastDateTime = std::max(todayDate, lastDateTime);
      maxTicksCount++;
    }
    const qreal timestamp = todayDate.toMSecsSinceEpoch();
    const qreal open = marketData.openPrice_;
    const qreal high = marketData.highPrice_;
    const qreal low = marketData.lowPrice_;
    const qreal close = marketData.closePrice_;

    auto candlestickSet = new QtCharts::QCandlestickSet(timestamp);
    candlestickSet->setOpen(open);
    candlestickSet->setHigh(high);
    candlestickSet->setLow(low);
    candlestickSet->setClose(close);
    candleSeries_->append(candlestickSet);

    marketDatesPrice_.insert(std::make_pair<>(todayDate, marketData.closePrice_));
    marketDates_.push_back(todayDate);
  }

  std::reverse(marketDates_.begin(), marketDates_.end());
  setupChart();

  auto deltaY = (maxPrice - minPrice) / MAX_TICK_COUNT_PER_TIME;
  minPrice = minPrice - deltaY;
  maxPrice = maxPrice + deltaY;
  axisY_->setRange(minPrice, maxPrice);
  axisX_->setRange(firstDateTime, lastDateTime);

  zoomIndex_.second = false;
  zoomChart(zoomIndex_.first);
  zoomIndex_.second = true;
}

common::Date CustomChartView::getLastTick(common::charts::ChartPeriodType::Enum period) {
  switch (period) {
    case common::charts::ChartPeriodType::ONE_DAY: {
      common::Date latestDate = common::Date::getCurrentTime();
      latestDate = common::Date::getDayBefore(latestDate, 1);
      return latestDate;
    }
    case common::charts::ChartPeriodType::ONE_WEEK: {
      common::Date latestDate = common::Date::getCurrentTime();
      latestDate = common::Date::getDayBefore(latestDate, 7);
      return latestDate;
    }
    case common::charts::ChartPeriodType::ONE_MONTH: {
      common::Date latestDate = common::Date::getCurrentTime();
      if (latestDate.month_ <= 1) {
        int difference = 1 - latestDate.month_;
        latestDate.year_ = latestDate.year_ - 1;
        latestDate.month_ = 12 - difference;
      } else {
        latestDate.month_ = latestDate.month_ - 1;
      }
      return latestDate;
    }
    case common::charts::ChartPeriodType::THREE_MONTH: {
      common::Date latestDate = common::Date::getCurrentTime();
      if (latestDate.month_ <= 3) {
        int difference = 3 - latestDate.month_;
        latestDate.year_ = latestDate.year_ - 1;
        latestDate.month_ = 12 - difference;
      } else {
        latestDate.month_ = latestDate.month_ - 3;
      }
      return latestDate;
    }
    case common::charts::ChartPeriodType::SIX_MONTH: {
      common::Date latestDate = common::Date::getCurrentTime();
      if (latestDate.month_ <= 6) {
        int difference = 6 - latestDate.month_;
        latestDate.year_ = latestDate.year_ - 1;
        latestDate.month_ = 12 - difference;
      } else {
        latestDate.month_ = latestDate.month_ - 6;
      }
      return latestDate;
    }
    case common::charts::ChartPeriodType::ONE_YEAR: {
      common::Date latestDate = common::Date::getCurrentTime();
      latestDate.year_ = latestDate.year_ - 1;
      return latestDate;
    }
    case common::charts::ChartPeriodType::ALL: {
      common::Date latestDate{0, 0, 0, 0, 0, 0};
      return latestDate;
    }
    default:
      throw common::exceptions::UndefinedTypeException("Chart Period Type");
  }
}

void CustomChartView::resetChart() {
  lineSeries_->clear();
  bottomSeries_->clear();
  candleSeries_->clear();
  marketDatesPrice_.clear();
  marketDates_.clear();

  auto currentDate = common::Date::getCurrentTime();
  QDateTime todayDate, oneHourAhead;
  todayDate.setDate(QDate(currentDate.year_, currentDate.month_, currentDate.day_));
  todayDate.setTime(QTime(currentDate.hour_, currentDate.minute_, currentDate.second_));
  auto currentHour = currentDate.hour_;
  auto oneHourFromNow = currentHour == 23 ? 1 : currentHour + 1;
  oneHourAhead.setDate(QDate(currentDate.year_, currentDate.month_, currentDate.day_));
  oneHourAhead.setTime(QTime(oneHourFromNow, currentDate.minute_, currentDate.second_));

  marketDates_.push_back(todayDate);
  marketDates_.push_back(oneHourAhead);
  marketDatesPrice_[todayDate] = 0.5;
  marketDatesPrice_[oneHourAhead] = 0.5;

  lineSeries_->append(todayDate.toMSecsSinceEpoch(), 0.5);
  lineSeries_->append(oneHourAhead.toMSecsSinceEpoch(), 0.5);

  bottomSeries_->append(todayDate.toMSecsSinceEpoch(), 0);
  bottomSeries_->append(oneHourAhead.toMSecsSinceEpoch(), 0);

  mountainSeries_->setUpperSeries(lineSeries_);
  mountainSeries_->setLowerSeries(bottomSeries_);
}

void CustomChartView::initializeSeries() {
  lineSeries_ = new QtCharts::QLineSeries(this);
  verticalSeries_ = new QtCharts::QLineSeries(this);
  bottomSeries_ = new QtCharts::QLineSeries(this);
  mountainSeries_ = new QtCharts::QAreaSeries(this);
  candleSeries_ = new QtCharts::QCandlestickSeries(this);
}

void CustomChartView::setupChart() {
  candleSeries_->setIncreasingColor(QColor("#49974E"));
  candleSeries_->setDecreasingColor(QColor("#C62B2B"));

  mountainSeries_->setUpperSeries(lineSeries_);
  mountainSeries_->setLowerSeries(bottomSeries_);

  mainChart_->addSeries(mountainSeries_);
  mainChart_->addSeries(candleSeries_);
  mainChart_->addSeries(verticalSeries_);

  mountainSeries_->attachAxis(axisX_);
  mountainSeries_->attachAxis(axisY_);

  candleSeries_->attachAxis(axisX_);
  candleSeries_->attachAxis(axisY_);

  verticalSeries_->attachAxis(axisX_);
  verticalSeries_->attachAxis(axisY_);
}

}  // namespace view
}  // namespace auto_trader