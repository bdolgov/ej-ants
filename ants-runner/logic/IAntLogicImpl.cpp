#include <iostream>
#include <string>
#include "IAntLogicImpl.hpp"
#include <stdlib.h>
#include <string.h>
#include <dlfcn.h>
#include <sys/types.h>
#include <signal.h>
#include <unistd.h>

namespace antlogic
{
    struct AntRequestMessage {
        char mem[32];
        bool hasFood;
        int teamId;
        AntSensor sensors[3][3];
    };

    struct AntResponseMessage {
        char mem[32];
        AntAction action;
    };

    struct MetaAnt : public Ant
    {
        int           teamId;
        char          mem[MAX_MEMORY];
        bool          withFood;

    public:
        MetaAnt() {};

        // logic needs following
        char* getMemory() const {return const_cast<char *>(mem);}

        // both need following
        bool hasFood() const {return withFood;}
        int getTeamId() const {return teamId;}
    };


    AntLogicDLL::AntLogicDLL(std::string filename, std::string p) {
        q1 = "in2" + p;
        q2 = "out2" + p;
        message_queue::remove(q1.c_str());
        message_queue::remove(q2.c_str());

        in = std::make_shared<message_queue>(create_only, q1.c_str(), 2, sizeof(AntRequestMessage));
        out = std::make_shared<message_queue>(create_only, q2.c_str(), 2, sizeof(AntResponseMessage));
        
        handle = dlopen(filename.c_str(), RTLD_GLOBAL | RTLD_NOW);

        if (!handle) {
            std::cerr << std::string(dlerror()) << std::endl;
            throw 1;
        }

        func = (tf)dlsym(handle, "GetAction");
        
        if (!func) {
            std::cerr << std::string(dlerror()) << std::endl;
            throw std::string("1");
        }
        
        
        this->pid = fork();
        if (this->pid == 0) {
            Worker();
        } else if (this->pid < 0) {
            std::cerr << "Error in fork" << std::endl;
        }
    }
    

    void AntLogicDLL::Worker() {
        
        try {
            while (1) {
                AntRequestMessage m;
                message_queue::size_type r;
                unsigned int p;
                try {
                    in->receive(&m, sizeof(m), r, p);
                }
                catch(interprocess_exception &ex) {
                    std::cerr << "worker recv" << std::endl;
                    message_queue::remove(q1.c_str());
                    std::cout << ex.what() << std::endl;
                    exit(0);
                }                
                MetaAnt ant;
                ant.teamId = m.teamId;
                ant.withFood = m.hasFood;
                memcpy(ant.mem, m.mem, 32);
                AntResponseMessage res;
                res.action = (*func)(ant, m.sensors);
                 
                memcpy(res.mem, ant.mem, 32);
                
                out->send(&res, sizeof(res), 1);
            }
        } catch (...) {
            exit(0);
        }
    }

    AntLogicDLL::~AntLogicDLL() {
        dlclose(handle);
        message_queue::remove(q1.c_str());
        message_queue::remove(q2.c_str());
        kill(pid, 9);        
//        worker.join();
    }

    AntAction AntLogicDLL::GetAction(const antlogic::Ant &ant, AntSensor sensors[3][3])
    {
		if (!alive)
		{
			return AntAction();
		}
        AntRequestMessage request;
        
        request.hasFood = ant.hasFood();
        request.teamId = ant.getTeamId();
        memcpy(request.mem, ant.getMemory(), 32);

        for (int i = 0; i < 3; ++i) {
            for (int j = 0; j < 3; ++j) {
                request.sensors[i][j] = sensors[i][j];
            }
        }
        AntResponseMessage res;
        try {
            in->send(&request, sizeof(request), 1);
        }
        catch(interprocess_exception &ex){
            std::cerr << "main send" << std::endl;
            message_queue::remove(q1.c_str());
            std::cout << ex.what() << std::endl;
            return res.action;
        }

        message_queue::size_type r;
        unsigned int p;
        try {
			boost::posix_time::ptime t(boost::posix_time::microsec_clock::universal_time());
			t += boost::posix_time::milliseconds(300);
            if (!out->timed_receive(&res, sizeof(AntResponseMessage), r, p, t))
			{
				/* report tl */
				alive = false;
				std::cerr << "Team " << q1 << " handler has just dead" << std::endl;
				return AntAction();
			}
        }
        catch(interprocess_exception &ex){
            std::cerr << "main receive" << std::endl;
            message_queue::remove(q1.c_str());
            std::cout << ex.what() << std::endl;
            return res.action;
        }
        memcpy(ant.getMemory(), res.mem, 32);
        return res.action;
    }
}
