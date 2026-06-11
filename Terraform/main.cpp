#include <SFML/Graphics.hpp>
#include <optional>
#include "PerlinNoise.h"
#include <iostream>
#include <vector>
#include <algorithm>
#include <ctime>

using namespace sf;
using namespace std;

// Variables

int moveDirectionX = 0;

float glowRadius = 20.0f;

float playerSize[2] = { 20.0f, 42.0f };

bool plrInAir = true;

float jumpCooldown = 0.2f;

Clock jumpTimer; // Calculates jump timing

float boxSize = 20.0f;

int extendOnXAxis = 2000;

const int GRID_WIDTH = 50000 / static_cast<int>(boxSize);

float baselineHeight = 250.0f;

float hillheight = 200.0f;

float speed = 4.0f;

float velSpeed = 60.0f;

bool collidingWithBlockCheck = false;

float gravity = 400.0f;

int main() {
    srand(static_cast<unsigned>(time(nullptr)));
    Clock clock; // Calculates delta time
    float dt = 0.016f;

    RenderWindow window(VideoMode({ 1000, 500 }), "Terraform: The First Edition");
    window.setFramerateLimit(60);

    PerlinNoise noisegen(rand());

    RectangleShape box(Vector2f({ boxSize, boxSize }));
    box.setFillColor(Color::White);
    FloatRect boxcollision(Vector2f({ 0.0f, 0.0f }), Vector2f({ boxSize, boxSize }));

    CircleShape glowy;

    Texture texture;
    if (!texture.loadFromFile("C:/Users/sengu/source/repos/Terraform/Terraform/moon.png")) {
        cout // Fix missing include side effect
            << "Could not load ground texture...\n";
    }
    texture.setRepeated(true);
    box.setTexture(&texture);

    Texture plrTex;
    if (!plrTex.loadFromFile("C:/Users/sengu/source/repos/Terraform/Terraform/thedestined.png")) {
        cout << "Could not load player texture...\n";
    }

    Vector2f plrPos = { 500.0f, 0.0f };
    Vector2f plrVelocity = { 0.0f, 0.0f };

    FloatRect thedestined(plrPos, Vector2f({ playerSize[0] - 2.0f, playerSize[1] - 1.0f }));

    RectangleShape followingPlr(Vector2f({ playerSize[0], playerSize[1] }));
    followingPlr.setTexture(&plrTex);

    View camera(Vector2f({ 500.0f, 250.0f }), Vector2f({ 1000.0f, 500.0f }));

    // Setting up the values for our groundLevel vector
    // so that the collision system knows which blocks 
    // to collide and, for the vertex array to scan the columns and rows
    vector<int> groundLevel(GRID_WIDTH);
    for (int i = 0; i < GRID_WIDTH; i++) {
        int x = i * static_cast<int>(boxSize);
        float noiseScale = 0.005f;
        float noise = noisegen.getNoise(x * noiseScale);
        groundLevel[i] = floor((baselineHeight + (noise * hillheight)) / boxSize) * boxSize;
    }

    // VertexArray to draw the blocks quickly
    VertexArray boxes(PrimitiveType::Triangles);
    for (int i = 0; i < GRID_WIDTH; i++) {
        float x = i * boxSize;
        for (float yy = groundLevel[i]; yy < 500.0f; yy += boxSize) {
            Vector2f tl = { x, yy };
            Vector2f tr = { x + boxSize, yy };
            Vector2f bl = { x, yy + boxSize };
            Vector2f br = { x + boxSize, yy + boxSize };

            Vertex v1, v2, v3, v4, v5, v6; // A triangle has 3 vertices and a square has two right angle triangles.
                                           // Hence, there are 6 vertices.

            v1.position = tl;   v1.texCoords = tl;
            v2.position = tr;   v2.texCoords = tr;
            v3.position = bl;   v3.texCoords = bl;

            v4.position = tr;   v4.texCoords = tr;
            v5.position = br;   v5.texCoords = br;
            v6.position = bl;   v6.texCoords = bl;

            boxes.append(v1);
            boxes.append(v2);
            boxes.append(v3);
            boxes.append(v4);
            boxes.append(v5);
            boxes.append(v6);
        }
    }

    while (window.isOpen()) { // Loop
        dt = clock.restart().asSeconds();
        if (dt > 0.1f) dt = 0.1f;

        while (const optional event = window.pollEvent()) {
            if (event->is<Event::Closed>()) {
                window.close();
            }
        }
        // Keybindings
        bool pressedKey = false;
        if (Keyboard::isKeyPressed(Keyboard::Key::A)) {
            plrVelocity.x = -speed * velSpeed;
            moveDirectionX = 1;
            pressedKey = true;
        }
        else if (Keyboard::isKeyPressed(Keyboard::Key::D)) {
            plrVelocity.x = speed * velSpeed;
            moveDirectionX = -1;
            pressedKey = true;
        }
        else {
            float frictionAmount = 1200.0f;
            if (plrVelocity.x > 0.0f) {
                plrVelocity.x -= frictionAmount * dt;
                if (plrVelocity.x < 0.0f) plrVelocity.x = 0.0f;
            }
            else if (plrVelocity.x < 0.0f) {
                plrVelocity.x += frictionAmount * dt;
                if (plrVelocity.x > 0.0f) plrVelocity.x = 0.0f;
            }
        }
        // Downward force
        plrVelocity.y += gravity * dt;
        
        if (Keyboard::isKeyPressed(Keyboard::Key::Space)) {
            if (!plrInAir && jumpTimer.getElapsedTime().asSeconds() >= jumpCooldown) {
                plrVelocity.y = -220.0f;
                plrInAir = true;
                jumpTimer.restart();
            }
        }

        int playerColumn = static_cast<int>(plrPos.x / boxSize);
        int sideChecks = 3;
        int startCol = max(0, playerColumn - sideChecks);
        int endCol = min(GRID_WIDTH - 1, playerColumn + sideChecks);

        plrPos.x += plrVelocity.x * dt;
        thedestined.position = plrPos;

        for (int i = startCol; i <= endCol; i++) {
            float x = i * boxSize;
            for (float yy = groundLevel[i]; yy < 500.0f; yy += boxSize) {
                boxcollision.position = { x, yy };
                if (thedestined.findIntersection(boxcollision)) {
                    if (plrVelocity.x > 0.0f) {
                        plrPos.x = boxcollision.position.x - thedestined.size.x;
                    }
                    else if (plrVelocity.x < 0.0f) {
                        plrPos.x = boxcollision.position.x + boxcollision.size.x;
                    }
                    plrVelocity.x = 0.0f;
                    thedestined.position = plrPos;
                }
            }
        }

        // Intersection checks using groundLevel's values

        collidingWithBlockCheck = false;
        plrPos.y += plrVelocity.y * dt;
        thedestined.position = plrPos;

        for (int i = startCol; i <= endCol; i++) {
            float x = i * boxSize;
            for (float yy = groundLevel[i]; yy < 500.0f; yy += boxSize) {
                boxcollision.position = { x, yy };
                if (thedestined.findIntersection(boxcollision)) {
                    if (plrVelocity.y > 0.0f) {
                        collidingWithBlockCheck = true;
                        plrInAir = false;
                        plrPos.y = boxcollision.position.y - thedestined.size.y;
                    }
                    else if (plrVelocity.y < 0.0f) {
                        plrPos.y = boxcollision.position.y + boxcollision.size.y;
                    }
                    plrVelocity.y = 0.0f;
                    thedestined.position = plrPos;
                }
            }
        }

        if (!collidingWithBlockCheck) {
            plrInAir = true;
        }

        // The formula: plrPos + ( playerSize / 2 ) - radius

        float finalPositionX = plrPos.x + (playerSize[0] / 2.0f) - glowRadius;
        float finalPositionY = plrPos.y + (playerSize[1] / 2.0f) - glowRadius;
        
        glowy.setPosition(Vector2f({ finalPositionX, finalPositionY }));
        glowy.setFillColor(Color(255, 255, 255, 75));
        glowy.setRadius(20.0f);

        followingPlr.setPosition(plrPos);
        camera.setCenter(plrPos);

        window.clear(Color::Black);
        window.setView(camera);

        // Draw
        window.draw(boxes, &texture);
        window.draw(glowy);
        window.draw(followingPlr);
        window.display();
    }

    return 0;
}
