~~GPU PRICING ENGINE~~

This file serves as a light summary for the GPU pricing project

There exists the GPU pricing experiment executable folder and multiple support projects.

~~GPUPricing ~~

This is the original exploritory functionality branch to shift core elements of pricing to the linux environment.
G_Tests are build off this code for now but as code is successfully run on GPU platforms then this will serve as the comparison to the previous model.

~~G_Tests && gtest-1.7.0 ~~

Gtests are google's testing framework being used for our unit test purposes. There is quite a bit of documntation on gtests online and there are some sameples written.

TODO: 
G_Tests are not complete. They currently offer around 30% code coverage for virtual year and nothing for Simulation 
When GPU platforms are completed, the tests will need to accomodate the GPU data structures. 
The kernal performs calculations as the host sends them the information so we wish to test the data before and after the host is updated with the kernal calculations.
The current makefile looks like it was written by a monkey please fix.




