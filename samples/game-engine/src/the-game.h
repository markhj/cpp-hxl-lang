#pragma once

#include <engine.h>
#include <hxl-lang/hxl-lang.h>

using namespace HXL;
using namespace TinyButEpic;

namespace WatchGrassGrowSimulator {
    /**
     * A custom entity developed specifically for this game.
     */
    class BirdFlock : public Entity3D {
    public:
        int count = 1;
        std::string type;
        bool attacksPlayer = false;
    };

    /**
     * Example of a class that can set up HXL on a per-project
     * basis.
     */
    class HXLSetup {
    public:
        /**
         * Define the schema
         *
         * @return
         */
        static Schema createSchema();

        /**
         * Define the deserialization protocol.
         *
         * @param renderables
         * @param materials
         * @param birdFlocks
         * @return
         */
        static DeserializationProtocol createDeserialization(std::vector<std::shared_ptr<Renderable>> &renderables,
                                                             MaterialsContainer &materials,
                                                             std::vector<BirdFlock> &birdFlocks);
    };
}
