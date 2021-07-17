#ifndef _ClassTerrain
#define _ClassTerrain

class Terrain : public sf::Drawable, public sf::Transformable {
public:

    void change_level(int lvl = 1) {
        Level _lvl = level(lvl, tilewh);
        current_level = lvl;
        spawn_point = _lvl.spawn_point;
        change_level(_lvl.blocks);
    }
    
    void next_level() {
        if (!(current_level+1 > Level::MAX_NUM)) {
            change_level(current_level+1);
        }
    }

    void prev_level() {
        if (!(current_level-1 <= 0)) {
            change_level(current_level-1);
        }
    }

    int get_level() {
        return current_level;
    }


    float tilewh = 100;
    sf::Vector2f world_bound;
    sf::Vector2f spawn_point;

    std::pair<int, sf::FloatRect> check_collision(sf::FloatRect const& other_rect) {
        
        if (!(other_rect.top == other_rect.left && other_rect.top == 0.f)) {
            auto vc = wall.getVertexCount();
            int iIntersects = -1;
            sf::FloatRect result;
            for (int i = 0; i < vc; i+=4) {
                sf::FloatRect wall_part(wall[i].position, sf::Vector2f(tilewh, tilewh));
                
                sf::FloatRect _result;
                bool intersects = wall_part.intersects(other_rect, _result);
                if (intersects) {
                    result = _result;
                    
                    if (wall[i].color == colortypes[0]) {
                        iIntersects = 0;
                    } else if (wall[i].color == colortypes[1]) {
                        iIntersects = 1;
                    } else if (wall[i].color == colortypes[2]) {
                        iIntersects = 2;
                    }
                   
                }
            }
            return std::pair<int, sf::FloatRect>(iIntersects, result);
        }
    }

    Terrain(int level, float worldBoundX = -1, float worldBoundY = -1) : 
        world_bound(worldBoundX, worldBoundY),
        spawn_point(0,0) {
        wall.setPrimitiveType(sf::Quads);
        change_level(level);
    }

    sf::VertexArray wall;

private:

    int current_level = -1;

    std::vector<sf::Color> colortypes;

    virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const {
        // apply the transform
        states.transform *= getTransform();

        // we don't use a texture
        states.texture = NULL;
    
        // draw the vertex array
        target.draw(wall, states);
    }


    void change_level(LevelBlocks level) {
        wall.resize(level.size()*4);

        colortypes = {sf::Color::Red, sf::Color::Green, sf::Color(0,0,150)};

        for (int i = 0; i < level.size(); i++) {
            wall[i*4].position = sf::Vector2f(level[i].second.x, level[i].second.y);
            wall[i*4+1].position = sf::Vector2f(level[i].second.x+tilewh, level[i].second.y);
            wall[i*4+2].position = sf::Vector2f(level[i].second.x+tilewh, level[i].second.y+tilewh);
            wall[i*4+3].position = sf::Vector2f(level[i].second.x, level[i].second.y+tilewh);
            auto color = colortypes[0];
            if (level[i].first == 1)
                color = colortypes[1];
            if (level[i].first == 2)
                color = colortypes[2];

            wall[i*4+0].color = color;
            wall[i*4+1].color = color;
            wall[i*4+2].color = color;
            wall[i*4+3].color = color;
        }
    }
};
#endif