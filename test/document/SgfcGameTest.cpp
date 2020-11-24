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

// Library includes
#include "../TestDataGenerator.h"
#include <document/SgfcGame.h>
#include <document/SgfcNode.h>
#include <document/SgfcProperty.h>
#include <document/SgfcTreeBuilder.h>
#include <document/typedproperty/SgfcBoardSizeProperty.h>
#include <document/typedproperty/SgfcGameTypeProperty.h>
#include <ISgfcComposedPropertyValue.h>
#include <ISgfcDoublePropertyValue.h>
#include <ISgfcNumberPropertyValue.h>
#include <ISgfcPropertyFactory.h>
#include <ISgfcPropertyValueFactory.h>
#include <SgfcConstants.h>
#include <SgfcPlusPlusFactory.h>
#include <SgfcUtility.h>

// Unit test library includes
#include <catch2/catch.hpp>

using namespace LibSgfcPlusPlus;


enum class InvalidValueTypeGameType
{
  MoreThanOneValue,
  ComposedValue,
  SingleValueTypeNotANumber,
};

enum class InvalidValueTypeBoardSize
{
  MoreThanOneValue,
  ComposedValueNotANumberAndNumber,
  SingleValueTypeNotANumber,
};

void SetupGameWithGameAndBoardSizeProperties(std::shared_ptr<SgfcGame> game, SgfcGameType gameType, SgfcBoardSize boardSize, bool setupWithTypedProperties);
void SetupGameWithBoardSizeProperty(std::shared_ptr<SgfcGame> game, SgfcBoardSize boardSize, bool setupWithTypedProperty);
void AddGameTypeProperty(SgfcGameType gameType, std::vector<std::shared_ptr<ISgfcProperty>>& properties, bool setupWithTypedProperty);
void AddGameTypeProperty(SgfcNumber gameTypeAsNumber, std::vector<std::shared_ptr<ISgfcProperty>>& properties, bool setupWithTypedProperty);
void AddGameTypePropertyWithInvalidValues(std::vector<std::shared_ptr<ISgfcProperty>>& properties, InvalidValueTypeGameType invalidValueType);
std::shared_ptr<ISgfcProperty> CreateGameTypeProperty(SgfcNumber gameTypeAsNumber, bool createTypedProperty);
void AddBoardSizeProperty(SgfcBoardSize boardSize, std::vector<std::shared_ptr<ISgfcProperty>>& properties, bool setupWithTypedProperty);
void AddBoardSizePropertyWithInvalidValues(std::vector<std::shared_ptr<ISgfcProperty>>& properties, InvalidValueTypeBoardSize invalidValueType);


SCENARIO( "SgfcGame is constructed", "[document]" )
{
  GIVEN( "The SgfcGame default constructor is used" )
  {
    WHEN( "SgfcGame is constructed" )
    {
      THEN( "SgfcGame is constructed successfully" )
      {
        REQUIRE_NOTHROW( SgfcGame() );
      }
    }

    WHEN( "SgfcGame is constructed" )
    {
      SgfcGame game;

      THEN( "SgfcGame has the expected default state" )
      {
        REQUIRE( game.GetGameType() == SgfcConstants::DefaultGameType );
        REQUIRE( game.GetGameTypeAsNumber() == SgfcUtility::MapGameTypeToNumberValue(SgfcConstants::DefaultGameType) );
        REQUIRE( game.HasBoardSize() == true );
        REQUIRE( game.GetBoardSize() == SgfcConstants::BoardSizeDefaultGo );
        REQUIRE( game.HasRootNode() == false );
        REQUIRE( game.GetRootNode() == nullptr );
        REQUIRE( game.GetGameInfoNodes().size() == 0 );
        REQUIRE( game.GetTreeBuilder() == nullptr );
      }
    }
  }

  GIVEN( "The SgfcGame constructor taking an SgfcNode parameter is used" )
  {
    WHEN( "SgfcGame is constructed with an SgfcNode object" )
    {
      std::shared_ptr<SgfcNode> rootNode = std::shared_ptr<SgfcNode>(new SgfcNode());

      THEN( "SgfcGame is constructed successfully" )
      {
        REQUIRE_NOTHROW( SgfcGame(rootNode) );
      }
    }

    WHEN( "SgfcGame is constructed with an SgfcNode object" )
    {
      std::shared_ptr<SgfcNode> rootNode = std::shared_ptr<SgfcNode>(new SgfcNode());
      SgfcGame game(rootNode);

      THEN( "SgfcGame has the expected state" )
      {
        REQUIRE( game.GetGameType() == SgfcConstants::DefaultGameType );
        REQUIRE( game.GetGameTypeAsNumber() == SgfcUtility::MapGameTypeToNumberValue(SgfcConstants::DefaultGameType) );
        REQUIRE( game.HasBoardSize() == true );
        REQUIRE( game.GetBoardSize() == SgfcConstants::BoardSizeDefaultGo );
        REQUIRE( game.HasRootNode() == true );
        REQUIRE( game.GetRootNode() == rootNode );
        REQUIRE( game.GetGameInfoNodes().size() == 0 );
        REQUIRE( game.GetTreeBuilder() == nullptr );
      }
    }

    WHEN( "SgfcGame is constructed with a nullptr SgfcNode object" )
    {
      std::shared_ptr<SgfcNode> rootNode = std::shared_ptr<SgfcNode>(nullptr);

      THEN( "The SgfcGame constructor throws an exception" )
      {
        REQUIRE_THROWS_AS(
          SgfcGame(rootNode),
          std::invalid_argument);
      }
    }
  }
}

SCENARIO( "SgfcGame is configured with an SgfcNode object", "[document]" )
{
  auto node = std::shared_ptr<SgfcNode>(new SgfcNode());
  auto secondNode = std::shared_ptr<SgfcNode>(new SgfcNode());
  auto nullNode = std::shared_ptr<SgfcNode>(nullptr);

  GIVEN( "The SgfcGame default constructor was used" )
  {
    SgfcGame game;

    WHEN( "SgfcGame is configured with an SgfcNode object" )
    {
      std::vector<std::shared_ptr<SgfcNode>> v = { node, nullNode };
      auto rootNode = GENERATE_COPY( from_range(v) );

      THEN( "The getter returns the SgfcNode object that was used in the setter" )
      {
        REQUIRE( game.HasRootNode() == false );
        REQUIRE( game.GetRootNode() == nullptr );
        game.SetRootNode(rootNode);
        REQUIRE( game.HasRootNode() == (rootNode != nullptr) );
        REQUIRE( game.GetRootNode() == rootNode );
      }
    }

    GIVEN( "The SgfcGame constructor taking an SgfcNode parameter was used" )
    {
      SgfcGame game(node);

      WHEN( "SgfcGame is configured with an SgfcNode object" )
      {
        // Include the same node that was used for construction
        std::vector<std::shared_ptr<SgfcNode>> v = { node, secondNode, nullNode };
        auto rootNode = GENERATE_COPY( from_range(v) );

        THEN( "The getter returns the SgfcNode object that was used in the setter" )
        {
          REQUIRE( game.GetRootNode() == node );
          game.SetRootNode(rootNode);
          REQUIRE( game.GetRootNode() == rootNode );
        }
      }
    }
  }
}

SCENARIO( "SgfcGame is configured with an SgfcTreeBuilder object", "[document]" )
{
  auto game = std::shared_ptr<SgfcGame>(new SgfcGame());
  auto treeBuilder = std::shared_ptr<SgfcTreeBuilder>(new SgfcTreeBuilder(game));
  auto nullTreeBuilder = std::shared_ptr<SgfcTreeBuilder>(nullptr);

  GIVEN( "SgfcGame is configured with an SgfcTreeBuilder object" )
  {
    WHEN( "SgfcGame is configured with an SgfcTreeBuilder object" )
    {
      std::vector<std::shared_ptr<SgfcTreeBuilder>> v = { treeBuilder, nullTreeBuilder };
      auto newTreeBuilder = GENERATE_COPY( from_range(v) );

      THEN( "The getter returns the SgfcTreeBuilder object that was used in the setter" )
      {
        REQUIRE( game->GetTreeBuilder() == nullptr );
        game->SetTreeBuilder(newTreeBuilder);
        REQUIRE( game->GetTreeBuilder() == newTreeBuilder );
      }
    }
  }
}

SCENARIO( "SgfcGame is queried for the game type", "[document]" )
{
  auto game = std::shared_ptr<SgfcGame>(new SgfcGame());
  auto node = std::shared_ptr<SgfcNode>(new SgfcNode());
  std::vector<std::shared_ptr<ISgfcProperty>> properties;

  auto propertyFactory = SgfcPlusPlusFactory::CreatePropertyFactory();
  auto propertyValueFactory = SgfcPlusPlusFactory::CreatePropertyValueFactory();

  GIVEN( "The game has no root node" )
  {
    WHEN( "SgfcGame is queried for the game type" )
    {
      THEN( "SgfcGame returns SgfcConstants::DefaultGameType" )
      {
        REQUIRE( game->GetGameType() == SgfcConstants::DefaultGameType );
        REQUIRE( game->GetGameTypeAsNumber() == SgfcUtility::MapGameTypeToNumberValue(SgfcConstants::DefaultGameType) );
      }
    }
  }

  GIVEN( "The game has a root node but there is no property of type SgfcPropertyType::GM" )
  {
    game->SetRootNode(node);

    WHEN( "SgfcGame is queried for the game type" )
    {
      THEN( "SgfcGame returns SgfcConstants::DefaultGameType" )
      {
        REQUIRE( game->GetGameType() == SgfcConstants::DefaultGameType );
        REQUIRE( game->GetGameTypeAsNumber() == SgfcUtility::MapGameTypeToNumberValue(SgfcConstants::DefaultGameType) );
      }
    }
  }

  GIVEN( "The game has a property of type SgfcPropertyType::GM but the property has no value" )
  {
    bool setupWithTypedProperty = GENERATE( true, false );

    std::shared_ptr<ISgfcProperty> gameTypePropertyWithoutValue;
    if (setupWithTypedProperty)
      gameTypePropertyWithoutValue = propertyFactory->CreateGameTypeProperty();
    else
      gameTypePropertyWithoutValue = propertyFactory->CreateProperty(SgfcPropertyType::GM);
    properties.push_back(gameTypePropertyWithoutValue);

    game->SetRootNode(node);
    node->SetProperties(properties);

    WHEN( "SgfcGame is queried for the game type" )
    {
      THEN( "SgfcGame returns SgfcConstants::DefaultGameType" )
      {
        REQUIRE( game->GetGameType() == SgfcConstants::DefaultGameType );
        REQUIRE( game->GetGameTypeAsNumber() == SgfcUtility::MapGameTypeToNumberValue(SgfcConstants::DefaultGameType) );
      }
    }
  }

  GIVEN( "The game has a property of type SgfcPropertyType::GM and the property value is not defined in the SGF standard" )
  {
    bool setupWithTypedProperty = GENERATE( true, false );

    SgfcNumber gameTypeAsNumberNotDefinedInSgfStandard = 1000;
    AddGameTypeProperty(gameTypeAsNumberNotDefinedInSgfStandard, properties, setupWithTypedProperty);

    game->SetRootNode(node);
    node->SetProperties(properties);

    WHEN( "SgfcGame is queried for the game type" )
    {
      THEN( "SgfcGame returns SgfcGameType::Unknown" )
      {
        REQUIRE( game->GetGameType() == SgfcGameType::Unknown );
        REQUIRE( game->GetGameTypeAsNumber() == gameTypeAsNumberNotDefinedInSgfStandard );
      }
    }
  }

  GIVEN( "The game has a property of type SgfcPropertyType::GM and the property value is defined in the SGF standard" )
  {
    bool setupWithTypedProperty = GENERATE( true, false );

    SgfcGameType gameTypeDefinedInSgfStandard = SgfcGameType::Kuba;
    AddGameTypeProperty(gameTypeDefinedInSgfStandard, properties, setupWithTypedProperty);

    game->SetRootNode(node);
    node->SetProperties(properties);

    WHEN( "SgfcGame is queried for the game type" )
    {
      THEN( "SgfcGame returns an SgfcGameType value that is not SgfcGameType::Unknown" )
      {
        REQUIRE( game->GetGameType() == gameTypeDefinedInSgfStandard );
        REQUIRE( game->GetGameTypeAsNumber() == SgfcUtility::MapGameTypeToNumberValue(gameTypeDefinedInSgfStandard) );
      }
    }
  }

  GIVEN( "The game has a property of type SgfcPropertyType::GM and the property value(s) cannot be converted to a Number value" )
  {
    InvalidValueTypeGameType setupWithInvalidValueType = GENERATE(
      InvalidValueTypeGameType::MoreThanOneValue,
      InvalidValueTypeGameType::ComposedValue,
      InvalidValueTypeGameType::SingleValueTypeNotANumber );

    AddGameTypePropertyWithInvalidValues(properties, setupWithInvalidValueType);

    game->SetRootNode(node);
    node->SetProperties(properties);

    WHEN( "SgfcGame is queried for the game type" )
    {
      THEN( "SgfcGame returns SgfcGameType::Unknown" )
      {
        REQUIRE( game->GetGameType() == SgfcGameType::Unknown );
        REQUIRE( game->GetGameTypeAsNumber() == SgfcConstants::GameTypeNaN );
      }
    }
  }
}

SCENARIO( "SgfcGame is queried for the board size", "[document]" )
{
  struct TestData1
  {
    std::shared_ptr<ISgfcProperty> GameTypeProperty;
    bool ExpectedHasBoardSize;
    SgfcBoardSize ExpectedBoardSize;
  };
  struct TestData2
  {
    SgfcBoardSize BoardSize;
    bool ExpectedHasBoardSize;
    SgfcBoardSize ExpectedBoardSize;
  };

  auto game = std::shared_ptr<SgfcGame>(new SgfcGame());
  auto node = std::shared_ptr<SgfcNode>(new SgfcNode());
  std::vector<std::shared_ptr<ISgfcProperty>> properties;

  auto propertyFactory = SgfcPlusPlusFactory::CreatePropertyFactory();
  auto propertyValueFactory = SgfcPlusPlusFactory::CreatePropertyValueFactory();

  auto gameTypePropertyValueGo = propertyValueFactory->CreateNumberPropertyValue(
    SgfcUtility::MapGameTypeToNumberValue(SgfcGameType::Go));
  auto gameTypePropertyGo = propertyFactory->CreateGameTypeProperty(
    gameTypePropertyValueGo);

  auto gameTypePropertyValueChess = propertyValueFactory->CreateNumberPropertyValue(
    SgfcUtility::MapGameTypeToNumberValue(SgfcGameType::Chess));
  auto gameTypePropertyChess = propertyFactory->CreateGameTypeProperty(
    gameTypePropertyValueChess);

  SgfcGameType gameTypeNeitherGoNorChess = SgfcGameType::Hnefatal;
  auto gameTypePropertyValueNeitherGoNorChess = propertyValueFactory->CreateNumberPropertyValue(
    SgfcUtility::MapGameTypeToNumberValue(gameTypeNeitherGoNorChess));
  auto gameTypePropertyNeitherGoNorChess = propertyFactory->CreateGameTypeProperty(
    gameTypePropertyValueNeitherGoNorChess);

  GIVEN( "The game has no root node" )
  {
    WHEN( "SgfcGame is queried for the board size" )
    {
      THEN( "SgfcGame returns SgfcConstants::BoardSizeDefaultGo" )
      {
        REQUIRE( game->HasBoardSize() == true );
        REQUIRE( game->GetBoardSize() == SgfcConstants::BoardSizeDefaultGo );
      }
    }
  }

  GIVEN( "The game has a root node but no properties" )
  {
    game->SetRootNode(node);

    WHEN( "SgfcGame is queried for the board size" )
    {
      THEN( "SgfcGame returns SgfcConstants::BoardSizeDefaultGo" )
      {
        REQUIRE( game->HasBoardSize() == true );
        REQUIRE( game->GetBoardSize() == SgfcConstants::BoardSizeDefaultGo );
      }
    }
  }

  GIVEN( "The game has a root node with a property of type SgfcPropertyType::GM, but with no property of type SgfcPropertyType::SZ" )
  {
    auto testData = GENERATE_COPY(
      TestData1 { gameTypePropertyGo, true, SgfcConstants::BoardSizeDefaultGo },
      TestData1 { gameTypePropertyChess, true, SgfcConstants::BoardSizeDefaultChess },
      TestData1 { gameTypePropertyNeitherGoNorChess, false, SgfcConstants::BoardSizeNone }
    );

    game->SetRootNode(node);
    properties.push_back(testData.GameTypeProperty);
    node->SetProperties(properties);

    WHEN( "SgfcGame is queried for the board size" )
    {
      THEN( "SgfcGame returns a default board size that matches the game type" )
      {
        REQUIRE( game->HasBoardSize() == testData.ExpectedHasBoardSize );
        REQUIRE( game->GetBoardSize() == testData.ExpectedBoardSize );
      }
    }
  }

  GIVEN( "The game has a root node with a property of type SgfcPropertyType::GM and a property of type SgfcPropertyType::SZ without value" )
  {
    auto boardSizePropertyWithoutValue = propertyFactory->CreateBoardSizeProperty();
    properties.push_back(boardSizePropertyWithoutValue);

    auto testData = GENERATE_COPY(
      TestData1 { gameTypePropertyGo, true, SgfcConstants::BoardSizeDefaultGo },
      TestData1 { gameTypePropertyChess, true, SgfcConstants::BoardSizeDefaultChess },
      TestData1 { gameTypePropertyNeitherGoNorChess, false, SgfcConstants::BoardSizeNone }
    );
    properties.push_back(testData.GameTypeProperty);

    game->SetRootNode(node);
    node->SetProperties(properties);

    WHEN( "SgfcGame is queried for the board size" )
    {
      THEN( "SgfcGame returns a default board size that matches the game type" )
      {
        REQUIRE( game->HasBoardSize() == testData.ExpectedHasBoardSize );
        REQUIRE( game->GetBoardSize() == testData.ExpectedBoardSize );
      }
    }
  }

  GIVEN( "The game has a root node with a property of type SgfcPropertyType::GM and a property of type SgfcPropertyType::SZ with a valid value" )
  {
    bool setupWithTypedProperties = GENERATE( true, false );
    auto testData = GENERATE( from_range(TestDataGenerator::GetValidBoardSizes()) );

    SetupGameWithGameAndBoardSizeProperties(game, std::get<1>(testData), std::get<0>(testData), setupWithTypedProperties);

    WHEN( "SgfcGame is queried for the board size" )
    {
      THEN( "SgfcGame returns an valid SgfcBoardSize value" )
      {
        REQUIRE( game->HasBoardSize() == true );
        REQUIRE( game->GetBoardSize() == std::get<2>(testData) );
      }
    }
  }

  GIVEN( "The game has a root node with a property of type SgfcPropertyType::GM and a property of type SgfcPropertyType::SZ with an invalid value" )
  {
    bool setupWithTypedProperties = GENERATE( true, false );
    auto testData = GENERATE( from_range(TestDataGenerator::GetInvalidBoardSizes()) );

    SetupGameWithGameAndBoardSizeProperties(game, std::get<1>(testData), std::get<0>(testData), setupWithTypedProperties);

    WHEN( "SgfcGame is queried for the board size" )
    {
      THEN( "SgfcGame returns SgfcConstants::BoardSizeInvalid" )
      {
        REQUIRE( game->HasBoardSize() == false );
        REQUIRE( game->GetBoardSize() == SgfcConstants::BoardSizeInvalid );
      }
    }
  }

  GIVEN( "The game has a property of type SgfcPropertyType::SZ and the property value(s) cannot be converted to an SgfcBoardSize value" )
  {
    bool setupWithGameTypePropery = GENERATE( true, false );

    if (setupWithGameTypePropery)
      properties.push_back(gameTypePropertyGo);

    InvalidValueTypeBoardSize setupWithInvalidValueType = GENERATE(
      InvalidValueTypeBoardSize::MoreThanOneValue,
      InvalidValueTypeBoardSize::ComposedValueNotANumberAndNumber,
      InvalidValueTypeBoardSize::SingleValueTypeNotANumber );

    AddBoardSizePropertyWithInvalidValues(properties, setupWithInvalidValueType);

    game->SetRootNode(node);
    node->SetProperties(properties);

    WHEN( "SgfcGame is queried for the board size" )
    {
      THEN( "SgfcGame returns SgfcConstants::BoardSizeInvalid" )
      {
        REQUIRE( game->HasBoardSize() == false );
        REQUIRE( game->GetBoardSize() == SgfcConstants::BoardSizeInvalid );
      }
    }
  }

  GIVEN( "The game has a root node with no property of type SgfcPropertyType::GM and a property of type SgfcPropertyType::SZ without value" )
  {
    auto boardSizePropertyWithoutValue = propertyFactory->CreateBoardSizeProperty();
    properties.push_back(boardSizePropertyWithoutValue);

    game->SetRootNode(node);
    node->SetProperties(properties);

    WHEN( "SgfcGame is queried for the board size" )
    {
      THEN( "SgfcGame returns SgfcConstants::BoardSizeDefaultGo" )
      {
        REQUIRE( game->HasBoardSize() == true );
        REQUIRE( game->GetBoardSize() == SgfcConstants::BoardSizeDefaultGo );
      }
    }
  }

  GIVEN( "The game has a root node with no property of type SgfcPropertyType::GM and a property of type SgfcPropertyType::SZ with a valid value" )
  {
    bool setupWithTypedProperty = GENERATE( true, false );
    auto testData = GENERATE(
      filter(
        // This is a predicate that examines the elements in the from_range
        // generator and returns true for those elements that the filter should
        // let pass
        [](const std::tuple<SgfcBoardSize, SgfcGameType, SgfcBoardSize>& tuple)
        {
          // Because the property of type SgfcPropertyType::GM is missing, the
          // default game type is SgfcGameType::Go. Because of this we can only
          // let test data through that is intended for that game type.
          return std::get<1>(tuple) == SgfcGameType::Go;
        },
        from_range(TestDataGenerator::GetValidBoardSizes())
      )
    );

    SetupGameWithBoardSizeProperty(game, std::get<0>(testData), setupWithTypedProperty);

    WHEN( "SgfcGame is queried for the board size" )
    {
      THEN( "SgfcGame returns an valid SgfcBoardSize value" )
      {
        REQUIRE( game->HasBoardSize() == true );
        REQUIRE( game->GetBoardSize() == std::get<2>(testData) );
      }
    }
  }

  GIVEN( "The game has a root node with no property of type SgfcPropertyType::GM and a property of type SgfcPropertyType::SZ with an invalid value" )
  {
    bool setupWithTypedProperty = GENERATE( true, false );
    auto testData = GENERATE(
      filter(
        // This is a predicate that examines the elements in the from_range
        // generator and returns true for those elements that the filter should
        // let pass
        [](const std::tuple<SgfcBoardSize, SgfcGameType>& tuple)
        {
          // Because the property of type SgfcPropertyType::GM is missing, the
          // default game type is SgfcGameType::Go. Because of this we can only
          // let test data through that is intended for that game type.
          return std::get<1>(tuple) == SgfcGameType::Go;
        },
        from_range(TestDataGenerator::GetInvalidBoardSizes())
      )
    );

    SetupGameWithBoardSizeProperty(game, std::get<0>(testData), setupWithTypedProperty);

    WHEN( "SgfcGame is queried for the board size" )
    {
      THEN( "SgfcGame returns an valid SgfcBoardSize value" )
      {
        REQUIRE( game->HasBoardSize() == false );
        REQUIRE( game->GetBoardSize() == SgfcConstants::BoardSizeInvalid );
      }
    }
  }
}

SCENARIO( "SgfcGame is queried for the game info nodes", "[document]" )
{
  auto game = std::make_shared<SgfcGame>();
  SgfcTreeBuilder treeBuilder(game);

  auto rootNode = std::make_shared<SgfcNode>();
  auto intermediateNode = std::make_shared<SgfcNode>();
  auto intermediateSiblingNode = std::make_shared<SgfcNode>();
  auto leafNode = std::make_shared<SgfcNode>();

  game->SetRootNode(rootNode);
  treeBuilder.SetFirstChild(rootNode, intermediateNode);
  treeBuilder.SetNextSibling(intermediateNode, intermediateSiblingNode);
  treeBuilder.SetFirstChild(intermediateNode, leafNode);

  auto gameInfoProperty = std::shared_ptr<ISgfcProperty>(new SgfcProperty(SgfcPropertyType::AN, "AN"));

  GIVEN( "SgfcGame has no root node" )
  {
    game->SetRootNode(nullptr);

    WHEN( "SgfcGame is queried" )
    {
      auto gameInfoNodes = game->GetGameInfoNodes();

      THEN( "SgfcGame returns an empty collection" )
      {
        REQUIRE( gameInfoNodes.size() == 0 );
      }
    }
  }

  GIVEN( "SgfcGame has a root node but no game info nodes" )
  {
    WHEN( "SgfcGame is queried" )
    {
      auto gameInfoNodes = game->GetGameInfoNodes();

      THEN( "SgfcGame returns an empty collection" )
      {
        REQUIRE( gameInfoNodes.size() == 0 );
      }
    }
  }

  GIVEN( "SgfcGame has a single game info node" )
  {
    std::vector<std::shared_ptr<ISgfcNode>> testNodes = { rootNode, intermediateNode, intermediateSiblingNode, leafNode };

    WHEN( "SgfcGame is queried" )
    {
      THEN( "SgfcGame returns a collection with a single element" )
      {
        // TODO: Get this to work with GENERATE_COPY
        for (auto gameInfoNode : testNodes)
        {
          gameInfoNode->SetProperties({gameInfoProperty});
          auto gameInfoNodes = game->GetGameInfoNodes();

          REQUIRE( gameInfoNodes.size() == 1 );
          REQUIRE( gameInfoNodes.front() == gameInfoNode );

          gameInfoNode->RemoveAllProperties();
        }
      }
    }
  }

  GIVEN( "SgfcGame has multiple game info nodes" )
  {
    rootNode->SetProperties({gameInfoProperty});
    intermediateNode->SetProperties({gameInfoProperty});
    intermediateSiblingNode->SetProperties({gameInfoProperty});
    leafNode->SetProperties({gameInfoProperty});

    WHEN( "The root node hides the other game nodes" )
    {
      auto gameInfoNodes = game->GetGameInfoNodes();

      THEN( "SgfcGame returns a collection that contains only the root node" )
      {
        REQUIRE( gameInfoNodes.size() == 1 );
        REQUIRE( gameInfoNodes.front() == rootNode );
      }
    }

    WHEN( "The intermediate node hides the leaf node" )
    {
      rootNode->RemoveAllProperties();
      auto gameInfoNodes = game->GetGameInfoNodes();

      THEN( "SgfcGame returns a collection that contains only the intermediate node and its sibling" )
      {
        REQUIRE( gameInfoNodes.size() == 2 );
        // The order is important - the search is performed depth-first
        REQUIRE( gameInfoNodes.front() == intermediateNode );
        REQUIRE( gameInfoNodes.back() == intermediateSiblingNode );
      }
    }

    WHEN( "The leaf node is not hidden by its ancestors" )
    {
      rootNode->RemoveAllProperties();
      intermediateNode->RemoveAllProperties();
      auto gameInfoNodes = game->GetGameInfoNodes();

      THEN( "SgfcGame returns a collection that contains the leaf node and the intermediate sibling" )
      {
        REQUIRE( gameInfoNodes.size() == 2 );
        // The order is important - the search is performed depth-first
        REQUIRE( gameInfoNodes.front() == leafNode );
        REQUIRE( gameInfoNodes.back() == intermediateSiblingNode );
      }
    }
  }
}

void SetupGameWithGameAndBoardSizeProperties(std::shared_ptr<SgfcGame> game, SgfcGameType gameType, SgfcBoardSize boardSize, bool setupWithTypedProperties)
{
  std::vector<std::shared_ptr<ISgfcProperty>> properties;
  AddGameTypeProperty(gameType, properties, setupWithTypedProperties);
  AddBoardSizeProperty(boardSize, properties, setupWithTypedProperties);

  auto rootNode = std::shared_ptr<SgfcNode>(new SgfcNode());
  rootNode->SetProperties(properties);

  game->SetRootNode(rootNode);
}

void SetupGameWithBoardSizeProperty(std::shared_ptr<SgfcGame> game, SgfcBoardSize boardSize, bool setupWithTypedProperty)
{
  std::vector<std::shared_ptr<ISgfcProperty>> properties;
  AddBoardSizeProperty(boardSize, properties, setupWithTypedProperty);

  auto rootNode = std::shared_ptr<SgfcNode>(new SgfcNode());
  rootNode->SetProperties(properties);

  game->SetRootNode(rootNode);
}

void AddGameTypeProperty(SgfcGameType gameType, std::vector<std::shared_ptr<ISgfcProperty>>& properties, bool setupWithTypedProperty)
{
  auto gameTypeProperty = CreateGameTypeProperty(
    SgfcUtility::MapGameTypeToNumberValue(gameType),
    setupWithTypedProperty);

  properties.push_back(gameTypeProperty);
}

void AddGameTypeProperty(SgfcNumber gameTypeAsNumber, std::vector<std::shared_ptr<ISgfcProperty>>& properties, bool setupWithTypedProperty)
{
  auto gameTypeProperty = CreateGameTypeProperty(
    gameTypeAsNumber,
    setupWithTypedProperty);

  properties.push_back(gameTypeProperty);
}

void AddGameTypePropertyWithInvalidValues(std::vector<std::shared_ptr<ISgfcProperty>>& properties, InvalidValueTypeGameType invalidValueType)
{
  auto propertyFactory = SgfcPlusPlusFactory::CreatePropertyFactory();
  std::shared_ptr<ISgfcProperty> gameTypeProperty = propertyFactory->CreateProperty(
      SgfcPropertyType::GM);

  auto propertyValueFactory = SgfcPlusPlusFactory::CreatePropertyValueFactory();
  std::vector<std::shared_ptr<ISgfcPropertyValue>> propertyValues;
  switch (invalidValueType)
  {
    case InvalidValueTypeGameType::MoreThanOneValue:
      propertyValues.push_back(propertyValueFactory->CreateNumberPropertyValue(1));
      propertyValues.push_back(propertyValueFactory->CreateNumberPropertyValue(12));
      break;
    case InvalidValueTypeGameType::ComposedValue:
      propertyValues.push_back(propertyValueFactory->CreateComposedNumberAndNumberPropertyValue(17, 42));
      break;
    case InvalidValueTypeGameType::SingleValueTypeNotANumber:
      propertyValues.push_back(propertyValueFactory->CreateDoublePropertyValue(SgfcDouble::Normal));
      break;
    default:
      throw std::logic_error("Unexpected InvalidValueTypeGameType value");
      break;
  }
  gameTypeProperty->SetPropertyValues(propertyValues);

  properties.push_back(gameTypeProperty);
}

std::shared_ptr<ISgfcProperty> CreateGameTypeProperty(SgfcNumber gameTypeAsNumber, bool createTypedProperty)
{
  auto propertyFactory = SgfcPlusPlusFactory::CreatePropertyFactory();
  auto propertyValueFactory = SgfcPlusPlusFactory::CreatePropertyValueFactory();

  auto gameTypePropertyValue = propertyValueFactory->CreateNumberPropertyValue(
    gameTypeAsNumber);

  std::shared_ptr<ISgfcProperty> gameTypeProperty;
  if (createTypedProperty)
  {
    gameTypeProperty = propertyFactory->CreateGameTypeProperty(
      gameTypePropertyValue);
  }
  else
  {
    gameTypeProperty = propertyFactory->CreateProperty(
      SgfcPropertyType::GM,
      gameTypePropertyValue);
  }

  return gameTypeProperty;
}

void AddBoardSizeProperty(SgfcBoardSize boardSize, std::vector<std::shared_ptr<ISgfcProperty>>& properties, bool setupWithTypedProperty)
{
  auto propertyFactory = SgfcPlusPlusFactory::CreatePropertyFactory();
  auto propertyValueFactory = SgfcPlusPlusFactory::CreatePropertyValueFactory();

  std::shared_ptr<ISgfcProperty> boardSizeProperty;
  if (boardSize.IsSquare())
  {
    auto boardSizePropertyValue = propertyValueFactory->CreateNumberPropertyValue(
      boardSize.Columns);
    if (setupWithTypedProperty)
    {
      boardSizeProperty = propertyFactory->CreateBoardSizeProperty(
        boardSizePropertyValue);
    }
    else
    {
      boardSizeProperty = propertyFactory->CreateProperty(
        SgfcPropertyType::SZ,
        boardSizePropertyValue);
    }
  }
  else
  {
    auto boardSizePropertyValue = propertyValueFactory->CreateComposedNumberAndNumberPropertyValue(
      boardSize.Columns,
      boardSize.Rows);
    if (setupWithTypedProperty)
    {
      boardSizeProperty = propertyFactory->CreateBoardSizeProperty(
        boardSizePropertyValue);
    }
    else
    {
      boardSizeProperty = propertyFactory->CreateProperty(
        SgfcPropertyType::SZ,
        boardSizePropertyValue);
    }
  }

  properties.push_back(boardSizeProperty);
}

void AddBoardSizePropertyWithInvalidValues(std::vector<std::shared_ptr<ISgfcProperty>>& properties, InvalidValueTypeBoardSize invalidValueType)
{
  auto propertyFactory = SgfcPlusPlusFactory::CreatePropertyFactory();
  std::shared_ptr<ISgfcProperty> gameTypeProperty = propertyFactory->CreateProperty(
      SgfcPropertyType::SZ);

  auto propertyValueFactory = SgfcPlusPlusFactory::CreatePropertyValueFactory();
  std::vector<std::shared_ptr<ISgfcPropertyValue>> propertyValues;
  switch (invalidValueType)
  {
    case InvalidValueTypeBoardSize::MoreThanOneValue:
      propertyValues.push_back(propertyValueFactory->CreateNumberPropertyValue(1));
      propertyValues.push_back(propertyValueFactory->CreateNumberPropertyValue(12));
      break;
    case InvalidValueTypeBoardSize::ComposedValueNotANumberAndNumber:
      propertyValues.push_back(propertyValueFactory->CreateComposedNumberAndSimpleTextPropertyValue(17, "foo"));
      break;
    case InvalidValueTypeBoardSize::SingleValueTypeNotANumber:
      propertyValues.push_back(propertyValueFactory->CreateDoublePropertyValue(SgfcDouble::Normal));
      break;
    default:
      throw std::logic_error("Unexpected InvalidValueTypeBoardSize value");
      break;
  }
  gameTypeProperty->SetPropertyValues(propertyValues);

  properties.push_back(gameTypeProperty);
}
