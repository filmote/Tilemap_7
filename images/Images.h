#include "Pokitto.h"
#include "Tilemap.hpp"

using PC = Pokitto::Core;
using PD = Pokitto::Display;


namespace Images {

    #include "Green.h"
    #include "Grass.h"
    #include "Tree.h"
    #include "Water.h"
    #include "Player.h"
    #include "Enemy.h"
    #include "Brick.h"
    #include "Door_Open.h"
    #include "Door_Closed.h"
    #include "Carpet.h"
    #include "Key.h"
    
    Tilemap::Tile green16 = { &Green[2], Green[0], Green[1] };
    Tilemap::Tile tree16 = { &Tree[2], Tree[0], Tree[1] };
    Tilemap::Tile grass16 = { &Grass[2], Grass[0], Grass[1] };
    Tilemap::Tile water16 = { &Water[2], Water[0], Water[1] };
    Tilemap::Tile brick16 = { &Brick[2], Brick[0], Brick[1] };
    Tilemap::Tile door_open16 = { &Door_Open[2], Door_Open[0], Door_Open[1] };
    Tilemap::Tile door_closed16 = { &Door_Closed[2], Door_Closed[0], Door_Closed[1] };
    Tilemap::Tile carpet16 = { &Carpet[2], Carpet[0], Carpet[1] };
    Tilemap::Tile key16 = { &Key[2], Key[0], Key[1] };

}