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

#include "include/dialogs/license_dialog.h"

#include <QtWidgets/QPushButton>

namespace auto_trader {
namespace view {
namespace dialogs {

LicenseDialog::LicenseDialog(features::license::License &license, QWidget *parent)
    : QDialog(parent), license_(license) {
  licenseDialog_.setupUi(this);

  licenseDialog_.lineEdit->setReadOnly(true);

  connect(licenseDialog_.pushButton, SIGNAL(clicked()), this, SLOT(buyLicense()));
  connect(licenseDialog_.pushButton_2, SIGNAL(clicked()), this, SLOT(buyTrial()));

  connect(licenseDialog_.pushButton_3, SIGNAL(clicked()), this, SLOT(exit()));
  connect(licenseDialog_.pushButton_4, SIGNAL(clicked()), this, SLOT(activate()));

  connect(licenseDialog_.plainTextEdit, SIGNAL(textChanged()), this, SLOT(checkOkButtonStatus()));

  checkOkButtonStatus();
}

void LicenseDialog::buyLicense() {
#ifdef WIN32
  ShellExecute(NULL, "open", "http://rapprise.com/b2s-trader-1-0/", NULL, NULL, SW_SHOWNORMAL);
#elif __APPLE__
  system("open http://rapprise.com/b2s-trader-1-0/");
#else
  system("x-www-browser http://rapprise.com/b2s-trader-1-0/");
#endif
}

void LicenseDialog::buyTrial() {
#ifdef WIN32
  ShellExecute(NULL, "open", "http://rapprise.com/b2s-trader-1-0/", NULL, NULL, SW_SHOWNORMAL);
#elif __APPLE__
  system("open http://rapprise.com/b2s-trader-1-0/");
#else
  system("x-www-browser http://rapprise.com/b2s-trader-1-0/");
#endif
}

void LicenseDialog::activate() {
  auto text = licenseDialog_.plainTextEdit->toPlainText().toStdString();
  if (text.empty()) {
    return;
  }

  if (license_.activateProduct(text)) {
    licenseStatus_ = true;
    licenseDialog_.lineEdit->setText("License activated");
    licenseDialog_.lineEdit->setStyleSheet("color: #000000");
    accept();
  } else {
    licenseDialog_.lineEdit->setText("Invalid license key");
    licenseDialog_.lineEdit->setStyleSheet("color: #FF0000");
  }
}

void LicenseDialog::exit() { accept(); }

void LicenseDialog::checkOkButtonStatus() const {
  bool isEmpty = licenseDialog_.plainTextEdit->toPlainText().isEmpty();
  licenseDialog_.pushButton_4->setDisabled(isEmpty);
}

bool LicenseDialog::getLicenseStatus() const { return licenseStatus_; }

}  // namespace dialogs
}  // namespace view
}  // namespace auto_trader