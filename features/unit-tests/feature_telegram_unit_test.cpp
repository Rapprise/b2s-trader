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

#include "feature_telegram_unit_test.h"

#include "common/exceptions/telegram_exception/telegram_rest_exception.h"
#include "common/exceptions/telegram_exception/unitialized_telegram_option_exception.h"
#include "resources/feature_utils.h"

namespace auto_trader {
namespace features {
namespace telegram_announcer {
namespace unit_test {

const std::string token = "614877791:AAGijfI_oDqFTxQyNcCT8Pl7VA4xLD1DZF8";

std::shared_ptr<MockTelegramAnnouncer> mockTelegramAnnouncer;

class TelegramAnnouncerFixture : public ::testing::Test {
 public:
  void SetUp() override { mockTelegramAnnouncer.reset(new MockTelegramAnnouncer()); }
  void TearDown() override { mockTelegramAnnouncer.reset(); }
};

TEST_F(TelegramAnnouncerFixture, InitWithOneMessage) {
  mockTelegramAnnouncer->DelegateToInitAnnouncerResponse();
  EXPECT_CALL(*mockTelegramAnnouncer, processHttpRequest(testing::_, testing::_)).Times(1);
  mockTelegramAnnouncer->createSession();
  EXPECT_NO_THROW(mockTelegramAnnouncer->init(token));
}

TEST_F(TelegramAnnouncerFixture, DoubleInit) {
  mockTelegramAnnouncer->DelegateToInitAnnouncerResponse();
  EXPECT_CALL(*mockTelegramAnnouncer, processHttpRequest(testing::_, testing::_)).Times(1);
  mockTelegramAnnouncer->createSession();
  EXPECT_NO_THROW(mockTelegramAnnouncer->init(token));

  EXPECT_CALL(*mockTelegramAnnouncer, processHttpRequest(testing::_, testing::_)).Times(0);
  EXPECT_NO_THROW(mockTelegramAnnouncer->init(token));
}

TEST_F(TelegramAnnouncerFixture, InitWithSettedTokenBefore) {
  EXPECT_NO_THROW(mockTelegramAnnouncer->setToken(token));

  mockTelegramAnnouncer->DelegateToInitAnnouncerResponse();
  EXPECT_CALL(*mockTelegramAnnouncer, processHttpRequest(testing::_, testing::_)).Times(0);
  mockTelegramAnnouncer->createSession();
  EXPECT_NO_THROW(mockTelegramAnnouncer->init(token));
}

TEST_F(TelegramAnnouncerFixture, InitWithoutFirstMessage) {
  mockTelegramAnnouncer->DelegateToInitAnnouncerResponseWithoutFirstMessageResponse();
  EXPECT_CALL(*mockTelegramAnnouncer, processHttpRequest(testing::_, testing::_)).Times(1);

  mockTelegramAnnouncer->createSession();
  EXPECT_THROW(mockTelegramAnnouncer->init(token),
               common::exceptions::UnitializedTelegramOptionException);
}

// init with 1 message, message_id == 1
// get last messages, size 2, return 1 new message, because init had 1 message
TEST_F(TelegramAnnouncerFixture, GetLastMessages) {
  mockTelegramAnnouncer->DelegateToInitAnnouncerResponse();
  EXPECT_CALL(*mockTelegramAnnouncer, processHttpRequest(testing::_, testing::_)).Times(1);

  mockTelegramAnnouncer->createSession();
  mockTelegramAnnouncer->init(token);

  mockTelegramAnnouncer->DelegateToGetLastMessagesNotEmptyResponse();
  EXPECT_CALL(*mockTelegramAnnouncer, processHttpRequest(testing::_, testing::_)).Times(1);
  auto messageCollection = mockTelegramAnnouncer->getLastMessages();

  EXPECT_EQ(messageCollection.size(), 1);
}

TEST_F(TelegramAnnouncerFixture, GetLastMessagesEmptyResponse) {
  mockTelegramAnnouncer->DelegateToInitAnnouncerResponse();
  EXPECT_CALL(*mockTelegramAnnouncer, processHttpRequest(testing::_, testing::_)).Times(1);
  mockTelegramAnnouncer->init(token);

  mockTelegramAnnouncer->DelegateToGetLastMessagesEmptyResponse();
  EXPECT_CALL(*mockTelegramAnnouncer, processHttpRequest(testing::_, testing::_)).Times(1);
  auto messageCollection = mockTelegramAnnouncer->getLastMessages();

  EXPECT_EQ(messageCollection.size(), 0);
}

TEST(Feature, Feature_Complex_compareMessageWithTelegramCommands_method_Test) {
  std::string command1 = "trading_on";
  std::string command2 = "trading_off";
  std::string command3 = "logging_on";
  std::string command4 = "logging_off";
  std::string command5 = "TRading_oN";
  std::string command6 = "trADIng_off";
  std::string command7 = "trading_onn";
  std::string command8 = "logging_off a";
  std::string command9 = " logging_on";
  std::string command10 = " logging_off      ";

  auto command1Info = feature_utils::compareMessageWithTelegramCommands(command1);
  EXPECT_EQ(command1Info.first, true);
  EXPECT_EQ(command1Info.second, command1);

  auto command2Info = feature_utils::compareMessageWithTelegramCommands(command2);
  EXPECT_EQ(command2Info.first, true);
  EXPECT_EQ(command2Info.second, command2);

  auto command3Info = feature_utils::compareMessageWithTelegramCommands(command3);
  EXPECT_EQ(command3Info.first, true);
  EXPECT_EQ(command3Info.second, command3);

  auto command4Info = feature_utils::compareMessageWithTelegramCommands(command4);
  EXPECT_EQ(command4Info.first, true);
  EXPECT_EQ(command4Info.second, command4);

  auto command5Info = feature_utils::compareMessageWithTelegramCommands(command5);
  EXPECT_EQ(command5Info.first, true);
  EXPECT_EQ(command5Info.second, "trading_on");

  auto command6Info = feature_utils::compareMessageWithTelegramCommands(command6);
  EXPECT_EQ(command6Info.first, true);
  EXPECT_EQ(command6Info.second, "trading_off");

  auto command7Info = feature_utils::compareMessageWithTelegramCommands(command7);
  EXPECT_EQ(command7Info.first, false);

  auto command8Info = feature_utils::compareMessageWithTelegramCommands(command8);
  EXPECT_EQ(command8Info.first, false);

  auto command9Info = feature_utils::compareMessageWithTelegramCommands(command9);
  EXPECT_EQ(command9Info.first, true);
  EXPECT_EQ(command9Info.second, "logging_on");

  auto command10Info = feature_utils::compareMessageWithTelegramCommands(command10);
  EXPECT_EQ(command10Info.first, true);
  EXPECT_EQ(command10Info.second, "logging_off");
}

TEST(TelegramAnnouncer, TelegramAnnouncer_Bad_Telegram_Token_Test) {
  auto& announcer = TelegramAnnouncer::instance();

  announcer.createSession();
  EXPECT_THROW(announcer.init("Bad_token"), common::exceptions::UnitializedTelegramOptionException);
  announcer.setToken("");
}

TEST(TelegramAnnouncer, TelegramAnnouncer_Empty_getLastMessages_Data_Without_Init_Test) {
  auto& announcer = TelegramAnnouncer::instance();

  announcer.createSession();
  auto messages = announcer.getLastMessages();
  EXPECT_TRUE(messages.size() == 0);
}

TEST(TelegramAnnouncer, TelegramAnnouncer_getLastMessages_With_Fake_Token_And_ChatId_Test) {
  auto& announcer = TelegramAnnouncer::instance();

  announcer.createSession();
  announcer.setToken("Fake token");
  announcer.setChatId(64324723);
  EXPECT_THROW(announcer.getLastMessages(), common::exceptions::UnitializedTelegramOptionException);

  announcer.setToken("");
  announcer.setChatId(0);
}

TEST(TelegramAnnouncer, TelegramAnnouncer_Send_Message_Without_Token_Test) {
  auto& announcer = TelegramAnnouncer::instance();

  announcer.createSession();
  EXPECT_THROW(announcer.sendMessage("Test message"), common::exceptions::TelegramRestException);
  EXPECT_FALSE(announcer.isInitialized());
}

}  // namespace unit_test
}  // namespace telegram_announcer
}  // namespace features
}  // namespace auto_trader