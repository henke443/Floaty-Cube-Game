#ifndef _ClassParticleSystem
#define _ClassParticleSystem

class ParticleSystem : public sf::Drawable, public sf::Transformable
{
public:

    ParticleSystem(unsigned int count, float lifetime = 1.f) :
        m_particles(count),
        m_lifetime(sf::seconds(lifetime)),
        m_emitter(0.f, 0.f) {

        for (int i = 0; i < count; i++) {
            m_shapes.push_back(sf::CircleShape(5.f));
        }
    }

    void setEmitter(sf::Vector2f position) {
        m_emitter = position;
    }

    sf::Vector2f getEmitter() {
        return m_emitter;
    }

    std::vector<sf::Vector2f> check_collision(sf::Transformable const& other, sf::Vector2f rect_bound) {
        std::vector<sf::Vector2f> vecvec2f;
        for (int i = 0; i < m_particles.size(); i++) {
            auto op = other.getPosition();
            auto sp = m_shapes[i].getPosition();
            auto sr = m_shapes[i].getRadius();
            // need to account for other shapes rect bound here obv., time to sleep.
            if (op.x+rect_bound.x >= sp.x+sr &&
                op.x <= sp.x+sr &&
                op.y+rect_bound.y >= sp.y+sr &&
                op.y <= sp.y+sr) {
                vecvec2f.push_back(sp);
                //resetParticle(i);
                m_particles[i].velocity.x = -m_particles[i].velocity.x;
                
                m_particles[i].velocity.y = -m_particles[i].velocity.y;
            }
        }
        return vecvec2f;
    }

    void update(sf::Time elapsed) {
        
        for (std::size_t i = 0; i < m_particles.size(); i++) {
            // update the particle lifetime
            Particle& p = m_particles[i];
            p.lifetime -= elapsed;

            // if the particle is dead, respawn it
            if (p.lifetime <= sf::Time::Zero)
                resetParticle(i);

            // update the position of the corresponding vertex
            //p.velocity -= sf::Vector2f(0.01, 0.01);
            if (p.velocity.x > 0) {
                p.velocity.x -= 0.2*p.lifetime.asSeconds();
            } else {
                p.velocity.x += 0.2*p.lifetime.asSeconds();
            }
            if (p.velocity.y > 0) {
                p.velocity.y -= 0.2*p.lifetime.asSeconds();
            } else {
                p.velocity.y += 0.2*p.lifetime.asSeconds();
            }
            m_shapes[i].setPosition(m_shapes[i].getPosition() + p.velocity * elapsed.asSeconds());

            // update the alpha (transparency) of the particle according to its lifetime
            float ratio = p.lifetime.asSeconds() / m_lifetime.asSeconds();
            m_shapes[i].setFillColor(
                sf::Color(
                    static_cast<sf::Uint8>(255),//std::max(0, std::min((int)(255-ratio*255)*2, 255))), 
                    static_cast<sf::Uint8>(100*ratio),
                    static_cast<sf::Uint8>(50*ratio),
                    static_cast<sf::Uint8>(std::max(0.f, (200*ratio)))
                )
            );
            //m_shapes[i].setRadius(std::max(5.f, 30*ratio));
        }
    }

private:

    virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const
    {
        // apply the transform
        states.transform *= getTransform();

        // our particles don't use a texture
        states.texture = NULL;

        for (int i = 0; i < m_particles.size(); i++) {
            target.draw(m_shapes[i], states);
        }
    }

private:


    struct Particle
    {
        sf::Vector2f velocity;
        sf::Time lifetime;
    };
    
    std::vector<Particle> m_particles;
    std::vector<sf::CircleShape> m_shapes;
    sf::Time m_lifetime;
    sf::Vector2f m_emitter;

    void resetParticle(std::size_t index)
    {
        // give a random velocity and lifetime to the particle
        float angle = (std::rand() % 360) * 3.14f / 180.f;
        float speed = 100.f;
        m_particles[index].velocity = sf::Vector2f(std::cos(angle) * speed, std::sin(angle) * speed);
        m_particles[index].lifetime = sf::milliseconds(std::rand() % 1000 + m_lifetime.asMilliseconds());

        // reset the position of the corresponding vertex
        m_shapes[index].setPosition(m_emitter);
    }
};
#endif