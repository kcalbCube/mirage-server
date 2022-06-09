#pragma once
#include <core/ecs.h>
#include <core/graphics.h>

struct Location
{
	using Coordinate = float;
	Coordinate 
		x = 0,
		y = 0,
	        z = 0;
};

namespace component
{
	class Body : public mirage::ecs::Component<Body>
	{
		Location loc{};
		float 
			width = 0.f,
			height = 0.f;
	public:

		Location& 	getLocation(void);
		const Location& getLocation(void) const;

		void setLocation(Location);

		Location::Coordinate getX(void) const;
		Location::Coordinate getY(void) const;
		Location::Coordinate getZ(void) const;

		void setX(Location::Coordinate);
		void setY(Location::Coordinate);
		void setZ(Location::Coordinate);

		void setWidth(float);
		void setHeight(float);
		
		float getWidth(void) const;
		float getHeight(void) const;
	};

	class Renderable : mirage::ecs::Component<Renderable>
	{
		bool visible = false;
	public:
		std::vector<mirage::graphics::Icon> 
			overlays,
			underlays;
		mirage::graphics::Icon icon;

		void setVisible(bool);	

		void publishUpdate(void) const;
	};

	class Object : mirage::ecs::ComponentTestator<Object, Body, Renderable> 
	{
		std::string name;
			
	public:	
	};
}
