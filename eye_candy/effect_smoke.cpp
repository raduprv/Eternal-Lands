#ifdef EYE_CANDY

// I N C L U D E S ////////////////////////////////////////////////////////////

#include "eye_candy.h"
#include "math_cache.h"

#include "effect_smoke.h"

namespace ec
{

// C L A S S   F U N C T I O N S //////////////////////////////////////////////

SmokeParticle::SmokeParticle(Effect* _effect, ParticleMover* _mover, const Vec3 _pos, const Vec3 _velocity, const coord_t _sqrt_scale, const coord_t _max_size, const coord_t size_scalar, const alpha_t alpha_scale) : Particle(_effect, _mover, _pos, _velocity)
{
  sqrt_scale = _sqrt_scale;
  max_size = _max_size;
  const color_t color_scale = square(randcolor(0.3));
  color[0] = square(randcolor(0.1)) + color_scale;
  color[1] = square(randcolor(0.1)) + color_scale;
  color[2] = square(randcolor(0.1)) + color_scale;
  size = size_scalar * (0.5 + randcoord());
  alpha = (0.05 + randcoord(0.1)) * alpha_scale;
  if (alpha > 1.0)
    alpha = 1.0;
  flare_max = 1.0;
  flare_exp = 1.0;
  flare_frequency = 1.0;
  state = 0;
}

void SmokeParticle::draw(const Uint64 usec)
{
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  Vec3 shifted_pos = pos - *(((SmokeEffect*)effect)->pos);

  Particle::draw(usec);

  glBlendFunc(GL_SRC_ALPHA, GL_ONE);
}

bool SmokeParticle::idle(const Uint64 delta_t)
{
  if (effect->recall)
    return false;

  const alpha_t alpha_scalar = 1.0 - math_cache.powf_05_close((float)delta_t / (60000000 * sqrt_scale));
  alpha -= alpha_scalar;

  if (alpha < 0.004)
    return false;
  
  const coord_t size_scalar = math_cache.powf_05_close((float)delta_t / (1500000 * sqrt_scale));
  size = size / size_scalar * 0.25 + size * 0.75;
  if (size >= max_size)
    size = max_size;
  
  Vec3 velocity_shift;
  velocity_shift.randomize();
  velocity_shift.y /= 3;
  velocity_shift.normalize(0.00002 * fastsqrt(delta_t));
  velocity += velocity_shift;
  return true;
}

GLuint SmokeParticle::get_texture(const Uint16 res_index)
{
  return base->TexSimple.get_texture(res_index);
}

SmokeEffect::SmokeEffect(EyeCandy* _base, bool* _dead, Vec3* _pos, const float _scale, const Uint16 _LOD)
{
  if (EC_DEBUG)
    std::cout << "SmokeEffect (" << this << ") created (" << *_pos << ", " << _scale << ")" << std::endl;
  base = _base;
  dead = _dead;
  pos = _pos;
  count = 0;
  scale = _scale;
  sqrt_scale = fastsqrt(scale);
  max_size = scale * 270 / (_LOD + 10);
  size_scalar = sqrt_scale * 50 / (_LOD + 5);
  alpha_scalar = 4.3 / (fastsqrt(_LOD) + 1.0);
  count_scalar = 1000000 / _LOD;
  LOD = _LOD;
  desired_LOD = _LOD;
  mover = new GradientMover(this);
  spawner = new FilledDiscSpawner(0.2 * sqrt_scale);

//  Test code:
//  Particle* p = new SmokeParticle(this, mover, *pos, Vec3(0.0, 0.0, 0.0), sqrt_scale, max_size, size_scalar, alpha_scalar);
//  base->push_back_particle(p);
/*  
  for (int i = 0; i < LOD * 4; i++)
  {
    const Vec3 coords = spawner->get_new_coords() + *pos;
    Vec3 velocity;
    velocity.randomize(0.015);
    velocity.y += 0.25 + randcoord(0.15);
    Particle* p = new SmokeParticle(this, mover, coords, velocity, sqrt_scale, max_size, size_scalar, alpha_scalar);
    if (!base->push_back_particle(p))
      break;
  }
*/
}

SmokeEffect::~SmokeEffect()
{
  delete mover;
  delete spawner;
  if (EC_DEBUG)
    std::cout << "SmokeEffect (" << this << ") destroyed." << std::endl;
}

bool SmokeEffect::idle(const Uint64 usec)
{
  if ((recall) && (particles.size() == 0))
    return false;
    
  if (recall)
    return true;
    
  count += usec;

  while (count > 0)
  {
    const Vec3 coords = spawner->get_new_coords() + *pos;
    Vec3 velocity;
    velocity.randomize(0.015);
    velocity.y += 0.3;
    Particle* p = new SmokeParticle(this, mover, coords, velocity, sqrt_scale, max_size, size_scalar, alpha_scalar);
    if (!base->push_back_particle(p))
    {
      count = 0;
      break;
    }
    count -= count_scalar;
  }

  return true;
}

///////////////////////////////////////////////////////////////////////////////

};

#endif	// #ifdef EYE_CANDY
