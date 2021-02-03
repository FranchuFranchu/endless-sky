#include "es-test.hpp"

// Include only the tested class's header.
#include "../../source/ConditionSet.h"

// Include DataFile & DataNode, to enable creating non-empty ConditionSets.
#include "../../source/DataFile.h"
#include "../../source/DataNode.h"

// Include ConditionStore, to enable usage of them for testing ConditionSets.
#include "../../source/ConditionsStore.h"

// ... and any system includes needed for the test file.
#include <map>
#include <sstream>
#include <string>
#include <vector>

namespace { // test namespace

// #region mock data
// Method to convert text input into consumable DataNodes.
std::vector<DataNode> AsDataNodes(std::string text)
{
	std::stringstream in;
	in.str(text);
	const auto file = DataFile{in};
	
	return std::vector<DataNode>{std::begin(file), std::end(file)};
}
// Convert the text to a list of nodes, and return the first node.
const DataNode AsDataNode(std::string text)
{
	auto nodes = AsDataNodes(text);
	return nodes.empty() ? DataNode{} : *nodes.begin();
}
// #endregion mock data



// #region unit tests
SCENARIO( "Creating a ConditionSet" , "[ConditionSet][Creation]" ) {
	GIVEN( "no arguments" ) {
		const auto set = ConditionSet{};
		THEN( "no conditions are created" ) {
			REQUIRE( set.IsEmpty() );
		}
	}
	GIVEN( "an empty DataNode" ) {
		const auto emptyNode = DataNode{};
		const auto set = ConditionSet{emptyNode};
		
		THEN( "no conditions are created" ) {
			REQUIRE( set.IsEmpty() );
		}
	}
	GIVEN( "a node with no children" ) {
		auto childlessNode = AsDataNode("never");
		const auto set = ConditionSet{childlessNode};
		
		THEN( "no conditions are created" ) {
			REQUIRE( set.IsEmpty() );
		}
	}
	GIVEN( "a node with valid children" ) {
		auto nodeWithChildren = AsDataNode("and\n\tnever");
		const auto set = ConditionSet{nodeWithChildren};
		
		THEN( "a non-empty ConditionSet is created" ) {
			REQUIRE_FALSE( set.IsEmpty() );
		}
	}
}

SCENARIO( "Extending a ConditionSet", "[ConditionSet][Creation]" ) {
	GIVEN( "an empty ConditionSet" ) {
		auto set = ConditionSet{};
		REQUIRE( set.IsEmpty() );
		
		THEN( "no expressions are added from empty nodes" ) {
			set.Add(DataNode{});
			REQUIRE( set.IsEmpty() );
		}
		THEN( "no expressions are added from invalid nodes" ) {
			set.Add(AsDataNode("has"));
			REQUIRE( set.IsEmpty() );
		}
		THEN( "new expressions can be added from valid nodes" ) {
			set.Add(AsDataNode("never"));
			REQUIRE_FALSE( set.IsEmpty() );
		}
	}
}

SCENARIO( "Determining if condition requirements are met", "[ConditionSet][Usage]" ) {
	GIVEN( "an empty ConditionSet" ) {
		const auto emptySet = ConditionSet{};
		REQUIRE( emptySet.IsEmpty() );
		
		AND_GIVEN( "an empty list of Conditions" ) {
			const auto emptyConditionList = ConditionSet::Conditions{};
			THEN( "the ConditionSet is satisfied" ) {
				REQUIRE( emptySet.Test(emptyConditionList) );
			}
		}
		AND_GIVEN( "a non-empty list of Conditions" ) {
			const auto conditionList = ConditionSet::Conditions{
				{"event: war begins", 1},
			};
			THEN( "the ConditionSet is satisfied" ) {
				REQUIRE( emptySet.Test(conditionList) );
			}
		}
	}
	GIVEN( "a set containing 'never'" ) {
		const auto neverSet = ConditionSet{AsDataNode("and\n\tnever")};
		REQUIRE_FALSE( neverSet.IsEmpty() );
		
		AND_GIVEN( "a condition list containing the literal 'never'" ) {
			const auto listWithNever = ConditionSet::Conditions{{"never", 1}};
			THEN( "the ConditionSet is not satisfied" ) {
				REQUIRE_FALSE( neverSet.Test(listWithNever) );
			}
		}
	}
}

SCENARIO( "Applying changes to conditions", "[ConditionSet][Usage]" ) {
	auto store = ConditionSet::Conditions{};
	REQUIRE( store.GetPrimaryConditions().empty() );
	
	GIVEN( "an empty ConditionSet" ) {
		const auto emptySet = ConditionSet{};
		REQUIRE( emptySet.IsEmpty() );
		
		THEN( "no conditions are added via Apply" ) {
			emptySet.Apply(store);
			REQUIRE( store.GetPrimaryConditions().empty() );
			
			store.SetCondition("event: war begins", 1);
			REQUIRE( store.GetPrimaryConditions().size() == 1 );
			emptySet.Apply(store);
			REQUIRE( store.GetPrimaryConditions().size() == 1 );
		}
	}
	GIVEN( "a ConditionSet with only comparison expressions" ) {
		std::string compareExpressions = "and\n"
			"\thas \"event: war begins\"\n"
			"\tnot b\n"
			"\tc >= random\n";
		const auto compareSet = ConditionSet{AsDataNode(compareExpressions)};
		REQUIRE_FALSE( compareSet.IsEmpty() );
		
		THEN( "no conditions are added via Apply" ) {
			compareSet.Apply(store);
			REQUIRE( store.GetPrimaryConditions().empty() );
			
			store.SetCondition("event: war begins", 1);
			REQUIRE( store.GetPrimaryConditions().size() == 1 );
			compareSet.Apply(store);
			REQUIRE( store.GetPrimaryConditions().size() == 1 );
		}
	}
	GIVEN( "a ConditionSet with an assignable expression" ) {
		const auto applySet = ConditionSet{AsDataNode("and\n\tyear = 3013")};
		REQUIRE_FALSE( applySet.IsEmpty() );
		
		THEN( "the condition list is updated via Apply" ) {
			applySet.Apply(store);
			REQUIRE_FALSE( store.GetPrimaryConditions().empty() );
			
			const auto &inserted = store.GetPrimaryConditions().find("year");
			REQUIRE( inserted != store.GetPrimaryConditions().end() );
			CHECK( inserted->second == 3013 );
		}
	}
}
// #endregion unit tests



} // test namespace
