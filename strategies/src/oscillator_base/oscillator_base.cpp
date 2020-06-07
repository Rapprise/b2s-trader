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

#include "include/oscillator_base/oscillator_base.h"

#include "resources/resources.h"

namespace auto_trader {
namespace strategies {

void OscillatorBase::setCrossingInterval(size_t interval) { crossingInterval_ = interval; }

void OscillatorBase::crossingToSellSignal() {
  auto quickLineSize = mainLine_.getSize() - resources::sizeOffset;
  auto slowLineSize = signalOscillatorLine_.getSize() - resources::sizeOffset;

  auto fromQuickLinePoint = mainLine_.getPoint(quickLineSize - resources::sizeOffset);
  auto toQuickLinePoint = mainLine_.getLastPoint();

  auto fromSlowLinePoint = signalOscillatorLine_.getPoint(slowLineSize - resources::sizeOffset);
  auto toSlowLinePoint = signalOscillatorLine_.getLastPoint();

  if (fromQuickLinePoint > fromSlowLinePoint && toQuickLinePoint < toSlowLinePoint) {
    bool isDuplicateCrossing = isSellCrossingDuplicatedOnInterval(quickLineSize);
    bool isCrossingTopBound = checkTopBound(quickLineSize);
    if (!isDuplicateCrossing && isCrossingTopBound) {
      lastSellCrossingPoint_ = mainLine_.getPoint(quickLineSize);
      crossingForSellSignal_.second = true;
    } else {
      crossingForSellSignal_.second = false;
    }

  } else {
    crossingForSellSignal_.second = false;
  }
}

void OscillatorBase::crossingToBuySignal() {
  auto quickLineSize = mainLine_.getSize() - resources::sizeOffset;
  auto slowLineSize = signalOscillatorLine_.getSize() - resources::sizeOffset;

  auto fromQuickLinePoint = mainLine_.getPoint(quickLineSize - resources::sizeOffset);
  auto toQuickLinePoint = mainLine_.getLastPoint();

  auto fromSlowLinePoint = signalOscillatorLine_.getPoint(slowLineSize - resources::sizeOffset);
  auto toSlowLinePoint = signalOscillatorLine_.getLastPoint();

  if (fromQuickLinePoint < fromSlowLinePoint && toQuickLinePoint > toSlowLinePoint) {
    bool isDuplicateCrossing = isBuyCrossingDuplicatedOnInterval(quickLineSize);
    bool isCrossingBottomBound = checkBottomBound(quickLineSize);
    if (!isDuplicateCrossing && isCrossingBottomBound) {
      lastBuyCrossingPoint_ = mainLine_.getPoint(quickLineSize);
      crossingForBuySignal_.second = true;
    } else {
      crossingForBuySignal_.second = false;
    }

  } else {
    crossingForBuySignal_.second = false;
  }
}

bool OscillatorBase::isBuyCrossingDuplicatedOnInterval(const int &lineSize) {
  for (int index = lineSize; index >= lineSize - crossingInterval_; --index) {
    auto currentPoint = mainLine_.getPoint(index);
    if (currentPoint == lastBuyCrossingPoint_) {
      return true;
    }
  }

  return false;
}

bool OscillatorBase::isSellCrossingDuplicatedOnInterval(const int &lineSize) {
  for (int index = lineSize; index >= lineSize - crossingInterval_; --index) {
    auto currentPoint = mainLine_.getPoint(index);
    if (currentPoint == lastSellCrossingPoint_) {
      return true;
    }
  }

  return false;
}

bool OscillatorBase::checkTopBound(int lineSize) const { return true; }

bool OscillatorBase::checkBottomBound(int lineSize) const { return true; }

}  // namespace strategies
}  // namespace auto_trader