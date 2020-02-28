#pragma once

// C++ Standard Library includes
#include <string>
#include <vector>

namespace LibSgfcPlusPlus
{
  /// @brief The SgfcMessageStream class stores the message stream that was
  /// generated by SGFC during its most recent parsing of SGF content.
  ///
  /// SgfcMessageStream acquires the message stream immediately upon
  /// construction, then clears the message stream in SGFC. This means
  /// that if two SgfcMessageStream objects are constructed in a row, the second
  /// object will see an empty message stream.
  class SgfcMessageStream
  {
  public:
    SgfcMessageStream();
    virtual ~SgfcMessageStream();

    /// @brief Returns the message stream content that SgfcMessageStream
    /// acquired during construction, split into lines.
    std::vector<std::string> GetMessageStreamLines() const;

    /// @brief Returns the raw message stream content that SgfcMessageStream
    /// acquired during construction.
    std::string GetRawMessageStreamContent() const;

  private:
    std::vector<std::string> messageStreamLines;
    std::string rawMessageStreamContent;

    void SplitRawMessageStreamContentIntoLines();
  };
}