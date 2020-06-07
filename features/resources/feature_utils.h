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

#ifndef AUTO_TRADER_FEATURE_UTILS_H
#define AUTO_TRADER_FEATURE_UTILS_H

#include <fstream>
#include <utility>

#include "resources.h"

#if defined(__linux__)
#include <dirent.h>
#elif (WIN32)
#include <iphlpapi.h>

#include <cctype>
#elif (__APPLE__)
#include <QList>
#include <QNetworkInterface>
#endif

#include <QDateTime>
#include <fstream>
#include <streambuf>
#include <vector>

#include "common/loggers/file_logger.h"
#include "common/utils.h"

namespace auto_trader {
namespace features {
namespace feature_utils {

#if defined(__linux__)
static std::vector<std::string> getLinuxMacAddresses() {
  std::vector<std::string> folders;

  DIR* dirrection;
  struct dirent* entity;
  if ((dirrection = opendir(
           resources::license::PATH_FOR_FOLDERS_WITH_MAC_ADDRESSES_LINUX.data())) != nullptr) {
    while ((entity = readdir(dirrection)) != nullptr) {
      if (entity->d_name[0] == '.') continue;
      folders.emplace_back(std::string(entity->d_name));
    }
    closedir(dirrection);
  } else {
    common::loggers::FileLogger::getLogger()
        << "Could not open directory : " +
               resources::license::PATH_FOR_FOLDERS_WITH_MAC_ADDRESSES_LINUX;
  }

  std::vector<std::string> macAddresses;
  for (const auto& folder : folders) {
    std::ifstream stream(resources::license::PATH_FOR_FOLDERS_WITH_MAC_ADDRESSES_LINUX + folder +
                         "/address");
    std::string macAddress((std::istreambuf_iterator<char>(stream)),
                           std::istreambuf_iterator<char>());
    macAddress.pop_back();
    macAddresses.emplace_back(macAddress);
  }

  return macAddresses;
}
#endif

#if defined(__APPLE__)
static std::vector<std::string> getMacOsAddresses() {
  std::vector<std::string> folders;
  QList<QNetworkInterface> interfaces = QNetworkInterface::allInterfaces();
  foreach (QNetworkInterface currentInterface, QNetworkInterface::allInterfaces()) {
    if (currentInterface.flags().testFlag(QNetworkInterface::IsUp) &&
        !currentInterface.flags().testFlag(QNetworkInterface::IsLoopBack)) {
      auto hardwareAddress = currentInterface.hardwareAddress();
      if (hardwareAddress != "00:00:00:00:00:00" && !hardwareAddress.isEmpty()) {
        folders.push_back(currentInterface.hardwareAddress().toStdString());
      }
    }
  }

  return folders;
}

#endif

#if defined(WIN32)
#pragma comment(lib, "IPHLPAPI.lib")

static std::string getWindowsStringMacAddress(BYTE* addr, unsigned int length) {
  std::stringstream stream;
  constexpr int MAC_ADDR_SIZE = 3;
  for (int i = 0; i < length; i++) {
    char address[MAC_ADDR_SIZE];
    snprintf(address, MAC_ADDR_SIZE, "%02X", *addr);
    stream << address;
    addr++;

    stream << ":";
  }

  std::string macAddress = stream.str();
  macAddress.pop_back();
  return macAddress;
}

static std::vector<std::string> getWinMacAddresses(void) {
  std::vector<std::string> macAddresses;

  IP_ADAPTER_INFO AdapterInfo[16];
  DWORD dwBufLen = sizeof(AdapterInfo);

  DWORD dwStatus = GetAdaptersInfo(AdapterInfo, &dwBufLen);
  if (dwStatus != ERROR_SUCCESS) {
    common::loggers::FileLogger::getLogger()
        << "GetAdaptersInfo failed. Error code : " + GetLastError();
    return macAddresses;
  }

  PIP_ADAPTER_INFO pAdapterInfo = AdapterInfo;
  getWindowsStringMacAddress(pAdapterInfo->Address, pAdapterInfo->AddressLength);
  while (pAdapterInfo) {
    std::string macAddress =
        getWindowsStringMacAddress(pAdapterInfo->Address, pAdapterInfo->AddressLength);
    macAddresses.push_back(macAddress);
    pAdapterInfo = pAdapterInfo->Next;
  }

  return macAddresses;
}
#endif

static std::vector<std::string> getMacAddresses() {
  std::vector<std::string> macAddresses;

#if defined(__linux__)
  macAddresses = getLinuxMacAddresses();
#elif (WIN32)
  macAddresses = getWinMacAddresses();
#elif (__APPLE__)
  macAddresses = getMacOsAddresses();
#endif

  return macAddresses;
}

static std::string eraseSpacesBeforeAndAfterWords(const std::string& message) {
  unsigned int spacesBeforeCount = 0;
  for (auto symbol : message) {
    if (symbol == ' ')
      ++spacesBeforeCount;
    else
      break;
  }

  int spacesAfterWords = 0;
  for (auto index = message.length() - 1; index > 0; --index) {
    if (message.at(index) == ' ')
      ++spacesAfterWords;
    else
      break;
  }

  if (spacesBeforeCount || spacesAfterWords) {
    auto messageWithoutAdditionalSpaces =
        message.substr(spacesBeforeCount, message.size() - 1 - spacesAfterWords);
    return messageWithoutAdditionalSpaces;
  }

  return message;
}

static std::pair<bool, std::string> compareMessageWithTelegramCommands(const std::string& message) {
  std::pair<bool, std::string> messageInfo{false, ""};
  std::string messageForCompareWithCommands;

  if (message.back() == ' ' || message.front() == ' ') {
    messageForCompareWithCommands = eraseSpacesBeforeAndAfterWords(message);
  } else {
    messageForCompareWithCommands = message;
  }

  bool equallResult = false;
  for (const auto& command : resources::TELEGRAM_COMMANDS) {
    if (command.size() != messageForCompareWithCommands.size()) {
      continue;
    }
    equallResult =
        std::equal(messageForCompareWithCommands.begin(), messageForCompareWithCommands.end(),
                   command.begin(), [](const char& left, const char& right) {
                     return (left == right || std::tolower(left) == std::tolower(right));
                   });

    if (equallResult) {
      messageInfo.first = equallResult;
      messageInfo.second = command;
      break;
    }
  }

  return messageInfo;
}

}  // namespace feature_utils
}  // namespace features
}  // namespace auto_trader

#endif  // AUTO_TRADER_FEATURE_UTILS_H
