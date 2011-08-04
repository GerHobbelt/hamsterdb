
#include <ham/types.h>

/**
 * @file montecarlo.hpp
 * @author Ger Hobbelt, ger@hobbelt.com
 * @version 1.1.12
 *
Extremely Simplified Monte Carlo analysis rig

takes a model and executes it a configurable number of rounds, passing it a time range
and a specified number of parameter values.

Note: the results are collected in an in-memory Hamster DB, sorted by each parameter for
      fast result summarization/graphing.
*/
class Monte_Carlo_simulator
{
protected:
	random_generator srng;

public:
	class argument_cfg
	{
	public:
		double value_min; // lowest allowed value
		double value_range;  // absolute maximum range of  the value

	public:
		argument_cfg()
			: value_min(0), value_range(0)
		{
		}
		argument_cfg(double m, double r)
			: value_min(m), value_range(r)
		{
		}
	};

	class model_argument_cfg : public argument_cfg
	{
	public:
		model_argument_cfg()
			: argument_cfg()
		{
		}
		model_argument_cfg(double m, double r)
			: argument_cfg(m, r)
		{
		}
	};

	class model_result
	{
	public:
		double value;  // result produced by the model_under_test
		bool failure;  // true when a 'fatal' failure occurred.

	public:
		model_result()
			: value(0.0), failure(true)
		{
		}
		model_result(double v, bool e)
			: value(v), failure(e)
		{
		}
	};

	class configuration
	{
	public:
		int m_sampling_interval; // seconds
		int m_runs; // number of test runs to execute
		argument_cfg m_start_time;
		argument_cfg m_end_time;

		int m_arg_count; // total number of arguments of the model
		model_argument_cfg *m_arg_params; // configuration per parameter

	public:
		configuration(int sampling_interval,
					int runs,
					argument_cfg &start_time,
					argument_cfg &end_time,
					int arg_count,
					model_argument_cfg *arg_params)
			: m_sampling_interval(sampling_interval),
			m_runs(runs),
			m_start_time(start_time),
			m_end_time(end_time),
			m_arg_count(arg_count),
			m_arg_params(0)
		{
			int i;
			if (m_arg_count > 0)
			{
			m_arg_params = new model_argument_cfg[m_arg_count];
			for (i = 0; i < m_arg_count; i++)
			{
				m_arg_params[i] = arg_params[i];
			}
			}
		}
		~configuration()
		{
			if (m_arg_params)
				delete[] m_arg_params;
		}
	};

	/**
	Provide a data feed for the @ref model_under_test.
	*/
	class data_provider
	{
	public:
		virtual void init(configuration &rig_cfg) = 0; // run once at start of simulation
		virtual void init4single_run(configuration &rig_cfg) = 0; // run once at start of each run
		virtual int fetch(int index) = 0;
		virtual void finish() = 0; // run once at end of simulation
	};

	class model_under_test
	{
	public:
		virtual void init(data_provider &data_src) = 0; // run once at start of simulation
		virtual model_result execute(int starting_time, int ending_time,
							int sampling_interval,
							int arg_count, double args[]) = 0;
		virtual void finish() = 0; // run once at end of simulation
	};

	/**
	* one sampling run is one probe executed by the simulator.
	* Here we collect the result as returned by the Model Under Test,
	* while also storing the related input parameters for the model,
	* so that we can correlate the data and generate useful reports
	* from all this.
	*/
	class sampling_run_result
	{
	public:
		model_result m_output;
		int m_input_arg_count;
		double *m_inputs;

	public:
		sampling_run_result()
			: m_output(), m_input_arg_count(0), m_inputs(0)
		{
		}
		~sampling_run_result()
		{
			if (m_inputs)
				delete[] m_inputs;
		}
		void init(model_result rv, int inlen, double ins[])
		{
			m_output = rv;
			m_input_arg_count = inlen;
			if (inlen > 0)
			{
				m_inputs = new double[inlen];
				ASSERT(m_inputs != NULL);
				::memcpy(m_inputs, ins, inlen * sizeof(m_inputs[0]));
			}
		}
	};

protected:
	model_under_test &m_model;
	configuration m_cfg;

protected:
	sampling_run_result *m_collected_results;


public:
	Monte_Carlo_simulator(ham_u32_t seed, model_under_test &mut, configuration &cfg)
		: srng(seed), m_model(mut), m_cfg(cfg), m_collected_results(0)
	{
	}
	~Monte_Carlo_simulator()
	{
		if (m_collected_results)
			delete[] m_collected_results;
	}

	void execute()
	{
		m_collected_results = new sampling_run_result[m_cfg.m_runs];

		double *args = new double[m_cfg.m_arg_count];
		ASSERT(m_cfg.m_arg_count ? m_cfg.m_arg_params != NULL : 1);
		ASSERT(!m_cfg.m_arg_count ? m_cfg.m_arg_params == NULL : 1);

		int i;
		for (i = 0; i < m_cfg.m_runs; i++)
		{
			// determine the time range for this one
			double ts;
			double te;
			ts = m_cfg.m_start_time.value_min + srng.frand() * m_cfg.m_start_time.value_range;
			te = m_cfg.m_end_time.value_min + srng.frand() * m_cfg.m_end_time.value_range;
			ASSERT(te > ts);

			// set up the argument values for this test run
			for (int ai = 0; ai < m_cfg.m_arg_count; ai++)
			{
				args[ai] = m_cfg.m_arg_params[ai].value_min + srng.frand() * m_cfg.m_arg_params[ai].value_range;
			}

			// execute the test and collect the results
			m_collected_results[i].init(m_model.execute(int(ts), int(te), m_cfg.m_sampling_interval, m_cfg.m_arg_count, args), m_cfg.m_arg_count, args);
		}

		// cleanup
		delete[] args;
	}

	void munch_collected_data()
	{
		// take all the collected results and
	}

	void generate_report()
	{
	}
};







