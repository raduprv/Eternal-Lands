
// I N C L U D E S ////////////////////////////////////////////////////////////

#include "eye_candy.h"
#include "math_cache.h"

#include "effect_firefly.h"

namespace ec
{

extern MathCache_Lorange math_cache;

// C L A S S   F U N C T I O N S //////////////////////////////////////////////

FireflyParticle::FireflyParticle(Effect* _effect, ParticleMover* _mover, const Vec3 _pos, const Vec3 _velocity, const coord_t _min_height, const coord_t _max_height) : Particle(_effect, _mover, _pos, _velocity)
{
  color[0] = 1.0;
  color[1] = 1.0;
  color[2] = 0.7;
  size = 0.35;
  alpha = 1.0;
  flare_max = 8.0;
  flare_exp = 0.2;
  flare_frequency = 0.13;
  min_height = _min_height;
  max_height = _max_height;
}

bool FireflyParticle::idle(const u_int64_t delta_t)
{
  if (effect->recall)
    return false;
    
  Vec3 velocity_shift;
  velocity_shift.randomize();
  velocity_shift.y /= 3;
  velocity_shift.normalize(0.00005 * fastsqrt(delta_t));
  velocity += velocity_shift;
  const coord_t magnitude = velocity.magnitude();
  if (magnitude > 0.15)
    velocity /= (magnitude / 0.15);
    
  if (fabs(velocity.y) > 0.1)
    velocity.y *= math_cache.powf_05_close(delta_t / 300000.0);

  if (pos.y < min_height)
    velocity.y += delta_t / 1000000.0;

  if (pos.y > max_height)
    velocity.y -= delta_t / 2000000.0;
    
  return true;
}

GLuint FireflyParticle::get_texture(const u_int16_t res_index)
{
  return base->TexVoid.get_texture(res_index);
}

FireflyEffect::FireflyEffect(EyeCandy* _base, bool* _dead, Vec3* _pos, const std::vector<ec::Obstruction*> _obstructions, const float _density, const std::vector<PolarCoordElement> bounding_range)
{
  if (EC_DEBUG)
    std::cout << "FireflyEffect (" << this << ") created." << std::endl;
  base = _base;
  dead = _dead;
  pos = _pos;
  center = *pos;
  obstructions = _obstructions;
  mover = new PolarCoordsBoundingMover(this, center, bounding_range, 1.0);
  spawner = new FilledPolarCoordsSpawner(bounding_range);
  const int count = (int)(spawner->get_area() * _density * 0.15);

  for (int i = 0; i < count; i++)
  {
    const Vec3 coords = spawner->get_new_coords() + center + Vec3(0.0, 0.1 + randcoord(1.0), 0.0);
    Vec3 velocity;
    velocity.randomize(0.2);
    velocity.y /= 3;
    Particle* p = new FireflyParticle(this, mover, coords, velocity, center.y + 0.1, center.y + 1.0);
    if (!base->push_back_particle(p))
      break;
  }
}

FireflyEffect::~FireflyEffect()
{
  delete mover;
  delete spawner;
  if (EC_DEBUG)
    std::cout << "FireflyEffect (" << this << ") destroyed." << std::endl;
}

bool FireflyEffect::idle(const u_int64_t usec)
{
  if (particles.size() == 0)
    return false;

  return true;
}

///////////////////////////////////////////////////////////////////////////////

};
