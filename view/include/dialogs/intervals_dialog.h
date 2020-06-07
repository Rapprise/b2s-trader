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

#ifndef AUTO_TRADER_FEATURES_INTERVAL_DIALOG_H
#define AUTO_TRADER_FEATURES_INTERVAL_DIALOG_H

#include "common/listeners/app_listener.h"
#include "ui_intervals_dialog.h"

namespace auto_trader {
namespace view {
namespace dialogs {

class IntervalsDialog : public QDialog {
  Q_OBJECT

 public:
  IntervalsDialog(common::AppListener &appListener, QWidget *parent = nullptr);
  ~IntervalsDialog() = default;

 private slots:
  void closeDialog();
  void intervalChanged(const QString &value);

 private:
  common::AppListener &appListener_;
  Ui::IntervalsDialog uiDialog_;
};

}  // namespace dialogs
}  // namespace view
}  // namespace auto_trader

#endif  // AUTO_TRADER_FEATURES_INTERVAL_DIALOG_H
