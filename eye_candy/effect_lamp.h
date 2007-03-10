#ifdef SFX

#ifndef EFFECT_LAMP_H
#define EFFECT_LAMP_H

// I N C L U D E S ////////////////////////////////////////////////////////////

#include "eye_candy.h"

namespace ec
{

// C L A S S E S //////////////////////////////////////////////////////////////

class LampParticle : public Particle
{
public:
  LampParticle(Effect* _effect, ParticleMover* _mover, const Vec3 _pos, const Vec3 _velocity, const u_int16_t _LOD);
  ~LampParticle() {}
  
  virtual bool idle(const u_int64_t delta_t);
  virtual GLuint get_texture(const u_int16_t res_index);
  virtual light_t estimate_light_level() const { return 0.0; };	// We don't want the particle system lights to be used on the lamp, since it will assumedly already have one.
  virtual light_t get_light_level() { return 0.0; }; // Same.
  
  u_int16_t LOD;
};

class LampBigParticle : public Particle
{
public:
  LampBigParticle(Effect* _effect, ParticleMover* _mover, const Vec3 _pos, const Vec3 _velocity, const u_int16_t _LOD);
  ~LampBigParticle() {}
  
  virtual bool idle(const u_int64_t delta_t);
  virtual GLuint get_texture(const u_int16_t res_index);
  virtual light_t estimate_light_level() const { return 0.0; };	// Like above
  virtual light_t get_light_level() { return 0.0; }; // Same.
  
  u_int16_t LOD;
};

class LampFlareParticle : public Particle
{
public:
  LampFlareParticle(Effect* _effect, ParticleMover* _mover, const Vec3 _pos, const Vec3 _velocity);
  ~LampFlareParticle() {}
  
  virtual bool idle(const u_int64_t delta_t);
  virtual GLuint get_texture(const u_int16_t res_index);
  virtual light_t estimate_light_level() const { return 0.0; };	// Like above
  virtual light_t get_light_level() { return 0.0; }; // Same.
  virtual bool deletable() { return false; };
  
  Vec3 true_pos;
  coord_t true_size;
};

class LampEffect : public Effect
{
public: 
  LampEffect(EyeCandy* _base, bool* _dead, Vec3* _pos, const u_int16_t _LOD);
  ~LampEffect(); 
  
  virtual EffectEnum get_type() { return EC_LAMP; };
  bool idle(const u_int64_t usec);

  GradientMover* mover;
  ParticleMover* stationary;
  ParticleSpawner* spawner;
  int big_particles;
};

///////////////////////////////////////////////////////////////////////////////

}	// End namespace ec

#endif	// defined EFFECT_LAMP_H

#endif	// #ifdef SFX
