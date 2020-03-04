#pragma once

// Project includes
#include "../../../include/ISgfcDocumentWriter.h"
#include "../backend/SgfcBackendController.h"

namespace LibSgfcPlusPlus
{
  /// @brief The SgfcDocumentWriter class provides an implementation of the
  /// ISgfcDocumentWriter interface. See the interface header file for
  /// documentation.
  class SgfcDocumentWriter : public ISgfcDocumentWriter
  {
  public:
    SgfcDocumentWriter();
    virtual ~SgfcDocumentWriter();

    virtual std::shared_ptr<ISgfcDocumentWriteResult> SaveSgfFile(
      std::shared_ptr<ISgfcDocument> document,
      const std::string& sgfFilePath);
    virtual std::shared_ptr<ISgfcDocumentWriteResult> SaveSgfContent(
      std::shared_ptr<ISgfcDocument> document,
      std::string& sgfContent);

  private:
    std::shared_ptr<SgfcBackendController> backendController;
  };
}
