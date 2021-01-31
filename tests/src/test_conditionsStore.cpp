#include "es-test.hpp"

// Include only the tested class's header.
#include "../../source/ConditionsStore.h"

// ... and any system includes needed for the test file.
#include <map>
#include <string>



namespace { // test namespace

// #region mock data
// #endregion mock data



// #region unit tests
SCENARIO( "Creating a ConditionsStore" , "[ConditionsStore][Creation]" ) {
	GIVEN( "no arguments" ) {
		const auto store = ConditionsStore();
		THEN( "no conditions are stored" ) {
			REQUIRE( store.Locals().empty() );
		}
	}
	GIVEN( "an initializer list" ) {
		const auto store = ConditionsStore{{"hello world", 100}, {"goodbye world", 404}};
		
		THEN( "given conditions are in the Store" ) {
			REQUIRE( store.GetCondition("hello world") == 100 );
			REQUIRE( store.GetCondition("goodbye world") == 404 );
			REQUIRE( store.Locals().size() == 2 );
		}
		THEN( "not given conditions return the default value" ) {
			REQUIRE( store.GetCondition("ungreeted world") == 0 );
			REQUIRE( store.Locals().size() == 2 );
		}
	}
	GIVEN( "an initializer map" ) {
		const std::map<std::string, int64_t> initmap {{"hello world", 100}, {"goodbye world", 404}};
		const auto store = ConditionsStore(initmap);
		
		THEN( "given conditions are in the Store" ) {
			REQUIRE( store.GetCondition("hello world") == 100 );
			REQUIRE( store.GetCondition("goodbye world") == 404 );
			REQUIRE( store.Locals().size() == 2 );
		}
		THEN( "not given conditions return the default value" ) {
			REQUIRE( store.GetCondition("ungreeted world") == 0 );
			REQUIRE( store.Locals().size() == 2 );
		}
	}
}

SCENARIO( "Setting and erasing conditions", "[ConditionSet][ConditionSetting]" ) {
	GIVEN( "an empty starting store" ) {
		auto store = ConditionsStore();
		THEN( "stored values can be retrieved" ) {
			REQUIRE( store.Locals().size() == 0 );
			REQUIRE( store.SetCondition("myFirstVar", 10) == true );
			REQUIRE( store.GetCondition("myFirstVar") == 10 );
			REQUIRE( store.HasCondition("myFirstVar") == true );
			REQUIRE( store.Locals().size() == 1 );
		}
		THEN( "defaults are returned for unstored values and are not stored" ) {
			REQUIRE( store.Locals().size() == 0 );
			REQUIRE( store.GetCondition("mySecondVar") == 0 );
			REQUIRE( store.Locals().size() == 0 );
			REQUIRE( store.HasCondition("mySecondVar") == false );
		}
		THEN( "erased conditions are indeed removed" ) {
			REQUIRE( store.Locals().size() == 0 );
			REQUIRE( store.SetCondition("myFirstVar", 10) == true );
			REQUIRE( store.HasCondition("myFirstVar") == true );
			REQUIRE( store.Locals().size() == 1 );
			REQUIRE( store.GetCondition("myFirstVar") == 10 );
			REQUIRE( store.Locals().size() == 1 );
			REQUIRE( store.EraseCondition("myFirstVar") == true );
			REQUIRE( store.HasCondition("myFirstVar") == false );
			REQUIRE( store.Locals().size() == 0 );
			REQUIRE( store.GetCondition("myFirstVar") == 0 );
			REQUIRE( store.HasCondition("myFirstVar") == false );
			REQUIRE( store.Locals().size() == 0 );
		}
	}
}


// #endregion unit tests



} // test namespace
