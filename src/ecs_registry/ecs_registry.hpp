#pragma once
#include <vector>

#include "ecs/ecs.hpp"
#include "components/components.hpp"

class ECSRegistry
{
	// Callbacks to remove a particular or all entities in the system
	std::vector<ContainerInterface*> registry_list;

public:
	// Manually created list of all components this game has
	ComponentContainer<DeathTimer> deathTimers;
	ComponentContainer<Obstacle> obstacles;
	ComponentContainer<Deadly> deadlies;
	ComponentContainer<Motion> motions;
	ComponentContainer<Collision> collisions;
	ComponentContainer<Player> players;
	ComponentContainer<Mesh*> meshPtrs;
	ComponentContainer<RenderRequest> renderRequests;
	ComponentContainer<ScreenState> screenStates;
	ComponentContainer<Projectile> projectiles;
	ComponentContainer<DebugComponent> debugComponents;
	ComponentContainer<vec3> colors;
	ComponentContainer<Health> healths;
	ComponentContainer<Shield> shields;
	ComponentContainer<Room> rooms;
	ComponentContainer<Text> texts;
	ComponentContainer<AI> ais;
	ComponentContainer<RoomTransitionTimer> roomTransitionTimers;
	ComponentContainer<Animation> animations;
	ComponentContainer<AnimationTimer> animationTimers;
	ComponentContainer<NoCollisionCheck> noCollisionChecks;
	ComponentContainer<OnFireTimer> onFireTimers;
	ComponentContainer<MuzzleFlashTimer> muzzleFlashTimers;
	ComponentContainer<DamagedTimer> damagedTimers;
	ComponentContainer<Level> levels;
	ComponentContainer<PowerupRandom> powerups;


	// constructor that adds all containers for looping over them
	// IMPORTANT: Don't forget to add any newly added containers!
	ECSRegistry()
	{
		registry_list.push_back(&deathTimers);
		registry_list.push_back(&obstacles);
		registry_list.push_back(&deadlies);
		registry_list.push_back(&motions);
		registry_list.push_back(&collisions);
		registry_list.push_back(&players);
		registry_list.push_back(&meshPtrs);
		registry_list.push_back(&renderRequests);
		registry_list.push_back(&screenStates);
		registry_list.push_back(&debugComponents);
		registry_list.push_back(&colors);
		registry_list.push_back(&projectiles);
		registry_list.push_back(&healths);
		registry_list.push_back(&shields);
		registry_list.push_back(&rooms);
		registry_list.push_back(&texts);
		registry_list.push_back(&ais);
		registry_list.push_back(&roomTransitionTimers);
		registry_list.push_back(&noCollisionChecks);
		registry_list.push_back(&animations);
		registry_list.push_back(&animationTimers);
		registry_list.push_back(&onFireTimers);
		registry_list.push_back(&damagedTimers);
		registry_list.push_back(&levels);
	}

	void clear_all_components() {
		for (ContainerInterface* reg : registry_list)
			reg->clear();
	}

	void list_all_components() {
		printf("Debug info on all registry entries:\n");
		for (ContainerInterface* reg : registry_list)
			if (reg->size() > 0)
				printf("%4d components of type %s\n", (int)reg->size(), typeid(*reg).name());
	}

	void list_all_components_of(Entity e) {
		printf("Debug info on components of entity %u:\n", (unsigned int)e);
		for (ContainerInterface* reg : registry_list)
			if (reg->has(e))
				printf("type %s\n", typeid(*reg).name());
	}

	void remove_all_components_of(Entity e) {
		for (ContainerInterface* reg : registry_list)
			reg->remove(e);
	}
};

extern ECSRegistry registry;
