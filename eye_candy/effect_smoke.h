
#ifndef EFFECT_SMOKE_H
#define EFFECT_SMOKE_H

// I N C L U D E S ////////////////////////////////////////////////////////////

#include "eye_candy.h"

namespace ec
{

// C L A S S E S //////////////////////////////////////////////////////////////

class SmokeParticle : public Particle
{
public:
  SmokeParticle(Effect* _effect, ParticleMover* _mover, const Vec3 _pos, const Vec3 _velocity, const coord_t _sqrt_scale, const coord_t _max_size, const coord_t size_scalar, const alpha_t alpha_scale);
  ~SmokeParticle() {}
  
  virtual bool idle(const u_int64_t delta_t);
  virtual GLuint get_texture(const u_int16_t res_index);
  virtual light_t estimate_light_level() const { return 0.0; };	// No glow.
  virtual light_t get_light_level() { return 0.0; }; // Same.
  virtual void draw(const u_int64_t usec);
  
  coord_t sqrt_scale;
  coord_t max_size;
};

class SmokeEffect : public Effect
{
public: 
  SmokeEffect(EyeCandy* _base, bool* _dead, Vec3* _pos, const float _scale, const u_int16_t _LOD);
  ~SmokeEffect(); 
  
  virtual EffectEnum get_type() { return EC_SMOKE; };
  bool idle(const u_int64_t usec);
  virtual void request_LOD(const u_int16_t _LOD)
  {
    if (_LOD <= desired_LOD)
      LOD = _LOD;
    else
      LOD = desired_LOD;
    max_size = scale * 270 / (_LOD + 10);
    size_scalar = sqrt_scale * 60 / (_LOD + 5);
    alpha_scalar = 4.3 / (fastsqrt(_LOD) + 1.0);
    count_scalar = 120000 / _LOD;
  };

  ParticleMover* mover;
  ParticleSpawner* spawner;
  interval_t count;
  float scale;
  coord_t sqrt_scale;
  coord_t max_size;
  coord_t size_scalar;
  alpha_t alpha_scalar;
  u_int32_t count_scalar;
};

///////////////////////////////////////////////////////////////////////////////

}	// End namespace ec

#endif	// defined EFFECT_SMOKE_H
