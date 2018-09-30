#include "Actor.h"
#include "StudentWorld.h"
#include <iostream>
#include <random>
using namespace std;

//Actor Implementation
Actor::Actor(StudentWorld* w, double startX, double startY, int imageID, double damageAmt, double deltaX, double deltaY, double distance, int imageDir, double size, int depth):GraphObject(imageID, startX, startY, imageDir, size, depth)
{
    m_world = w;
    m_isDead = false;
    m_damageAmt = damageAmt;
    m_deltaX = deltaX;
    m_deltaY = deltaY;
    m_distance = distance;
}

bool Actor::isDead() const
{
    return m_isDead;
}

bool Actor::isAlien() const
{
    return false;
}

StudentWorld* Actor::world() const
{
    return m_world;
}

double Actor::damageAmt() const
{
    return m_damageAmt;
}

double Actor::distance() const
{
    return m_distance;
}

double Actor::deltaX() const
{
    return m_deltaX;
}

double Actor::deltaY() const
{
    return m_deltaY;
}

void Actor::setDeltaY(double dy)
{
    if (isAlien()) {  //this is to ensure that only aliens can use this function, as it is the only class that ever needs to
        m_deltaY = dy;
    }
}

void Actor::setDistance(double d)
{
    if (isAlien()) {  //allow aliens to change travel distance per tick, if necessary
        m_distance = d;
    }
}

void Actor::setDead()
{
    m_isDead = true;
}

void Actor::moveTo(double x, double y)
{
    if ((getX() >= 0 && getX() < VIEW_WIDTH && getY() >= 0 && getY() < VIEW_HEIGHT) || (isAlien() && getX() >= 0 && getX() < VIEW_WIDTH)) { //aliens are allowed to be out of bounds vertically for a bit before they correct their flight path
        GraphObject::moveTo(x, y);
    }
    else {
        setDead();
    }
}

//Star Implementation
Star::Star(StudentWorld* w, double startX, double startY, double size):Actor(w, startX, startY, IID_STAR, 0.0, -1.0, 0.0, 1.0, 0, size, 3)
{
    
}

void Star::doSomething()
{
    double x = getX();
    double y = getY();
    double d = distance();
    double dx = deltaX();
    moveTo(x + (dx * d), y);
}

//Explosion Implementation
Explosion::Explosion(StudentWorld* w, double startX, double startY):Actor(w, startX, startY, IID_EXPLOSION, 0.0, 0.0, 0.0, 0.0, 0, 1.0, 0)
{
    m_cycle = 0;
}

void Explosion::doSomething()
{
    if (m_cycle == 4) {
        setDead();
    }
    else {
        double size = getSize();
        setSize(size * 1.5);
        m_cycle++;
    }
}

//DamageableObject Implementation
DamageableObject::DamageableObject(StudentWorld* w, double startX, double startY, int imageID, int startDir, double size, int depth, double hitPoints, double damageAmt, double deltaX, double deltaY, double distance):Actor(w, startX, startY, imageID, damageAmt, deltaX, deltaY, distance, startDir, size, depth)
{
    m_hitPoints = hitPoints;
}

double DamageableObject::hitPoints() const
{
    return m_hitPoints;
}

void DamageableObject::increaseHitPoints(double amt)
{
    m_hitPoints += amt;  //when player obtains repair kit and to faciliate setting up alien's initial health per round
}

void DamageableObject::sufferDamage(double amt, int cause)
{
    if ((cause == HIT_BY_PROJECTILE) || (!isAlien() && cause == HIT_BY_SHIP)) { //player hit alien or either player or alien got hit by projectile
        m_hitPoints -= amt;
        if (m_hitPoints <= 0) {
            setDead();
            if (!isAlien()) {
                world()->decLives(); //player loses a life
            }
        }
    }
    else if (isAlien() && cause == HIT_BY_SHIP){ //automatic death for any alien that hits player
        setDead();
    }
}

//Player Implementation
Player::Player(StudentWorld* w): DamageableObject(w, 0.0, 128.0, IID_NACHENBLASTER, 0, 1.0, 0, 50.0, 0.0, 1.0, 1.0, 6.0)
{
    m_cabbagePower = 30;
    m_torpedos = 0;
}

void Player::doSomething()
{
    if (!isDead()) {
        int key;
        double x = getX();
        double y = getY();
        if (world()->getKey(key)) { //user pressed key
            switch (key) {
                case KEY_PRESS_UP:  //try to move up
                    if (y + 6.0 < VIEW_HEIGHT) {
                        moveTo(x, y + 6.0);
                    }
                    break;
                case KEY_PRESS_RIGHT:  //try to move right
                    if (x + 6.0 < VIEW_WIDTH) {
                        moveTo(x + 6.0, y);
                    }
                    break;
                case KEY_PRESS_DOWN:  //try to move down
                    if (y - 6.0 >= 0) {
                        moveTo(x, y - 6.0);
                    }
                    break;
                case KEY_PRESS_LEFT:   //try to move left
                    if (x - 6.0 >= 0) {
                        moveTo(x - 6.0, y);
                    }
                    break;
                case KEY_PRESS_SPACE:  //shoot cabbages, if enough fire power
                    if (m_cabbagePower >= 5) {
                        world()->addActor(new Cabbage(world(), getX() + 12.0, getY()));
                        world()->playSound(SOUND_PLAYER_SHOOT);
                        m_cabbagePower -= 5;
                    }
                    break;
                case KEY_PRESS_TAB: //fire torpedos, if player has any
                    if (m_torpedos > 0) {
                        world()->addActor(new PlayerLaunchedTorpedo(world(), getX() + 12.0, getY()));
                        world()->playSound(SOUND_TORPEDO);
                        m_torpedos -= 1;
                    }
                    break;
                default: //invalid key
                    break;
            }
        }
        //Collisions handled in alien and projectile classes classes.
        if (m_cabbagePower < 30) { //restore cabbage power, no matter what player does
            m_cabbagePower++;
        }
    }
}

void Player::increaseTorpedoes(int amt)
{
    m_torpedos += amt;
}

int Player::healthPct() const
{
    return hitPoints() * 100.0 / MAX_HEALTH;
}

int Player::cabbagePct() const
{
    return m_cabbagePower * 100 / MAX_CABBAGE_POWER;
}

int Player::numTorpedoes() const
{
    return m_torpedos;
}

//Alien Implementation
Alien::Alien(StudentWorld* w, double startX, double startY, int imageID, double hitPoints, double damageAmt, double deltaX, double deltaY, double distance, unsigned int scoreValue):DamageableObject(w, startX, startY, imageID, 0, 1.5, 1, hitPoints, damageAmt, deltaX, deltaY, distance)
{
    m_flightPlan = 0;
    m_scoreValue = scoreValue;
    increaseHitPoints(computeHealth());
}

bool Alien::isAlien() const
{
    return true;
}

unsigned int Alien::scoreValue() const
{
    return m_scoreValue;
}

void Alien::doSomething()
{
    if (!isDead()) {
        if (getX() < 0) {
            setDead();
            return;
        }
        if (!damageCollidingPlayer(damageAmt())) { //collision with projectiles fired by Nachenblaster will be handled in respective projectile class. aliens are only responsible for checking for collisions with NachenBlaster.
            pickNewFlightPlan();  //pick a new flight plan
            if (!attack()) {
                changeFlightPlanAgain();
                double x = getX();
                double y = getY();
                double dx = deltaX();
                double dy = deltaY();
                double d = distance();
                moveTo(x + (d * dx), y + (d * dy));
                m_flightPlan--;
                if (getX() < 0) {
                    setDead();
                    return;
                }
                damageCollidingPlayer(damageAmt()); //check for collision again
            }
        }
    }
}

bool Alien::damageCollidingPlayer(double amt)
{
    Player* p = world()->getCollidingPlayer(this);
    if (p != nullptr) {
        p->sufferDamage(amt, HIT_BY_SHIP);
        setDead();
        world()->increaseScore(m_scoreValue);
        world()->recordAlienDestroyed();
        possiblyDropGoodie();
        world()->playSound(SOUND_DEATH);
        world()->addActor(new Explosion(world(), getX(), getY()));
        return true;
    }
    return false;
}

//protected functions
double Alien::computeHealth() const
{
    double currentLevel = world()->getLevel();
    return 5 * (1 + (currentLevel-1) * 0.1);
}

void Alien::chooseInitialDirection()
{
    int chance = randInt(0, 2);
    switch (chance) {
        case 0:
            setDeltaY(0.0); //just left (y distance stays constant)
            break;
        case 1:
            setDeltaY(1.0); //left and up
            break;
        case 2:
            setDeltaY(-1.0); //left and down
            break;
        default:
            setDeltaY(-1.0); //left and down
            break;
    }
}

void Alien::setFlightPlan(double amt)
{
    m_flightPlan = amt;
}

//private algorithms
void Alien::pickNewFlightPlan()
{
    setFlightPlan1();
    setFlightPlan2();
}

void Alien::setFlightPlan1()
{
    if (getY() >= VIEW_HEIGHT - 1) {
        setDeltaY(-1.0);
    }
    else if (getY() <= 0){
        setDeltaY(1.0);
    }
}

void Alien::setFlightPlan2()
{
    if (m_flightPlan == 0){
        chooseInitialDirection();
    }
    m_flightPlan = randInt(1, 32);
}

void Alien::chooseProjectile()
{
    world()->addActor(new Turnip(world(), getX() - 14, getY()));
}

void Alien::playAttackSound()
{
    world()->playSound(SOUND_ALIEN_SHOOT);
}

int Alien::decideAttackHelper() const
{
    int currentLevel = world()->getLevel();
    return (20/currentLevel) + 5;
}

bool Alien::attack()
{
    if (world()->playerInLineOfFire(this)) {
        int sampleSize = decideAttackHelper();
        int r = randInt(1, sampleSize);
        if (r == 1) {
            chooseProjectile();
            playAttackSound();
            return true;
        }
    }
    return false;
}

//Smallgon Implementation
Smallgon::Smallgon(StudentWorld* w, double startX, double startY):Alien(w, startX, startY, IID_SMALLGON, 0.0, 5.0, -1.0, 0.0, 2.0, 250)
{
    
}

//Smoregon Implementation
Smoregon::Smoregon(StudentWorld* w, double startX, double startY):Alien(w, startX, startY, IID_SMOREGON, 0.0, 5.0, -1.0, 0.0, 2.0, 250)
{
    
}

void Smoregon::possiblyDropGoodie()
{
    int r1 = randInt(1, 3);
    if (r1 == 1) {
        int r2 = randInt(1, 2);
        if (r2 == 1) {
            world()->addActor(new RepairGoodie(world(), getX(), getY()));
        }
        else {
            world()->addActor(new TorpedoGoodie(world(), getX(), getY()));
        }
    }
}

void Smoregon::changeFlightPlanAgain()
{
    int currentLevel = world()->getLevel();
    int sampleSize = (20/currentLevel) + 5;
    int r = randInt(1, sampleSize);
    if (r == 1) {
        setDeltaY(0.0);
        setFlightPlan(VIEW_WIDTH);
        setDistance(5.0);
    }
}

//Snagglegon Implementation
Snagglegon::Snagglegon(StudentWorld* w, double startX, double startY):Alien(w, startX, startY, IID_SNAGGLEGON, 0.0, 15.0, -1.0, -1.0, 1.75, 1000)
{
    increaseHitPoints(computeHealth());
    //cout << hitPoints() << endl;
}

void Snagglegon::possiblyDropGoodie()
{
    int r = randInt(1, 6);
    if (r == 1) {
        world()->addActor(new ExtraLifeGoodie(world(), getX(), getY()));
    }
}

void Snagglegon::chooseInitialDirection()
{
    setDeltaY(-1.0);
}

void Snagglegon::chooseProjectile()
{
    world()->addActor(new AlienLaunchedTorpedo(world(), getX() - 14, getY()));
    world()->playSound(SOUND_TORPEDO);
}

void Snagglegon::playAttackSound()
{
    world()->playSound(SOUND_TORPEDO);
}

int Snagglegon::decideAttackHelper() const
{
    int currentLevel = world()->getLevel();
    return (10/currentLevel) + 10;
}

//Projectile Implementation
Projectile::Projectile(StudentWorld* w, double startX, double startY, int imageID, double damageAmt, double deltaX, bool rotates, int dir):Actor(w, startX, startY, imageID, damageAmt, deltaX, 0.0, 8.0, dir, 0.5, 1)
{
    m_rotates = rotates;
}

void Projectile::doSomething()
{
    if (!isDead()) {
        double x = getX();
        double y = getY();
        double d = distance();
        double dx = deltaX();
        if (x <= 0 || x > VIEW_WIDTH) { //off screen
            setDead();
            return;
        }
        Alien* a = world()->getOneCollidingAlien(this);
        Player* p = world()->getCollidingPlayer(this);
        if (dx > 0 && a != nullptr) {    //fired by player
            a->sufferDamage(damageAmt(), HIT_BY_PROJECTILE);
            if (a->isDead()) {
                a->possiblyDropGoodie();
                world()->addActor(new Explosion(world(), x, y));
                world()->recordAlienDestroyed();
                world()->increaseScore(a->scoreValue());
                world()->playSound(SOUND_DEATH);
            }
            else {
                world()->playSound(SOUND_BLAST);
            }
            setDead();
            return;
        }
        else if (dx < 0 && p != nullptr){ //fired by alien
            p->sufferDamage(damageAmt(), HIT_BY_PROJECTILE);
            if (!p->isDead()) {  //player's death case is handled internally in suffer damage method
                world()->playSound(SOUND_BLAST);
            }
            setDead();
            return;
        }
        moveTo(x + (d * dx), y);
        if (m_rotates) { //cabbages and turnips
            int direction = getDirection();
            setDirection(direction + 20);
        }
        //check for collision again if alive
        if (!isDead()) {
            if (dx > 0 && a != nullptr) {    //fired by player
                a->sufferDamage(damageAmt(), HIT_BY_PROJECTILE);
                if (a->isDead()) {
                    a->possiblyDropGoodie();
                    world()->addActor(new Explosion(world(), x, y));
                    world()->recordAlienDestroyed();
                    world()->increaseScore(a->scoreValue());
                    world()->playSound(SOUND_DEATH);
                }
                else {
                    world()->playSound(SOUND_BLAST);
                }
                setDead();
            }
            else if (dx < 0 && p != nullptr){ //fired by alien
                p->sufferDamage(damageAmt(), HIT_BY_PROJECTILE);
                if (!p->isDead()) {  //player's death case is handled internally in suffer damage method
                    world()->playSound(SOUND_BLAST);
                }
                setDead();
            }
        }
    }
}

//Cabbage Implementation
Cabbage::Cabbage(StudentWorld* w, double startX, double startY):Projectile(w, startX, startY, IID_CABBAGE, 2.0, 1.0, true, 0)
{
    
}

//Turnip Implementation
Turnip::Turnip(StudentWorld* w, double startX, double startY):Projectile(w, startX, startY, IID_TURNIP, 2.0, -1.0, true, 0)
{
    
}

//Torpedo Implementaion
Torpedo::Torpedo(StudentWorld* w, double startX, double startY, double deltaX, int dir):Projectile(w, startX, startY, IID_TORPEDO, 8.0, deltaX, false, dir)
{
    
}

//PlayerLaunchedTorpedo Implementation
PlayerLaunchedTorpedo::PlayerLaunchedTorpedo(StudentWorld* w, double startX, double startY):Torpedo(w, startX, startY, 1.0, 0)
{
    
}

//AlienLaunchedTorpedo Implementation
AlienLaunchedTorpedo::AlienLaunchedTorpedo(StudentWorld* w, double startX, double startY):Torpedo(w, startX, startY, -1.0, 180)
{
    
}

//Goodie Implementation
Goodie::Goodie(StudentWorld* w, double startX, double startY, int imageID):Actor(w, startX, startY, imageID, 0.0, -1.0, -1.0, 0.75, 0, 0.5, 1)
{
    
}

void Goodie::doSomething()
{
    if (!isDead()) {
        double x = getX();
        double y = getY();
        double d = distance();
        double dx = deltaX();
        double dy = deltaY();
        if (x <= 0 || y <= 0) {
            setDead();
            return;
        }
        Player* p = world()->getCollidingPlayer(this);
        if (p != nullptr) {
            grantReward(p);
            world()->increaseScore(100);
            world()->playSound(SOUND_GOODIE);
            setDead();
            return;
        }
        moveTo(x + (d * dx), y + (d * dy));
        /*
        p = world()->getCollidingPlayer(this);
        if (p != nullptr) {
            grantReward(p);
            world()->increaseScore(100);
            world()->playSound(SOUND_GOODIE);
            setDead();
            return;
        }
         */
        if (!isDead()) {
            p = world()->getCollidingPlayer(this);
            if (p != nullptr) {
                grantReward(p);
                world()->increaseScore(100);
                world()->playSound(SOUND_GOODIE);
                setDead();
            }
        }
    }
}

//ExtraLifeGoodie Implementation
ExtraLifeGoodie::ExtraLifeGoodie(StudentWorld* w, double startX, double startY):Goodie(w, startX, startY, IID_LIFE_GOODIE)
{
    
}

void ExtraLifeGoodie::grantReward(Player* p)
{
    p->world()->incLives();
}

//RepairGoodie Implementation
RepairGoodie::RepairGoodie(StudentWorld* w, double startX, double startY):Goodie(w, startX, startY, IID_REPAIR_GOODIE)
{
    
}

void RepairGoodie::grantReward(Player* p)
{
    if (p->hitPoints() <= 40) { //make sure not to increment player's hitpoints past the 50 max
        p->increaseHitPoints(10.0);
    }
}

//TorpedoGoodie Implementation
TorpedoGoodie::TorpedoGoodie(StudentWorld* w, double startX, double startY):Goodie(w, startX, startY, IID_TORPEDO_GOODIE)
{
    
}

void TorpedoGoodie::grantReward(Player* p)
{
    p->increaseTorpedoes(5);
}




















