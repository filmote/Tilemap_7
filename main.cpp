// Copyright (C) 2019 Hannu Viitala
// 
// The source code in this file is released under the MIT license.
// Go to http://opensource.org/licenses/MIT for the full license details.
// 
// Converted to C++ and extended by Filmote

// *** A TILEMAP DEMO FOR THE POKITTO ***

#include "Pokitto.h"
#include "Tilemap.hpp"
#include "Data.h"
#include "images/Images.h"

using PC = Pokitto::Core;
using PD = Pokitto::Display;

namespace Constants {
    
    const uint8_t tileWidth = 16;
    const uint8_t tileHeight = 16;
    
    const uint8_t mapTileWidth = 16;                              // Map width in tiles ..
    const uint8_t mapTileHeight = 16;                             // Map height in tiles ..
    
    const uint16_t worldWidth = mapTileWidth * tileWidth;         // 16 tiles of 16 pixels
    const uint16_t worldHeight = mapTileHeight * tileHeight;      // 16 tiles of 16 pixels
    
    const uint16_t screenCentreX = PD::width / 2; 
    const uint16_t screenCentreY = PD::height / 2;
    
}



// ---------------------------------------------------------------------------------------


enum TileType {
    
	Water = 0,
	Green = 1,
	Tree = 2,
	Grass = 3,
	Brick = 4,
	Door_Closed = 5,
	Door_Open = 6,
	Key = 7,
	Carpet = 8,
	
}; 


enum Direction {
    
	Up,
	Down,
	Left,
	Right
	
}; 


struct Entity {
    
    int16_t x;
    int16_t y;

    const uint8_t width = 12;
    const uint8_t height = 15;

};


struct Player : Entity {

    bool hasKey = false;

};


struct Enemy : Entity {

};


// ---------------------------------------------------------------------------------------


#if MAX_TILE_COUNT == 16

    uint8_t worldMap[Constants::mapTileWidth * Constants::mapTileHeight / 2];
    
#else

    uint8_t worldMap[Constants::mapTileWidth * Constants::mapTileHeight];
    
#endif

uint8_t currentWorld = 0;
uint16_t numberOfEnemies = 0;

Tilemap tilemap;
Player player;
Enemy enemies[3];


// ---------------------------------------------------------------------------------------


void initWorld(uint8_t worldIndex) {


    // Populate the world data ..
    
    #if MAX_TILE_COUNT == 16
    
        for (uint16_t i = 0; i < Constants::mapTileWidth * Constants::mapTileHeight / 2; i++) {
            
            worldMap[i] = Data::worldMaps[worldIndex][i];
        }

    #else    
    
        for (uint16_t i = 0; i < Constants::mapTileWidth * Constants::mapTileHeight; i++) {
            
            worldMap[i] = Data::worldMaps[worldIndex][i];
        }

    #endif
    

    // Populate the enemy starting positions ..
    
    numberOfEnemies = Data::startingPostions[worldIndex][0];

    for (uint8_t i = 0; i < numberOfEnemies; i++) {

        enemies[i].x = Data::startingPostions[worldIndex][(i * 2) + 1];
        enemies[i].y = Data::startingPostions[worldIndex][(i * 2) + 2];
        
    }

}


// ---------------------------------------------------------------------------------------


void calculateViewPortPosition(Entity &entity, int16_t &xViewPort, int16_t &yViewPort) {
    
    if (entity.x < PD::width / 2) {
        
        xViewPort = 0;
        
    }
    else if (entity.x > Constants::worldWidth - PD::width / 2) {

        xViewPort = PD::width - Constants::worldWidth;
        
    }
    else {
        
        xViewPort = PD::width / 2 - entity.x;

    }
    
    if (entity.y < PD::height / 2) {
        
        yViewPort = 0;
        
    }
    else if (entity.y > Constants::worldHeight - PD::height / 2) {

        yViewPort = PD::height - Constants::worldHeight;
        
    }
    else {
        
        yViewPort = PD::height / 2 - entity.y;

    }
    
}


// ---------------------------------------------------------------------------------------


void calculatePlayerPosition(Entity &entity, int16_t &xPlayer, int16_t &yPlayer) {
    
    if (entity.x < PD::width / 2) {
        
        xPlayer = entity.x;
        
    }
    else if (entity.x > Constants::worldWidth - PD::width / 2) {

        xPlayer = entity.x - (Constants::worldWidth - PD::width);
        
    }
    else {
        
        xPlayer = PD::width / 2;

    }
    
    if (entity.y < PD::height / 2) {
        
        yPlayer = entity.y;
        
    }
    else if (entity.y > Constants::worldHeight - PD::height / 2) {

        yPlayer = entity.y - (Constants::worldHeight - PD::height);
        
    }
    else {
        
        yPlayer = PD::height / 2;

    }

}


// ---------------------------------------------------------------------------------------
//
//  Do the two entities overlap?
//
bool collide(Player &player, Enemy &enemy) {

    return !(enemy.x                >= player.x + player.width  ||
             enemy.x + enemy.width  <= player.x                 ||
             enemy.y                >= player.y + player.height ||
             enemy.y + enemy.height <= player.y);

}


// ---------------------------------------------------------------------------------------


uint16_t getTileIndex(int32_t x, int32_t y, uint16_t width) {

    uint32_t tx = x / width;
    uint32_t ty = y / width;

    return (ty * width + tx);

}


// ---------------------------------------------------------------------------------------


void updateTileType(uint16_t tileIndex, TileType tileType) {

    #if MAX_TILE_COUNT == 16

        if (tileIndex % 2 == 0) {
    
            worldMap[tileIndex / 2] = (worldMap[tileIndex / 2] & 0x0f) | (tileType << 4);
            
        }   
        else {
    
            worldMap[tileIndex / 2] = (worldMap[tileIndex / 2] & 0xf0) | tileType;
    
        }
        
    #else

        worldMap[tileIndex] = tileType;
    
    #endif
    
            
}


// ---------------------------------------------------------------------------------------
//
//  Check to see if the move the entity is about to make is into a green (empty) tile.  As 
//  the player and enemy tiles are 12 x 15 pixels and the tiles are bigger (16 x 16) its 
//  possible that the entity could be straddling two tiles in either direction ..
//
bool checkMovement(Entity &entity, int16_t x, int16_t y, Direction direction) {

    int8_t tileId1 = 0;
    int8_t tileId2 = 0;

    uint16_t tile1Index;
    uint16_t tile2Index;


    switch (direction) {
        
        case Direction::Left:

            tileId1 = tilemap.GetTileId(x, y, 16);
            tileId2 = tilemap.GetTileId(x, y + entity.height, 16);
            
            tile1Index = getTileIndex(x, y, 16);
            tile2Index = getTileIndex(x, y + entity.height, 16);

            break;
        
        case Direction::Right:

            tileId1 = tilemap.GetTileId(x + entity.width, y, 16);
            tileId2 = tilemap.GetTileId(x + entity.width, y + entity.height, 16);
            
            tile1Index = getTileIndex(x + entity.width, y, 16);
            tile2Index = getTileIndex(x + entity.width, y + entity.height, 16);

            break;

        case Direction::Up:
        
            tileId1 = tilemap.GetTileId(x, y, 16);
            tileId2 = tilemap.GetTileId(x + entity.width, y, 16);
            
            tile1Index = getTileIndex(x, y, 16);
            tile2Index = getTileIndex(x + entity.width, y, 16);

            break;
        
        case Direction::Down:

            tileId1 = tilemap.GetTileId(x, y + entity.height, 16);
            tileId2 = tilemap.GetTileId(x + entity.width, y + entity.height, 16);
            
            tile1Index = getTileIndex(x, y + entity.height, 16);
            tile2Index = getTileIndex(x + entity.width, y + entity.height, 16);

            break;
            
    }


    // Handle player actions
    
    if (&entity == &player) {


        // If we have found a key, pick it up ..
        
        if (tileId1 == TileType::Key) {

            player.hasKey = true;
            updateTileType(tile1Index, TileType::Carpet);

        }

        else if (tileId2 == TileType::Key) {

            player.hasKey = true;
            updateTileType(tile2Index, TileType::Carpet);

        }


        // Open the door?  Only if we have a key ..

        if (player.hasKey) {
            
            if (tileId1 == TileType::Door_Closed) {
    
                player.hasKey = false;
                updateTileType(tile1Index, TileType::Door_Open);
    
            }
    
            else if (tileId2 == TileType::Door_Closed) {
    
                player.hasKey = false;
                updateTileType(tile2Index, TileType::Door_Open);
    
            }
            
        }

    }


    // If either tile is not green, do not move.
    
    if ((tileId1 == TileType::Green || tileId1 == TileType::Carpet || tileId1 == TileType::Door_Open) && 
        (tileId2 == TileType::Green || tileId2 == TileType::Carpet || tileId2 == TileType::Door_Open)) {
        return true;
        
    }

    return false;
        
}



// ---------------------------------------------------------------------------------------
//
//  Handle the player movements ..
//
void handlePlayerMovements() {

    if (PC::buttons.pressed(BTN_LEFT) || PC::buttons.repeat(BTN_LEFT, 1))    { 

        if (player.x - 1 <= 0) {
            
            player.x--;
            
            if (player.x = -1) {

                player.x = Constants::worldWidth - 1;
                currentWorld--;
                initWorld(currentWorld);
                
            }
            
        }
        else if (checkMovement(player, player.x - 1, player.y, Direction::Left)) {

            player.x--;

        }

    }
    
    if (PC::buttons.pressed(BTN_RIGHT) || PC::buttons.repeat(BTN_RIGHT, 1))   { 

        if (player.x + 1 >= Constants::worldWidth - player.width) {

            player.x++;
            
            if (player.x == Constants::worldWidth) {

                player.x = -player.width + 1;
                currentWorld++;
                initWorld(currentWorld);
                
            }

        }
        else if (checkMovement(player, player.x + 1, player.y, Direction::Right)) {

            player.x++;

        }

    }
    
    
    if (PC::buttons.pressed(BTN_UP) || PC::buttons.repeat(BTN_UP, 1))      { 

        if (player.y > 0 && checkMovement(player, player.x, player.y - 1, Direction::Up)) {

            player.y--;

        }
        
    }
    
    if (PC::buttons.pressed(BTN_DOWN) || PC::buttons.repeat(BTN_DOWN, 1))    { 

        if (player.y < Constants::worldHeight && checkMovement(player, player.x, player.y + 1, Direction::Down)) {

            player.y++;
            
        }

    }
    
}




// ---------------------------------------------------------------------------------------
//
//  Handle the enemy movements ..
//
void handleEnemyMovements() {

    
    // Move each enemy individually ..
    
    for (uint8_t i = 0; i < numberOfEnemies; i++) {

        if (player.x < enemies[i].x) {

            if (checkMovement(enemies[i], enemies[i].x - 1, enemies[i].y, Direction::Left)) {
                enemies[i].x--;
            }
            
        }
        
        if (player.x > enemies[i].x) {

            if (checkMovement(enemies[i], enemies[i].x + 1, enemies[i].y, Direction::Right)) {
                enemies[i].x++;
            }
            
        }

        if (player.y < enemies[i].y) {

            if (checkMovement(enemies[i], enemies[i].x, enemies[i].y - 1, Direction::Up)) {
                enemies[i].y--;
            }
            
        }
        
        if (player.y > enemies[i].y) {

            if (checkMovement(enemies[i], enemies[i].x, enemies[i].y + 1, Direction::Down)) {
                enemies[i].y++;
            }
            
        }
        
    }
    
}


// ---------------------------------------------------------------------------------------


int main() {

    PC::begin();
    PD::loadRGBPalette(palettePico);   
    PD::persistence = true;
    PD::invisiblecolor = 12;
    PD::loadRGBPalette(palettePico);   
    PD::setFont(fontC64);
    PC::setFrameRate(200);



    // Initialise the map ..
    
    tilemap.set(16, 16, worldMap);
    tilemap.tiles[TileType::Green] = Images::green16;
    tilemap.tiles[TileType::Tree] = Images::tree16;
    tilemap.tiles[TileType::Grass] = Images::grass16;
    tilemap.tiles[TileType::Water] = Images::water16;
    tilemap.tiles[TileType::Brick] = Images::brick16;
    tilemap.tiles[TileType::Door_Open] = Images::door_open16;
    tilemap.tiles[TileType::Door_Closed] = Images::door_closed16;
    tilemap.tiles[TileType::Key] = Images::key16;
    tilemap.tiles[TileType::Carpet] = Images::carpet16;


    // Position the player into a vacant spot on the map ..
    
    player.x = 16;
    player.y = 150;
    
    
    
    // Initialise the world map and enemies ..

    initWorld(0);

    
    while (PC::isRunning()) {
        
        if (!PC::update()) continue;

        PC::buttons.pollButtons();        



        // Handle player movements ..

        handlePlayerMovements();


        // Move enemies every second frame ..
        
        if (Pokitto::Core::frameCount % 4 == 0) {
            handleEnemyMovements();
        }


        // Render screen ..

        int16_t xViewPort;
        int16_t yViewPort;
        
        calculateViewPortPosition(player, xViewPort, yViewPort);
        tilemap.draw(xViewPort, yViewPort);
        
        
        // Render player ..
        
        int16_t xPlayer;
        int16_t yPlayer;
        
        calculatePlayerPosition(player, xPlayer, yPlayer);
        PD::drawBitmap(xPlayer, yPlayer, Images::Player);        
        
        
        // Render enemies ..

        for (uint8_t i = 0; i < numberOfEnemies; i++) {

            PD::drawBitmap(enemies[i].x + xViewPort, enemies[i].y + yViewPort, Images::Enemy);
            
        }


        // Check for collisions between the player and the enemy ..

        for (uint8_t i = 0; i < numberOfEnemies; i++) {

            if (collide(player, enemies[i])) {
                
                PD::setColor(2);
                PD::fillRect(78, 78, 80, 10);
                PD::setColor(0, 2);
                PD::setCursor(80, 80);
                PD::print("Game Over");

            }
            
        }

    }
    
    return 0;
    
}

