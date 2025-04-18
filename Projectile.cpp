#include "Projectile.h"
#include "Game.h"



Projectile::Projectile(Vector2D setPos, float setAngle, int setDamage,
    float setDistanceTravelMax, bool setShotFromPlayer) : 
    Sprite(setPos, setAngle, 0.172f),
    damage(setDamage), distanceTravelMax(setDistanceTravelMax),
    shotFromPlayer(setShotFromPlayer) {

}



void Projectile::update(float dT, Game& game) {
    if (collisionOccurred == false) {
        //Move the projectile forward.
        float distanceMoveThisFrame = speed * dT;
        pos += Vector2D(angle) * distanceMoveThisFrame;
        distanceTraveled += distanceMoveThisFrame;

        //Check if this projectile needs to be removed for the game.
        if (distanceTraveled >= distanceTravelMax)
            collisionOccurred = true;
        else
            checkCollisions(game);
    }
}



void Projectile::draw(RendererGL* rendererGL, int tileSize, Vector2D posCamera) {
    if (shotFromPlayer) {
        drawBody(rendererGL, tileSize, posCamera, 0, 0, SDL_Color{ 0, 126, 255, 255 });
        drawBody(rendererGL, tileSize, posCamera, 0, 4, SDL_Color{ 0, 67, 216, 255 });
    }
    else {
        drawBody(rendererGL, tileSize, posCamera, 0, 0, SDL_Color{ 255, 132, 132, 255 });
        drawBody(rendererGL, tileSize, posCamera, 0, 4, SDL_Color{ 255, 6, 6, 255 });
    }
}


void Projectile::drawShadow(RendererGL* rendererGL, int tileSize, Vector2D posCamera) {
    drawBody(rendererGL, tileSize, posCamera, 5, 0, SDL_Color{ 109, 25, 72, 255 });
}



bool Projectile::getCollisionOccurred() {
    return collisionOccurred;
}



void Projectile::checkCollisions(Game& game) {
    if (shotFromPlayer) {
        //Check for a collision with any of the enemy units.
        for (auto& selected : game.getListUnitEnemies()) {
            if (selected.checkCircleOverlap(this)) {
                selected.removeHealth(damage);
                game.getUnitPlayer().addExperience(selected.getExperienceGiven());
                collisionOccurred = true;
                return;
            }
        }

        //Check for a collision with any of the destructibles.
        for (auto& selected : game.getListDestructibles()) {
            if (selected.checkCircleOverlap(pos, radius)) {
                selected.removeHealth(damage);
                game.getUnitPlayer().addExperience(selected.getExperienceGiven());
                collisionOccurred = true;
                return;
            }
        }
    }
    else {
        //Check for a collision with the player.
        if (game.getUnitPlayer().checkCircleOverlap(pos, radius)) {
            game.getUnitPlayer().removeHealth(damage);
            collisionOccurred = true;
            return;
        }
    }
}