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

#include "include/view_model/trading_account_orders_view_model.h"

namespace auto_trader {
namespace view {

constexpr int ORDERS_COLUMN_COUNT = 7;

TradingAccountOrdersViewModel::TradingAccountOrdersViewModel(
    const std::vector<common::MarketOrder> &order, QObject *parent)
    : QAbstractTableModel(parent), orders_(order) {}

int TradingAccountOrdersViewModel::rowCount(const QModelIndex &parent) const {
  Q_UNUSED(parent);
  return orders_.size();
}

int TradingAccountOrdersViewModel::columnCount(const QModelIndex &parent) const {
  Q_UNUSED(parent);
  return ORDERS_COLUMN_COUNT;
}

QVariant TradingAccountOrdersViewModel::data(const QModelIndex &index, int role) const {
  if (!index.isValid()) return QVariant();

  if (index.row() >= orders_.size() || index.row() < 0) return QVariant();

  if (role == Qt::DisplayRole) {
    const auto &marketOrder = orders_.at(index.row());

    if (index.column() == 0) {
      return common::Currency::toString(marketOrder.fromCurrency_).c_str();
    } else if (index.column() == 1) {
      return common::Currency::toString(marketOrder.toCurrency_).c_str();
    } else if (index.column() == 2) {
      return common::convertOrderTypeToString(marketOrder.orderType_).c_str();
    } else if (index.column() == 3) {
      return common::convertStockExchangeTypeToString(marketOrder.stockExchangeType_).c_str();
    } else if (index.column() == 4) {
      std::ostringstream stream;
      stream << std::fixed << std::setprecision(8);
      stream << marketOrder.quantity_;
      std::string value = stream.str();
      return value.c_str();
    } else if (index.column() == 5) {
      std::ostringstream stream;
      stream << std::fixed << std::setprecision(8);
      stream << marketOrder.price_;
      std::string value = stream.str();
      return value.c_str();
    } else if (index.column() == 6) {
      return common::Date::toString(marketOrder.opened_).c_str();
    }

    return QVariant();
  }

  return QVariant();
}

bool TradingAccountOrdersViewModel::setData(const QModelIndex &index, const QVariant &value,
                                            int role) {
  if (index.isValid() && role == Qt::EditRole) {
    int row = index.row();

    auto &marketOrder = orders_.at(row);

    if (index.column() == 0)
      marketOrder.fromCurrency_ = common::Currency::fromString(value.toString().toStdString());
    else if (index.column() == 1)
      marketOrder.toCurrency_ = common::Currency::fromString(value.toString().toStdString());
    else if (index.column() == 2)
      marketOrder.orderType_ = common::convertOrderTypeFromString(value.toString().toStdString());
    else if (index.column() == 3)
      marketOrder.stockExchangeType_ =
          common::convertStockExchangeTypeFromString(value.toString().toStdString());
    else if (index.column() == 4)
      marketOrder.quantity_ = std::stod(value.toString().toStdString());
    else if (index.column() == 5)
      marketOrder.price_ = std::stod(value.toString().toStdString());
    else if (index.column() == 6)
      marketOrder.opened_ = common::Date::parseDate(value.toString().toStdString());
    else
      return false;

    emit dataChanged(index, index, {role});

    return true;
  }

  return false;
}

QVariant TradingAccountOrdersViewModel::headerData(int section, Qt::Orientation orientation,
                                                   int role) const {
  if (role != Qt::DisplayRole) return QVariant();

  if (orientation == Qt::Horizontal) {
    switch (section) {
      case 0:
        return tr("From currency");
      case 1:
        return tr("To currency");
      case 2:
        return tr("Order type");
      case 3:
        return tr("Stock exchange");
      case 4:
        return tr("Quantity");
      case 5:
        return tr("Price");
      case 6:
        return tr("Opened");
      default:
        return QVariant();
    }
  }

  return QVariant();
}

Qt::ItemFlags TradingAccountOrdersViewModel::flags(const QModelIndex &index) const {
  if (!index.isValid()) {
    return Qt::ItemIsEnabled;
  }

  return QAbstractTableModel::flags(index) | Qt::ItemIsEditable;
}

}  // namespace view
}  // namespace auto_trader