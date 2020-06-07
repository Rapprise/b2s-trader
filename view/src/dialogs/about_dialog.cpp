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

#include "include/dialogs/about_dialog.h"

#include <QValidator>
#include <QtWidgets/QPushButton>
#include <sstream>

#include "features/include/stop_loss_announcer.h"
#include "include/validators/float_validator.h"

namespace auto_trader {
namespace view {
namespace dialogs {

AboutDialog::AboutDialog(common::AppListener& appListener, const std::string& licenseOwner,
                         const std::string& expirationDate, QWidget* parent)
    : QDialog(parent),
      appListener_(appListener),
      licenseOwner_(licenseOwner),
      expirationDate_(expirationDate) {
  uiDialog_.setupUi(this);

  setObjectName("QAboutCustomDialog");

  uiDialog_.layoutWidget->setStyleSheet("background-color: transparent");
  uiDialog_.label->setStyleSheet("color: black");
  uiDialog_.label_2->setStyleSheet("color: black");
  uiDialog_.label_3->setStyleSheet("color: black");
  uiDialog_.label_4->setStyleSheet("color: black");

  uiDialog_.label_3->setText(QString::fromStdString(licenseOwner_));
  uiDialog_.label_4->setText(QString::fromStdString(expirationDate_));

  QPixmap bkgnd(":/b2s_images/about_window.png");
  bkgnd = bkgnd.scaled(this->size(), Qt::IgnoreAspectRatio);
  QPalette palette;
  palette.setBrush(QPalette::Background, bkgnd);
  this->setPalette(palette);
}

}  // namespace dialogs
}  // namespace view
}  // namespace auto_trader