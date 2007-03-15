#ifdef EYE_CANDY

// I N C L U D E S ////////////////////////////////////////////////////////////

#include "eye_candy.h"
#include "math_cache.h"

#include "effect_campfire.h"

namespace ec
{

extern MathCache_Lorange math_cache;

// C L A S S   F U N C T I O N S //////////////////////////////////////////////

CampfireParticle::CampfireParticle(Effect* _effect, ParticleMover* _mover, const Vec3 _pos, const Vec3 _velocity, const float _scale, const float _sqrt_scale, const int _state, const Uint16 _LOD) : Particle(_effect, _mover, _pos, _velocity)
{
  color[0] = 1.0;
  color[1] = 0.35 + randfloat() / 2.5;
  color[2] = 0.2;
  LOD = _LOD;
  size = _sqrt_scale * 6 * (0.5 + 5 * randfloat()) / (_LOD + 2.0);
  size_max = 270 * _scale / (_LOD + 10);
  alpha = randfloat(0.5) + (1.0 - (_LOD + 20.0) / 60.0);
  velocity /= size;
  flare_max = 1.5;
  flare_exp = 0.3;
  flare_frequency = 5.0;
  state = _state;
  if (state)
    size *= 0.7;
}

bool CampfireParticle::idle(const Uint64 delta_t)
{
  if (effect->recall)
    return false;

  if (alpha < 0.13 - LOD * 0.01)
    return false;
  
  const float scalar = math_cache.powf_05_close((interval_t)delta_t / (3000000 - LOD * 200000));
  if (state != 2)
    color[0] = color[0] * scalar * 0.3 + color[0] * 0.7;

  if (state == 1)
  {
    alpha *= scalar;
    if ((alpha <= 0.25 - LOD * 0.01) && (random() & 1))
    {
      state = 2;
      size *= 1.8;
      alpha = 0.13 + randfloat(0.16);
      pos.y -= 0.29 + randfloat(0.13);
      color[0] = 0.07;
      color[1] = 0.05;
      color[2] = 0.05;
      velocity.x *= 2.3;
      velocity.z *= 2.3;
      while (size > 5.0)
      {
        size *= 0.75;
        alpha = fastsqrt(alpha);
      }
    }
  }
  else if (state == 0)
    alpha = alpha * square(scalar) * 0.6 + alpha * 0.4;
  else
    alpha *= scalar;

  velocity.x = velocity.x * scalar;
  velocity.z = velocity.z * scalar;
  size = size / scalar * 0.3 + size * 0.7;
  if (size >= size_max)
    size = size_max;
    
  return true;
}

GLuint CampfireParticle::get_texture(const Uint16 res_index)
{
  if (state == 0)
    return base->TexFlare.get_texture(res_index);
  else
   return base->TexSimple.get_texture(res_index);
}

void CampfireParticle::draw(const Uint64 usec)
{
  if (state == 0)
  {
    Particle::draw(usec);
  }
  else
  {
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    Particle::draw(usec);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);
  }
}

CampfireBigParticle::CampfireBigParticle(Effect* _effect, ParticleMover* _mover, const Vec3 _pos, const Vec3 _velocity, const float _sqrt_scale, const Uint16 _LOD) : Particle(_effect, _mover, _pos, _velocity)
{
  const float LOD = _LOD / 10.0;
  color[0] = 1.0;
  color[1] = 0.5 + randfloat() / 2;
  color[2] = 0.3;
  size = 7.5 * (2.0 + randfloat()) / 2.5;
  alpha = 7.0 / size / (LOD + 5);
  size *= _sqrt_scale;
  if (alpha > 1.0)
    alpha = 1.0;
  velocity = Vec3(0.0, 0.0, 0.0);
  flare_max = 1.0;
  flare_exp = 0.0;
  flare_frequency = 2.0;
}

bool CampfireBigParticle::idle(const Uint64 delta_t)
{
  if (effect->recall)
    return false;

  return true;
}

GLuint CampfireBigParticle::get_texture(const Uint16 res_index)
{
  return base->TexFlare.get_texture(res_index);
}

CampfireEffect::CampfireEffect(EyeCandy* _base, bool* _dead, Vec3* _pos, const std::vector<ec::Obstruction*> _obstructions, const float _scale, const Uint16 _LOD)
{
  if (EC_DEBUG)
    std::cout << "CampfireEffect (" << this << ") created." << std::endl;
  base = _base;
  dead = _dead;
  pos = _pos;
  obstructions = _obstructions;
  scale = _scale;
  sqrt_scale = fastsqrt(scale);
  LOD = _LOD;
  desired_LOD = _LOD;
  mover = new SmokeMover(this);
  stationary = new ParticleMover(this);
  spawner = new FilledSphereSpawner(0.2 * sqrt_scale);
  active = true;
/*
  for (int i = 0; i < LOD * 10; i++)
  {
    const Vec3 coords = spawner->get_new_coords() + *pos + Vec3(0.0, 0.10, 0.0);
    Vec3 velocity;
    velocity.randomize(0.15 * sqrt_scale);
    velocity += Vec3(0.0, 0.15 * scale, 0.0);
    Particle* p = new CampfireParticle(this, mover, coords, velocity, scale, sqrt_scale, 0, LOD);
    if (!base->push_back_particle(p))
      break;
  }
*/
  big_particles = 0;
  for (int i = 0; i < 20; i++)
  {
    const Vec3 coords = spawner->get_new_coords() * 1.3 + *pos + Vec3(0.0, 0.15, 0.0);
    Particle* p = new CampfireBigParticle(this, stationary, coords, Vec3(0.0, 0.0, 0.0), sqrt_scale, LOD);
    if (!base->push_back_particle(p))
      break;
    big_particles++;
  }
}

CampfireEffect::~CampfireEffect()
{
  delete mover;
  delete spawner;
  if (EC_DEBUG)
    std::cout << "CampfireEffect (" << this << ") destroyed." << std::endl;
}

bool CampfireEffect::idle(const Uint64 usec)
{
  if ((recall) && (particles.size() == 0))
    return false;
    
  if (recall)
    return true;
  
  while (((int)particles.size() < LOD * 100) && (math_cache.powf_0_1_rough_close(randfloat(), (interval_t)usec / 80000 * LOD) < 0.5))
  {
    int state = 1;
    if (random() & 1)
      state = 0;

    Vec3 coords;
    if (!state)
      coords = spawner->get_new_coords() + *pos + Vec3(0.0, 0.07, 0.0);
    else
      coords = spawner->get_new_coords() * 0.3 + *pos + Vec3(0.0, 0.12, 0.0);
    Vec3 velocity;
    velocity.randomize(0.1 * sqrt_scale);
    velocity.y = velocity.y * 0.75 + 0.15 * scale;
    Particle* p = new CampfireParticle(this, mover, coords, velocity, scale, sqrt_scale, state, LOD);
    if (!base->push_back_particle(p))
      break;
  }

  return true;
}

///////////////////////////////////////////////////////////////////////////////

};

#endif	// #ifdef EYE_CANDY
