// Project includes
#include "../../../include/ISgfcSgfContent.h"
#include "../../../include/SgfcConstants.h"
#include "../../SgfcPrivateConstants.h"
#include "../message/SgfcMessage.h"
#include "../message/SgfcMessageParser.h"
#include "../message/SgfcMessageStream.h"
#include "../save/SgfcSaveStream.h"
#include "SgfcBackendController.h"

// SGFC includes
extern "C"
{
  #include "../../../sgfc/all.h"
  #include "../../../sgfc/protos.h"
}

// C++ Standard Library includes
#include <algorithm>
#include <fstream>
#include <mutex>
#include <stdexcept>

namespace LibSgfcPlusPlus
{
  static std::mutex sgfcMutex;

  SgfcBackendController::SgfcBackendController()
    : invalidCommandLineReason(nullptr)
  {
    std::lock_guard sgfcGuard(sgfcMutex);

    ParseArguments(this->arguments);
  }

  SgfcBackendController::SgfcBackendController(const std::vector<std::shared_ptr<ISgfcArgument>>& arguments)
    : arguments(arguments)
    , invalidCommandLineReason(nullptr)
  {
    std::lock_guard sgfcGuard(sgfcMutex);

    ParseArguments(arguments);
  }

  SgfcBackendController::~SgfcBackendController()
  {
  }

  std::vector<std::shared_ptr<ISgfcArgument>> SgfcBackendController::GetArguments() const
  {
    return this->arguments;
  }

  bool SgfcBackendController::IsCommandLineValid() const
  {
    if (this->invalidCommandLineReason == nullptr)
      return true;
    else
      return false;
  }

  std::shared_ptr<ISgfcMessage> SgfcBackendController::GetInvalidCommandLineReason() const
  {
    ThrowIfIsCommandLineValidReturnsTrue();

    return this->invalidCommandLineReason;
  }

  std::shared_ptr<SgfcBackendLoadResult> SgfcBackendController::LoadSgfFile(const std::string& sgfFilePath)
  {
    std::string sgfContent;
    return LoadSgfContentFromFilesystemOrInMemoryBuffer(sgfFilePath, sgfContent, SgfcDataLocation::Filesystem);
  }

  std::shared_ptr<SgfcBackendLoadResult> SgfcBackendController::LoadSgfContent(const std::string& sgfContent)
  {
    std::string sgfFilePath;
    return LoadSgfContentFromFilesystemOrInMemoryBuffer(sgfFilePath, sgfContent, SgfcDataLocation::InMemoryBuffer);
  }

  std::shared_ptr<SgfcBackendSaveResult> SgfcBackendController::SaveSgfFile(
    const std::string& sgfFilePath,
    std::shared_ptr<SgfcBackendDataWrapper> sgfDataWrapper)
  {
    std::string sgfContent;
    return SaveSgfContentToFilesystemOrInMemoryBuffer(sgfFilePath, sgfContent, sgfDataWrapper, SgfcDataLocation::Filesystem);
  }

  std::shared_ptr<SgfcBackendSaveResult> SgfcBackendController::SaveSgfContent(
    std::string& sgfContent,
    std::shared_ptr<SgfcBackendDataWrapper> sgfDataWrapper)
  {
    std::string sgfFilePath;
    return SaveSgfContentToFilesystemOrInMemoryBuffer(sgfFilePath, sgfContent, sgfDataWrapper, SgfcDataLocation::InMemoryBuffer);
  }

  void SgfcBackendController::ParseArguments(const std::vector<std::shared_ptr<ISgfcArgument>>& arguments)
  {
    ResetGlobalVariables();

    SetInvalidCommandLineReasonIfSgfcFailsToParseArguments(arguments);
    if (! this->IsCommandLineValid())
      return;

    // Capture the changed SGFC option values so that we can re-apply them
    // later on when we perform a load or save operation. There's no need for
    // doing that if the command line arguments are not valid, because in that
    // case a client is not allowed to perform load or save operations.
    this->sgfcOptions.CaptureOptions();
  }

  void SgfcBackendController::SetInvalidCommandLineReasonIfSgfcFailsToParseArguments(const std::vector<std::shared_ptr<ISgfcArgument>>& arguments)
  {
    std::vector<std::string> argvArguments = ConvertArgumentsToArgvStyle(arguments);

    int argc = static_cast<int>(argvArguments.size());
    const char** argv = new const char*[argc];

    try
    {
      InitializeArgv(argv, argvArguments);

      InvokeSgfcParseArgs(argc, argv);
    }
    catch (...)
    {
      delete[] argv;
      throw;
    }

    delete[] argv;
  }

  std::vector<std::string> SgfcBackendController::ConvertArgumentsToArgvStyle(const std::vector<std::shared_ptr<ISgfcArgument>>& arguments) const
  {
    std::vector<std::string> argvArguments =
    {
      // argv[0] in a main() function is the program name
      SgfcConstants::LibraryName,
    };

    for (auto argument : arguments)
    {
      argvArguments.push_back(argument->ToString());
    }

    return argvArguments;
  }

  void SgfcBackendController::InitializeArgv(const char** argv, const std::vector<std::string>& argvArguments) const
  {
    for (unsigned int indexOfArgument = 0; indexOfArgument < argvArguments.size(); indexOfArgument++)
    {
      const char* arg = argvArguments[indexOfArgument].c_str();

      // It is safe to keep the pointer to the internal string buffer as long
      // as we don't change the string content and as long as the vector of
      // strings does not change and remains in scope.
      argv[indexOfArgument] = arg;
    }
  }

  void SgfcBackendController::InvokeSgfcParseArgs(int argc, const char** argv)
  {
    if (argc == 1)
    {
      // Don't call ParseArgs() if there are no arguments, because that causes
      // ParseArgs() to print a usage line to stdout.
    }
    else
    {
      try
      {
        // It is safe to remove const'ness because we know that ParseArgs()
        // won't change any of the char buffers.
        ParseArgs(argc, const_cast<char**>(argv));

        // In theory ParseArgs() can return TRUE or FALSE. Practically we
        // removed all possibilities for it to return FALSE, so we don't have
        // to check the return value. ParseArgs() returns FALSE in two cases:
        // - If no argument is specified. We excluded this with the initial
        //   argc == 1 check.
        // - If an argument is specified that requires SGFC to exit without
        //   error. These are -h, --help and --version. These are all in the
        //   list of banned arguments.
      }
      catch (std::runtime_error&)
      {
        std::vector<std::shared_ptr<ISgfcMessage>> parseArgsResult = GetMessageStreamResult();
        SetInvalidCommandLineReasonFromParseArgsResults(parseArgsResult);
      }
    }
  }

  std::shared_ptr<SgfcBackendLoadResult> SgfcBackendController::LoadSgfContentFromFilesystemOrInMemoryBuffer(
    const std::string& sgfFilePath,
    const std::string& sgfContent,
    SgfcDataLocation dataLocation)
  {
    ThrowIfIsCommandLineValidReturnsFalse();

    std::lock_guard sgfcGuard(sgfcMutex);

    // Reset global variables, then re-apply the outcome of ParseArgs() so that
    // SGFC behaves the same on each invocation.
    ResetGlobalVariables();
    this->sgfcOptions.RestoreOptions();

    std::shared_ptr<SgfcBackendDataWrapper> sgfDataWrapper;
    if (dataLocation == SgfcDataLocation::Filesystem)
    {
      // It is safe to keep the pointer to the internal string buffer as long
      // as the string remains in scope and we don't change the string content.
      // It is safe to remove const'ness because we know that LoadSGF() and
      // ParseSGF() won't change the char buffer.
      option_infile = const_cast<char*>(sgfFilePath.c_str());

      sgfDataWrapper =
        std::shared_ptr<SgfcBackendDataWrapper>(new SgfcBackendDataWrapper());

      // Prepare the SGFInfo struct for LoadSGF()
      sgfDataWrapper->GetSgfData()->name = option_infile;
    }
    else
    {
      sgfDataWrapper =
        std::shared_ptr<SgfcBackendDataWrapper>(new SgfcBackendDataWrapper(sgfContent));
    }

    try
    {
      // All three of the following functions set the global variable sgfc as a
      // side effect
      if (dataLocation == SgfcDataLocation::Filesystem)
        LoadSGF(sgfDataWrapper->GetSgfData());
      else
        LoadSGFFromFileBuffer(sgfDataWrapper->GetSgfData());
      ParseSGF(sgfDataWrapper->GetSgfData());
    }
    catch (std::runtime_error&)
    {
      // Handle the exception. The SGFC message stream should now hold a
      // fatal error message that we get access to after
      // GetMessageStreamResult().
    }

    std::vector<std::shared_ptr<ISgfcMessage>> parseSgfResult = GetMessageStreamResult();

    sgfDataWrapper->SetDataState(SgfcBackendDataState::FullyLoaded);

    std::shared_ptr<SgfcBackendLoadResult> backendLoadResult =
      std::shared_ptr<SgfcBackendLoadResult>(new SgfcBackendLoadResult(parseSgfResult, sgfDataWrapper));
    return backendLoadResult;
  }

  std::shared_ptr<SgfcBackendSaveResult> SgfcBackendController::SaveSgfContentToFilesystemOrInMemoryBuffer(
    const std::string& sgfFilePath,
    std::string& sgfContent,
    std::shared_ptr<SgfcBackendDataWrapper> sgfDataWrapper,
    SgfcDataLocation dataLocation)
  {
    ThrowIfIsCommandLineValidReturnsFalse();

    std::lock_guard sgfcGuard(sgfcMutex);

    // Reset global variables, then re-apply the outcome of ParseArgs() so that
    // SGFC behaves the same on each invocation.
    ResetGlobalVariables();
    this->sgfcOptions.RestoreOptions();

    bool parseSgfWasSuccessful = true;

    if (sgfDataWrapper->GetDataState() == SgfcBackendDataState::PartiallyLoaded)
    {
      try
      {
        // Both of the following functions set the global variable sgfc as a
        // side effect
        LoadSGFFromFileBuffer(sgfDataWrapper->GetSgfData());
        ParseSGF(sgfDataWrapper->GetSgfData());
      }
      catch (std::runtime_error&)
      {
        // Handle the exception. The SGFC message stream should now hold a
        // fatal error message that we get access to after
        // GetMessageStreamResult().
        parseSgfWasSuccessful = false;
      }

      sgfDataWrapper->SetDataState(SgfcBackendDataState::FullyLoaded);
    }

    // Don't attempt to save if parsing was not successful
    // TODO: Currently we only skip saving if a fatal error occurred. Shouldn't
    // we skip saving also for critical errors?
    if (parseSgfWasSuccessful)
    {
      // Setting up option_outfile and SGFInfo.name (the next two statements)
      // is necessary even for SgfcDataLocation::InMemoryBuffer where we dont
      // actually interact with the filesystem. Reason: Even the patched version
      // of SaveSGF() expects to receive a file name. SaveSGF() can handle an
      // empty file name.

      // It is safe to keep the pointer to the internal string buffer as long
      // as the string remains in scope and we don't change the string content.
      // It is safe to remove const'ness because we know that SaveSGF() won't
      // change the char buffer.
      option_outfile = const_cast<char*>(sgfFilePath.c_str());

      // Prepare the SGFInfo struct for SaveSGF()
      sgfDataWrapper->GetSgfData()->name = option_outfile;

      try
      {
        // The following function sets the global variable sgfc as a side effect
        SaveSGF(sgfDataWrapper->GetSgfData());
      }
      catch (std::runtime_error&)
      {
        // Handle the exception. The SGFC message stream should now hold a
        // fatal error message that we get access to after
        // GetMessageStreamResult().
      }
    }

    std::vector<std::shared_ptr<ISgfcSgfContent>> sgfContents = GetSaveStreamResult();
    std::vector<std::shared_ptr<ISgfcMessage>> saveSgfResult = GetMessageStreamResult();

    if (dataLocation == SgfcDataLocation::InMemoryBuffer)
      sgfContent = std::string();

    for (auto sgfContentLoop : sgfContents)
    {
      if (dataLocation == SgfcDataLocation::Filesystem)
      {
        bool success = SaveSgfContentToFilesystem(sgfContentLoop);
        if (! success)
        {
          std::string messageString = "Writing SGF file failed: " + sgfContentLoop->GetFileName();

          auto message = std::shared_ptr<ISgfcMessage>(new SgfcMessage(
            SgfcConstants::SaveSgfContentToFilesystemErrorMessageID,
            messageString));

          saveSgfResult.push_back(message);
        }
      }
      else
      {
        sgfContent += sgfContentLoop->GetSgfContent();
      }
    }

    std::shared_ptr<SgfcBackendSaveResult> backendSaveResult =
      std::shared_ptr<SgfcBackendSaveResult>(new SgfcBackendSaveResult(saveSgfResult));
    return backendSaveResult;
  }

  void SgfcBackendController::ResetGlobalVariables()
  {
    ResetGlobalVariablesInMain();
    ResetGlobalVariablesInSave();
    ResetGlobalVariablesInProperties();
    ResetGlobalVariablesInParse2();
    ResetGlobalVariablesInExecute();
    ResetGlobalVariablesInUtil();
  }

  std::vector<std::shared_ptr<ISgfcMessage>> SgfcBackendController::GetMessageStreamResult() const
  {
    std::vector<std::shared_ptr<ISgfcMessage>> result;

    SgfcMessageStream messageStream;
    std::vector<std::string> messageStreamLines = messageStream.GetMessageStreamLines();

    for (const auto& messageStreamLine : messageStreamLines)
    {
      std::shared_ptr<ISgfcMessage> message = SgfcMessageParser::CreateSgfcMessage(messageStreamLine);
      result.push_back(message);
    }

    return result;
  }

  std::vector<std::shared_ptr<ISgfcSgfContent>> SgfcBackendController::GetSaveStreamResult() const
  {
    SgfcSaveStream saveStream;
    return saveStream.GetSgfContents();
  }

  bool SgfcBackendController::SaveSgfContentToFilesystem(std::shared_ptr<ISgfcSgfContent> sgfContent) const
  {
    std::string fileName = sgfContent->GetFileName();

    std::ofstream out(fileName);
    if (out.fail())
      return false;  // SGFC generates fatal error FE_DEST_FILE_OPEN for this

    out << sgfContent->GetSgfContent();
    out.close();

    // Because of buffering we don't check immediately after outputting the
    // SGF content. Closing the stream flushes it, so afterwards the fail bit
    // is guaranteed to be set if an error occurred.
    if (out.fail())
      return false;  // SGFC generates fatal error FE_DEST_FILE_WRITE for this

    return true;
  }

  void SgfcBackendController::SetInvalidCommandLineReasonFromParseArgsResults(
    const std::vector<std::shared_ptr<ISgfcMessage>>& parseArgsResult)
  {
    for (const auto& message : parseArgsResult)
    {
      if (message->GetMessageType() == SgfcMessageType::FatalError)
      {
        this->invalidCommandLineReason = message;
        return;
      }
    }

    // This should not happen. If it does there was an error parsing the
    // message text.
    this->invalidCommandLineReason = std::shared_ptr<ISgfcMessage>(new SgfcMessage(
      SgfcConstants::ParseArgumentErrorMessageID,
      "SGFC failed to parse the specified arguments"));
  }

  void SgfcBackendController::ThrowIfIsCommandLineValidReturnsTrue() const
  {
    if (this->IsCommandLineValid())
      throw std::logic_error("Interface protocol violation: IsCommandLineValid() returns true");
  }

  void SgfcBackendController::ThrowIfIsCommandLineValidReturnsFalse() const
  {
    if (! this->IsCommandLineValid())
      throw std::logic_error("Interface protocol violation: IsCommandLineValid() returns false");
  }
}
