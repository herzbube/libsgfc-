// Project includes
#include "../../SgfcConstants.h"
#include "SgfcMessageStream.h"

// C++ Standardd Library includes
#include <sstream>

namespace LibSgfcPlusPlus
{
  // ----------------------------------------------------------------------
  // Function prototypes used to interact with SGFC.
  // ----------------------------------------------------------------------

  /// @brief Acquires the message stream that was generated by SGFC during its
  /// most recent parsing of SGF content.
  ///
  /// This is a function prototype. The function is defined in SGFC, in the
  /// modified util.cpp.
  std::string AcquireSgfcMessageStream();

  /// @brief Resets the message stream stored in SGFC so that SGFC starts with
  /// a clean slate when it parses SGF content the next time.
  ///
  /// This is a function prototype. The function is defined in SGFC, in the
  /// modified util.cpp.
  void ResetSgfcMessageStream();

  // ----------------------------------------------------------------------
  // The SgfcMessageStream class.
  // ----------------------------------------------------------------------

  SgfcMessageStream::SgfcMessageStream()
    : rawMessageStreamContent(SgfcConstants::EmptyString)
    , messageStreamLines()
  {
    // TODO: Add multi-threading protection.
    // TODO: Ensure that nobody is stealing the stream content from an SGFC controller.

    this->rawMessageStreamContent = AcquireSgfcMessageStream();
    SplitRawMessageStreamContentIntoLines();

    ResetSgfcMessageStream();
  }

  SgfcMessageStream::~SgfcMessageStream()
  {
  }

  void SgfcMessageStream::SplitRawMessageStreamContentIntoLines()
  {
    std::stringstream messageStream(this->rawMessageStreamContent);
    std::string line;

    while (std::getline(messageStream, line, SgfcConstants::NewlineCharacter))
    {
      this->messageStreamLines.push_back(line);
    }
  }

  std::vector<std::string> SgfcMessageStream::GetMessageStreamLines() const
  {
    return this->messageStreamLines;
  }

  std::string SgfcMessageStream::GetRawMessageStreamContent() const
  {
    return this->rawMessageStreamContent;
  }
}