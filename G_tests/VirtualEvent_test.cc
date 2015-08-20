// Copyright 2005, Google Inc.
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
//     * Redistributions of source code must retain the above copyright
// notice, this list of conditions and the following disclaimer.
//     * Redistributions in binary form must reproduce the above
// copyright notice, this list of conditions and the following disclaimer
// in the documentation and/or other materials provided with the
// distribution.
//     * Neither the name of Google Inc. nor the names of its
// contributors may be used to endorse or promote products derived from
// this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

// A sample program demonstrating using Google C++ testing framework.
//
// Author: wan@google.com (Zhanyong Wan)


// This sample shows how to write a simple unit test for a function,
// using Google C++ testing framework.
//
// Writing a unit test using Google C++ testing framework is easy as 1-2-3:


// Step 1. Include necessary header files such that the stuff your
// test logic needs is declared.
//
// Don't forget gtest.h, which declares the testing framework.

#include <limits.h>
#include "VirtualEvent.h"
#include "gtest/gtest.h"


// Step 2. Use the TEST macro to define your tests.
//
// TEST has two parameters: the test case name and the test name.
// After using the macro, you should define your test logic between a
// pair of braces.  You can use a bunch of macros to indicate the
// success or failure of a test.  EXPECT_TRUE and EXPECT_EQ are
// examples of such macros.  For a complete list, see gtest.h.
//
// <TechnicalDetails>
//
// In Google Test, tests are grouped into test cases.  This is how we
// keep test code organized.  You should put logically related tests
// into the same test case.
//
// The test case name and the test name should both be valid C++
// identifiers.  And you should not use underscore (_) in the names.
//
// Google Test guarantees that each test you define is run exactly
// once, but it makes no guarantee on the order the tests are
// executed.  Therefore, you should write your tests in such a way
// that their results don't depend on their order.
//
// </TechnicalDetails>


//Test class environment definition.
//During the execution of the tests the SetUp() method is run
//Garbage collection then cleans up anything set up as we run through tests
class DefaultVirtualEventTests : public testing::Test{
	protected:
	virtual void SetUp() {
		default_event = VCAPS::VirtualEvent();
	}
	VCAPS::VirtualEvent default_event;
};

TEST_F(DefaultVirtualEventTests, Default_Values) {
    //Make sure the fields are correct
	EXPECT_EQ(0, default_event.loss);
	EXPECT_EQ(0, default_event.reinstatementPrem);
	EXPECT_EQ(0, default_event.get_lossNetOfReinstatePrem() );
	EXPECT_EQ(0, default_event.get_lossNetOfFullRip());

	default_event = VCAPS::VirtualEvent();
	EXPECT_EQ(0, default_event.loss);
	EXPECT_EQ(0, default_event.reinstatementPrem);
	EXPECT_EQ(0, default_event.get_lossNetOfReinstatePrem());
	EXPECT_EQ(0, default_event.get_lossNetOfFullRip());
}

TEST_F(DefaultVirtualEventTests, Default_Direct_Scale) {
    //Make sure the fields are scaled properly
	default_event *= 2;
    EXPECT_EQ(0, default_event.get_lossNetOfReinstatePrem() );
	EXPECT_EQ(0, default_event.get_lossNetOfFullRip());
}

TEST_F(DefaultVirtualEventTests, RG_Scale) {
    //Make sure the scaling isn't done for incorrect RG
	default_event.scale(2,"incorrect_rg");
    EXPECT_EQ(0, default_event.get_lossNetOfReinstatePrem() );
	EXPECT_EQ(0, default_event.get_lossNetOfFullRip());
    
    //Make sure the scaling is done for correct RG
    default_event.scale(2,"NA");
    EXPECT_EQ(0, default_event.get_lossNetOfReinstatePrem() );
	EXPECT_EQ(0, default_event.get_lossNetOfFullRip());
    
    //Make sure the scaling is done for correct RG
    default_event.scale(2,"ALL");
    EXPECT_EQ(0, default_event.get_lossNetOfReinstatePrem() );
	EXPECT_EQ(0, default_event.get_lossNetOfFullRip());
}

TEST_F(DefaultVirtualEventTests, RG_Vector_Scale) {
    //Use vector to ensure proper scaling
    string risks [] = {"incorrect_rg","NA"};
    std::vector<string> risk_groups(risks, risks + sizeof(risks)/sizeof(string)) ;
	default_event.scale(2,risk_groups);
    EXPECT_EQ(0, default_event.get_lossNetOfReinstatePrem());
	EXPECT_EQ(0, default_event.get_lossNetOfFullRip());    
}

TEST_F(DefaultVirtualEventTests, RP_Scale) {
    //Scale RP and check values
	default_event.scaleRP(2);
    EXPECT_EQ(0, default_event.get_lossNetOfReinstatePrem());
	EXPECT_EQ(0, default_event.get_lossNetOfFullRip());    
}

TEST_F(DefaultVirtualEventTests, Full_Rip_Scale) {
    //Scale RIP and check values
	default_event.scaleFullRip(2);
    EXPECT_EQ(0, default_event.get_lossNetOfReinstatePrem());
	EXPECT_EQ(0, default_event.get_lossNetOfFullRip());    
}

TEST_F(DefaultVirtualEventTests, Combine_RIP_To_Loss) {
    //Ensure loss and reinstatementPrem are changed
	default_event.combineReinstatePremToLoss();
    EXPECT_EQ(0, default_event.get_lossNetOfReinstatePrem());
	EXPECT_EQ(0, default_event.get_lossNetOfFullRip());    
}

TEST_F(DefaultVirtualEventTests, Add_Fields) {
    //Ensure linear scaling work
	default_event.add_loss(5.);
    EXPECT_EQ(5., default_event.get_lossNetOfReinstatePrem());
	EXPECT_EQ(5., default_event.get_lossNetOfFullRip());    
    default_event.add_reinstatementPrem(2.);
    EXPECT_EQ(3., default_event.get_lossNetOfReinstatePrem());
	EXPECT_EQ(5., default_event.get_lossNetOfFullRip());   
    default_event.add_fullRip(3.);
    EXPECT_EQ(3., default_event.get_lossNetOfReinstatePrem());
	EXPECT_EQ(2., default_event.get_lossNetOfFullRip());   
}



class NoRGVirtualEventTests : public testing::Test{
	protected:
	virtual void SetUp() {
		no_RG_event = VCAPS::VirtualEvent(1, 2.,1.);
	}
	VCAPS::VirtualEvent no_RG_event;
};
// Tests factorial of negative numbers.
TEST_F(NoRGVirtualEventTests, Constructor_Fields) {
	EXPECT_EQ(2, no_RG_event.loss);
	EXPECT_EQ(1, no_RG_event.reinstatementPrem);
	EXPECT_EQ(1, no_RG_event.get_lossNetOfReinstatePrem());
	EXPECT_EQ(2, no_RG_event.get_lossNetOfFullRip());

	no_RG_event = VCAPS::VirtualEvent(1, 7., 1.);
	EXPECT_EQ(7, no_RG_event.loss);
	EXPECT_EQ(1, no_RG_event.reinstatementPrem);
	EXPECT_EQ(6, no_RG_event.get_lossNetOfReinstatePrem());
	EXPECT_EQ(7, no_RG_event.get_lossNetOfFullRip());
}

TEST_F(NoRGVirtualEventTests, Direct_Scale) {
    int factor = 1;
    for(int a = 1; a< 5; a++){
        no_RG_event *= a;
        factor *= a;
        EXPECT_EQ(1*factor, no_RG_event.get_lossNetOfReinstatePrem());
        EXPECT_EQ(2*factor, no_RG_event.get_lossNetOfFullRip());
    }
}

TEST_F(NoRGVirtualEventTests, RG_Scale) {
    int factor = 1;
    for(int a = 1; a< 5; a++){
        //Make sure the scaling isn't done for incorrect RG
        
        no_RG_event.scale(a,"incorrect_rg");
        EXPECT_EQ(1*factor, no_RG_event.get_lossNetOfReinstatePrem() );
        EXPECT_EQ(2*factor, no_RG_event.get_lossNetOfFullRip());
        factor *= a;
        //Make sure the scaling is done for correct RG
        no_RG_event.scale(a,"NA");
        EXPECT_EQ(1*factor, no_RG_event.get_lossNetOfReinstatePrem() );
        EXPECT_EQ(2*factor, no_RG_event.get_lossNetOfFullRip());
        
        factor *= a;
        //Make sure the scaling is done for correct RG
        no_RG_event.scale(a,"ALL");
        EXPECT_EQ(1*factor, no_RG_event.get_lossNetOfReinstatePrem() );
        EXPECT_EQ(2*factor, no_RG_event.get_lossNetOfFullRip());
    }
}

TEST_F(NoRGVirtualEventTests, RG_Vector_Scale) {
    //Use vector to ensure proper scaling
    string risks [] = {"incorrect_rg","NA","ALL"};
    std::vector<string> risk_groups(risks, risks + sizeof(risks)/sizeof(string)) ;
	no_RG_event.scale(2,risk_groups);
    EXPECT_EQ(2, no_RG_event.get_lossNetOfReinstatePrem());
	EXPECT_EQ(4, no_RG_event.get_lossNetOfFullRip());    
}

TEST_F(NoRGVirtualEventTests, RP_Scale) {
    //Scale RP and check values
    int factor = 1;
    for(int a = 1; a< 5; a++){
        factor *= a;
        no_RG_event.scaleRP(a);
        if (factor > 2){
            factor = 2;
        }
        EXPECT_EQ(2-factor, no_RG_event.get_lossNetOfReinstatePrem());
        EXPECT_EQ(2, no_RG_event.get_lossNetOfFullRip());
    }
}

TEST_F(NoRGVirtualEventTests, Full_Rip_Scale) {
    //Scale RIP and check values
    int factor = 1;
    for(int a = 1; a< 5; a++){
        factor *= a;
        no_RG_event.scaleFullRip(a);
        EXPECT_EQ(1, no_RG_event.get_lossNetOfReinstatePrem());
        EXPECT_EQ(2, no_RG_event.get_lossNetOfFullRip());  
    }
    no_RG_event.add_fullRip(1.);
    factor = 1;
    for(int a = 1; a< 5; a++){
        factor *= a;
        no_RG_event.scaleFullRip(a);
        if (factor > 2){
            factor = 2;
        }
        EXPECT_EQ(1, no_RG_event.get_lossNetOfReinstatePrem());
        EXPECT_EQ(2-factor, no_RG_event.get_lossNetOfFullRip());  
    }
}

TEST_F(NoRGVirtualEventTests, Combine_RIP_To_Loss) {
    //Ensure loss and reinstatementPrem are changed
	no_RG_event.combineReinstatePremToLoss();
    EXPECT_EQ(1, no_RG_event.get_lossNetOfReinstatePrem());
	EXPECT_EQ(1, no_RG_event.get_lossNetOfFullRip());    
}

TEST_F(NoRGVirtualEventTests, Add_Fields) {
    //Ensure linear scaling work
	no_RG_event.add_loss(5.);
    EXPECT_EQ(6., no_RG_event.get_lossNetOfReinstatePrem());
	EXPECT_EQ(7., no_RG_event.get_lossNetOfFullRip());    
    no_RG_event.add_reinstatementPrem(2.);
    EXPECT_EQ(4., no_RG_event.get_lossNetOfReinstatePrem());
	EXPECT_EQ(7., no_RG_event.get_lossNetOfFullRip());   
    no_RG_event.add_fullRip(3.);
    EXPECT_EQ(4., no_RG_event.get_lossNetOfReinstatePrem());
	EXPECT_EQ(4., no_RG_event.get_lossNetOfFullRip());   
}



class RiskGroupedVirtualEventTests : public testing::Test{
	protected:
	virtual void SetUp() {
		rg_event_fullrip = VCAPS::VirtualEvent(1, 10.,2.,"Risk1", 1.);
        rg_event_nofull = VCAPS::VirtualEvent(2, 10.,1.,"Risk2");
	}
	VCAPS::VirtualEvent rg_event_fullrip;
    VCAPS::VirtualEvent rg_event_nofull;
};
// Tests factorial of negative numbers.
TEST_F(RiskGroupedVirtualEventTests, Constructor_Fields) {
	EXPECT_EQ(8, rg_event_fullrip.get_lossNetOfReinstatePrem());
	EXPECT_EQ(9, rg_event_fullrip.get_lossNetOfFullRip());
    EXPECT_EQ(9, rg_event_nofull.get_lossNetOfReinstatePrem());
	EXPECT_EQ(10, rg_event_nofull.get_lossNetOfFullRip());
}

TEST_F(RiskGroupedVirtualEventTests, Direct_Scale) {
    int factor = 1;
    for(int a = 1; a< 5; a++){
        rg_event_fullrip *= a;
        rg_event_nofull *= a;
        factor *= a;
        EXPECT_EQ(8*factor, rg_event_fullrip.get_lossNetOfReinstatePrem());
        EXPECT_EQ(((10*factor )-1), rg_event_fullrip.get_lossNetOfFullRip());
        EXPECT_EQ(9*factor, rg_event_nofull.get_lossNetOfReinstatePrem());
        EXPECT_EQ(10*factor, rg_event_nofull.get_lossNetOfFullRip());
    }
}

TEST_F(RiskGroupedVirtualEventTests, RG_Scale) {
    int factor1 = 1;
    int factor2 = 1;
    for(int a = 1; a< 5; a++){
        //Make sure the scaling isn't done for incorrect RG
        
        rg_event_fullrip.scale(a,"incorrect_rg");
        rg_event_nofull.scale(a,"incorrect_rg");
        EXPECT_EQ(8*factor1, rg_event_fullrip.get_lossNetOfReinstatePrem());
        EXPECT_EQ(((10*factor1 )-1), rg_event_fullrip.get_lossNetOfFullRip());
        EXPECT_EQ(9*factor2, rg_event_nofull.get_lossNetOfReinstatePrem());
        EXPECT_EQ(10*factor2, rg_event_nofull.get_lossNetOfFullRip());
        
        factor1 *= a;
        //Make sure the scaling is done for correct RG
        rg_event_fullrip.scale(a,"Risk1");
        rg_event_nofull.scale(a,"Risk1");
        EXPECT_EQ(8*factor1, rg_event_fullrip.get_lossNetOfReinstatePrem());
        EXPECT_EQ(((10*factor1 )-1), rg_event_fullrip.get_lossNetOfFullRip());
        EXPECT_EQ(9*factor2, rg_event_nofull.get_lossNetOfReinstatePrem());
        EXPECT_EQ(10*factor2, rg_event_nofull.get_lossNetOfFullRip());
        
        factor2 *= a;
         //Make sure the scaling is done for correct RG
        rg_event_fullrip.scale(a,"Risk2");
        rg_event_nofull.scale(a,"Risk2");
        EXPECT_EQ(8*factor1, rg_event_fullrip.get_lossNetOfReinstatePrem());
        EXPECT_EQ(((10*factor1 )-1), rg_event_fullrip.get_lossNetOfFullRip());
        EXPECT_EQ(9*factor2, rg_event_nofull.get_lossNetOfReinstatePrem());
        EXPECT_EQ(10*factor2, rg_event_nofull.get_lossNetOfFullRip());
        
        factor2 *= a;
        factor1 *= a;
        //Make sure the scaling is done for correct RG
        rg_event_fullrip.scale(a,"ALL");
        rg_event_nofull.scale(a,"ALL");
        EXPECT_EQ(8*factor1, rg_event_fullrip.get_lossNetOfReinstatePrem());
        EXPECT_EQ(((10*factor1 )-1), rg_event_fullrip.get_lossNetOfFullRip());
        EXPECT_EQ(9*factor2, rg_event_nofull.get_lossNetOfReinstatePrem());
        EXPECT_EQ(10*factor2, rg_event_nofull.get_lossNetOfFullRip());
    }
}

TEST_F(RiskGroupedVirtualEventTests, RG_Vector_Scale) {
    //Use vector to ensure proper scaling
    string risks [] = {"incorrect_rg","NA","Risk1","Risk2","ALL"};
    std::vector<string> risk_groups(risks, risks + sizeof(risks)/sizeof(string)) ;
	rg_event_fullrip.scale(2,risk_groups);
    rg_event_nofull.scale(2,risk_groups);
    EXPECT_EQ(8*2, rg_event_fullrip.get_lossNetOfReinstatePrem());
    EXPECT_EQ(((10*2 )-1), rg_event_fullrip.get_lossNetOfFullRip());
    EXPECT_EQ(9*2, rg_event_nofull.get_lossNetOfReinstatePrem());
    EXPECT_EQ(10*2, rg_event_nofull.get_lossNetOfFullRip());    
}

TEST_F(RiskGroupedVirtualEventTests, RP_Scale) {
    //Scale RP and check values
    int factor1 = 2;
    int factor2 = 1;
    for(int a = 1; a< 5; a++){
        factor1 *= a;
        rg_event_fullrip.scaleRP(a);
        if (factor1 > 10){
            factor1 = 10;
        }
        factor2 *= a;
        rg_event_nofull.scaleRP(a);
        if (factor2 > 10){
            factor2 = 10;
        }
        EXPECT_EQ(10-factor1, rg_event_fullrip.get_lossNetOfReinstatePrem());
        EXPECT_EQ(10-1, rg_event_fullrip.get_lossNetOfFullRip());
        EXPECT_EQ(10-factor2, rg_event_nofull.get_lossNetOfReinstatePrem());
        EXPECT_EQ(10, rg_event_nofull.get_lossNetOfFullRip());
    }
    factor1 *= 0;
    rg_event_fullrip.scaleRP(0);
    if (factor1 > 10){
        factor1 = 10;
    }
    factor2 *= 0;
    rg_event_nofull.scaleRP(0);
    if (factor2 > 10){
        factor2 = 10;
    }
    EXPECT_EQ(10-factor1, rg_event_fullrip.get_lossNetOfReinstatePrem());
    EXPECT_EQ(10-1, rg_event_fullrip.get_lossNetOfFullRip());
    EXPECT_EQ(10-factor2, rg_event_nofull.get_lossNetOfReinstatePrem());
    EXPECT_EQ(10, rg_event_nofull.get_lossNetOfFullRip());
}

TEST_F(RiskGroupedVirtualEventTests, Full_Rip_Scale) {
    //Scale RIP and check values
    int factor = 1;
    for(int a = 1; a< 5; a++){
        factor *= a;
        rg_event_fullrip.scaleFullRip(a);
        if (factor > 10){
            factor = 10;
        }
        EXPECT_EQ(8, rg_event_fullrip.get_lossNetOfReinstatePrem());
        EXPECT_EQ(10-factor, rg_event_fullrip.get_lossNetOfFullRip());  
        EXPECT_EQ(9, rg_event_nofull.get_lossNetOfReinstatePrem());
        EXPECT_EQ(10, rg_event_nofull.get_lossNetOfFullRip());
    }
}

TEST_F(RiskGroupedVirtualEventTests, Combine_RIP_To_Loss) {
    //Ensure loss and reinstatementPrem are changed
	rg_event_fullrip.combineReinstatePremToLoss();
    EXPECT_EQ(8, rg_event_fullrip.get_lossNetOfReinstatePrem());
	EXPECT_EQ(7, rg_event_fullrip.get_lossNetOfFullRip());  
    rg_event_nofull.combineReinstatePremToLoss();
    EXPECT_EQ(9, rg_event_nofull.get_lossNetOfReinstatePrem());
	EXPECT_EQ(9, rg_event_nofull.get_lossNetOfFullRip());     
}

TEST_F(RiskGroupedVirtualEventTests, Add_Fields) {
    //Ensure linear scaling work
	rg_event_fullrip.add_loss(5.);
    EXPECT_EQ(13., rg_event_fullrip.get_lossNetOfReinstatePrem());
	EXPECT_EQ(14., rg_event_fullrip.get_lossNetOfFullRip());    
    rg_event_fullrip.add_reinstatementPrem(2.);
    EXPECT_EQ(11., rg_event_fullrip.get_lossNetOfReinstatePrem());
	EXPECT_EQ(14., rg_event_fullrip.get_lossNetOfFullRip());   
    rg_event_fullrip.add_fullRip(3.);
    EXPECT_EQ(11., rg_event_fullrip.get_lossNetOfReinstatePrem());
	EXPECT_EQ(11., rg_event_fullrip.get_lossNetOfFullRip()); 

    rg_event_nofull.add_loss(5.);
    EXPECT_EQ(14., rg_event_nofull.get_lossNetOfReinstatePrem());
	EXPECT_EQ(15., rg_event_nofull.get_lossNetOfFullRip());    
    rg_event_nofull.add_reinstatementPrem(2.);
    EXPECT_EQ(12., rg_event_nofull.get_lossNetOfReinstatePrem());
	EXPECT_EQ(15., rg_event_nofull.get_lossNetOfFullRip());   
    rg_event_nofull.add_fullRip(3.);
    EXPECT_EQ(12., rg_event_nofull.get_lossNetOfReinstatePrem());
	EXPECT_EQ(12., rg_event_nofull.get_lossNetOfFullRip());    
}


class MAPVirtualEventTests : public testing::Test{
	protected:
	virtual void SetUp() {

		_events[1] = VCAPS::VirtualEvent(1, 10., 2., "Risk1", 1.);
		_events[2] = VCAPS::VirtualEvent(2, 10., 1., "Risk2");
		_events[3] = VCAPS::VirtualEvent();
	}
	VCAPS::VirtualEvent::MAP _events;
};

TEST_F(MAPVirtualEventTests, IterationValues){
	for (VCAPS::VirtualEvent::Iterator iterator = _events.begin(); iterator != _events.end(); iterator++){
		if (iterator->first == 1){
			EXPECT_EQ(8, iterator->second.get_lossNetOfReinstatePrem());
			EXPECT_EQ(9, iterator->second.get_lossNetOfFullRip());
			iterator->second = VCAPS::VirtualEvent(1, 45., 2., "Risk1", 1.);

		}
		else if (iterator->first == 2){
			EXPECT_EQ(9, iterator->second.get_lossNetOfReinstatePrem());
			EXPECT_EQ(10, iterator->second.get_lossNetOfFullRip());
			iterator->second = VCAPS::VirtualEvent(2, 23., 1., "Risk1");
		}
		else if (iterator->first == 3){
			EXPECT_EQ(0, iterator->second.get_lossNetOfReinstatePrem());
			EXPECT_EQ(0, iterator->second.get_lossNetOfFullRip());
		}
	}
}
// Step 3. Call RUN_ALL_TESTS() in main().
//
// We do this by linking in src/gtest_main.cc file, which consists of
// a main() function which calls RUN_ALL_TESTS() for us.
//
// This runs all the tests you've defined, prints the result, and
// returns 0 if successful, or 1 otherwise.
//
// Did you notice that we didn't register the tests?  The
// RUN_ALL_TESTS() macro magically knows about all the tests we
// defined.  Isn't this convenient?
