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

#ifndef AUTO_TRADER_COMMON_DATE_H
#define AUTO_TRADER_COMMON_DATE_H

#include <chrono>
#include <ctime>
#include <sstream>
#include <string>
#include <vector>

namespace auto_trader {
namespace common {

struct Date {
  int second_{0};
  int minute_{0};
  int hour_{0};
  int day_{0};
  int month_{0};
  int year_{0};

  // Pase string representation of date in format 2018-05-31T00:00:00
  static Date parseDate(const std::string& date_) {
    std::vector<std::string> tokens;
    std::string token;
    std::stringstream stream(date_);
    while (std::getline(stream, token)) {
      std::size_t previousPosition = 0;
      std::size_t currentPosition = 0;
      while ((currentPosition = token.find_first_of("-:T", previousPosition)) !=
             std::string::npos) {
        if (currentPosition > previousPosition)
          tokens.push_back(token.substr(previousPosition, currentPosition - previousPosition));
        previousPosition = currentPosition + 1;
      }
      if (previousPosition < token.length())
        tokens.push_back(token.substr(previousPosition, std::string::npos));
    }

    Date date{0, 0, 0, 0, 0, 0};
    date.year_ = std::atoi(tokens[0].c_str());
    date.month_ = std::atoi(tokens[1].c_str());
    date.day_ = std::atoi(tokens[2].c_str());
    date.hour_ = std::atoi(tokens[3].c_str());
    date.minute_ = std::atoi(tokens[4].c_str());
    date.second_ = std::atoi(tokens[5].c_str());

    return date;
  }

  static Date hoursToDate(unsigned int hours) {
    Date date{0, 0, (int)hours, 0, 0, 0};
    return date;
  }

  static Date minutesToDate(unsigned int minutes) {
    if (minutes >= 60) {
      int hours = minutes / 60;
      int reminder = minutes % 60;
      return Date{0, reminder, hours, 0, 0, 0};
    } else {
      return Date{0, (int)minutes, 0, 0, 0, 0};
    }
  }

  static std::string toString(const Date& date) {
    std::string strDate = std::to_string(date.year_) + "-" + std::to_string(date.month_) + "-" +
                          std::to_string(date.day_) + "T" + std::to_string(date.hour_) + ":" +
                          std::to_string(date.minute_) + ":" + std::to_string(date.second_);

    return strDate;
  }

  bool operator==(const Date& date) const {
    return second_ == date.second_ && minute_ == date.minute_ && hour_ == date.hour_ &&
           day_ == date.day_ && month_ == date.month_ && year_ == date.year_;
  }

  static time_t getTimestamp() {
    auto tick = std::chrono::system_clock::now();
    auto time = std::chrono::system_clock::to_time_t(tick);
    return time;
  }

  static Date convertTimestampToDate(const time_t& timestamp) {
    std::tm* now = std::localtime(&timestamp);
    Date currentDate;
    currentDate.year_ = now->tm_year + 1900;
    currentDate.day_ = now->tm_mday;
    currentDate.month_ = now->tm_mon + 1;  // tm_month range [0 - 11]
    currentDate.hour_ = now->tm_hour;
    currentDate.minute_ = now->tm_min;
    currentDate.second_ = now->tm_sec;

    return currentDate;
  }

  static Date getCurrentTime() {
    auto tick = std::chrono::system_clock::now();
    auto time = std::chrono::system_clock::to_time_t(tick);
    std::tm* now = std::localtime(&time);
    Date currentDate;
    currentDate.year_ = now->tm_year + 1900;
    currentDate.day_ = now->tm_mday;
    currentDate.month_ = now->tm_mon + 1;  // tm_month range [0 - 11]
    currentDate.hour_ = now->tm_hour;
    currentDate.minute_ = now->tm_min;
    currentDate.second_ = now->tm_sec;

    return currentDate;
  }

  static Date getDayBefore(const common::Date& currentDate, int daysCount) {
    auto date = currentDate;
    if (date.day_ <= daysCount) {
      int difference = daysCount - date.day_;
      if (date.month_ == 1) {
        date.year_ = date.year_ - 1;
        date.month_ = 12;
        date.day_ = 31 - difference;
      } else {
        date.month_ = date.month_ - 1;
        if (date.month_ == 1 || date.month_ == 3 || date.month_ == 5 || date.month_ == 7 ||
            date.month_ == 8 || date.month_ == 10 || date.month_ == 12) {
          date.day_ = 31 - difference;
        } else if (date.month_ == 4 || date.month_ == 6 || date.month_ == 9 || date.month_ == 11) {
          date.day_ = 30 - difference;
        } else if (date.month_ == 2) {
          if (date.year_ % 4 == 0) {
            date.day_ = 29 - difference;
          } else {
            date.day_ = 28 - difference;
          }
        }
      }
    } else {
      date.day_ = date.day_ - daysCount;
    }

    return date;
  }

  Date operator-(const Date& date) {
    Date result;
    result.year_ = this->year_ - date.year_;
    result.month_ = this->month_ - date.month_;
    result.day_ = this->day_ - date.day_;
    result.hour_ = this->hour_ - date.hour_;
    result.minute_ = this->minute_ - date.minute_;
    result.second_ = this->second_ - date.second_;
    return result;
  }

  bool operator<(const Date& date) const {
    if (this->year_ < date.year_) {
      return true;
    } else if (this->year_ == date.year_) {
      if (this->month_ < date.month_) {
        return true;
      } else if (this->month_ == date.month_) {
        if (this->day_ < date.day_) {
          return true;
        } else if (this->day_ == date.day_) {
          if (this->hour_ < date.hour_) {
            return true;
          } else if (this->hour_ == date.hour_) {
            if (this->minute_ < date.minute_) {
              return true;
            } else if (this->minute_ == date.minute_) {
              return (this->second_ < date.second_);
            }
          }
        }
      }
    }

    return false;
  }

  bool operator>(const Date& date) const { return date < *this; }

  bool operator>=(const Date& date) const {
    if (*this > date) {
      return true;
    }

    if (*this == date) {
      return true;
    }

    return false;
  }

  bool operator<=(const Date& date) const { return date >= *this; }
};

}  // namespace common
}  // namespace auto_trader

#endif  // DATA_DATE_H
