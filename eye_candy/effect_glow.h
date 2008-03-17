/*!
\brief Special effects for glows.
*/


#ifndef EFFECT_GLOW_H
#define EFFECT_GLOW_H

// I N C L U D E S ////////////////////////////////////////////////////////////

#include "eye_candy.h"

namespace ec
{

// C L A S S E S //////////////////////////////////////////////////////////////

class GlowEffect : public Effect
{
public: 
  enum GlowType
  {
    REMOTE_HEAL_GLOW,
    HARM_GLOW,
    POISON_GLOW,
	LEVEL_UP_OA_GLOW,
	LEVEL_UP_ATT_GLOW,
	LEVEL_UP_DEF_GLOW,
  };

  GlowEffect(EyeCandy* _base, bool* _dead, Vec3* _pos, const GlowType _type, const Uint16 _LOD);
  ~GlowEffect(); 
  
  virtual EffectEnum get_type() { return EC_GLOW; };
  bool idle(const Uint64 usec);
  static Uint64 get_max_end_time() { return 5000000; };
  virtual Uint64 get_expire_time() { return 5000000 + born; };

  ParticleSpawner* spawner;
  ParticleSpawner* spawner2;
  ParticleSpawner* spawner3;
  ParticleMover* mover;
  ParticleMover* mover2;
  ParticleMover* mover3;
  Vec3 effect_center;
  GlowType type;
  Vec3 shift;
};

class GlowParticle : public Particle
{
public:
  GlowParticle(Effect* _effect, ParticleMover* _mover, const Vec3 _pos, const Vec3 _velocity, const coord_t _size, const alpha_t _alpha, const color_t red, const color_t green, const color_t blue, Texture* _texture, const Uint16 _LOD, const GlowEffect::GlowType _type);
  ~GlowParticle() {}
  
  virtual bool idle(const Uint64 delta_t);
  void draw(const Uint64 usec);
  virtual GLuint get_texture(const Uint16 res_index);
  virtual light_t estimate_light_level() const { return 0.0015; };
  virtual light_t get_light_level();
  
  Texture* texture;
  Uint16 LOD;
  GlowEffect::GlowType type;
};


///////////////////////////////////////////////////////////////////////////////

}	// End namespace ec

#endif	// defined EFFECT_GLOW_H

