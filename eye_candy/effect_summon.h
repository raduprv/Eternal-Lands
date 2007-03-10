#ifdef SFX

#ifndef EFFECT_SUMMON_H
#define EFFECT_SUMMON_H

// I N C L U D E S ////////////////////////////////////////////////////////////

#include "eye_candy.h"

namespace ec
{

// C L A S S E S //////////////////////////////////////////////////////////////

class OuterSummonParticle : public Particle
{
public:
  OuterSummonParticle(Effect* _effect, ParticleMover* _mover, const Vec3 _pos, const Vec3 _velocity, const coord_t _size, const alpha_t _alpha, const color_t red, const color_t green, const color_t blue, const u_int16_t _LOD);
  ~OuterSummonParticle() {}
  
  virtual bool idle(const u_int64_t delta_t);
  virtual GLuint get_texture(const u_int16_t res_index);
  virtual light_t estimate_light_level() const { return 0.002; };
  
  u_int16_t LOD;
};

class InnerSummonParticle : public Particle
{
public:
  InnerSummonParticle(Effect* _effect, ParticleMover* _mover, const Vec3 _pos, const Vec3 _velocity, const coord_t _size, const alpha_t _alpha, const color_t red, const color_t green, const color_t blue, Texture* _texture, const u_int16_t _LOD);
  ~InnerSummonParticle() {}
  
  virtual bool idle(const u_int64_t delta_t);
  virtual GLuint get_texture(const u_int16_t res_index);
  virtual light_t estimate_light_level() const { return 0.002; };
  static u_int64_t get_max_end_time() { return 6000000; };
  virtual u_int64_t get_expire_time() { return 6000000 + born; };
  virtual bool deletable() { return false; };
  
  Texture* texture;
  u_int16_t LOD;
};

class SummonEffect : public Effect
{
public: 
  enum SummonType
  {
    RABBIT,
    RAT,
    BEAVER,
    DEER,
    GREEN_SNAKE,
    RED_SNAKE,
    BROWN_SNAKE,
    FOX,
    BOAR,
    WOLF,
    PUMA,
    BEAR,
    SKELETON,
    SMALL_GARGOYLE,
    MEDIUM_GARGOYLE,
    LARGE_GARGOYLE,
    FLUFFY,
    CHIMERAN_WOLF,
    YETI,
    ARCTIC_CHIMERAN,
    GIANT
  };

  SummonEffect(EyeCandy* _base, bool* _dead, Vec3* _pos, const SummonType _type, const u_int16_t _LOD);
  ~SummonEffect(); 
  
  virtual EffectEnum get_type() { return EC_SUMMON; };
  bool idle(const u_int64_t usec);
  virtual void request_LOD(const u_int16_t _LOD);

  IFSParticleSpawner* inner_spawner;
  IFSParticleSpawner* outer_spawner;
  SmokeMover* smoke_mover;
  GravityMover* gravity_mover;
  Vec3* pos;
  Vec3 gravity_center;
  coord_t outer_size;
  coord_t outer_radius;
  coord_t inner_size;
  alpha_t outer_alpha;
  alpha_t inner_alpha;
  color_t outer_color[3];
  color_t inner_color[3];
  Texture* inner_texture;
  int64_t count;
  u_int32_t count_scalar;
  SummonType type;
};

///////////////////////////////////////////////////////////////////////////////

}	// End namespace ec

#endif	// defined EFFECT_SUMMON_H

#endif	// #ifdef SFX
