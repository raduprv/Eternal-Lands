#ifdef SFX

#ifndef EFFECT_BAG_H
#define EFFECT_BAG_H

// I N C L U D E S ////////////////////////////////////////////////////////////

#include "eye_candy.h"

namespace ec
{

// C L A S S E S //////////////////////////////////////////////////////////////

class BagParticle : public Particle
{
public:
  BagParticle(Effect* _effect, ParticleMover* _mover, const Vec3 _pos, const Vec3 _velocity, const coord_t size);
  ~BagParticle() {}
  
  virtual bool idle(const u_int64_t delta_t);
  virtual GLuint get_texture(const u_int16_t res_index);
  virtual light_t estimate_light_level() const { return 0.0; };	// No glow.
  virtual light_t get_light_level() { return 0.0; }; // Same.
  virtual void draw(const u_int64_t usec);
};

class BagEffect : public Effect
{
public: 
  BagEffect(EyeCandy* _base, bool* _dead, Vec3* _pos, const bool _picked_up, const u_int16_t _LOD);
  ~BagEffect(); 
  
  virtual EffectEnum get_type() { return EC_BAG; };
  bool idle(const u_int64_t usec);
  static u_int64_t get_max_end_time() { return 2000000; };
  virtual u_int64_t get_expire_time() { return 2000000 + born; };

  GravityMover* mover;
  ParticleSpawner* spawner;
  Vec3 effect_center;
  bool picked_up;
};

///////////////////////////////////////////////////////////////////////////////

}	// End namespace ec

#endif	// defined EFFECT_BAG_H

#endif	// #ifdef SFX
