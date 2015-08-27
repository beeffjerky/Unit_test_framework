#include <limits.h>
#include "virtualYear.h"
#include "VirtualEvent.h"
#include "gtest/gtest.h"
#include <chrono>

using namespace std;
using namespace std::chrono;

class DefaultVirtualYearTests : public testing::Test{
	protected:
	virtual void SetUp() {
		default_year = VCAPS::VirtualYear();
	}
	VCAPS::VirtualYear default_year;
};

//Make sure default empty size
TEST_F(DefaultVirtualYearTests, Default_Values) {
    //Make sure the fields are correct
	EXPECT_EQ(0, default_year.size());
}

//Add an empty event and make sure size changes
TEST_F(DefaultVirtualYearTests, Add_Empty_Event) {

	VCAPS::VirtualEvent added_event = VCAPS::VirtualEvent();
	default_year.addVirtualEvent(1, added_event);
	EXPECT_EQ(1, default_year.size());

	//Add Duplicate Event
	added_event = VCAPS::VirtualEvent();
	default_year.addVirtualEvent(1, added_event);
	EXPECT_EQ(1, default_year.size());
	
	
	

	//added_event = VCAPS::VirtualEvent(1, 1., 1.);
	//default_year.addVirtualEvent(1, added_event);
	//EXPECT_EQ(1, default_year.size());
}

//Make sure when we add events with the same ID number that their fields are added
TEST_F(DefaultVirtualYearTests, Add_Event_IDMatch) {
	//Make sure the fields are correct

	VCAPS::VirtualEvent added_event = VCAPS::VirtualEvent(0, 7., 1.);
	default_year.addVirtualEvent(1, added_event);
	EXPECT_EQ(1, default_year.size());
	VCAPS::VirtualEvent::Iterator iE = default_year.get_events().find(1);
	EXPECT_EQ(0, iE->second.ripBase);
	EXPECT_EQ(6, iE->second.get_lossNetOfReinstatePrem());
	EXPECT_EQ(1, iE->second.reinstatementPrem);
	EXPECT_EQ(0, iE->second.fullRip);
	EXPECT_EQ("NA", iE->second.riskGroup);
	EXPECT_EQ(0, iE->second.eventId);

	added_event = VCAPS::VirtualEvent();
	default_year.addVirtualEvent(1, added_event);
	EXPECT_EQ(1, default_year.size());
	iE = default_year.get_events().find(1);
	EXPECT_EQ(0, iE->second.ripBase);
	EXPECT_EQ(6, iE->second.get_lossNetOfReinstatePrem());
	EXPECT_EQ(1, iE->second.reinstatementPrem);
	EXPECT_EQ(0, iE->second.fullRip);
	EXPECT_EQ("NA", iE->second.riskGroup);
	EXPECT_EQ(0, iE->second.eventId);

	added_event = VCAPS::VirtualEvent(0,1,1);
	default_year.addVirtualEvent(1, added_event);
	EXPECT_EQ(1, default_year.size());
	iE = default_year.get_events().find(1);
	EXPECT_EQ(0, iE->second.ripBase);
	EXPECT_EQ(8, iE->second.loss);
	EXPECT_EQ(6, iE->second.get_lossNetOfReinstatePrem());
	EXPECT_EQ(2, iE->second.reinstatementPrem);
	EXPECT_EQ(0, iE->second.fullRip);
	EXPECT_EQ("NA", iE->second.riskGroup);
	EXPECT_EQ(0, iE->second.eventId);

}

//Adding events with different ID's should result in added events to the map and increased size
TEST_F(DefaultVirtualYearTests, Add_Event) {
	//Make sure the fields are correct

	VCAPS::VirtualEvent added_event = VCAPS::VirtualEvent(0, 7., 1.);
	default_year.addVirtualEvent(1, added_event);
	EXPECT_EQ(1, default_year.size());
	VCAPS::VirtualEvent::Iterator iE = default_year.get_events().find(1);
	EXPECT_EQ(0, iE->second.ripBase);
	EXPECT_EQ(6, iE->second.get_lossNetOfReinstatePrem());
	EXPECT_EQ(1, iE->second.reinstatementPrem);
	EXPECT_EQ(0, iE->second.fullRip);
	EXPECT_EQ("NA", iE->second.riskGroup);
	EXPECT_EQ(0, iE->second.eventId);

	added_event = VCAPS::VirtualEvent(1, 1, 1);
	default_year.addVirtualEvent(2, added_event);
	EXPECT_EQ(2, default_year.size());
	iE = default_year.get_events().find(2);
	EXPECT_EQ(0, iE->second.ripBase);
	EXPECT_EQ(1, iE->second.loss);
	EXPECT_EQ(0, iE->second.get_lossNetOfReinstatePrem());
	EXPECT_EQ(1, iE->second.reinstatementPrem);
	EXPECT_EQ(0, iE->second.fullRip);
	EXPECT_EQ("NA", iE->second.riskGroup);
	EXPECT_EQ(1, iE->second.eventId);
	iE = default_year.get_events().find(1);
	EXPECT_EQ(0, iE->second.ripBase);
	EXPECT_EQ(6, iE->second.get_lossNetOfReinstatePrem());
	EXPECT_EQ(1, iE->second.reinstatementPrem);
	EXPECT_EQ(0, iE->second.fullRip);
	EXPECT_EQ("NA", iE->second.riskGroup);
	EXPECT_EQ(0, iE->second.eventId);
}

//Replace an event with another event
TEST_F(DefaultVirtualYearTests, Update_Event) {
	//Make sure the fields are correct

	VCAPS::VirtualEvent added_event = VCAPS::VirtualEvent(0, 7., 1.);
	default_year.addVirtualEvent(1, added_event);
	EXPECT_EQ(1, default_year.size());
	VCAPS::VirtualEvent::Iterator iE = default_year.get_events().find(1);
	EXPECT_EQ(0, iE->second.ripBase);
	EXPECT_EQ(6, iE->second.get_lossNetOfReinstatePrem());
	EXPECT_EQ(1, iE->second.reinstatementPrem);
	EXPECT_EQ(0, iE->second.fullRip);
	EXPECT_EQ("NA", iE->second.riskGroup);
	EXPECT_EQ(0, iE->second.eventId);

	added_event = VCAPS::VirtualEvent(0, 1, 1);
	default_year.updateVirtualEvent(1, added_event);
	EXPECT_EQ(1, default_year.size());
	iE = default_year.get_events().find(1);
	EXPECT_EQ(0, iE->second.ripBase);
	EXPECT_EQ(1, iE->second.loss);
	EXPECT_EQ(0, iE->second.get_lossNetOfReinstatePrem());
	EXPECT_EQ(1, iE->second.reinstatementPrem);
	EXPECT_EQ(0, iE->second.fullRip);
	EXPECT_EQ("NA", iE->second.riskGroup);
	EXPECT_EQ(0, iE->second.eventId);

	added_event = VCAPS::VirtualEvent(1, 7., 1.);
	default_year.updateVirtualEvent(1, added_event);
	EXPECT_EQ(1, default_year.size());
	iE = default_year.get_events().find(1);
	EXPECT_EQ(0, iE->second.ripBase);
	EXPECT_EQ(6, iE->second.get_lossNetOfReinstatePrem());
	EXPECT_EQ(1, iE->second.reinstatementPrem);
	EXPECT_EQ(0, iE->second.fullRip);
	EXPECT_EQ("NA", iE->second.riskGroup);
	EXPECT_EQ(1, iE->second.eventId);
}

//Delete an event and expect the size to decrease and to not find it when iterating
TEST_F(DefaultVirtualYearTests, Delete_Event) {
	//Make sure the fields are correct

	VCAPS::VirtualEvent added_event = VCAPS::VirtualEvent(0, 7., 1.);
	default_year.addVirtualEvent(1, added_event);
	EXPECT_EQ(1, default_year.size());

	added_event = VCAPS::VirtualEvent(1, 1, 1);
	default_year.addVirtualEvent(2, added_event);
	EXPECT_EQ(2, default_year.size());

	added_event = VCAPS::VirtualEvent(2, 1, 1);
	default_year.addVirtualEvent(3, added_event);
	EXPECT_EQ(3, default_year.size());

	added_event = VCAPS::VirtualEvent(3, 1, 1);
	default_year.addVirtualEvent(4, added_event);
	EXPECT_EQ(4, default_year.size());

	default_year.deleteVirtualEvent(1);
	EXPECT_EQ(3, default_year.size());
	VCAPS::VirtualEvent::Iterator iE = default_year.get_events().find(1);
	EXPECT_EQ(iE, default_year.get_events().end());

}

//Use an operator to add a year to sure it calls the correct function
TEST_F(DefaultVirtualYearTests, PlusEq_Year) {
	//Make sure the fields are correct
	VCAPS::VirtualYear added_year = VCAPS::VirtualYear();
	VCAPS::VirtualEvent added_event;
	for (int i = 0; i < 4; i++){
		added_event = VCAPS::VirtualEvent(i, (i+1)*10, i+1);
		default_year.addVirtualEvent(i, added_event);

		added_event = VCAPS::VirtualEvent(i, (i + 1) * 100, (i + 1)*0.1);
		added_year.addVirtualEvent(i, added_event);
	}
	added_year.addVirtualEvent(4, VCAPS::VirtualEvent(4,4,4));
	EXPECT_EQ(4, default_year.size());
	EXPECT_EQ(5, added_year.size());

	default_year += added_year;

	EXPECT_EQ(5, default_year.size());

	VCAPS::VirtualEvent::Iterator iE;
	for (int i = 0; i < 4; i++){
		iE = default_year.get_events().find(i);
		EXPECT_EQ(i, iE->second.eventId);
		EXPECT_EQ((i + 1) * 10 + (i + 1) * 100, iE->second.loss);
		EXPECT_EQ(long((i + 1)*1.1) , long(iE->second.reinstatementPrem));
		EXPECT_EQ("NA", iE->second.riskGroup);
	}
	iE = default_year.get_events().find(4);
	EXPECT_EQ(4, iE->second.eventId);
	EXPECT_EQ(4, iE->second.loss);
	EXPECT_EQ(long double(4), long double(iE->second.reinstatementPrem));
	EXPECT_EQ("NA", iE->second.riskGroup);	
}

//Subtract equal year 
TEST_F(DefaultVirtualYearTests, SubEq_Year) {
	//Make sure the fields are correct
	VCAPS::VirtualYear added_year = VCAPS::VirtualYear();
	VCAPS::VirtualEvent added_event;
	for (int i = 0; i < 4; i++){
		added_event = VCAPS::VirtualEvent(i, (i + 1) * 10, i + 1);
		default_year.addVirtualEvent(i, added_event);

		added_event = VCAPS::VirtualEvent(i, (i + 1) * 100, (i + 1)*0.1);
		added_year.addVirtualEvent(i, added_event);
	}
	added_year.addVirtualEvent(4, VCAPS::VirtualEvent(4, 4, 4));
	EXPECT_EQ(4, default_year.size());
	EXPECT_EQ(5, added_year.size());

	default_year -= added_year;

	//size shouldn't change 
	EXPECT_EQ(5, default_year.size());

	VCAPS::VirtualEvent::Iterator iE;
	for (int i = 0; i < 4; i++){
		iE = default_year.get_events().find(i);
		EXPECT_EQ(i, iE->second.eventId);
		EXPECT_EQ((i + 1) * 10 - (i + 1) * 100, iE->second.loss);
		EXPECT_EQ(long double((i+1)*0.9), long double(iE->second.reinstatementPrem));
		EXPECT_EQ("NA", iE->second.riskGroup);
	}
	iE = default_year.get_events().find(4);
	EXPECT_EQ(4, iE->second.eventId);
	EXPECT_EQ(-4, iE->second.loss);
	EXPECT_EQ(long double(-4), long double(iE->second.reinstatementPrem));
	EXPECT_EQ("NA", iE->second.riskGroup);
}

//Make sure we can accexx events directly using operator
TEST_F(DefaultVirtualYearTests, Array_Operator) {
	//Make sure the fields are correct
	VCAPS::VirtualYear added_year = VCAPS::VirtualYear();
	VCAPS::VirtualEvent added_event;
	for (int i = 0; i < 4; i++){
		added_event = VCAPS::VirtualEvent(i, (i + 1) * 10, i + 1);
		default_year.addVirtualEvent(i, added_event);
	}
	EXPECT_EQ(4, default_year.size());

	for (int i = 0; i < 4; i++){
		EXPECT_EQ(i, default_year[i].eventId);
		EXPECT_EQ((i + 1) * 10, default_year[i].loss);
		EXPECT_EQ(long double(i + 1), long double(default_year[i].reinstatementPrem));
		EXPECT_EQ("NA", default_year[i].riskGroup);
	}

	for (int i = 3; i >= 0; i--){
		EXPECT_EQ(i, default_year[i].eventId);
		EXPECT_EQ((i + 1) * 10, default_year[i].loss);
		EXPECT_EQ(long double(i + 1), long double(default_year[i].reinstatementPrem));
		EXPECT_EQ("NA", default_year[i].riskGroup);
	}
}

//look into this one for the multi processor project
TEST_F(DefaultVirtualYearTests, FilterEvent) {
	//Make sure the fields are correct

	VCAPS::VirtualEvent added_event;
	for (int i = 0; i < 40000; i++){
		added_event = VCAPS::VirtualEvent(i, (i ) , (39999-i) * 0.0001 );
		default_year.addVirtualEvent(i, added_event);
	}
	EXPECT_EQ(40000, default_year.size());

	for (int i = 0; i < 40000; i++){
		EXPECT_EQ(i, default_year[i].eventId);
		EXPECT_EQ((i), default_year[i].loss);
		EXPECT_EQ(long double((39999 - i) * 0.0001), long double(default_year[i].reinstatementPrem));
		EXPECT_EQ("NA", default_year[i].riskGroup);
	}
	default_year.filterOutEvent(20000, "NA");
	EXPECT_EQ(20000, default_year.size());
	for (int i = 20000; i < 40000; i++){
		EXPECT_EQ(i, default_year[i].eventId);
		EXPECT_EQ((i), default_year[i].loss);
		if (long double (fabs((39999 - i) * 0.0001)) < 1){
			EXPECT_EQ(long double(0), long double(default_year[i].reinstatementPrem));
		}
		else{
			EXPECT_EQ(long double((39999 - i) * 0.0001), long double(default_year[i].reinstatementPrem));
		}
		EXPECT_EQ("NA", default_year[i].riskGroup);
	}
}


TEST_F(DefaultVirtualYearTests, copyWithFilter) {

	VCAPS::VirtualEvent added_event;
	VCAPS::VirtualYear added_year;
	for (int i = 0; i < 40000; i++){
		added_event = VCAPS::VirtualEvent(i, (i), (39999 - i) * 0.0001);
		added_year.addVirtualEvent(i, added_event);
	}
	EXPECT_EQ(40000, added_year.size());

	for (int i = 0; i < 40000; i++){
		EXPECT_EQ(i, added_year[i].eventId);
		EXPECT_EQ((i), added_year[i].loss);
		EXPECT_EQ(long double((39999 - i) * 0.0001), long double(added_year[i].reinstatementPrem));
		EXPECT_EQ("NA", added_year[i].riskGroup);
	}
	int i = 0;
	default_year.copyWithFilter(added_year,20000,2.0, i);
	EXPECT_EQ(20000, default_year.size());

	for (int i = 20000; i < 40000; i++){
		EXPECT_EQ(i, default_year[i].eventId);
		EXPECT_EQ((i)*2, default_year[i].loss);
		EXPECT_EQ(long double((39999 - i) * 0.0001), long double(added_year[i].reinstatementPrem));
		EXPECT_EQ("NA", default_year[i].riskGroup);
	}
}

TEST_F(DefaultVirtualYearTests, addNoncatVirtualYear) {
	//Make sure the fields are correct

	VCAPS::VirtualEvent added_event;
	VCAPS::VirtualYear added_year;
	
	for (int i = 0; i < 40000; i++){
		added_event = VCAPS::VirtualEvent(i, (i), (39999 - i) * 0.0001);
		added_year.addVirtualEvent(i, added_event);
	}
	EXPECT_EQ(40000, added_year.size());
	for (int i = 0; i < 40000; i++){
		EXPECT_EQ(i, added_year[i].eventId);
		EXPECT_EQ((i), added_year[i].loss);
		EXPECT_EQ(long double((39999 - i) * 0.0001), long double(added_year[i].reinstatementPrem));
		EXPECT_EQ("NA", added_year[i].riskGroup);
	}
}

class VirtualYearMapTest : public testing::Test{
protected:
	virtual void SetUp() {
		VCAPS::VirtualYear added_year;
		VCAPS::VirtualEvent added_event;
		for (int j = 0; j < 1000; j++){
			added_year = VCAPS::VirtualYear();
			for (int i = 0; i < 4000; i++){
				added_event = VCAPS::VirtualEvent(i, (i), (39999 - i) * 0.0001);
				added_year.addVirtualEvent(i, added_event);
			}
			year_map[j] = added_year;
		}
	}
	VCAPS::VirtualYear::MAP year_map;
};

//This is to make sure that we can iterate through the virtual year map properly
TEST_F(VirtualYearMapTest, ensure_initial) {
	high_resolution_clock::time_point t1 = high_resolution_clock::now();
	//Make sure the fields are correct
	VCAPS::VirtualYear::Iterator year_iE ;
	VCAPS::VirtualEvent::Iterator event_iE;

	high_resolution_clock::time_point t2 = high_resolution_clock::now();

	auto duration = std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count();

	for (int j = 0; j < 10; j++){
		year_iE = year_map.find(j);
		for (int i = 0; i < 10; i++){
			event_iE = year_iE->second.get_events().find(i);
			EXPECT_EQ(i, event_iE->second.eventId);
			EXPECT_EQ((i), event_iE->second.loss);
			EXPECT_EQ(long double((39999 - i) * 0.0001), long double(event_iE->second.reinstatementPrem));
			EXPECT_EQ("NA", event_iE->second.riskGroup);
		}
	}
	t2 = high_resolution_clock::now();

	duration = std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count();

}

TEST_F(VirtualYearMapTest, copy_year_map) {
	high_resolution_clock::time_point t1 = high_resolution_clock::now();
	//Make sure the fields are correct
	VCAPS::VirtualYear::Iterator year_iE;
	VCAPS::VirtualEvent::Iterator event_iE;

	
	VCAPS::VirtualYear::MAP year_map_copy = year_map;
	high_resolution_clock::time_point t2 = high_resolution_clock::now();

	auto duration = std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count();

	for (int j = 0; j < 10; j++){
		year_iE = year_map_copy.find(j);
		for (int i = 0; i < 10; i++){
			event_iE = year_iE->second.get_events().find(i);
			EXPECT_EQ(i, event_iE->second.eventId);
			EXPECT_EQ((i), event_iE->second.loss);
			EXPECT_EQ(long double((39999 - i) * 0.0001), long double(event_iE->second.reinstatementPrem));
			EXPECT_EQ("NA", event_iE->second.riskGroup);
		}
	}
	t2 = high_resolution_clock::now();

	duration = std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count();
}


