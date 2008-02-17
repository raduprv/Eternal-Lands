#ifdef MISSILES

/*!
\brief Special effects for special missiles (fire, ice, explosive...)
*/


#ifndef EFFECT_MISSILE_H
#define EFFECT_MISSILE_H

// I N C L U D E S ////////////////////////////////////////////////////////////

#include "eye_candy.h"

namespace ec
{

// C L A S S E S //////////////////////////////////////////////////////////////

class MissileParticle : public Particle
{
public:
  MissileParticle(Effect* _effect, ParticleMover* _mover, const Vec3 _pos, const Vec3 _velocity, const coord_t _size, const alpha_t _alpha, const color_t red, const color_t green, const color_t blue, Texture* _texture, const Uint16 _LOD);
  ~MissileParticle() {}
  
  virtual bool idle(const Uint64 delta_t);
  virtual GLuint get_texture(const Uint16 res_index);
  virtual light_t estimate_light_level() const { return 0.0; };
  virtual light_t get_light_level() { return 0.0; };
  
  Texture* texture;
  Uint16 LOD;
};

class MissileEffect : public Effect
{
public: 
  enum MissileType
  {
    MAGIC,
    FIRE,
    ICE,
    EXPLOSIVE
  };

  MissileEffect(EyeCandy* _base, bool* _dead, Vec3* _pos, const MissileType _type, const Uint16 _LOD);
  ~MissileEffect(); 
  
  virtual EffectEnum get_type() { return EC_MISSILE; };
  bool idle(const Uint64 usec);
  virtual void request_LOD(const float _LOD);

  ParticleMover* mover;
  Vec3 old_pos;
  coord_t size;
  alpha_t alpha;
  color_t color[3];
  Texture* texture;
  MissileType type;
};

///////////////////////////////////////////////////////////////////////////////

}	// End namespace ec

#endif	// defined EFFECT_SWORD_H

#endif // MISSILES
