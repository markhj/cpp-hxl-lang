#include "hxl-lang/hxl-lang.h"
#include "hxl-lang/utilities/prfr-printer.h"

#include "engine.h"
#include "the-game.h"

#include <fstream>
#include <iostream>
#include <sstream>
#include <thread>

using namespace HXL;
using namespace WatchGrassGrowSimulator;

int main() {
    MaterialsContainer materials;
    std::vector<BirdFlock> birdFlocks;
    std::vector<std::shared_ptr<Renderable>> renderables;

    DeserializationProtocol protocol = HXLSetup::createDeserialization(renderables, materials, birdFlocks);
    Schema schema = HXLSetup::createSchema();

    try {
        std::ifstream file("./scene.hxl");
        if (!file) {
            throw std::runtime_error("Could not open file - verify it's copied to right location with CMake");
        }

        std::ostringstream oss;
        oss << file.rdbuf();

        // Run the HXL source through the entire translation process
        ProcessResult result = Processor::process(oss.str(), schema, protocol);

        // IF there are errors, we show them
        if (!result.errors.empty()) {
            throw std::runtime_error(result.errors[0].message);
        }

        // Render the scene
        std::cout << "SETTING THE SCENE:" << std::endl;
        Renderer().render(renderables);

        float delta = 0.5,
              time = 0.0,
              until = 3.0,
              grassLength = 1.0,
              grassGrows = 0.04,
              enemyAppears = 1.4;

        bool enemyEval = false,
             playerDied = false;

        std::cout << "\nACTION-PACKED GAMEPLAY:" << std::endl;
        while (time <= until) {
            std::cout << "Your grass is: " << grassLength << " cm" << std::endl;
            if (time > enemyAppears && !enemyEval) {
                std::cout << "\nA BAD FEELING WASHES OVER YOU..." << std::endl;
                std::this_thread::sleep_for(std::chrono::milliseconds(1500));
                if (birdFlocks.empty()) {
                    std::cout << "But it seems like you're just being paranoid for no reason...\n" << std::endl;
                } else if (birdFlocks[0].attacksPlayer) {
                    std::cout << "You were attacked by " << birdFlocks[0].count << " " << birdFlocks[0].type << "s and died horribly." << std::endl;
                    std::this_thread::sleep_for(std::chrono::milliseconds(1500));
                    std::cout << "Good news is: Your grass still grows :-)" << std::endl;
                    std::this_thread::sleep_for(std::chrono::milliseconds(1500));
                    std::cout << "But for you it's GAME OVER!" << std::endl;
                    playerDied = true;
                    break;
                } else {
                    std::cout << "A flock of " << birdFlocks[0].count << " " << birdFlocks[0].type << "s just flew by. They looked friendly and nice, though.\n" << std::endl;
                }
                enemyEval = true;
            }
            grassLength += grassGrows;
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
            time += delta;
        }

        if (!playerDied) {
            std::cout << "\nAchievement unlocked! You have longer grass" << std::endl;
        }

    } catch (const std::exception &e) {
        std::cerr << e.what() << std::endl;
    }
}
