#ifndef POSITIONMANAGER_H
#define POSITIONMANAGER_H

#include "Core/Object.h"
#include <unordered_map>
#include <stdint.h>
#include "Core/Timer.h"

namespace GameServer {

struct Region;

class Movable {
public:
	virtual void onMoveEnd() {}
	virtual void onRegionChange(const Region* oldRegion, const Region* newRegion) {}
	virtual void onLeaveRegion(const Region* region) {}
	virtual void onEnterRegion(const Region* region) {}

protected:
	friend class PositionManager;

	float x;
	float y;
	uint8_t layer;
	uint64_t lastUpdate;

	float currentSpeed;

	float targetX;
	float targetY;
};

struct Region
{
	static const int32_t REGION_SIZE = 180;
	static const int32_t VISIBILITY_RADIUS = 3;

	int32_t rx;
	int32_t ry;
	uint8_t layer;

	bool operator==(const Region &other) const {
		return other.rx == rx && other.ry == ry && other.layer == layer;
	}
	Region(float x, float y, uint8_t layer);
	Region(int32_t rx, int32_t ry, uint8_t layer);

	bool isVisibleRegion(const Region& other);
};

class PositionManager : public Object
{
	//DECLARE_CLASS(GameServer::PositionManager)
public:

	static PositionManager* get();

	void start();
	void stop();

	void moveObject(Movable* movable, float x, float y, uint8_t layer);

protected:
	void updatePosition(Movable* movable, uint64_t timestamp);
	void updateRegion(Movable* movable, float oldX, float oldY, uint8_t oldLayer);
	void update();

private:
	PositionManager();

	struct RegionHasher {
		size_t operator()(const Region& k) const {
			return (size_t)((((uint64_t)12638153115695167455u ^ k.rx) * 1099511628211 ^ k.ry) * 1099511628211 ^ k.layer);
		}
	};

	std::unordered_multimap<Region, Movable*, RegionHasher> objects;
	std::vector<Movable*> movingObjects;
	Timer<PositionManager> positionsUpdateTimer;
};

}

#endif // POSITIONMANAGER_H
