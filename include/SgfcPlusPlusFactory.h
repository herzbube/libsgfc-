#pragma once

// Project includes
#include "ISgfcCommandLine.h"
#include "ISgfcDocumentReader.h"
#include "ISgfcDocumentWriter.h"

// C++ Standard Library includes
#include <memory>
#include <string>
#include <vector>

namespace LibSgfcPlusPlus
{
  // Forward declarations
  class ISgfcCommandLine;
  class ISgfcPropertyFactory;
  class ISgfcPropertyValueFactory;

  /// @brief The SgfcPlusPlusFactory class is a class that contains only static
  /// functions. The functions are used to construct various library objects.
  class SgfcPlusPlusFactory
  {
  public:
    /// @brief Returns a newly constructed ISgfcCommandLine object that passes
    /// the specified command line arguments to SGFC.
    ///
    /// @param arguments The command line arguments to pass to SGFC, exactly
    /// as you would specify them on the command line. Invoke the
    /// ISgfcCommandLine::IsCommandLineValid() method on the resulting object
    /// to find out whether the arguments are valid.
    ///
    /// @todo Specify arguments with types instead of with std::string.
    static std::shared_ptr<ISgfcCommandLine> CreateSgfcCommandLine(
      const std::vector<std::string>& arguments);

    /// @brief Returns a newly constructed ISgfcCommandLine object that passes
    /// no command line arguments to SGFC.
    static std::shared_ptr<ISgfcCommandLine> CreateSgfcCommandLine();

    /// @brief Returns a newly constructed ISgfcDocumentReader object.
    static std::shared_ptr<ISgfcDocumentReader> CreateDocumentReader();

    /// @brief Returns a newly constructed ISgfcDocumentWriter object.
    static std::shared_ptr<ISgfcDocumentWriter> CreateDocumentWriter();

    /// @brief Returns a newly constructed ISgfcDocument object. The
    /// ISgfcDocument is empty and contains no games.
    static std::shared_ptr<ISgfcDocument> CreateDocument();

    /// @brief Returns a newly constructed ISgfcDocument object. The
    /// ISgfcDocument content consists of the specified game @a game. The
    /// ISgfcDocument object takes ownership of @a game.
    static std::shared_ptr<ISgfcDocument> CreateDocument(std::shared_ptr<ISgfcGame> game);

    /// @brief Returns a newly constructed ISgfcGame object. The game type is
    /// SgfcGameType::Unknown. The game is not associated with any document.
    /// The game has no game tree.
    static std::shared_ptr<ISgfcGame> CreateGame();

    /// @brief Returns a newly constructed ISgfcGame object. The game tree's
    /// root node is @a rootNode. The game type depends on the content of the
    /// root node. The game is not associated with any document. The ISgfcGame
    /// object takes ownership of @a rootNode.
    static std::shared_ptr<ISgfcGame> CreateGame(std::shared_ptr<ISgfcNode> rootNode);

    /// @brief Returns a newly constructed ISgfcNode object that has no parent,
    /// child or sibling and is not associated with any game.
    static std::shared_ptr<ISgfcNode> CreateNode();

    /// @brief Returns a newly constructed ISgfcNode object that is a child node
    /// of @a parentNode. The node is appended as the last child to @a parent.
    static std::shared_ptr<ISgfcNode> CreateChildNode(std::shared_ptr<ISgfcNode> parentNode);


    /// @brief Returns a newly constructed ISgfcPropertyFactory object
    /// that can be used to create ISgfcProperty objects, and objects of every
    /// known sub-type of ISgfcProperty.
    static std::shared_ptr<ISgfcPropertyFactory> CreatePropertyFactory();

    /// @brief Returns a newly constructed ISgfcPropertyValueFactory object
    /// that can be used to create objects of every known sub-type of
    /// ISgfcPropertyValue.
    static std::shared_ptr<ISgfcPropertyValueFactory> CreatePropertyValueFactory();

  private:
    SgfcPlusPlusFactory();
    ~SgfcPlusPlusFactory();
  };
}
