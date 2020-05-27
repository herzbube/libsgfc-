#pragma once

// Project includes
#include "../../../include/ISgfcMessage.h"

namespace LibSgfcPlusPlus
{
  /// @brief The SgfcMessage class provides an implementation of the
  /// ISgfcMessage interface. See the interface header file for
  /// documentation.
  class SgfcMessage : public ISgfcMessage
  {
  public:
    /// @brief Initializes a newly constructed SgfcMessage object with the
    /// supplied parameter values. The message ID must be zero or a positive
    /// number.
    ///
    /// This constructor is intended to be used for messages that are
    /// generated by SGFC.
    ///
    /// @exception std::invalid_argument Is thrown if @a messageID is not zero
    /// or a positive number. Is thrown if @a messageType is
    /// SgfcMessageType::FatalError and @e lineNumber or @e columnNumber are not
    /// SgfcConstants::InvalidLineNumber or SgfcConstants::InvalidColumnNumber,
    /// respectively. Is thrown if the message type is
    /// SgfcMessageType::FatalError and @e isCriticalMessage is true.
    ///
    /// @note The exceptions enforce the details specified in the ISgfcMessage
    /// interface documentation.
    SgfcMessage(
      int messageID,
      SgfcMessageType messageType,
      int lineNumber,
      int columnNumber,
      bool isCriticalMessage,
      const std::string& messageText,
      const std::string& rawMessageText);

    /// @brief Initializes a newly constructed SgfcMessage object with the
    /// supplied parameter values. The SgfcMessage has message type
    /// SgfcMessageType::FatalError. The message ID must be a negative number.
    ///
    /// This constructor is intended to be used for messages that are
    /// generated by libsgfc++.
    ///
    /// @exception std::invalid_argument Is thrown if @a messageID is not a
    /// negative number
    SgfcMessage(
      int messageID,
      const std::string& messageText);

    /// @brief Destroys and cleans up the ISgfcMessage object.
    virtual ~SgfcMessage();
  
    virtual int GetMessageID() const;

    virtual SgfcMessageType GetMessageType() const;

    virtual int GetLineNumber() const;
    virtual int GetColumnNumber() const;

    virtual bool IsCriticalMessage() const;

    virtual std::string GetMessageText() const;
    virtual std::string GetRawMessageText() const;

  private:
    int messageID;
    SgfcMessageType messageType;
    int lineNumber;
    int columnNumber;
    bool isCriticalMessage;
    std::string messageText;
    std::string rawMessageText;
  };
}
