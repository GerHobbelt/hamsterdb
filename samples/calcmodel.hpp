
#include <ham/types.h>

using namespace ham_ex;



class StockTradeModel: public Monte_Carlo_simulator::model_under_test
{
protected:
	env &m_env;
	db m_stockdata;

public:
	StockTradeModel(env &store)
		: m_env(store), m_stockdata()
	{
	}

	// run once at start of simulation
	virtual void init(Monte_Carlo_simulator::data_provider &data_src)
	{
		ham_u16_t tradestable_name = 2;

		m_env.open("stocks");
		m_stockdata = m_env.open_db(tradestable_name);
	}

	virtual Monte_Carlo_simulator::model_result 
				execute(int starting_time, int ending_time, 
						int sampling_interval, 
						int arg_count, double args[])
	{
		ASSERT(arg_count == 3);

		return Monte_Carlo_simulator::model_result(0.0, false);
	}

	// run once at end of simulation
	virtual void finish() 
	{
		m_stockdata.close();
		m_env.close();
	}
};

