#ifndef MOVABLE_H
#define MOVABLE_H

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

}  // namespace GameServer

#endif
