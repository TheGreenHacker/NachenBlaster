#ifndef STUDENTWORLD_H_
#define STUDENTWORLD_H_

#include "GameWorld.h"
#include <string>
#include <vector>
#include <iostream>

class Actor;
class Alien;
class Player;

const int MAX_STARS = 30;
const double MIN_STAR_SIZE = 0.05;
const double MAX_STAR_SIZE = 0.5;

class StudentWorld : public GameWorld
{
public:
    StudentWorld(std::string assetDir);
    virtual ~StudentWorld();
    std::string updateText() const;
    virtual int init();
    virtual int move();
    virtual void cleanUp();
    // If there's at least one alien that's collided with a, return
    // a pointer to one of them; otherwise, return a null pointer.
    Alien* getOneCollidingAlien(const Actor* a) const;
    // If the player has collided with a, return a pointer to the player;
    // otherwise, return a null pointer.
    Player* getCollidingPlayer(const Actor* a) const;
    // Is the player in the line of fire of a, which might cause a to attack?
    bool playerInLineOfFire(const Alien* a) const;
    // Add an actor to the world.
    void addActor(Actor* a);
    // Record that one more alien on the current level has been destroyed.
    void recordAlienDestroyed();
private:
    std::vector<Actor*> m_actors;
    Player* m_player;
    int m_alienShipsDestroyed;
    int m_alienShipsOnScreen;  //help determine if any new aliens need to be added
    int alienShipsNeededToBeDestroyed() const;
    int maxAlienShipsOnScreen() const;
    double randDouble(double min, double max) const;  //random double generator
    double euclidianDistance(double x1, double x2, double y1, double y2) const;
    bool shouldAddAlien() const;
    void introduceStars();
    void introduceAliens();
};

#endif // STUDENTWORLD_H_
