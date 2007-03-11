#ifdef SFX

// I N C L U D E S ////////////////////////////////////////////////////////////

#include "eye_candy.h"
#include "math_cache.h"

#include "effect_bag.h"

namespace ec
{

extern MathCache_Lorange math_cache;

// C L A S S   F U N C T I O N S //////////////////////////////////////////////

BagParticle::BagParticle(Effect* _effect, ParticleMover* _mover, const Vec3 _pos, const Vec3 _velocity, const coord_t _size) : Particle(_effect, _mover, _pos, _velocity)
{
  color[0] = randcolor(0.3) + 0.7;
  color[1] = randcolor(0.3) + 0.5;
  color[2] = randcolor(0.3) + 0.3;
  size = _size;
  alpha = 0.75;
  flare_max = 1.0;
  flare_exp = 1.0;
  flare_frequency = 1.0;
  state = 0;
}

void BagParticle::draw(const Uint64 usec)
{
  glEnable(GL_LIGHTING);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  Vec3 shifted_pos = pos - ((BagEffect*)effect)->effect_center;
#if 0	// Clear, but slow.  Consider this a comment.
  Vec3 velocity2 = velocity;
//    velocity2.normalize();
  shifted_pos.normalize();
  Vec3 cross = velocity2.cross(shifted_pos).cross(velocity2);
  glNormal3f(cross.x, cross.y, cross.z);
#else	// Faster, but obfuscated.  NOTE: For some reason, it's not producing the same output!  However, I like the new output better.
  const coord_t vel_mag_squared = velocity.magnitude_squared();
  const coord_t sp_mag = shifted_pos.magnitude();
  const coord_t scalar = vel_mag_squared * sp_mag;
  const coord_t vx_squared = velocity.x * velocity.x;
  const coord_t vy_squared = velocity.y * velocity.y;
  const coord_t vz_squared = velocity.z * velocity.z;
  const coord_t x = (shifted_pos.x * (vz_squared - vy_squared) - velocity.x * (shifted_pos.z * velocity.z + shifted_pos.y * velocity.y)) / scalar;
  const coord_t y = (shifted_pos.y * (vx_squared - vz_squared) - velocity.y * (shifted_pos.x * velocity.x + shifted_pos.z * velocity.z)) / scalar;
  const coord_t z = (shifted_pos.z * (vy_squared - vx_squared) - velocity.z * (shifted_pos.y * velocity.y + shifted_pos.x * velocity.x)) / scalar;
  glNormal3f(-x, -y, -z);
#endif

  Particle::draw(usec);

  glDisable(GL_LIGHTING);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE);
}

bool BagParticle::idle(const Uint64 delta_t)
{
  if (effect->recall)
    return false;

  const Uint64 age = get_time() - born;
  const interval_t float_time = delta_t / 1000000.0;
  if (age > 220000)
  {
    const alpha_t alpha_scalar = math_cache.powf_05_close(float_time * 6.0);
    alpha *= alpha_scalar;

    if (alpha < 0.02)
      return false;
  }
  
  return true;
}

GLuint BagParticle::get_texture(const Uint16 res_index)
{
  return base->TexFlare.get_texture(res_index);
}

BagEffect::BagEffect(EyeCandy* _base, bool* _dead, Vec3* _pos, const bool _picked_up, const Uint16 _LOD)
{
  if (EC_DEBUG)
    std::cout << "BagEffect (" << this << ") created." << std::endl;
  base = _base;
  dead = _dead;
  pos = _pos;
  picked_up = _picked_up;
  effect_center = *pos;
  if (!picked_up)
    effect_center.y += 0.2;
  LOD = _LOD;
  desired_LOD = _LOD;
  mover = new GravityMover(this, &effect_center, 2e10);
  spawner = new HollowSphereSpawner(0.3);

  const coord_t size = 20 / LOD;
  for (int i = 0; i < LOD * (30 + 30 * (int)picked_up); i++)
  {
    Vec3 coords = spawner->get_new_coords();
    Vec3 velocity = coords / 10.0;
    coords += effect_center;
    Particle* p = new BagParticle(this, mover, coords, velocity, size);
    if (!base->push_back_particle(p))
      break;
  }
}

BagEffect::~BagEffect()
{
  delete mover;
  delete spawner;
  if (EC_DEBUG)
    std::cout << "BagEffect (" << this << ") destroyed." << std::endl;
}

bool BagEffect::idle(const Uint64 usec)
{
  if (particles.size() == 0)
    return false;
    
  effect_center.x = pos->x;
  if (picked_up)
    effect_center.y += usec / 1000000.0;
  else
    effect_center.y -= usec / 1000000.0;
  effect_center.z = pos->z;
  
  return true;
}

///////////////////////////////////////////////////////////////////////////////

};

#endif	// #ifdef SFX
