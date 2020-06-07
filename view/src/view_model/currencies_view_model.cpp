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

#include "include/view_model/currencies_view_model.h"

#include <iomanip>
#include <sstream>

namespace auto_trader {
namespace view {

constexpr int CURRENCIES_VIEW_MODEL_COLUMN_COUNT = 3;

CurrenciesViewModel::CurrenciesViewModel(QObject *parent) : QAbstractTableModel(parent) {}

CurrenciesViewModel::CurrenciesViewModel(const std::vector<common::CurrencyTick> &currencies,
                                         QObject *parent)
    : QAbstractTableModel(parent), currencies_(currencies) {}

int CurrenciesViewModel::rowCount(const QModelIndex &parent) const {
  Q_UNUSED(parent);
  return currencies_.size();
}

int CurrenciesViewModel::columnCount(const QModelIndex &parent) const {
  Q_UNUSED(parent);
  return CURRENCIES_VIEW_MODEL_COLUMN_COUNT;
}

QVariant CurrenciesViewModel::data(const QModelIndex &index, int role) const {
  if (!index.isValid()) return QVariant();

  if (index.row() >= currencies_.size() || index.row() < 0) return QVariant();

  if (role == Qt::DisplayRole) {
    const auto &currency = currencies_.at(index.row());

    if (index.column() == 0) {
      return common::Currency::toString(currency.toCurrency_).c_str();
    } else if (index.column() == 1) {
      std::ostringstream stream;
      stream << std::fixed << std::setprecision(8);
      stream << currency.ask_;
      std::string value = stream.str();
      return value.c_str();
    } else if (index.column() == 2) {
      std::ostringstream stream;
      stream << std::fixed << std::setprecision(8);
      stream << currency.bid_;
      std::string value = stream.str();
      return value.c_str();
    }
  }
  return QVariant();
}

QVariant CurrenciesViewModel::headerData(int section, Qt::Orientation orientation, int role) const {
  if (role != Qt::DisplayRole) return QVariant();

  if (orientation == Qt::Horizontal) {
    switch (section) {
      case 0:
        return tr("Currency");
      case 1:
        return tr("Ask");
      case 2:
        return tr("Bid");

      default:
        return QVariant();
    }
  }
  return QVariant();
}

bool CurrenciesViewModel::setData(const QModelIndex &index, const QVariant &value, int role) {
  if (index.isValid() && role == Qt::EditRole) {
    int row = index.row();

    auto &currency = currencies_.at(row);

    if (index.column() == 0)
      currency.toCurrency_ = common::Currency::fromString(value.toString().toStdString());
    else if (index.column() == 1)
      currency.ask_ = value.toDouble();
    else if (index.column() == 2)
      currency.bid_ = value.toDouble();
    else
      return false;

    emit dataChanged(index, index, {role});

    return true;
  }

  return false;
}

Qt::ItemFlags CurrenciesViewModel::flags(const QModelIndex &index) const {
  if (!index.isValid()) {
    return Qt::ItemIsEnabled;
  }

  return QAbstractTableModel::flags(index) | Qt::ItemIsEditable;
}

}  // namespace view
}  // namespace auto_trader