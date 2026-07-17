#pragma once
#include "Tile.h"
#include <memory>

class Player;
class Random;
class Level;
class Mob;

class ObserverTile : public Tile
{
public:
    static const int TRIGGERED_BIT = 8;
    static const int FACING_MASK = 7; 

    ObserverTile(int id);
    
    virtual Icon *getTexture(int face, int data);
    virtual int getTickDelay();
    
    virtual bool isSignalSource();
    virtual bool getSignal(LevelSource *level, int x, int y, int z, int dir);
    virtual bool getDirectSignal(Level *level, int x, int y, int z, int dir);
    
    virtual void neighborChanged(Level *level, int x, int y, int z, int type);
    virtual void tick(Level *level, int x, int y, int z, Random *random);
    
    virtual void onPlace(Level *level, int x, int y, int z);
    virtual void onRemove(Level *level, int x, int y, int z, int id, int data);
    virtual void setPlacedBy(Level *level, int x, int y, int z, shared_ptr<Mob> by);
    
private:
    void updateNeighbours(Level *level, int x, int y, int z, int data);
    int getFrontFace(int data);
    int getBackFace(int data);
};
