#include "ObserverTile.h"
#include "Level.h"
#include "Mob.h"
#include "Math.h"
#include "Random.h"

ObserverTile::ObserverTile(int id) : Tile(id, Material::stone)
{
}

Icon *ObserverTile::getTexture(int face, int data)
{
    int front = getFrontFace(data);
    int back = getBackFace(data);
    
    if (face == front) {
        return Tile::dispenser->getTexture(2, 2); // use dispenser front face
    }
    if (face == back) {
        if ((data & TRIGGERED_BIT) != 0) {
            return Tile::redStoneOre_lit->getTexture(face, 0); // lit redstone
        }
        return Tile::redStoneOre->getTexture(face, 0); // unlit redstone
    }
    return Tile::rock->getTexture(face, 0); // side
}

int ObserverTile::getTickDelay()
{
    return 2; 
}

bool ObserverTile::isSignalSource()
{
    return true;
}

int ObserverTile::getFrontFace(int data)
{
    return data & FACING_MASK;
}

int ObserverTile::getBackFace(int data)
{
    int front = getFrontFace(data);
    if (front % 2 == 0) return front + 1;
    return front - 1;
}

bool ObserverTile::getSignal(LevelSource *level, int x, int y, int z, int dir)
{
    int data = level->getData(x, y, z);
    if ((data & TRIGGERED_BIT) != 0)
    {
        return true; // Simplified: just emit if triggered
    }
    return false;
}

bool ObserverTile::getDirectSignal(Level *level, int x, int y, int z, int dir)
{
    return getSignal(level, x, y, z, dir);
}

void ObserverTile::neighborChanged(Level *level, int x, int y, int z, int type)
{
    int data = level->getData(x, y, z);
    if ((data & TRIGGERED_BIT) == 0)
    {
        level->addToTickNextTick(x, y, z, this->id, getTickDelay());
    }
}

void ObserverTile::tick(Level *level, int x, int y, int z, Random *random)
{
    int data = level->getData(x, y, z);
    
    if ((data & TRIGGERED_BIT) == 0)
    {
        level->setTileAndData(x, y, z, this->id, data | TRIGGERED_BIT);
        updateNeighbours(level, x, y, z, data | TRIGGERED_BIT);
        level->addToTickNextTick(x, y, z, this->id, getTickDelay());
    }
    else
    {
        level->setTileAndData(x, y, z, this->id, data & ~TRIGGERED_BIT);
        updateNeighbours(level, x, y, z, data & ~TRIGGERED_BIT);
    }
}

void ObserverTile::updateNeighbours(Level *level, int x, int y, int z, int data)
{
    level->updateNeighborsAt(x, y, z, this->id);
    
    int back = getBackFace(data);
    int nx = x, ny = y, nz = z;
    if (back == 0) ny--;
    else if (back == 1) ny++;
    else if (back == 2) nz--;
    else if (back == 3) nz++;
    else if (back == 4) nx--;
    else if (back == 5) nx++;
    
    level->updateNeighborsAt(nx, ny, nz, this->id);
}

void ObserverTile::setPlacedBy(Level *level, int x, int y, int z, shared_ptr<Mob> by)
{
    level->setData(x, y, z, 3); // Face Z+ for now
}

void ObserverTile::onPlace(Level *level, int x, int y, int z)
{
}

void ObserverTile::onRemove(Level *level, int x, int y, int z, int id, int data)
{
    if ((data & TRIGGERED_BIT) != 0)
    {
        updateNeighbours(level, x, y, z, data);
    }
    Tile::onRemove(level, x, y, z, id, data);
}
