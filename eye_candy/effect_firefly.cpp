
// I N C L U D E S ////////////////////////////////////////////////////////////

#include "eye_candy.h"
#include "math_cache.h"

#include "effect_firefly.h"

namespace ec
{

// C L A S S   F U N C T I O N S //////////////////////////////////////////////

FireflyParticle::FireflyParticle(Effect* _effect, ParticleMover* _mover, const Vec3 _pos, const Vec3 _velocity, const color_t hue_adjust, const color_t saturation_adjust, const coord_t _size, const coord_t _min_height, const coord_t _max_height) : Particle(_effect, _mover, _pos, _velocity)
{
  color_t hue, saturation, value;
  hue = 0.17;
  saturation = 0.3;
  value = 1.0;
  hue += hue_adjust;
  if (hue > 1.0)
    hue -= 1.0;
  saturation *= saturation_adjust;
  if (saturation > 1.0)
    saturation = 1.0;
  hsv_to_rgb(hue, saturation, value, color[0], color[1], color[2]);
  size = _size;
  alpha = 1.0;
  flare_max = 8.0;
  flare_exp = 0.2;
  flare_frequency = 0.13;
  min_height = _min_height;
  max_height = _max_height;
}

bool FireflyParticle::idle(const Uint64 delta_t)
{
  if (effect->recall)
    return false;
    
  if ((pos - base->center).magnitude_squared() > MAX_DRAW_DISTANCE_SQUARED)
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

GLuint FireflyParticle::get_texture(const Uint16 res_index)
{
  return base->TexVoid.get_texture(res_index);
}

FireflyEffect::FireflyEffect(EyeCandy* _base, bool* _dead, Vec3* _pos, std::vector<ec::Obstruction*>* _obstructions, const color_t _hue_adjust, const color_t _saturation_adjust, const float _density, const float _size, BoundingRange* bounding_range)
{
  if (EC_DEBUG)
    std::cout << "FireflyEffect (" << this << ") created." << std::endl;
  base = _base;
  dead = _dead;
  pos = _pos;
  center = *pos;
  obstructions = _obstructions;
  hue_adjust = _hue_adjust;
  saturation_adjust = _saturation_adjust;
  size = _size * 0.35;
  bounds = bounding_range;
  mover = new BoundingMover(this, center, bounding_range, 1.0);
  spawner = new NoncheckingFilledBoundingSpawner(bounding_range);
//  firefly_count = (int)(spawner->get_area() * _density * 0.15);
  firefly_count = (int)(MAX_DRAW_DISTANCE_SQUARED * PI * _density * 0.25);

  for (int i = 0; i < firefly_count; i++)
  {
    Vec3 coords = spawner->get_new_coords();
    if (coords.x == -32768.0)
      continue;
    coords += center + Vec3(0.0, 0.1 + randcoord(1.0), 0.0);
    Vec3 velocity;
    velocity.randomize(0.2);
    velocity.y /= 3;
    Particle* p = new FireflyParticle(this, mover, coords, velocity, hue_adjust, saturation_adjust, size, center.y + 0.1, center.y + 1.0);
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

bool FireflyEffect::idle(const Uint64 usec)
{
  if (particles.size() == 0)
    return false;
  
  if (recall)
    return true;
  
  center = *pos;

  for (int i = firefly_count - (int)particles.size(); i >= 0; i--)
  {
    Vec3 coords = spawner->get_new_coords();
    if (coords.x == -32768.0)
      continue;
    coords += center + Vec3(0.0, 0.1 + randcoord(1.0), 0.0);
    Vec3 velocity;
    velocity.randomize(0.2);
    velocity.y /= 3;
    Particle* p = new FireflyParticle(this, mover, coords, velocity, hue_adjust, saturation_adjust, size, center.y + 0.1, center.y + 1.0);
    if (!base->push_back_particle(p))
      break;
  }

  return true;
}

///////////////////////////////////////////////////////////////////////////////

};

