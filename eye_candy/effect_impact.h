
#ifndef EFFECT_IMPACT_H
#define EFFECT_IMPACT_H

// I N C L U D E S ////////////////////////////////////////////////////////////

#include "eye_candy.h"

namespace ec
{

// C L A S S E S //////////////////////////////////////////////////////////////

class ImpactEffect : public Effect
{
public: 
  enum ImpactType
  {
    MAGIC_PROTECTION,
    SHIELD,
    MAGIC_IMMUNITY,
    POISON,
    BLOOD
  };

  ImpactEffect(EyeCandy* _base, bool* _dead, Vec3* _pos, const Vec3 _angle, const ImpactType _type, const u_int16_t _LOD, const float _strength);
  ~ImpactEffect(); 
  
  virtual EffectEnum get_type() { return EC_IMPACT; };
  bool idle(const u_int64_t usec);
  static u_int64_t get_max_end_time() { return 5000000; };
  virtual u_int64_t get_expire_time() { return 5000000 + born; };

  ParticleSpawner* spawner;
  ParticleMover* mover;
  Vec3 center;
  Vec3 angle;
  Vec3 effect_center;
  ImpactType type;
  float strength;
};

class ImpactParticle : public Particle
{
public:
  ImpactParticle(Effect* _effect, ParticleMover* _mover, const Vec3 _pos, const Vec3 _velocity, const coord_t _size, const alpha_t _alpha, const color_t red, const color_t green, const color_t blue, Texture* _texture, const u_int16_t _LOD, const ImpactEffect::ImpactType _type);
  ~ImpactParticle() {}
  
  virtual bool idle(const u_int64_t delta_t);
  void draw(const u_int64_t usec);
  virtual GLuint get_texture(const u_int16_t res_index);
  virtual light_t estimate_light_level() const { return 0.002; };
  
  Texture* texture;
  u_int16_t LOD;
  ImpactEffect::ImpactType type;
};


///////////////////////////////////////////////////////////////////////////////

}	// End namespace ec

#endif	// defined EFFECT_IMPACT_H
