#include <SFML/Graphics.hpp>

#include "FrameBuffer.h"

int main()
{
    const int WIDTH  = 1280;
    const int HEIGHT = 720;

    sf::RenderWindow window(sf::VideoMode(WIDTH, HEIGHT), "SFML works!");

    FrameBuffer::Init(WIDTH, HEIGHT);

    // Generate image and texture to display
    sf::Image   image;
    sf::Texture texture;
    sf::Sprite  sprite;
    texture.create(WIDTH, HEIGHT);
    image.create(WIDTH, HEIGHT, sf::Color::Black);

    // Init the clock
    sf::Clock clock;
    while (window.isOpen())
    {
        // Handle input
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
                window.close();
        }

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Escape))
            window.close();

        // Fill framebuffer
        sf::Time elapsed = clock.getElapsedTime();
        int      time    = static_cast<int>(elapsed.asSeconds());

        for (unsigned x = 0; x < WIDTH; x++)
        {
            for (unsigned y = 0; y < HEIGHT; y++)
            {
                if (time % 2 == 0)
                {
                    if (y % 50 < 25 && x % 50 < 25)
                        FrameBuffer::SetPixel(x, y, 255, 0, 0);
                    else
                        FrameBuffer::SetPixel(x, y, 0, 255, 0);
                }
                else
                {
                    if (y % 50 < 25 && x % 50 < 25)
                        FrameBuffer::SetPixel(x, y, 0, 255, 0);
                    else
                        FrameBuffer::SetPixel(x, y, 255, 0, 0);
                }
            }
        }

        // Show image on screen
        FrameBuffer::ConvertFrameBufferToSFMLImage(image);

        texture.update(image);
        sprite.setTexture(texture);
        window.draw(sprite);
        window.display();
    }
    
    FrameBuffer::Free();

    return 0;
}