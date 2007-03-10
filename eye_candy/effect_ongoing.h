
#ifndef EFFECT_ONGOING_H
#define EFFECT_ONGOING_H

// I N C L U D E S ////////////////////////////////////////////////////////////

#include "eye_candy.h"

namespace ec
{

// C L A S S E S //////////////////////////////////////////////////////////////

class OngoingEffect : public Effect
{
public: 
  enum OngoingType
  {
    MAGIC_PROTECTION,
    SHIELD,
    MAGIC_IMMUNITY,
    POISON
  };

  OngoingEffect(EyeCandy* _base, bool* _dead, Vec3* _pos, const OngoingType _type, const u_int16_t _LOD, const float _strength);
  ~OngoingEffect(); 
  
  virtual EffectEnum get_type() { return EC_ONGOING; };
  bool idle(const u_int64_t usec);

  ParticleSpawner* spawner;
  ParticleMover* mover;
  Vec3* pos;
  Vec3 effect_center;
  OngoingType type;
  float strength;
};

class OngoingParticle : public Particle
{
public:
  OngoingParticle(Effect* _effect, ParticleMover* _mover, const Vec3 _pos, const Vec3 _velocity, const coord_t _size, const alpha_t _alpha, const color_t red, const color_t green, const color_t blue, Texture* _texture, const u_int16_t _LOD, const OngoingEffect::OngoingType _type);
  ~OngoingParticle() {}
  
  virtual bool idle(const u_int64_t delta_t);
  void draw(const u_int64_t usec);
  virtual GLuint get_texture(const u_int16_t res_index);
  virtual light_t estimate_light_level() const { return 0.002; };
  
  Texture* texture;
  u_int16_t LOD;
  OngoingEffect::OngoingType type;
};


///////////////////////////////////////////////////////////////////////////////

}	// End namespace ec

#endif	// defined EFFECT_ONGOING_H
