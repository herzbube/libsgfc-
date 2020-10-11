// -----------------------------------------------------------------------------
// Copyright 2020 Patrick Näf (herzbube@herzbube.ch)
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
// http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
// -----------------------------------------------------------------------------

// Project includes
#include "../../../include/SgfcConstants.h"
#include "SgfcMessage.h"

// C++ Standard Library includes
#include <stdexcept>

namespace LibSgfcPlusPlus
{
  SgfcMessage::SgfcMessage(
    int messageID,
    SgfcMessageType messageType,
    int lineNumber,
    int columnNumber,
    bool isCriticalMessage,
    const std::string& messageText,
    const std::string& rawMessageText)
    : messageID(messageID)
    , messageType(messageType)
    , lineNumber(lineNumber)
    , columnNumber(columnNumber)
    , isCriticalMessage(isCriticalMessage)
    , messageText(messageText)
    , rawMessageText(rawMessageText)
  {
    if (messageID < 0)
      throw std::invalid_argument("SgfcMessage constructor failed: Message ID is a negative number. This constructor is reserved for messages generated by SGFC.");

    if (messageType == SgfcMessageType::FatalError)
    {
      if (isCriticalMessage)
        throw std::invalid_argument("SgfcMessage constructor failed: Message is marked as critical although message type is SgfcMessageType::FatalError");
    }
  }

  SgfcMessage::SgfcMessage(
    int messageID,
    const std::string& messageText)
    : messageID(messageID)
    , messageType(SgfcMessageType::FatalError)
    , lineNumber(SgfcConstants::InvalidLineNumber)
    , columnNumber(SgfcConstants::InvalidColumnNumber)
    , isCriticalMessage(false)
    , messageText(messageText)
    , rawMessageText(messageText)
  {
    if (messageID >= 0)
      throw std::invalid_argument("SgfcMessage constructor failed: Message ID is zero or a positive number. This constructor is reserved for messages generated by libsgfc++.");
  }

  SgfcMessage::~SgfcMessage()
  {
  }

  int SgfcMessage::GetMessageID() const
  {
    return this->messageID;
  }

  SgfcMessageType SgfcMessage::GetMessageType() const
  {
    return this->messageType;
  }

  int SgfcMessage::GetLineNumber() const
  {
    return this->lineNumber;
  }

  int SgfcMessage::GetColumnNumber() const
  {
    return this->columnNumber;
  }

  bool SgfcMessage::IsCriticalMessage() const
  {
    return this->isCriticalMessage;
  }

  std::string SgfcMessage::GetMessageText() const
  {
    return this->messageText;
  }

  std::string SgfcMessage::GetRawMessageText() const
  {
    return this->rawMessageText;
  }
}
