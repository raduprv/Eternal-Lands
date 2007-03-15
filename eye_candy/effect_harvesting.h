#ifdef EYE_CANDY

#ifndef EFFECT_HARVESTING_H
#define EFFECT_HARVESTING_H

// I N C L U D E S ////////////////////////////////////////////////////////////

#include "eye_candy.h"

namespace ec
{

// C L A S S E S //////////////////////////////////////////////////////////////

class HarvestingEffect : public Effect
{
public: 
  enum HarvestingType
  {
    RADON_POUCH,
    CAVERN_WALL,
    MOTHER_NATURE,
    QUEEN_OF_NATURE,
    BEES,
    BAG_OF_GOLD,
    RARE_STONE,
  };

  HarvestingEffect(EyeCandy* _base, bool* _dead, Vec3* _pos, const HarvestingType _type, const Uint16 _LOD);
  ~HarvestingEffect(); 
  
  virtual EffectEnum get_type() { return EC_HARVESTING; };
  bool idle(const Uint64 usec);
  static Uint64 get_max_end_time() { return 5000000; };
  virtual Uint64 get_expire_time() { return 5000000 + born; };

  ParticleSpawner* spawner;
  ParticleMover* mover;
  ParticleSpawner* spawner2;
  ParticleMover* mover2;
  Vec3 effect_center;
  Vec3 gravity_center;
  HarvestingType type;
};

class HarvestingParticle : public Particle
{
public:
  HarvestingParticle(Effect* _effect, ParticleMover* _mover, const Vec3 _pos, const Vec3 _velocity, const coord_t _size, const alpha_t _alpha, const color_t red, const color_t green, const color_t blue, Texture* _texture, const Uint16 _LOD, const HarvestingEffect::HarvestingType _type);
  ~HarvestingParticle() {}
  
  virtual bool idle(const Uint64 delta_t);
  void draw(const Uint64 usec);
  virtual GLuint get_texture(const Uint16 res_index);
  virtual light_t estimate_light_level() const { return 0.0015; };
  virtual light_t get_light_level();
  
  Texture* texture;
  Uint16 LOD;
  HarvestingEffect::HarvestingType type;
};


///////////////////////////////////////////////////////////////////////////////

}	// End namespace ec

#endif	// defined EFFECT_HARVESTING_H

#endif	// #ifdef EYE_CANDY
