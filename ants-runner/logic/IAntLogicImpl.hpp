
#ifndef IANTLOGICIMPL_HPP
#define IANTLOGICIMPL_HPP

#include <thread>
#include "IAntLogic.hpp"
//#include <queue>
#include <boost/interprocess/ipc/message_queue.hpp>
#include <future>
#include <memory>

namespace antlogic
{
	using namespace boost::interprocess;
    typedef AntAction (*tf)(const antlogic::Ant&, AntSensor[3][3]);

	class AntLogicDLL : public IAntLogic
	{
		bool alive = true;
		void *handle;
		tf func;
		std::thread worker;
		void Worker();

		int pid;

		std::shared_ptr<message_queue> in, out;
		std::string q1, q2;

	public:
	    AntLogicDLL(std::string fileName, std::string id);
		AntAction GetAction(const Ant&, AntSensor[][3]);
		~AntLogicDLL();
	};


}

#endif
