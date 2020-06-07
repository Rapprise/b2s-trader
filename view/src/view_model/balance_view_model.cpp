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

#include "include/view_model/balance_view_model.h"

#include <common/listeners/gui_listener.h>

namespace auto_trader {
namespace view {

constexpr int ACCOUNT_CURRENCIES_BALANCE_COLUMN = 2;

BalanceViewModel::BalanceViewModel(std::vector<common::GuiListener::AccountBalance> accountBalance,
                                   QObject *parent)
    : QAbstractTableModel(parent), accountBalance_(accountBalance) {}

int BalanceViewModel::rowCount(const QModelIndex &parent) const {
  Q_UNUSED(parent);
  return accountBalance_.size();
}

int BalanceViewModel::columnCount(const QModelIndex &parent) const {
  return ACCOUNT_CURRENCIES_BALANCE_COLUMN;
}

QVariant BalanceViewModel::data(const QModelIndex &index, int role) const {
  if (!index.isValid()) return QVariant();

  if (index.row() >= accountBalance_.size() || index.row() < 0) return QVariant();

  if (role == Qt::DisplayRole) {
    const auto &currentCurrency = accountBalance_.at(index.row());

    if (index.column() == 0) {
      return common::Currency::toString(currentCurrency.first).c_str();
    } else if (index.column() == 1) {
      std::ostringstream stream;
      stream << std::fixed << std::setprecision(8);
      stream << currentCurrency.second;
      std::string value = stream.str();
      return value.c_str();
    }
  }

  return QVariant();
}

bool BalanceViewModel::setData(const QModelIndex &index, const QVariant &value, int role) {
  if (index.isValid() && role == Qt::EditRole) {
    int row = index.row();

    auto &currencyBalance = accountBalance_.at(row);

    if (index.column() == 0) {
      currencyBalance.first = common::Currency::fromString(value.toString().toStdString());
    } else if (index.column() == 1) {
      currencyBalance.second = value.toDouble();
    } else {
      return false;
    }

    emit dataChanged(index, index, {role});

    return true;
  }

  return false;
}

QVariant BalanceViewModel::headerData(int section, Qt::Orientation orientation, int role) const {
  if (role != Qt::DisplayRole) {
    return QVariant();
  }

  if (orientation == Qt::Horizontal) {
    switch (section) {
      case 0:
        return tr("Currency");
      case 1:
        return tr("Balance");

      default:
        return QVariant();
    }
  }

  return QVariant();
}

Qt::ItemFlags BalanceViewModel::flags(const QModelIndex &index) const {
  if (!index.isValid()) {
    return Qt::ItemIsEnabled;
  }

  return QAbstractTableModel::flags(index) | Qt::ItemIsEditable;
}

}  // namespace view
}  // namespace auto_trader