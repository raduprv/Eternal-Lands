#ifdef EYE_CANDY

#ifndef EFFECT_BREATH_H
#define EFFECT_BREATH_H

// I N C L U D E S ////////////////////////////////////////////////////////////

#include "eye_candy.h"

namespace ec
{

// C L A S S E S //////////////////////////////////////////////////////////////

class BreathEffect : public Effect
{
public: 
  enum BreathType
  {
    FIRE,
    ICE,
    POISON,
    MAGIC,
    LIGHTNING,
    WIND
  };

  BreathEffect(EyeCandy* _base, bool* _dead, Vec3* _pos, Vec3* _target, const std::vector<ec::Obstruction*> _obstructions, const BreathType _type, const Uint16 _LOD, const percent_t _scale);
  ~BreathEffect(); 
  
  virtual EffectEnum get_type() { return EC_BREATH; };
  bool idle(const Uint64 usec);
  virtual void request_LOD(const Uint16 _LOD)
  {
    if (_LOD <= desired_LOD)
      LOD = _LOD;
    else
      LOD = desired_LOD;
    count_scalar = 3000 / LOD;
    size_scalar = scale * fastsqrt(LOD) / sqrt(10.0);
  };
  static Uint64 get_max_end_time() { return 5000000; };
  virtual Uint64 get_expire_time() { return 5000000 + born; };

  ParticleSpawner* spawner;
  ParticleMover* mover;
  Vec3* target;
  percent_t scale;
  BreathType type;
  interval_t count;
  interval_t count_scalar;
  interval_t size_scalar;
};

class BreathParticle : public Particle
{
public:
  BreathParticle(Effect* _effect, ParticleMover* _mover, const Vec3 _pos, const Vec3 _velocity, const coord_t _size, const alpha_t _alpha, const color_t red, const color_t green, const color_t blue, Texture* _texture, const Uint16 _LOD, const BreathEffect::BreathType _type);
  ~BreathParticle() {}
  
  virtual bool idle(const Uint64 delta_t);
  virtual GLuint get_texture(const Uint16 res_index);
  virtual light_t estimate_light_level() const { return 0.002; };
  
  Texture* texture;
  Uint16 LOD;
  BreathEffect::BreathType type;
};

class BreathSmokeParticle : public Particle
{
public:
  BreathSmokeParticle(Effect* _effect, ParticleMover* _mover, const Vec3 _pos, const Vec3 _velocity, const coord_t _size, const alpha_t _alpha, Texture* _texture, const Uint16 _LOD, const BreathEffect::BreathType _type);
  ~BreathSmokeParticle() {}
  
  virtual bool idle(const Uint64 delta_t);
  virtual GLuint get_texture(const Uint16 res_index);
  virtual light_t estimate_light_level() const { return 0.0; };	// No glow.
  virtual light_t get_light_level() { return 0.0; }; // Same.
  virtual void draw(const Uint64 usec);
  
  Texture* texture;
  BreathEffect::BreathType type;
};


///////////////////////////////////////////////////////////////////////////////

}	// End namespace ec

#endif	// defined EFFECT_BREATH_H

#endif	// #ifdef EYE_CANDY
