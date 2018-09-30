#ifndef ACTOR_H_
#define ACTOR_H_

#include "GraphObject.h"

const int HIT_BY_SHIP = 0;
const int HIT_BY_PROJECTILE = 1;

class StudentWorld;

class Actor : public GraphObject
{
public:
    Actor(StudentWorld* w, double startX, double startY, int imageID,
          double damageAmt, double deltaX, double deltaY, double distance, int imageDir, double size, int depth);
    // Action to perform for each tick.
    virtual void doSomething() = 0;
    // Is this actor dead?
    bool isDead() const;
    // Is this actor an alien?
    virtual bool isAlien() const;
    // Get this actor's world
    StudentWorld* world() const;
    //Get this actor's damageAmount
    double damageAmt() const;
    //Get this actor's travel distance
    double distance() const;
    //Get this actor's deltaX, aka it's direction where 1.0 means to the right and -1.0 means to the left
    double deltaX() const;
    //Get this actor's deltaY, aka it's direction where 1.0 means up and -1.0 means down
    double deltaY() const;
    //Set vertical direction
    void setDeltaY(double dy);
    //Set distance
    void setDistance(double d);
    // Mark this actor as dead.
    void setDead();
    // Move this actor to x,y if onscreen; otherwise, don't move and mark
    // this actor as dead.
    virtual void moveTo(double x, double y);
private:
    bool m_isDead;
    StudentWorld* m_world;
    double m_damageAmt;
    double m_deltaX; 
    double m_deltaY;
    double m_distance;
};

class Star : public Actor
{
public:
    Star(StudentWorld* w, double startX, double startY, double size);
    virtual void doSomething();
};

class Explosion : public Actor
{
public:
    Explosion(StudentWorld* w, double startX, double startY);
    virtual void doSomething();
private:
    int m_cycle; //keep track of time after 4 ticks
};

class DamageableObject : public Actor
{
public:
    DamageableObject(StudentWorld* w, double startX, double startY, int imageID, int startDir, double size, int depth, double hitPoints, double damageAmt, double deltaX, double deltaY, double distance);
    // How many hit points does this actor have left?
    double hitPoints() const;
    // Increase this actor's hit points by amt.
    void increaseHitPoints(double amt);
    // This actor suffers an amount of damage caused by being hit by either
    // a ship or a projectile (see constants above).
    void sufferDamage(double amt, int cause);
private:
    double m_hitPoints;
};

const int MAX_CABBAGE_POWER = 30;
const double MAX_HEALTH = 50.0;

class Player : public DamageableObject
{
public:
    Player(StudentWorld* w);
    virtual void doSomething();
    // Incease the number of torpedoes the player has by amt.
    void increaseTorpedoes(int amt);
    // Get the player's health percentage.
    int healthPct() const;
    // Get the player's cabbage energy percentage.
    int cabbagePct() const;
    // Get the number of torpedoes the player has.
    int numTorpedoes() const;
private:
    int m_cabbagePower;
    int m_torpedos;
};

class Alien : public DamageableObject
{
public:
    Alien(StudentWorld* w, double startX, double startY, int imageID,
          double hitPoints, double damageAmt, double deltaX,
          double deltaY, double distance, unsigned int scoreValue);
    virtual bool isAlien() const;
    unsigned int scoreValue() const;
    virtual void doSomething();
    // If this alien collided with the player, damage the player and return
    // true; otherwise, return false.
    bool damageCollidingPlayer(double amt);
    // If this alien drops goodies, drop one with the appropriate probability.
    virtual void possiblyDropGoodie() = 0;
protected:
    double computeHealth() const;
    virtual void chooseInitialDirection();
    void setFlightPlan(double amt);
private:
    double m_flightPlan;
    unsigned int m_scoreValue;
    virtual void setFlightPlan1();
    virtual void setFlightPlan2();
    virtual void pickNewFlightPlan();
    virtual void chooseProjectile();
    virtual void playAttackSound();
    virtual int decideAttackHelper() const;
    bool attack();
    virtual void changeFlightPlanAgain(){};
};

class Smallgon : public Alien
{
public:
    Smallgon(StudentWorld* w, double startX, double startY);
    virtual void possiblyDropGoodie(){};
};

class Smoregon : public Alien
{
public:
    Smoregon(StudentWorld* w, double startX, double startY);
    virtual void possiblyDropGoodie();
private:
    virtual void changeFlightPlanAgain();
};

class Snagglegon : public Alien
{
public:
    Snagglegon(StudentWorld* w, double startX, double startY);
    virtual void possiblyDropGoodie();
protected:
    virtual void chooseInitialDirection();
private:
    virtual void setFlightPlan2(){};
    virtual void chooseProjectile();
    virtual void playAttackSound();
    virtual int decideAttackHelper() const;
    bool attack();
};

class Projectile : public Actor
{
public:
    Projectile(StudentWorld* w, double startX, double startY, int imageID, double damageAmt, double deltaX, bool rotates, int dir);
    virtual void doSomething();
private:
    bool m_rotates;
};

class Cabbage : public Projectile
{
public:
    Cabbage(StudentWorld* w, double startX, double startY);
};

class Turnip : public Projectile
{
public:
    Turnip(StudentWorld* w, double startX, double startY);
};

class Torpedo : public Projectile
{
public:
    Torpedo(StudentWorld* w, double startX, double startY, double deltaX,
            int imageDir);
};

class PlayerLaunchedTorpedo : public Torpedo
{
public:
    PlayerLaunchedTorpedo(StudentWorld* w, double startX, double startY);
};

class AlienLaunchedTorpedo : public Torpedo
{
public:
    AlienLaunchedTorpedo(StudentWorld* w, double startX, double startY);
};

class Goodie : public Actor
{
public:
    Goodie(StudentWorld* w, double startX, double startY, int imageID);
    virtual void doSomething();
private:
    virtual void grantReward(Player* p) = 0;
};

class ExtraLifeGoodie : public Goodie
{
public:
    ExtraLifeGoodie(StudentWorld* w, double startX, double startY);
private:
    virtual void grantReward(Player* p);
};

class RepairGoodie : public Goodie
{
public:
    RepairGoodie(StudentWorld* w, double startX, double startY);
private:
    virtual void grantReward(Player* p);
};

class TorpedoGoodie : public Goodie
{
public:
    TorpedoGoodie(StudentWorld* w, double startX, double startY);
private:
    virtual void grantReward(Player* p);
};

#endif // ACTOR_H_
