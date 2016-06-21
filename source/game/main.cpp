#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>

int main()
{
    sf::ContextSettings settings;
    settings.antialiasingLevel = 16;
    auto window = std::make_unique<sf::RenderWindow>(sf::VideoMode(1280, 960), "game", sf::Style::Titlebar | sf::Style::Close, settings);
    window->setVerticalSyncEnabled(true);

    sf::Texture bgTexture;
    bgTexture.loadFromFile("assets/bg.png");
    sf::Sprite bgSprite(bgTexture);

    sf::Texture tile1Texture;
    tile1Texture.loadFromFile("assets/tiles/1.png");
    sf::Sprite tile1Sprite(tile1Texture);
    tile1Sprite.setPosition(0, 960 - 128);
    sf::Sprite tile1bSprite(tile1Texture);
    tile1bSprite.setPosition(1280 - 128, 960 - 128);

    sf::Texture tile2Texture;
    tile2Texture.loadFromFile("assets/tiles/2.png");
    sf::Sprite tile2Sprite(tile2Texture);
    tile2Sprite.setPosition(128, 960 - 128);

    sf::Texture tile3Texture;
    tile3Texture.loadFromFile("assets/tiles/3.png");
    sf::Sprite tile3Sprite(tile3Texture);
    tile3Sprite.setPosition(256, 960 - 128);

    sf::Texture treeTexture;
    treeTexture.loadFromFile("assets/objects/tree.png");
    sf::Sprite treeSprite(treeTexture);
    treeSprite.setPosition(0, 960 - 128 - treeTexture.getSize().y);

    sf::Texture grassTexture;
    grassTexture.loadFromFile("assets/objects/grass2.png");
    sf::Sprite grassSprite(grassTexture);
    grassSprite.setPosition(256, 960 - 128 - grassTexture.getSize().y);

    sf::Texture cactusTexture;
    cactusTexture.loadFromFile("assets/objects/cactus3.png");
    sf::Sprite cactusSprite(cactusTexture);
    cactusSprite.setPosition(1280 - 128, 960 - 128 - cactusTexture.getSize().y);

    while (window->isOpen())
    {
        sf::Event event;
        while (window->pollEvent(event))
            if (event.type == sf::Event::Closed)
                window->close();

        window->clear();
        window->draw(bgSprite);
        window->draw(tile1Sprite);
        window->draw(tile1bSprite);
        window->draw(tile2Sprite);
        window->draw(tile3Sprite);
        window->draw(treeSprite);
        window->draw(grassSprite);
        window->draw(cactusSprite);
        window->display();
    }
}
