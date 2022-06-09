#include "map.h"

Location::Coordinate component::Body::getX(void) const
{
	return loc.x;
}

Location::Coordinate component::Body::getY(void) const
{
	return loc.y;
}

Location::Coordinate component::Body::getZ(void) const
{
	return loc.z;
}

void component::Body::setLocation(Location nLoc)
{
	loc = std::move(nLoc);
}

void component::Body::setX(Location::Coordinate nX)
{
	loc.x = std::move(nX);
	setLocation(loc);
}

void component::Body::setY(Location::Coordinate nY)
{
	loc.y = std::move(nY);
	setLocation(loc);
}

void component::Body::setZ(Location::Coordinate nZ)
{
	loc.z = std::move(nZ);
	setLocation(loc);
}
