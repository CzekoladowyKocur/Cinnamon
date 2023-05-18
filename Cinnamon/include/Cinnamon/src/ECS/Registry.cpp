#include "Cinnamon/include/ECS/Registry.hpp"

namespace Cinnamon {
	namespace ECS {
		ComponentID e_ComponentCounter{ 0 };
		STL::UMap<ComponentID, ComponentDeletionFunction> s_DeletionFunctions;
	}
}