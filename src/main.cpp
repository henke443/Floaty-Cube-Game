#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>

#include <cmath>
#include <algorithm>
#include <iostream>
#include <random>
#include <exception>
#include <sstream>
#include <fstream>
#include <iomanip>
#include <map>

#include "json.hpp"
using json = nlohmann::json;

#include "levels.hpp" // Has to be included before terrain as of now.
#include "terrain.hpp"
#include "hud.hpp"
#include "player.hpp"
#include "particles.hpp"

using namespace player;
using namespace sf;

class Keypress {
public:
    Keyboard::Key key;
    std::map<Keyboard::Key, bool> can_press;
    bool pressed(bool sticky = true) {
        if (Keyboard::isKeyPressed(key)) {
            if (!sticky)
                return true;
            else if(can_press[key]) {
                can_press[key] = false;
                return true;
            }
        } else {
            can_press[key] = true;
            return false;
        }
    }
    bool pressed(Keyboard::Key _key, bool sticky = true) {
        key = _key;
        return pressed(sticky);
    }
};

int main() {


    VideoMode desktop = VideoMode().getDesktopMode();
	ContextSettings settings;
	settings.antialiasingLevel = 8;
    RenderWindow window(desktop, "cubo.id", Style::Fullscreen, settings);
	window.setMouseCursorVisible(false);
    
    CircleShape mouse_shape(5.f);
    mouse_shape.setFillColor(Color::Yellow);

    View view = window.getDefaultView();
    //view.zoom(2.f);
    //view.reset(FloatRect(200.f, 200.f, 300.f, 200.f));
    view.setViewport(FloatRect(0.f, 0.f, 1.f, 1.f));
    window.setView(view);
    
    // Terrain(int level, float worldBoundX = -1, float worldBoundY = -1) : 
    Terrain terrain{1, 10000.0};

    // HUD
    HUD hud{view};

    // Create our player and using our terrain 
    Player player{window, terrain, hud, 32, 32};

    auto mouse_pos = Vector2f(Mouse::getPosition(window));

    Keypress key;

    // create a clock to track the elapsed time
    Clock clock;
    
    // run the program as long as the window is open
    while (window.isOpen()) {

        mouse_pos = Vector2f(Mouse::getPosition(window));
        // draw everything here...
        // window.draw(...);
        mouse_shape.setPosition(mouse_pos-Vector2f(mouse_shape.getRadius(), mouse_shape.getRadius()));
        // check all the window's events that were triggered since the last iteration of the loop
        Event event;
        while (window.pollEvent(event)) {
            // "close requested" event: we close the window
            if (event.type == Event::Closed) {
                window.close();
            }
        }

        Time elapsed = clock.restart();

        // clear the window with black color
        window.clear(Color::Black);

        window.draw(terrain);
        window.draw(mouse_shape);
        window.draw(player);
        window.draw(hud);

        // end the current frame
        window.display();

        auto ppos = player.getPosition();
        auto view_pos = ppos;
        view_pos.y = std::min((float)window.getSize().y/2, view_pos.y);
        view.setCenter(view_pos);
        
        window.setView(view);

        if (key.pressed(Keyboard::Escape)) {
            return 0;
        }
        if (key.pressed(Keyboard::W, false)) {
            player.move(Player::Direction::Up);
        }
        if (key.pressed(Keyboard::A, false)) {
            player.move(Player::Direction::Left);
        }
        if (key.pressed(Keyboard::S, false)) {
            player.move(Player::Direction::Down);
        }
        if (key.pressed(Keyboard::D, false)) {
            player.move(Player::Direction::Right);
        }
        if (key.pressed(Keyboard::R)) {
            terrain.change_level(1);
            player.respawn();
        }
        if (key.pressed(Keyboard::T)) {
            terrain.next_level();
            player.respawn();
        }
        if (key.pressed(Keyboard::F)) {
            terrain.prev_level();
            player.respawn();
        }
        
        player.update(elapsed);
        hud.update(elapsed, terrain.get_level());
    }

    return 0;
}