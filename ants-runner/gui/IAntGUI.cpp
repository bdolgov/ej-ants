#include "IAntGUI.hpp"

#include "IAntGuiImpl.hpp"

#include <QPixmap>
#include <QPainter>

#include <map>
#include <algorithm>
#include <memory>

namespace antgui
{
    using namespace std;

    std::shared_ptr<IAntGui> IAntGui::getGUI()
    {
        return std::shared_ptr<IAntGui>(new AntGuiLogImpl("out.json"));
    }




}
