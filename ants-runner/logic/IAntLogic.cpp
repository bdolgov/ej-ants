#include "IAntLogic.hpp"

#include "IAntLogicImpl.hpp"

namespace antlogic
{
    std::shared_ptr<IAntLogic> IAntLogic::GetAntLogic(int teamId)
    {
	return std::shared_ptr<IAntLogic>();
    }

    std::shared_ptr<IAntLogic> IAntLogic::GetAntLogic(std::string fileName, std::string id)
    {
        return std::make_shared<AntLogicDLL>(fileName, id);
    }


}

