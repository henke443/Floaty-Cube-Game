#ifndef _ClassPlayer
#define _ClassPlayer
namespace player {

class Physics {
public: 
    float earth = M_E; // number to divide with to yield approximate earth values,.
    float mass = 1.0/earth;
    float mov_resistance = 0.001/earth;
    float mov_bouncyness = 0.6;
    float mov_force = 0.8; // 1.5 is enough to lift 1.0/earth
    float mov_max_speed = 10000.0; // very roughly, ~10m/s is our maximum speed, 
                                   // if a 32x32 cube is 1 meters across.
    float mov_gravity = 9.82/earth; // 9.82/earth
    float mov_speed = 0.0;

    bool can_move = true; // Used for freezing movement.

    sf::Vector2f mov_velocity = sf::Vector2f(0.f, 0.f);
    virtual void update(sf::Time elapsed, bool gravity) = 0;
};

class Player : 
    public sf::Drawable, 
    public sf::Transformable, 
    public Physics {

public:

    float health = 100.0;
    float ground_resistance = 0.05; // additional resistance from ground.
    bool mov_no_forward_force = true;
    
    float up_to_sidethrust_ratio = M_E;

    sf::RenderWindow const& window;

    int width;
    int height;

    sf::Vector2i mov_vec_direction; // unit vector

    float time_dead = 0.f;

    enum class Direction {Up, Down, Left, Right};

    
    void setTerrain(Terrain& t) {
        terrain = t;
    }
    
    void respawn() {
        health = 100;
        time_dead = 0.f;

        setPosition(terrain.spawn_point);
        player_shape.setRotation(0.f);
        player_shape.setScale(1.f, 1.f);

        terrain.change_level(terrain.get_level());
        mov_velocity = {0.f, 0.f};
        hud.reset();
        can_move = true;
        //setPosition( window.getSize().x/2, window.getSize().y/2 );
    }

    void die(sf::Time elapsed) {

        bool first_move = false;
        float respawn_time = 1.f;

        if (can_move == true) { 
            first_move = true; 
        }
        else if (time_dead >= respawn_time) {
            respawn();
            return;
        }

        //player_shape.setRotation((360*M_PI_2)*(time_dead/3.f));
        player_shape.rotate(6*(time_dead/respawn_time));
        auto sz = 1.f-(time_dead/respawn_time);
        player_shape.setScale(sz, sz);

        can_move = false;
        time_dead += elapsed.asSeconds();
        mov_velocity = {0.f, 0.f};/**/

        if (first_move) {
            death_sound.play();
        }
        
        //death_sound.play();
        //respawn();
    }

    // TODO: this is the most important function to change soon because
    // it is messy af. Moves the sprite based on forces active on the player and updates the physics variables.
    void update(sf::Time elapsed, bool gravity = true) {
        if (!can_move) {
            gravity = false;
            mov_velocity = {0.f, 0.f};
            mov_vec_direction = {0, 0};
        }

        auto boxpos = getPosition()-player_shape.getOrigin();

        // Check for collision with worldbox bounds.
        // note that we've not yet restricted the positive y direction.
        if (boxpos.y + height >= window.getSize().y) {
            setPosition(getPosition().x, window.getSize().y - height + player_shape.getOrigin().y);
            mov_velocity.y = -mov_velocity.y*mov_bouncyness;
            mov_velocity.x *= 1-ground_resistance;
        }
        if (terrain.world_bound.x != -1.f
            && boxpos.x + width >= terrain.world_bound.x) {

            setPosition(terrain.world_bound.x - width, getPosition().y);
            mov_velocity.x = -mov_velocity.x*mov_bouncyness;
        }
        if (boxpos.x <= 0) {
            setPosition(0, getPosition().y);
            mov_velocity.x = -mov_velocity.x*mov_bouncyness;
        }

        if (mov_vec_direction != sf::Vector2i(0,0)) {
            mov_no_forward_force = false;
        } else {
            mov_no_forward_force = true;
            mov_velocity.x *= 1-mov_resistance;
            mov_velocity.y *= 1-mov_resistance;
        }

        // Check for collision with terrain
        
        sf::FloatRect playerRect(boxpos, 
            sf::Vector2f(player_shape.getSize().x, player_shape.getSize().y)
        );
        
        auto col = terrain.check_collision(playerRect);
        if (col.first != -1) {
            auto _x = col.second.left;
            auto _y = col.second.top;
            bool collided_vert = false; // has collided vertically
            
            //if (!(col.second.width <= 5.f && col.second.height <= height/2)) {
            if (col.first == 1) {

                if (col.second.width <= col.second.height) {
                    mov_velocity.x = -mov_velocity.x*mov_bouncyness;
                    mov_velocity.y *= 1-ground_resistance;

                    if (col.second.left-boxpos.x <= 0) {
                        setPosition(getPosition()+(
                            sf::Vector2f(col.second.left+1.f, col.second.top)-boxpos)
                        );
                    } else {
                        setPosition(getPosition()+(
                            sf::Vector2f(col.second.left-width, col.second.top)-boxpos)
                        );
                    }
                } else {
                    mov_velocity.y = -mov_velocity.y*mov_bouncyness;
                    mov_velocity.x *= 1-ground_resistance;

                    if (col.second.top-boxpos.y <= 0) {
                        setPosition(getPosition()+(sf::Vector2f(col.second.left, 
                            col.second.top+1.f)-boxpos));
                    } else {
                        setPosition(getPosition()+(sf::Vector2f(col.second.left, 
                            col.second.top-height)-boxpos));
                    }
                }

            } else if (col.first == 0) {
                die(elapsed);
            } else if (col.first == 2) {
                std::cout << "WON!" << std::endl;
                victory_sound.play();
                hud.victory(terrain.get_level());
                terrain.next_level();
                respawn();
            }
        }

        mov_velocity.x += (static_cast<float>(mov_vec_direction.x)*mov_force/mass);
        mov_velocity.y += (static_cast<float>(mov_vec_direction.y)*up_to_sidethrust_ratio*mov_force/mass);
        
        if (std::abs(mov_velocity.x) > mov_max_speed)
            mov_velocity.x = (mov_velocity.x > 0) ? mov_max_speed : -mov_max_speed;

        if (std::abs(mov_velocity.y) > mov_max_speed)
            mov_velocity.y = (mov_velocity.y > 0) ? mov_max_speed : -mov_max_speed;

        if (gravity)
            mov_velocity.y += mov_gravity;

        //float angle = 180 * M_PI / 180.f;
        //mov_velocity += sf::Vector2f(std::cos(angle), std::sin(angle));

        setPosition(getPosition()+mov_velocity*elapsed.asSeconds());
        mov_vec_direction = sf::Vector2i(0,0);
        mov_no_forward_force = true;
    }
    
    // Applies force in a direction, but does not change the actual position of the sprite,
    // call update() to calculate all physics etc and change it later.
    void move(Direction d) {
        switch (d) {
            case Direction::Up: {
                mov_vec_direction.y -= 1.f;
                break;
            };
            case Direction::Down: {
                mov_vec_direction.y += 1.f;
                break;
            };
            case Direction::Left: {
                mov_vec_direction.x -= 1.f;
                break;
            };
            case Direction::Right: {
                mov_vec_direction.x += 1.f;
                break;
            }
        }
    }

    Player(sf::RenderWindow const& window, Terrain& terrain, HUD& hud, int width = 200, int height = 100) : 
        terrain(terrain),
        width(width), 
        height(height),
        hud(hud),
        player_shape(sf::RectangleShape(sf::Vector2f(width, height))),
        window(window) {
        
            if (!death_sound_buf.loadFromFile("plop.wav"))
                throw std::runtime_error("Could not load death.wav.\n");

            death_sound.setBuffer(death_sound_buf);
            death_sound.setVolume(50.f);


            if (!victory_sound_buf.loadFromFile("pling.wav"))
                throw std::runtime_error("Could not load death.wav.\n");

            victory_sound.setBuffer(victory_sound_buf);
            victory_sound.setVolume(50.f);

            player_shape.setOrigin(player_shape.getSize().x/2, player_shape.getSize().y/2);
            player_shape.setFillColor(sf::Color(0, 0, 25));
            player_shape.setOutlineColor(sf::Color::Blue);
            player_shape.setOutlineThickness(3);
            
        }
    
private:
    Terrain& terrain;
    HUD& hud;
    sf::RectangleShape player_shape;

    sf::SoundBuffer death_sound_buf;
    sf::Sound death_sound;

    sf::SoundBuffer victory_sound_buf;
    sf::Sound victory_sound;


    virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const
    {
        // apply the transform
        states.transform *= getTransform();

        // we don't use a texture
        states.texture = NULL;
    
        // draw the vertex array
        target.draw(player_shape, states);
    }
        
};
}

#endif