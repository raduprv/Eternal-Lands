#ifdef SFX

// I N C L U D E S ////////////////////////////////////////////////////////////

#include "eye_candy.h"
#include "math_cache.h"

#include "effect_lamp.h"

namespace ec
{

extern MathCache_Lorange math_cache;

// C L A S S   F U N C T I O N S //////////////////////////////////////////////

LampParticle::LampParticle(Effect* _effect, ParticleMover* _mover, const Vec3 _pos, const Vec3 _velocity, const u_int16_t _LOD) : Particle(_effect, _mover, _pos, _velocity)
{
  LOD = _LOD;
  color[0] = 1.0;
  color[1] = randfloat() / 2;
  color[2] = 0.2;
  size = 16 * (0.5 + 1.5 * randcoord()) / (LOD + 2);
  alpha = 7.0 / size;
  if (alpha > 1.0)
    alpha = 1.0;
  velocity /= size;
  flare_max = 1.0;
  flare_exp = 0.0;
  flare_frequency = 2.0;
}

bool LampParticle::idle(const u_int64_t delta_t)
{
  if (effect->recall)
    return false;

  if (alpha < 0.02)
    return false;

  const float scalar = math_cache.powf_05_close((float)delta_t / 800000);
  color[0] = color[0] * scalar * 0.25 + color[0] * 0.75;
  color[1] = color[1] * scalar * 0.5 + color[0] * 0.5;
  color[2] = color[2] * scalar * 0.75 + color[0] * 0.25;
  alpha *= scalar;
  
  return true;
}

GLuint LampParticle::get_texture(const u_int16_t res_index)
{
  return base->TexFlare.get_texture(res_index);
}

LampBigParticle::LampBigParticle(Effect* _effect, ParticleMover* _mover, const Vec3 _pos, const Vec3 _velocity, const u_int16_t _LOD) : Particle(_effect, _mover, _pos, _velocity)
{
  LOD = _LOD;
  color[0] = 1.0;
  color[1] = randfloat() / 2;
  color[2] = 0.2;
  size = 19 * (2.0 + randcoord()) / (LOD + 2);
  alpha = 5.0 * 5 / size / (LOD + 2);
  if (alpha > 1.0)
    alpha = 1.0;
  velocity = Vec3(0.0, 0.0, 0.0);
  flare_max = 1.0;
  flare_exp = 0.0;
  flare_frequency = 2.0;
}

bool LampBigParticle::idle(const u_int64_t delta_t)
{
  if (effect->recall)
    return false;

  if (alpha < 0.25)
  {
    ((LampEffect*)effect)->big_particles--;
    return false;
  }
  
  const float scalar = math_cache.powf_05_close((interval_t)delta_t * LOD / 50000000);
  color[0] = color[0] * scalar * 0.25 + color[0] * 0.75;
  color[1] = color[1] * scalar * 0.5 + color[0] * 0.5;
  color[2] = color[2] * scalar * 0.75 + color[0] * 0.25;
  alpha *= scalar;
  
  return true;
}

GLuint LampBigParticle::get_texture(const u_int16_t res_index)
{
  return base->TexFlare.get_texture(res_index);
}

LampFlareParticle::LampFlareParticle(Effect* _effect, ParticleMover* _mover, const Vec3 _pos, const Vec3 _velocity) : Particle(_effect, _mover, _pos, _velocity)
{
  color[0] = 1.0;
  color[1] = 0.5;
  color[2] = 0.1;
  size = 11.3;
  true_size = size;
  alpha = 1.0;
  velocity = Vec3(0.0, 0.0, 0.0);
  flare_max = 1.2;
  flare_exp = 0.5;
  flare_frequency = 0.01;
  true_pos = _pos;
}

bool LampFlareParticle::idle(const u_int64_t delta_t)
{
  if (effect->recall)
    return false;
    
  return true;
}

GLuint LampFlareParticle::get_texture(const u_int16_t res_index)
{
  return base->TexVoid.get_texture(res_index);
}

LampEffect::LampEffect(EyeCandy* _base, bool* _dead, Vec3* _pos, const u_int16_t _LOD)
{
  if (EC_DEBUG)
    std::cout << "LampEffect (" << this << ") created." << std::endl;
  base = _base;
  dead = _dead,
  pos = _pos;
  LOD = _LOD;
  desired_LOD = _LOD;
  mover = new SmokeMover(this);
  stationary = new ParticleMover(this);
  spawner = new FilledSphereSpawner(0.15);

/*
  for (int i = 0; i < LOD * 1.5; i++)
  {
    const Vec3 coords = spawner->get_new_coords() + *pos + Vec3(0.0, 0.05, 0.0);
    Vec3 velocity;
    velocity.randomize(0.2);
    Particle* p = new LampParticle(this, mover, coords, velocity, LOD);
    if (!base->push_back_particle(p))
      break;
  }
*/
  big_particles = 0;
  for (int i = 0; i < LOD * 0.5; i++)
  {
    const Vec3 coords = spawner->get_new_coords() + *pos + Vec3(0.0, 0.1, 0.0);
    Particle* p = new LampBigParticle(this, stationary, coords, Vec3(0.0, 0.0, 0.0), LOD);
    if (!base->push_back_particle(p))
      break;
    big_particles++;
  }
  
  base->push_back_particle(new LampFlareParticle(this, stationary, *pos + Vec3(0.0, 0.15, 0.0), Vec3(0.0, 0.0, 0.0)));
}

LampEffect::~LampEffect()
{
  delete mover;
  delete spawner;
  if (EC_DEBUG)
    std::cout << "LampEffect (" << this << ") destroyed." << std::endl;
}

bool LampEffect::idle(const u_int64_t usec)
{
  if ((recall) && (particles.size() == 0))
    return false;
    
  if (recall)
    return true;
  
  while (((int)particles.size() < LOD * 6) && (math_cache.powf_0_1_rough_close(randfloat(), (interval_t)usec / 5000 / LOD) < 0.5))
  {
    const Vec3 coords = spawner->get_new_coords() + *pos + Vec3(0.0, 0.05, 0.0);
    Vec3 velocity;
    velocity.randomize(0.2);
    Particle* p = new LampParticle(this, mover, coords, velocity, LOD);
    if (!base->push_back_particle(p))
      break;
  }

  while ((big_particles < LOD * 2) && (math_cache.powf_0_1_rough_close(randfloat(), (2 * LOD - big_particles) * (interval_t)usec / 50000) < 0.5))
  {
    const Vec3 coords = spawner->get_new_coords() + *pos + Vec3(0.0, 0.1, 0.0);
    Particle* p = new LampBigParticle(this, stationary, coords, Vec3(0.0, 0.0, 0.0), LOD);
    if (!base->push_back_particle(p))
      break;
    big_particles++;
  }
    
  return true;
}

///////////////////////////////////////////////////////////////////////////////

};

#endif	// #ifdef SFX
