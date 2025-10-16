#ifndef NBODY_H
#define NBODY_H

#include <vector>
#include <string>
#include <cmath> 

namespace nbody {

struct Point2D {
  float x, y;
  Point2D(): Point2D(0.0f,0.0f){}
  Point2D(float x, float y): x(x), y(y) {}
  float SquareLength() {
    return (x*x + y*y);
  }
  float Length() {
	return sqrt(SquareLength());
  }
  Point2D UnitVector() {
	return Scale(1.0f / Length());
  }
  Point2D Add(const Point2D& p2) {
	return Point2D(x+p2.x, y+p2.y);
  }
  Point2D Sub(const Point2D& p2) {
	return Point2D(x-p2.x, y-p2.y);
  }
  Point2D Scale(float scale) {
	return Point2D(x*scale, y*scale);
  }
  Point2D Scale(float xscale, float yscale) {
	return Point2D(x*xscale, y*yscale);
  }  
  bool Inside(const Point2D& min, const Point2D& max) {
    return (x >= min.x && x < max.x && y >= min.y && y < max.y);
  }
};

struct PointMass {
  Point2D position;
  Point2D velocity;
  float mass = 1.0f;
  float data[3] = {0.0f}; // pad to 32 bytes
};

class ParticleSystem {
  public:
    // Setup the particle system using a random seed.
	void Initialize(int count, Point2D min, Point2D max, unsigned int seed);
	
	// Compute the effect of all other points on point at `index`.
	void CalculatePoint(int index, float timestep);
	
	// Update the position of point at `index` based on its current velocity.
	void UpdatePoint(int index, float timestep);
	
	// Simulate the whole system using a time scaling factor `timestep`.
    virtual void Simulate(float timestep);
	
	int Count() { return data.size(); }
	
	// Access to point data.
	PointMass Get(int index) { return data[index]; }
	
	// Save file for visualization.
	bool SaveToFile(const std::string& file_name, Point2D min, Point2D max, int width, int height);
  protected:
    std::vector<PointMass> data;
};

class ParticleSystemOpenMP : public ParticleSystem {
  public:
    void Simulate(float timestep) override;
};

class ParticleSystemOpenMPDynamic : public ParticleSystem {
  public:
    void Simulate(float timestep) override;
};

class ParticleSystemStdThread : public ParticleSystem {
  public:
    void Simulate(float timestep) override;
};

} // namespace nbody.

#endif // NBODY_H