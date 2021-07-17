#ifndef _ClassHUD
#define _ClassHUD

class HUD : public sf::Drawable, public sf::Transformable {
public:

    sf::Font font_p;
    sf::View& view;

    HUD(sf::View& v) : 
        view(v) {
        if (!font_p.loadFromFile("GermaniaOne-Regular.ttf")) {
            throw std::runtime_error("Could not load GermaniaOne-Regular.ttf");
        }

        auto state = read_state();
        for (auto& level_state : state) {
            best_times.push_back(level_state[0]);
        }

        auto tta = &text_time_alive;
        auto tbt = &text_best_time;

        tta->setFont(font_p);
        tta->setCharacterSize(30); //pixels
        tta->setFillColor(sf::Color::White);
        tta->setStyle(sf::Text::Bold);
        tta->setString("Loading...");

        tbt->setPosition(tta->getPosition()
            + sf::Vector2f(0, 30)
        );
        tbt->setFont(font_p);
        tbt->setCharacterSize(24); //pixels
        tbt->setFillColor(sf::Color::Red);
    }

    void inc_time_alive(sf::Time const& t) {
        time_alive += t;
        std::ostringstream strs;
        float secs = time_alive.asSeconds();
        strs.precision(2);
        strs << std::fixed << secs;
        text_time_alive.setString(strs.str());
    }

    void write_state(json state) {
        std::ofstream out("state.json");
        out << std::setw(4) << state << std::endl;
    }

    void write_score(int level) {
        if (best_times.size() <= level-1) {
            throw std::runtime_error("best_times.size() was too small :/ Weird error.");
        }
        //std::ofstream state_file("state.json");
        auto state = read_state();
        state[level-1][0] = best_times[level-1];
        write_state(state);
    }

    // Returns the highscore for a level
    float read_score(int level) {
        auto state = read_state(level);
        return state[0];
    }

    // Read_state should return a json object which contains state for all levels, like high score etc.
    json read_state() {
        json j;
        try {
            std::ifstream state("state.json");
            state >> j;
        } 
        catch (std::exception& e) {
            //std::cout << e.what() << std::endl;
        }
        
        return j;
    }

    // The return value of this convenience function should be a json list and the first element is the highscore.
    json read_state(int level) {
        return read_state()[level-1];
    }

    void victory(int level) {
        while (best_times.size() < level) {
            best_times.push_back(std::numeric_limits<float>::max());
        }
        
        auto ta = time_alive.asSeconds();

        if (ta < best_times[level-1] && ta > 0.001f) {
            best_times[level-1] = ta;
        }

        write_score(level);
    }
    
    void reset() {
        time_alive = sf::Time::Zero;
    }

    void update(sf::Time const& t, int level) {
        
        if (best_times.size() >= level) {
            std::ostringstream strs;
            strs.precision(2);
            strs << std::fixed << best_times[level-1];
            text_best_time.setString(strs.str());
            //text_best_time.setString(strs.str() + " for level " + std::to_string(level));
        } else {
            text_best_time.setString(" - - -");
        }

        inc_time_alive(t);
        //text_time_alive.setPosition(view.getCenter());
        text_time_alive.setPosition(view.getCenter()-sf::Vector2f(
                view.getSize().x/2-15, view.getSize().y/2-10
            )
        );
        //text_best_time.setPosition(view.getCenter());
        text_best_time.setPosition(text_time_alive.getPosition()
            + sf::Vector2f(0, 30)
        );
    }

private:

    sf::Time time_alive;
    std::vector<float> best_times;
    sf::Text text_time_alive;
    sf::Text text_best_time;

    virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const
    {
        // apply the transform
        states.transform *= getTransform();

        // we don't use a texture
        states.texture = NULL;
    
        target.draw(text_time_alive, states);
        target.draw(text_best_time, states);
    }
};


#endif