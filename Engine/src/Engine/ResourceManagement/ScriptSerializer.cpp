#include <string>
#include "ScriptSerializer.h"
#include "Engine/Scripts/MoveRightScript.h"
#include "Engine/Scripts/PlayerMovementScript.h"


namespace Engine 
{
    void ScriptSerializer::linkAndDeserializeScript(Entity& entity, std::string scriptName)
    {
        if (scriptName == "MoveRightScript")
        {
            entity.addComponent<ScriptComponent>().linkBehavior<MoveRightScript>();
        }
        if (scriptName == "PlayerMovementScript")
        {
            entity.addComponent<ScriptComponent>().linkBehavior<PlayerMovementScript>();
        }
   
    }
}