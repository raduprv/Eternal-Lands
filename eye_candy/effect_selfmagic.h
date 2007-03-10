#ifdef SFX

#ifndef EFFECT_SELFMAGIC_H
#define EFFECT_SELFMAGIC_H

// I N C L U D E S ////////////////////////////////////////////////////////////

#include "eye_candy.h"

namespace ec
{

// C L A S S E S //////////////////////////////////////////////////////////////

class SelfMagicEffect : public Effect
{
public: 
  enum SelfMagicType
  {
    HEAL,
    MAGIC_PROTECTION,
    SHIELD,
    RESTORATION,
    BONES_TO_GOLD,
    TELEPORT_TO_THE_PORTALS_ROOM,
    MAGIC_IMMUNITY,
    ALERT	// Not really a spell, but functions like one.
  };

  SelfMagicEffect(EyeCandy* _base, bool* _dead, Vec3* _pos, const SelfMagicType _type, const u_int16_t _LOD);
  ~SelfMagicEffect(); 
  
  virtual EffectEnum get_type() { return EC_SELFMAGIC; };
  bool idle(const u_int64_t usec);
  void draw(const u_int64_t usec);
  static u_int64_t get_max_end_time() { return 4000000; };
  virtual u_int64_t get_expire_time() { return 4000000 + born; };

  ParticleSpawner* spawner;
  ParticleMover* mover;
  ParticleSpawner* spawner2;
  ParticleMover* mover2;
  Vec3 effect_center;
  int64_t count;
  u_int64_t count_scalar;
  SelfMagicType type;
  std::vector<Shape*> capless_cylinders;
  coord_t size_scalar;
  float* target_alpha;
};

class SelfMagicParticle : public Particle
{
public:
  SelfMagicParticle(Effect* _effect, ParticleMover* _mover, const Vec3 _pos, const Vec3 _velocity, const coord_t _size, const alpha_t _alpha, const color_t red, const color_t green, const color_t blue, Texture* _texture, const u_int16_t _LOD, const SelfMagicEffect::SelfMagicType _type);
  ~SelfMagicParticle() {}
  
  virtual bool idle(const u_int64_t delta_t);
  virtual GLuint get_texture(const u_int16_t res_index);
  virtual light_t estimate_light_level() const { return 0.002; };
  
  Texture* texture;
  u_int16_t LOD;
  SelfMagicEffect::SelfMagicType type;
};


///////////////////////////////////////////////////////////////////////////////

}	// End namespace ec

#endif	// defined EFFECT_SELFMAGIC_H

#endif	// #ifdef SFX
