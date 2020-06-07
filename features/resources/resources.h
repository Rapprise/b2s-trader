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

#ifndef AUTO_TRADER_FEATURES_RESOURSES_H
#define AUTO_TRADER_FEATURES_RESOURSES_H

#include <map>
#include <string>
#include <vector>

namespace auto_trader {
namespace features {
namespace resources {

const std::string TELEGRAM_URL = "https://api.telegram.org/bot";
const std::string TELEGRAM_HOST = "api.telegram.org";
const std::string TELEGRAM_GET_UPDATES = "/getUpdates";
const std::string TELEGRAM_SEND_MESSAGE = "/sendMessage?";
const std::string TELEGRAM_CHAT_ID = "chat_id";
const std::string TELEGRAM_TEXT = "text";

const std::string TELEGRAM_JSON_BLOCK_RESULT = "result";
const std::string TELEGRAM_JSON_BLOCK_MESSAGE = "message";
const std::string TELEGRAM_JSON_BLOCK_CHAT = "chat";

const std::string TELEGRAM_JSON_FIELD_ID = "id";
const std::string TELEGRAM_JSON_FIELD_IS_OK = "ok";
const std::string TELEGRAM_JSON_FIELD_MESSAGE_ID = "message_id";

const std::string TELEGRAM_ERROR_DESCRIPTION = "description";

const std::vector<std::string> TELEGRAM_COMMANDS = {"trading_on", "trading_off", "logging_on",
                                                    "logging_off"};

const std::string AND = "&";
const std::string EQUAL = "=";
const std::string EMPTY_STR = "";
const std::string SLASH = "/";

namespace license {

const std::string PUBLIC_KEY =
    "-----BEGIN PUBLIC KEY-----\n"
    "MIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEA7CKChWwCzG6AUusVcSos\n"
    "lUxmMctQf5Fh8S4JG1VywXMRE1FSYhAP5DzGy1tsoVIeHbjs13KaslpIldbYZX05\n"
    "q39OncLURKEGUmPVZbe8ESt2Mqcp3MSj+b7AWbI7cz2iSq7hr10Q2wE2KQS2hWGa\n"
    "JEV3T/3q6+8Sa083V0Lvl6ip4DFTIV1Sk1CY0Xw+l5ULVjArD/NjztS9bLF7q8ZC\n"
    "lhupnQnWt3n4fMoFDlcNB1HKbbGkzB3HLFVGNsoUqbQdz5NfKQas1yts3/mT5p1n\n"
    "Vnirr8HvTVBUcqeOTZdI5buo2Le+ZcbifubkId0pRpdXwiUNct6UpSnrzYsxMYYc\n"
    "jwIDAQAB\n"
    "-----END PUBLIC KEY-----";

const std::string PRIVATE_KEY =
    "-----BEGIN RSA PRIVATE KEY-----\n"
    "MIIEpAIBAAKCAQEA7CKChWwCzG6AUusVcSoslUxmMctQf5Fh8S4JG1VywXMRE1FS\n"
    "YhAP5DzGy1tsoVIeHbjs13KaslpIldbYZX05q39OncLURKEGUmPVZbe8ESt2Mqcp\n"
    "3MSj+b7AWbI7cz2iSq7hr10Q2wE2KQS2hWGaJEV3T/3q6+8Sa083V0Lvl6ip4DFT\n"
    "IV1Sk1CY0Xw+l5ULVjArD/NjztS9bLF7q8ZClhupnQnWt3n4fMoFDlcNB1HKbbGk\n"
    "zB3HLFVGNsoUqbQdz5NfKQas1yts3/mT5p1nVnirr8HvTVBUcqeOTZdI5buo2Le+\n"
    "ZcbifubkId0pRpdXwiUNct6UpSnrzYsxMYYcjwIDAQABAoIBAGo0S0f/AHM57/Vb\n"
    "TlWLhmnP5tbWfuohVU/ISMhGckqXmkJ5/PemCb+5ph+oiv8zk+ACYGO2Y+V7vdHG\n"
    "w6VB4UJFC/Lgwd/sHI/J3xzUun1ucsbtcGKe/rFBHfo5s4ttWSUF6vOYhBA8R0PD\n"
    "nuP2tgFCTRtpNNPQvxQMmPNiY6htkmo2Q6weHwoQhTQBbmA90NT3OG4MTLw2B0gF\n"
    "SnEZBg7abE6hpnbkDkWPb6WmUvzHf8YVoF0hEQ9jJ7ABJ1XSkdP8asl1UWUOBWzG\n"
    "Pr37Xdr7n8aToqMcNNGnleycGm52zlB8ZzKQum3ssBaWUgdEm/nZPlu8u63va5M8\n"
    "dJUBVQECgYEA+UA4QjB/BtVCrwSwwoRrkZlnSOCB3JRm27BSSrWez+q9CfRnjgCu\n"
    "dNuOdFsWwvw6mZb6Z/EVLpMsVRSnKm6wzP0J+SbfpOuUjGlTRmqzrQZlRv2dj2ml\n"
    "OIYH9drmrcoPxvIjE1J5uo1qpGQWqN/IpG3kBqc029ztvJ5JxZQ8Ue8CgYEA8ode\n"
    "+jM9u/xzkbqsIN4T0retbT0xuJ/PyKufQ/GSsrJQP6C7HRxIo1VhUTh2X8OBG86k\n"
    "1sPiXOkqIZGBhJ/huZx2KcCzfsfHAU2ovRoizqZtRt6mOP3Era1/QY+uJc2Zf4XX\n"
    "Msd/w4VzgwEme7KoLiYbyIhhFdPqZJeCqfXS/2ECgYEApP6c/zvJKsXLYb4SIIG2\n"
    "D2jkFTsA2/qk6DKdFvWJTotOXiOa1UagNZ0NfPq7Y5HCXN+znVDau70S8LM+QXmK\n"
    "sfejPXo9O5fy+svSI2AI1rXQHyFNxpyhkXwwEtC4OTz0Grc4/tZu3E421OGomluh\n"
    "Hy/bbYgMFqr1+REzNoZ9M6MCgYEAsuhLlvQf0UHsmw8ClQhdxS67336p5BD3/Yck\n"
    "6MEWv32slxNhlJrdtqdwL7MgF4qn/jRVAxzjKjjSXIry6TvsDrktraU6hAapdfus\n"
    "wzq5fInmyMXMBvSklp4K5vkvtTFSz3CTDCGAzWHvkS9TDSMAz3JLQoXgYHLAUM43\n"
    "lFsD+GECgYAXj/EH5O+C2ySIRFnVfEmPcCpPM73j4g1IlBc6GZWQ4HPbXdtLuI4I\n"
    "7j06PW7rBxY7WDNoW3PDXaGIpIGQfgYTLQQ+yGZdvWXIOk1QFJtvBt/+BUgtwNqf\n"
    "6M1s42oY3ip7+dBoWAt2wfRyTLadZu88lkoulhVZb0BItbLQIHTnyg==\n"
    "-----END RSA PRIVATE KEY-----";

const int ENCRYPTED_LENGHT = 256;
const int ENCRYPTION_DECRYPTION_ERROR_CODE = -1;

const std::string PATH_FOR_FOLDERS_WITH_MAC_ADDRESSES_LINUX = "/sys/class/net/";

const std::map<unsigned int, std::string> RAPPRISE_INDEXES = {
    {5, "r"}, {47, "a"}, {84, "p"}, {102, "p"}, {113, "r"}, {179, "i"}, {286, "s"}, {301, "e"}};

const std::vector<std::string> LINUX_PATHES_TO_LICENSE_FILES = {"/var/log/"};

const std::string DECRYPTED_DATA_DELIMETER = "_";
const std::string DATE_FORMAT = "dd.MM.yyyy HH:mm:ss";
const std::string LICENSE_FILE_NAME = "b2s.txt";
const std::string LICENSE_FILE_NAME_FOR_TESTS = "B2S_test_license.txt";
const std::string SLASH = "/";
const std::string POINT = ".";
const std::string HIDDEN_FOLDER_NAME = "B2SLicense";
const std::string CONFIG_FOLDER_NAME = "config";

const int NAME_INDEX = 0;
const int SURNAME_INDEX = 1;
const int MAC_ADDRESS_INDEX = 2;
const int DATE_INDEX = 3;
const int TIME_INDEX = 4;

const int DECRYPTED_ARGUMENTS_COUNT = 5;
}  // namespace license

}  // namespace resources
}  // namespace features
}  // namespace auto_trader

#endif  // AUTO_TRADER_FEATURES_RESOURSES_H
