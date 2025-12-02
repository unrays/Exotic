*I literally could have called it Wave 2.0, but I was a bit too lazy to have to justify myself and make the world understand that there was a version 1, so I chose another name that I think is super cool. This project, as of Friday, November 28, 2025, is one of my first real experiments with sfinae features and srtp, like std::enable_if... for example. Furthermore, I intend to add lots of super cool stuff in the future, so stay tuned.*

*I also want to mention that I've tried my best to avoid looking at popular ECS like Beacy, EnTT, or Flecs in order to maintain a "healthy" direction and learn as much as possible without being influenced by "best" practices. I enjoy making mistakes, failing, and getting back up; that's what I find fun about learning. If I watched and copied others, I simply wouldn't learn. Here, I'm quite proud of the TagsGroup system I've come up with; a component has one or more tags that it automatically assigns to its entity when the tag is applied. Basically, I want a dynamic tagging system. For example, if I add the Position component, the entity is automatically (by default) assigned the position tags. Finally, there's always the option to manually disable, add, or remove tags to control everything in very specific situations.*

*Also, before you read on, I want to say that 95% of the code here is written by me. I'm an honest person, and I must tell you that the struct Tags (only the struct, TagsGroup was my idea) was found online and copied and pasted. However, I made numerous modifications and adapted it to my system and my needs. I fully understand what I'm copying and pasting; otherwise, what would be the point of working on and learning these concepts if, in the end, I'm not able to code what I claim to be doing?*

*Under construction*

```cpp
// Copyright (c) December 2025 Félix-Olivier Dumas. All rights reserved.
// Licensed under the terms described in the LICENSE file.

#pragma once
#include <iostream>
#include <vector>
#include <chrono>
#include <cstdint>
#include <stdexcept>
#include <type_traits>
#include <tuple>


namespace Tags {
    struct Colored {};
    struct Transparent {};
    struct Visible {};
    struct ShadowCaster {};
    struct ShadowReceiver {};
    struct Animated {};
    struct Sprite {};
    struct Mesh {};
    struct LightSource {};
    struct ParticleEmitter {};

    struct Rigid {};
    struct Movable {};
    struct Physics {};
    struct Collidable {};
    struct Trigger {};
    struct GravityAffected {};
    struct Kinematic {};
    struct Projectile {};
    struct Pickup {};
    struct Destructible {};
    struct Interactable {};

    struct AIControlled {};
    struct Pathfinding {};
    struct Patrol {};
    struct Attackable {};
    struct Friendly {};
    struct Hostile {};
    struct NPC {};
    struct Boss {};
    struct QuestObjective {};

    struct InputSensitive {};
    struct PlayerControlled {};
    struct Networked {};
    struct LocalPlayer {};
    struct RemotePlayer {};

    struct Persistent {};
    struct Temporary {};
    struct Dynamic {};
    struct Static {};
    struct Debug {};
    struct EditorOnly {};

    struct Replicated {};
    struct SyncPosition {};
    struct SyncState {};
    struct SyncAnimation {};

    struct SoundEmitter {};
    struct MusicEmitter {};
    struct AudioListener {};

    struct DisabledTag {};

    template<typename T>
    struct is_tag : std::false_type {};

    template<typename T>
    inline constexpr bool is_tag_v = is_tag<T>::value;

#define REGISTER_TAG(T) template<> struct is_tag<T> : std::true_type {}
    REGISTER_TAG(Colored);
    REGISTER_TAG(Transparent);
    REGISTER_TAG(Visible);
    REGISTER_TAG(ShadowCaster);
    REGISTER_TAG(ShadowReceiver);
    REGISTER_TAG(Animated);
    REGISTER_TAG(Sprite);
    REGISTER_TAG(Mesh);
    REGISTER_TAG(LightSource);
    REGISTER_TAG(ParticleEmitter);

    REGISTER_TAG(Rigid);
    REGISTER_TAG(Movable);
    REGISTER_TAG(Physics);
    REGISTER_TAG(Collidable);
    REGISTER_TAG(Trigger);
    REGISTER_TAG(GravityAffected);
    REGISTER_TAG(Kinematic);
    REGISTER_TAG(Projectile);
    REGISTER_TAG(Pickup);
    REGISTER_TAG(Destructible);
    REGISTER_TAG(Interactable);

    REGISTER_TAG(AIControlled);
    REGISTER_TAG(Pathfinding);
    REGISTER_TAG(Patrol);
    REGISTER_TAG(Attackable);
    REGISTER_TAG(Friendly);
    REGISTER_TAG(Hostile);
    REGISTER_TAG(NPC);
    REGISTER_TAG(Boss);
    REGISTER_TAG(QuestObjective);

    REGISTER_TAG(InputSensitive);
    REGISTER_TAG(PlayerControlled);
    REGISTER_TAG(Networked);
    REGISTER_TAG(LocalPlayer);
    REGISTER_TAG(RemotePlayer);

    REGISTER_TAG(Persistent);
    REGISTER_TAG(Temporary);
    REGISTER_TAG(Dynamic);
    REGISTER_TAG(Static);
    REGISTER_TAG(Debug);
    REGISTER_TAG(EditorOnly);

    REGISTER_TAG(Replicated);
    REGISTER_TAG(SyncPosition);
    REGISTER_TAG(SyncState);
    REGISTER_TAG(SyncAnimation);

    REGISTER_TAG(SoundEmitter);
    REGISTER_TAG(MusicEmitter);
    REGISTER_TAG(AudioListener);
#undef REGISTER_TAG
}

template <typename Derived>
struct Component {
    //using BaseType = Component<T>;
    void test() {
        printf("Just for testing purposes :)");
    }

    auto self() {
        //poubelle
        return static_cast<Derived*>(this);
    }


};

template<typename... Ts>
struct TagsGroup {
    //static_assert((Tags::is_tag<Ts>::value && ...), "All types must be valid tags");
    static_assert((Tags::is_tag_v<Ts> && ...), "All types must be valid tags");



    void test2() {
        printf("Just for testing purposes :)");
    }
};

namespace Components {
    namespace Internal {
        struct raw_position { std::uint32_t x, y; };
        struct raw_velocity { std::uint32_t vx, vy; };
        struct raw_rotation { std::uint8_t angle; };
        struct raw_scale { std::uint8_t sx, sy; };
        struct raw_color { std::uint8_t r, g, b, a; };
    }

    template<typename T>
    struct is_type : std::false_type {};

    template<typename T>
    static constexpr bool is_component_v = is_type<T>::value;

    //genre mettre des utilitaires comme same component
    //ou plus whatever

    struct Position : public Component <Position>,
        public TagsGroup<Tags::Movable, Tags::Physics>,
        public Internal::raw_position {
        void operator()(std::uint32_t x, std::uint32_t y) {
            this->x = x; this->y = y;
        }
    };

    //mettre les using ici avec les tags et le srtp de component<derived>

#define REGISTER_COMPONENT(T) template<> struct is_type<T> : std::true_type {} 
    REGISTER_COMPONENT(Position);
    REGISTER_COMPONENT(Internal::raw_velocity);
    REGISTER_COMPONENT(Internal::raw_rotation);
    REGISTER_COMPONENT(Internal::raw_scale);
    REGISTER_COMPONENT(Internal::raw_color);
    //prob remplacer par genre les struct externes directement
#undef REGISTER_COMPONENT
}

class EntityId {
private:
    static std::uint32_t _nextId;

public:
    static std::uint32_t Next() { return _nextId++; }
};

std::uint32_t EntityId::_nextId = 0;

struct Entity {
    std::uint32_t Value;

    Entity() : Value(EntityId::Next()) {}
};

struct Position : public Component<Position>,
    public TagsGroup<Tags::Movable, Tags::SyncPosition>
{
    std::uint32_t X, Y;
};

struct Velocity : public Component<Velocity>,
    public TagsGroup<Tags::Movable, Tags::Physics>
{
    std::uint32_t VX, VY;
};

struct Rotation : public Component<Rotation>,
    public TagsGroup<Tags::Physics, Tags::SyncState>
{
    std::uint8_t Angle;
};

struct Scale : public Component<Scale>,
    public TagsGroup<Tags::Visible>
{
    std::uint8_t X, Y;
};

struct Color : public Component<Color>,
    public TagsGroup<Tags::Colored, Tags::Visible>
{
    std::uint8_t R, G, B, A;
};

class Registry {
private:
    struct Constants {
        static constexpr std::uint32_t InitialEntityCapacity = 131072;
        static constexpr std::uint32_t InitialPoolCapacity = 262143;
    };
    struct Variables {
        std::vector<Position> _positions;
        std::vector<int> _entityToPosIndex;

        std::vector<Velocity> _velocities;
        std::vector<int> _entityToVelIndex;

        std::vector<Rotation> _rotations;
        std::vector<int> _entityToRotIndex;

        std::vector<Scale> _scales;
        std::vector<int> _entityToScaleIndex;

        std::vector<Color> _colors;
        std::vector<int> _entityToColorIndex;
    };

    //std::unordered_map<std::type_index, std::unique_ptr<void>> pools;

private:
    Constants constants;
    Variables variables;

private:
    using u8 = std::uint8_t;
    using u16 = std::uint16_t;
    using u32 = std::uint32_t;
    using u64 = std::uint64_t;

    using szt = std::size_t;

private:
    template<typename T>
    auto internal_add_component(std::uint32_t eidx) noexcept ->
        std::enable_if_t<std::is_class_v<T>
        && std::is_base_of_v<Component<T>, T>,
        void> /* Internal interface for adding components */ {
        std::cout << "[WXR Component] Added " << typeid(T).name() << " to Entity " << eidx << std::endl;

        //peut etre tuple pool ou whatever avec des types et ajouter un type
        //a la pool a chaque ajout de "nouveau" component et faire un unpacking 
        //du tuple<typename... Ts> et faire la lambda if constexpr... std::same...
        //pour chacun.

        if constexpr (std::is_same_v<T, Position>) {
            if (variables._entityToPosIndex[eidx] != -1) {
                printf("[ERROR] Entity %u already has Position component\n", eidx);
                return;
            }
            variables._entityToPosIndex[eidx] = variables._positions.size();
            variables._positions.emplace_back(Position{});
        }
        else if constexpr (std::is_same_v<T, Velocity>) {
            if (variables._entityToVelIndex[eidx] != -1) {
                printf("[ERROR] Entity %u already has Velocity component\n", eidx);
                return;
            }
            variables._entityToVelIndex[eidx] = variables._velocities.size();
            variables._velocities.emplace_back(Velocity{});
        }
        else if constexpr (std::is_same_v<T, Rotation>) {
            if (variables._entityToRotIndex[eidx] != -1) {
                printf("[ERROR] Entity %u already has Rotation component\n", eidx);
                return;
            }
            variables._entityToRotIndex[eidx] = variables._rotations.size();
            variables._rotations.emplace_back(Rotation{});
        }
        else if constexpr (std::is_same_v<T, Scale>) {
            if (variables._entityToScaleIndex[eidx] != -1) {
                printf("[ERROR] Entity %u already has Scale component\n", eidx);
                return;
            }
            variables._entityToScaleIndex[eidx] = variables._scales.size();
            variables._scales.emplace_back(Scale{});
        }
        else if constexpr (std::is_same_v<T, Color>) {
            if (variables._entityToColorIndex[eidx] != -1) {
                printf("[ERROR] Entity %u already has Color component\n", eidx);
                return;
            }
            variables._entityToColorIndex[eidx] = variables._colors.size();
            variables._colors.emplace_back(Color{});
        }
    }

    void internal_get_component() noexcept {

    }

    void internal_remove_component() noexcept {

    }

public:
    Registry(std::size_t maxEntities, std::size_t maxPool) {
        variables._entityToPosIndex.resize(maxEntities, -1);
        variables._entityToVelIndex.resize(maxEntities, -1);
        variables._entityToRotIndex.resize(maxEntities, -1);
        variables._entityToScaleIndex.resize(maxEntities, -1);
        variables._entityToColorIndex.resize(maxEntities, -1);

        variables._positions.reserve(maxPool);
        variables._velocities.reserve(maxPool);
        variables._rotations.reserve(maxPool);
        variables._scales.reserve(maxPool);
        variables._colors.reserve(maxPool);
    }


    template<typename... Ts>
    auto Add(std::uint32_t eidx) noexcept ->
        std::enable_if_t<(std::is_class_v<Ts> && ...) &&
        (std::is_base_of_v<Component<Ts>, Ts> && ...),
        void> /* Serves as a entry point for adding components */ {
        ((internal_add_component<Ts>(eidx)), ...);
    }

    template<typename... Ts>
    auto AddTag(std::uint32_t eidx) noexcept ->
        std::enable_if_t<(
            (Tags::is_tag_v<Ts>), ...),
        void> /* Securly add tags to an entity */ {
        ((std::cout << "[WXR Tag] Added "
            << typeid(Ts).name()
            << " to Entity "
            << eidx << std::endl)
            , ...);
    }


    template <typename T> void Add(std::uint32_t eidx) {
        if constexpr (std::is_same_v<T, Position>) {
            if (variables._entityToPosIndex[eidx] != -1) {
                printf("[ERROR] Entity %u already has Position component\n", eidx);
                return;
            }
            variables._entityToPosIndex[eidx] = variables._positions.size();
            variables._positions.emplace_back(Position{});
        }
        else if constexpr (std::is_same_v<T, Velocity>) {
            if (variables._entityToVelIndex[eidx] != -1) {
                printf("[ERROR] Entity %u already has Velocity component\n", eidx);
                return;
            }
            variables._entityToVelIndex[eidx] = variables._velocities.size();
            variables._velocities.emplace_back(Velocity{});
        }
        else if constexpr (std::is_same_v<T, Rotation>) {
            if (variables._entityToRotIndex[eidx] != -1) {
                printf("[ERROR] Entity %u already has Rotation component\n", eidx);
                return;
            }
            variables._entityToRotIndex[eidx] = variables._rotations.size();
            variables._rotations.emplace_back(Rotation{});
        }
        else if constexpr (std::is_same_v<T, Scale>) {
            if (variables._entityToScaleIndex[eidx] != -1) {
                printf("[ERROR] Entity %u already has Scale component\n", eidx);
                return;
            }
            variables._entityToScaleIndex[eidx] = variables._scales.size();
            variables._scales.emplace_back(Scale{});
        }
        else if constexpr (std::is_same_v<T, Color>) {
            if (variables._entityToColorIndex[eidx] != -1) {
                printf("[ERROR] Entity %u already has Color component\n", eidx);
                return;
            }
            variables._entityToColorIndex[eidx] = variables._colors.size();
            variables._colors.emplace_back(Color{});
        }
    }


    template<typename T> T& Get(std::uint32_t eidx) {
        if constexpr (std::is_same_v<T, Position>) {
            if (variables._entityToPosIndex[eidx] == -1)
                throw std::runtime_error(
                    "Entity " + std::to_string(eidx) + " does NOT have Position component"
                );
            return variables._positions[variables._entityToPosIndex[eidx]];
        }
        else if constexpr (std::is_same_v<T, Velocity>) {
            if (variables._entityToVelIndex[eidx] == -1)
                throw std::runtime_error(
                    "Entity " + std::to_string(eidx) + " does NOT have Velocity component"
                );
            return variables._velocities[variables._entityToVelIndex[eidx]];
        }
        else if constexpr (std::is_same_v<T, Rotation>) {
            if (variables._entityToRotIndex[eidx] == -1)
                throw std::runtime_error(
                    "Entity " + std::to_string(eidx) + " does NOT have Rotation component"
                );
            return variables._rotations[variables._entityToRotIndex[eidx]];
        }
        else if constexpr (std::is_same_v<T, Scale>) {
            if (variables._entityToScaleIndex[eidx] == -1)
                throw std::runtime_error(
                    "Entity " + std::to_string(eidx) + " does NOT have Scale component"
                );
            return variables._scales[variables._entityToScaleIndex[eidx]];
        }
        else if constexpr (std::is_same_v<T, Color>) {
            if (variables._entityToColorIndex[eidx] == -1)
                throw std::runtime_error(
                    "Entity " + std::to_string(eidx) + " does NOT have Color component"
                );
            return variables._colors[variables._entityToColorIndex[eidx]];
        }
        throw std::runtime_error("Component type not supported");
    }
};

int main() {
    std::cout << "Hello World!\n";

    constexpr size_t entityCount = 100000;
    constexpr size_t maxPool = 200000;

    Registry registry(entityCount, maxPool);

    registry.Add<Position, Velocity>(0);
    registry.Add<Position, Velocity>(1);

    registry.AddTag<Tags::Debug, Tags::NPC>(0);

    registry.AddTag<Tags::Colored>(0);
    registry.AddTag<Tags::Colored>(1);

    auto pos = registry.Get<Position>(0);
    pos.X = 2;

    std::cout << pos.X << std::endl;

    Components::Position position;

    position.x = 5;

    position(25, 50);

    std::cout << position.x << ", " << position.y << std::endl;
}
```
