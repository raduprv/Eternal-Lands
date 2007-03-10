#ifdef SFX

#ifndef EFFECT_CLOUD_H
#define EFFECT_CLOUD_H

// I N C L U D E S ////////////////////////////////////////////////////////////

#include "eye_candy.h"

namespace ec
{

// C L A S S E S //////////////////////////////////////////////////////////////

class CloudParticle : public Particle
{
public:
  CloudParticle(Effect* _effect, ParticleMover* _mover, const Vec3 _pos, const Vec3 _velocity, const coord_t _min_height, const coord_t _max_height, const coord_t _size, const alpha_t _alpha);
  ~CloudParticle() {}
  
  virtual bool idle(const u_int64_t delta_t);
  void draw(const u_int64_t usec);
  virtual GLuint get_texture(const u_int16_t res_index);
  virtual light_t estimate_light_level() const { return 0.0; };	// Clouds don't glow.  :)
  virtual light_t get_light_level() { return 0.0; };
  virtual bool deletable() { return false; };
  
  coord_t min_height;
  coord_t max_height;
  std::vector<CloudParticle*> neighbors;
  Vec3 normal;
  light_t brightness;
};

class CloudEffect : public Effect
{
public: 
  CloudEffect(EyeCandy* _base, bool* _dead, Vec3* _pos, const float _density, const std::vector<PolarCoordElement> bounding_range, const u_int16_t _LOD);
  ~CloudEffect(); 
  
  virtual EffectEnum get_type() { return EC_CLOUD; };
  bool idle(const u_int64_t usec);

  PolarCoordsBoundingMover* mover;
  FilledPolarCoordsSpawner* spawner;
  Vec3 center;
};

///////////////////////////////////////////////////////////////////////////////

}	// End namespace ec

#endif	// defined EFFECT_CLOUD_H

#endif	// #ifdef SFX
