#include "PositionManager.h"
#include "Core/Utils.h"
#include <algorithm>
#include <array>
#include <math.h>
#include <stdlib.h>

namespace GameServer {

Region::Region(float x, float y, uint8_t layer) {
	this->rx = int32_t(x / REGION_SIZE);
	this->ry = int32_t(y / REGION_SIZE);
	this->layer = layer;
}

Region::Region(int32_t rx, int32_t ry, uint8_t layer) {
	this->rx = rx;
	this->ry = ry;
	this->layer = layer;
}

bool Region::isVisibleRegion(const Region& other) {
	if(other.layer != layer)
		return false;

	int32_t distance = ::abs(rx - other.rx) + ::abs(ry - other.ry);
	return distance <= VISIBILITY_RADIUS;
}

PositionManager* PositionManager::get() {
	static PositionManager positionManager;
	return &positionManager;
}

PositionManager::PositionManager() {}

void PositionManager::start() {
	positionsUpdateTimer.start(this, &PositionManager::update, 100, 100);
}

void PositionManager::stop() {
	positionsUpdateTimer.stop();
}

void PositionManager::moveObject(Movable* movable, float x, float y, uint8_t layer) {
	if(movable->x == x && movable->y == y && movable->layer == layer)
		return;

	if(std::find(movingObjects.begin(), movingObjects.end(), movable) == movingObjects.end()) {
		movingObjects.push_back(movable);
	}

	movable->targetX = x;
	movable->targetY = y;
	//	movable->targerLayer = layer;
}

void PositionManager::updatePosition(Movable* movable, uint64_t timestamp) {
	float deltaX = movable->targetX - movable->x;
	float deltaY = movable->targetY - movable->y;
	float distance = sqrt(deltaX * deltaX + deltaY * deltaY);

	float oldX = movable->x;
	float oldY = movable->y;

	float timeDelta = float(movable->lastUpdate - timestamp) / 1000.0f;
	float distanceToMoveRatio = movable->currentSpeed * timeDelta / distance;

	if(distanceToMoveRatio < 1.0f) {
		movable->x += distanceToMoveRatio * deltaX;
		movable->y += distanceToMoveRatio * deltaY;
	} else {
		movable->x = movable->targetX;
		movable->y = movable->targetY;
		movable->currentSpeed = 0;
		movable->onMoveEnd();
	}

	updateRegion(movable, oldX, oldY, movable->layer);

	movable->lastUpdate = timestamp;
}

void PositionManager::updateRegion(Movable* movable, float oldX, float oldY, uint8_t oldLayer) {
	Region oldRegion(oldX, oldY, oldLayer);
	Region newRegion(movable->x, movable->y, movable->layer);

	if(oldRegion == newRegion) {
		return;
	}

	movable->onRegionChange(&oldRegion, &newRegion);

	for(int32_t rx = oldRegion.rx - Region::VISIBILITY_RADIUS; rx <= oldRegion.rx + Region::VISIBILITY_RADIUS; rx++) {
		int32_t ryStart = oldRegion.ry - Region::VISIBILITY_RADIUS + ::abs(rx - oldRegion.rx);
		int32_t ryEnd = oldRegion.ry + Region::VISIBILITY_RADIUS - ::abs(rx - oldRegion.rx);

		for(int32_t ry = ryStart; ry <= ryEnd; ry++) {
			Region testRegion(rx, ry, oldRegion.layer);
			if(oldRegion.isVisibleRegion(testRegion) && !newRegion.isVisibleRegion(testRegion)) {
				// disappear in old region
				movable->onLeaveRegion(&testRegion);
			}
		}
	}

	for(int32_t rx = newRegion.rx - Region::VISIBILITY_RADIUS; rx <= newRegion.rx + Region::VISIBILITY_RADIUS; rx++) {
		int32_t ryStart = newRegion.ry - Region::VISIBILITY_RADIUS + ::abs(rx - newRegion.rx);
		int32_t ryEnd = newRegion.ry + Region::VISIBILITY_RADIUS - ::abs(rx - newRegion.rx);

		for(int32_t ry = ryStart; ry <= ryEnd; ry++) {
			Region testRegion(rx, ry, newRegion.layer);
			if(newRegion.isVisibleRegion(testRegion) && !oldRegion.isVisibleRegion(testRegion)) {
				// appear in new region
				movable->onEnterRegion(&testRegion);
			}
		}
	}
}

void PositionManager::update() {
	auto it = movingObjects.begin();
	auto itEnd = movingObjects.begin();
	uint64_t timestamp = Utils::getTimeInMsec();
	for(; it != itEnd; ++it) {
		updatePosition(*it, timestamp);
	}
}

}  // namespace GameServer
