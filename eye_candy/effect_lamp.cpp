#ifdef EYE_CANDY

// I N C L U D E S ////////////////////////////////////////////////////////////

#include "eye_candy.h"
#include "math_cache.h"

#include "effect_lamp.h"

namespace ec
{

// C L A S S   F U N C T I O N S //////////////////////////////////////////////

LampParticle::LampParticle(Effect* _effect, ParticleMover* _mover, const Vec3 _pos, const Vec3 _velocity, const float _scale, const Uint16 _LOD) : Particle(_effect, _mover, _pos, _velocity)
{
  LOD = _LOD;
  color[0] = 1.0;
  color[1] = 0.7 + randfloat(0.2);
  color[2] = 0.6;
  size = 9 * (0.5 + 1.5 * randcoord()) / (LOD + 2);
  alpha = 7.0 / size;
  if (alpha > 1.0)
    alpha = 1.0;
  velocity /= size;
  size *= _scale;
  flare_max = 1.0;
  flare_exp = 0.0;
  flare_frequency = 2.0;
}

bool LampParticle::idle(const Uint64 delta_t)
{
  if (effect->recall)
    return false;

  if (alpha < 0.02)
  {
    return false;
  }
  
  const float scalar = math_cache.powf_05_close((float)delta_t / 800000);
  color[0] = color[0] * scalar * 0.5 + color[0] * 0.5;
  color[1] = color[1] * scalar * 0.5 + color[1] * 0.5;
  color[2] = color[2] * scalar * 0.5 + color[2] * 0.5;
  alpha *= scalar;
  
  return true;
}

GLuint LampParticle::get_texture(const Uint16 res_index)
{
  return base->TexFlare.get_texture(res_index);
}

LampBigParticle::LampBigParticle(Effect* _effect, ParticleMover* _mover, const Vec3 _pos, const Vec3 _velocity, const float _scale, const Uint16 _LOD) : Particle(_effect, _mover, _pos, _velocity)
{
  LOD = _LOD;
  color[0] = 1.0;
  color[1] = 0.4 + randfloat(0.3);
  color[2] = 0.3;
  size = 15 * (2.0 + randcoord()) / (LOD + 2);
  alpha = 5.0 * 5 / size / (LOD + 2);
  if (alpha > 1.0)
    alpha = 1.0;
  size *= _scale;
  velocity = Vec3(0.0, 0.0, 0.0);
  flare_max = 1.0;
  flare_exp = 0.0;
  flare_frequency = 2.0;
}

bool LampBigParticle::idle(const Uint64 delta_t)
{
  if (effect->recall)
    return false;

  if ((alpha < 0.25) || (get_time() - born > 8000000))
  {
    ((LampEffect*)effect)->big_particles--;
    return false;
  }
  
  const float scalar = math_cache.powf_05_close((interval_t)delta_t * LOD / 50000000.0);
  color[0] = color[0] * scalar * 0.5 + color[0] * 0.5;
  color[1] = color[1] * scalar * 0.5 + color[1] * 0.5;
  color[2] = color[2] * scalar * 0.5 + color[2] * 0.5;
  alpha *= scalar;
  
  return true;
}

GLuint LampBigParticle::get_texture(const Uint16 res_index)
{
  return base->TexFlare.get_texture(res_index);
}

LampFlareParticle::LampFlareParticle(Effect* _effect, ParticleMover* _mover, const Vec3 _pos, const Vec3 _velocity, const float _scale) : Particle(_effect, _mover, _pos, _velocity)
{
  color[0] = 1.0;
  color[1] = 0.5;
  color[2] = 0.1;
  size = _scale * 8;
  true_size = size;
  alpha = 1.0;
  velocity = Vec3(0.0, 0.0, 0.0);
  flare_max = 1.2;
  flare_exp = 0.5;
  flare_frequency = 0.01;
  true_pos = _pos;
}

bool LampFlareParticle::idle(const Uint64 delta_t)
{
  if (effect->recall)
    return false;
    
  return true;
}

GLuint LampFlareParticle::get_texture(const Uint16 res_index)
{
  return base->TexVoid.get_texture(res_index);
}

LampEffect::LampEffect(EyeCandy* _base, bool* _dead, Vec3* _pos, const float _scale, const bool _halo, const Uint16 _LOD)
{
  if (EC_DEBUG)
    std::cout << "LampEffect (" << this << ") created." << std::endl;
  base = _base;
  dead = _dead,
  pos = _pos;
  scale = _scale;
  halo = _halo;
  if (!halo)
    new_scale = scale * 1.25;
  else
    new_scale = scale;
  sqrt_scale = fastsqrt(new_scale);
  LOD = base->last_forced_LOD;
  desired_LOD = _LOD;
  mover = new SmokeMover(this);
  stationary = new ParticleMover(this);
  spawner = new FilledSphereSpawner(0.05 * sqrt_scale);
  
  
/*
  for (int i = 0; i < LOD * 1.5; i++)
  {
    const Vec3 coords = spawner->get_new_coords() + *pos + Vec3(0.0, 0.05, 0.0);
    Vec3 velocity;
    velocity.randomize(0.2);
    Particle* p = new LampParticle(this, mover, coords, velocity, scale, LOD);
    if (!base->push_back_particle(p))
      break;
  }
*/
  big_particles = 0;
  for (int i = 0; i < LOD * 0.5; i++)
  {
    Vec3 coords = spawner->get_new_coords();
    coords.x *= 0.5;
    coords.z *= 0.5;
    coords.y += 0.1 * sqrt_scale;
    coords += *pos;
    Particle* p = new LampBigParticle(this, stationary, coords, Vec3(0.0, 0.0, 0.0), new_scale, LOD);
    if (!base->push_back_particle(p))
      break;
    big_particles++;
  }
  
  if (halo)
    base->push_back_particle(new LampFlareParticle(this, stationary, *pos + Vec3(0.0, 0.15 * sqrt_scale, 0.0), Vec3(0.0, 0.0, 0.0), sqrt_scale));
}

LampEffect::~LampEffect()
{
  delete mover;
  delete stationary;
  delete spawner;
  if (EC_DEBUG)
    std::cout << "LampEffect (" << this << ") destroyed." << std::endl;
}

bool LampEffect::idle(const Uint64 usec)
{
  if ((recall) && (particles.size() == 0))
    return false;
    
  if (recall)
    return true;

  while (((int)particles.size() < LOD * 7) && (math_cache.powf_0_1_rough_close(randfloat(), (LOD * 7 - particles.size()) * (interval_t)usec / 15000 / square(LOD)) < 0.5))
  {
    Vec3 coords = spawner->get_new_coords() + *pos;
    coords.y += 0.05 * sqrt_scale;
    Vec3 velocity;
    velocity.randomize(0.025 * sqrt_scale);
    Particle* p = new LampParticle(this, mover, coords, velocity, new_scale, LOD);
    if (!base->push_back_particle(p))
      break;
  }

  while ((big_particles < LOD * 2) && (math_cache.powf_0_1_rough_close(randfloat(), (LOD * 2 - big_particles) * (interval_t)usec / 35000 / square(LOD)) < 0.5))
  {
    Vec3 coords = spawner->get_new_coords();
    coords.x *= 0.5;
    coords.z *= 0.5;
    coords.y += 0.1 * sqrt_scale;
    coords += *pos;
    Particle* p = new LampBigParticle(this, stationary, coords, Vec3(0.0, 0.0, 0.0), new_scale, LOD);
    if (!base->push_back_particle(p))
      break;
    big_particles++;
  }

  return true;
}

///////////////////////////////////////////////////////////////////////////////

};

#endif	// #ifdef EYE_CANDY
