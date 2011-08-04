
#include <ham/types.h>


/*
 * @file datafeedsim.hpp
 * @author Ger Hobbelt, ger@hobbelt.com
 * @version 1.1.2
 *
Data Feed Simulator

generates a (noisy) time series which has a few hardwired characteristics.

Used as a source for test data of HamsterDB sample env4
*/
class DataFeedSimulator: public Monte_Carlo_simulator::data_provider
{
};



