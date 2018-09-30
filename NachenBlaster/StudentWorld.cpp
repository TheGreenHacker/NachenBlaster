#include "StudentWorld.h"
#include "GameConstants.h"
#include "Actor.h"
#include <string>
#include <random>
#include <iostream>
#include <cmath>
#include <math.h>
#include <sstream>
#include <iomanip>
#include <algorithm>
using namespace std;

string StudentWorld::updateText() const
{
    ostringstream oss;
    oss.setf(ios::fixed);
    oss.precision(0);
    
    double lives = getLives();
    
    double health = m_player->hitPoints();
    double health_pct = health/0.5;
    
    double score = getScore();
    
    unsigned int level = getLevel();
    
    double cabbagePct = m_player->cabbagePct();
    
    int torpedos = m_player->numTorpedoes();
    
    oss << setw(10);
    
    oss << "Lives:"<< setw(2) << lives;
    
    oss << setw(9) << "Health:" <<setw(5) << health_pct <<"%";
    
    oss << setw(9) << "Score:" << setw(6) << score;
    
    oss << setw(7) << "Level:" << setw(3) << level;
    
    oss << setw(10) << "Cabbages:" << setw(5) << cabbagePct << "%";
    
    oss << setw(11) << "Torpedoes:" << setw(3) << torpedos;
    
    string s = oss.str();
    return s;
}

GameWorld* createStudentWorld(string assetDir)
{
	return new StudentWorld(assetDir);
}

//StudentWorld Implementation
StudentWorld::StudentWorld(string assetDir):GameWorld(assetDir)
{
    
}

StudentWorld::~StudentWorld()
{
    cleanUp();
}

int StudentWorld::init()
{
    for (int i = 0; i < MAX_STARS; i++) {   //initialize stars
        double size = randDouble(MIN_STAR_SIZE, MAX_STAR_SIZE);
        int x = randInt(0, VIEW_WIDTH - 1);
        int y = randInt(0, VIEW_HEIGHT - 1);
        m_actors.push_back(new Star(this, x, y, size));
    }
    m_player = new Player(this); //player restarts with full health and 0 torpedos each round
    m_alienShipsDestroyed = 0;
    m_alienShipsOnScreen = 0;
    return GWSTATUS_CONTINUE_GAME;
}

int StudentWorld::move()
{
    if (!m_player->isDead()) {
        m_player->doSomething();
    }
    else {
        return GWSTATUS_PLAYER_DIED;
    }
    for (vector<Actor*>::iterator it = m_actors.begin(); it != m_actors.end();)
    {
        (*it)->doSomething();  //ask each alive actor to do something
        if (m_player->isDead()) {  //check continuously to see if an action of another actor has caused the player to die
            return GWSTATUS_PLAYER_DIED;
        }
        if ((*it)->isDead()) {  //remove dead objects if they've flown off the screen or have lost all hitPoints
            if ((*it)->isAlien()) {
                m_alienShipsOnScreen--;
            }
            delete (*it);
            it = m_actors.erase(it);
        }
        else {
            it++;
        }
        if (m_alienShipsDestroyed == alienShipsNeededToBeDestroyed()) { //finished level
            playSound(SOUND_FINISHED_LEVEL);
            return GWSTATUS_FINISHED_LEVEL;
        }
    }
    //end of tick update actions
    introduceStars();
    introduceAliens();
    setGameStatText(updateText());
    return GWSTATUS_CONTINUE_GAME;
}

void StudentWorld::cleanUp()  //deletes all actors after current round is over
{
    delete m_player;
    m_player = nullptr;
    for (vector<Actor*>::iterator it = m_actors.begin(); it != m_actors.end();) {
        delete (*it);
        //(*it) = nullptr;
        it = m_actors.erase(it);
    }
}

Alien* StudentWorld::getOneCollidingAlien(const Actor* a) const
{
    double x1 = a->getX();
    double y1 = a->getY();
    double r1 = a->getRadius();
    for (int i = 0; i < m_actors.size(); i++) {
        if (m_actors[i]->isAlien()) {
            double x2 = m_actors[i]->getX();
            double y2 = m_actors[i]->getY();
            double r2 = m_actors[i]->getRadius();
            double distance = euclidianDistance(x1, x2, y1, y2);
            double radiusFactor = 0.75 * (r1 + r2);
            if (distance < radiusFactor) {
                return dynamic_cast<Alien*>(m_actors[i]);
            }
        }
    }
    return nullptr;
}

Player* StudentWorld::getCollidingPlayer(const Actor* a) const
{
    double x1 = a->getX();
    double y1 = a->getY();
    double r1 = a->getRadius();
    double x2 = m_player->getX();
    double y2 = m_player->getY();
    double r2 = m_player->getRadius();
    double distance = euclidianDistance(x1, x2, y1, y2);
    double radiusFactor = 0.75 * (r1 + r2);
    if (distance < radiusFactor) {
        return m_player;
    }
    else {
        return nullptr;
    }
}

bool StudentWorld::playerInLineOfFire(const Alien* a) const
{
    return a->getX() > m_player->getX() && a->getY() >= m_player->getY() - 4 && a->getY() <= m_player->getY() + 4;
}

void StudentWorld::addActor(Actor* a)
{
    m_actors.push_back(a);
    if (a->isAlien()) {
        m_alienShipsOnScreen++;
    }
}

void StudentWorld::recordAlienDestroyed()
{
    m_alienShipsDestroyed++;
}

//private algorithms
int StudentWorld::alienShipsNeededToBeDestroyed() const
{
    return 6 + 4 * getLevel();
}

int StudentWorld::maxAlienShipsOnScreen() const
{
    return 4 + 0.5 * getLevel();
}

double StudentWorld::randDouble(double min, double max) const
{
    if (max < min)
        std::swap(max, min);
    static std::random_device rd;
    static std::mt19937 generator(rd());
    std::uniform_real_distribution<> dis(min, max); //modification for double and float values
    return dis(generator);
}

double StudentWorld::euclidianDistance(double x1, double x2, double y1, double y2) const
{
    double x = abs(x1 - x2);
    double y = abs(y1 - y2);
    double xSq = pow(x, 2.0);
    double ySq = pow(y, 2.0);
    double distanceSq = xSq + ySq;
    double distance = sqrt(distanceSq);
    return distance;
}

bool StudentWorld::shouldAddAlien() const
{
    int d = m_alienShipsDestroyed;
    int r = alienShipsNeededToBeDestroyed() - d;
    if (m_alienShipsOnScreen < min(maxAlienShipsOnScreen(), r)) {
        return true;
    }
    return false;
}

void StudentWorld::introduceStars()
{
    int r = randInt(1, 15);
    if (r == 1) {
        double size = randDouble(MIN_STAR_SIZE, MAX_STAR_SIZE);
        int x = VIEW_WIDTH - 1;
        int y = randInt(0, VIEW_HEIGHT - 1);
        m_actors.push_back(new Star(this, x, y, size));
    }
}

void StudentWorld::introduceAliens()
{
    if (shouldAddAlien()) {
        vector<int> chances;
        int s1 = 60;
        int s2 = 20 + 5 * getLevel();
        int s3 = 5 + 10 * getLevel();
        chances.push_back(s1);
        chances.push_back(s2);
        chances.push_back(s3);
        sort(chances.begin(), chances.end());
        int s = s1 + s2 + s3;
        int y = randInt(0, VIEW_HEIGHT - 1);
        int r = randInt(1, s);
        if (r <= chances[0]) {
            if (chances[0] == s1) {
                addActor(new Smallgon(this, VIEW_WIDTH - 1, y));
            }
            else if (chances[0] == s2){
                addActor(new Smoregon(this, VIEW_WIDTH - 1, y));
            }
            else {
                addActor(new Snagglegon(this, VIEW_WIDTH - 1, y));
            }
        }
        else if (r > chances[0] && r <= (chances[0] + chances[1])){
            if (chances[1] == s1) {
                addActor(new Smallgon(this, VIEW_WIDTH - 1, y));
            }
            else if (chances[1] == s2){
                addActor(new Smoregon(this, VIEW_WIDTH - 1, y));
            }
            else {
                addActor(new Snagglegon(this, VIEW_WIDTH - 1, y));
            }
        }
        else{
            if (chances[2] == s1) {
                addActor(new Smallgon(this, VIEW_WIDTH - 1, y));
            }
            else if (chances[2] == s2){
                addActor(new Smoregon(this, VIEW_WIDTH - 1, y));
            }
            else {
                addActor(new Snagglegon(this, VIEW_WIDTH - 1, y));
            }
        }
    }
}











