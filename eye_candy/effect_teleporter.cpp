#ifdef EYE_CANDY

// I N C L U D E S ////////////////////////////////////////////////////////////

#include "eye_candy.h"
#include "math_cache.h"

#include "effect_teleporter.h"

namespace ec
{

// C L A S S   F U N C T I O N S //////////////////////////////////////////////

TeleporterParticle::TeleporterParticle(Effect* _effect, ParticleMover* _mover, const Vec3 _pos, const Vec3 _velocity, const coord_t size_scalar) : Particle(_effect, _mover, _pos, _velocity)
{
  color[0] = 0.8 + randcolor(0.2);
  color[1] = 0.8 + randcolor(0.2);
  color[2] = 0.8 + randcolor(0.2);
  size = size_scalar * (0.5 + 1.5 * randcoord());
  alpha = 5.0 / size;
  if (alpha > 1.0)
    alpha = 1.0;
  velocity /= size;
  flare_max = 1.6;
  flare_exp = 0.2;
  flare_frequency = 2.0;
}

bool TeleporterParticle::idle(const Uint64 delta_t)
{
  if (effect->recall)
    return false;

  if (alpha < 0.03)
    return false;

  const alpha_t scalar = math_cache.powf_05_close((float)delta_t / 800000);
  alpha *= scalar;
  
  return true;
}

GLuint TeleporterParticle::get_texture(const Uint16 res_index)
{
  return base->TexShimmer.get_texture(res_index);
}

TeleporterEffect::TeleporterEffect(EyeCandy* _base, bool* _dead, Vec3* _pos, const Uint16 _LOD)
{
  if (EC_DEBUG)
    std::cout << "TeleporterEffect (" << this << ") created." << std::endl;
  base = _base;
  dead = _dead;
  pos = _pos;
  LOD = _LOD;
  desired_LOD = _LOD;
  sqrt_LOD = fastsqrt(LOD);
  size_scalar = 15 / (LOD + 5);
  mover = new ParticleMover(this);
  spawner = new FilledDiscSpawner(0.2);

/*
  for (int i = 0; i < LOD * 10; i++)
  {
    const Vec3 coords = spawner->get_new_coords() + *pos + Vec3(0.0, randcoord() * randcoord() * 8.0 * sqrt_LOD, 0.0);
    Vec3 velocity(0.0, randcoord(0.1), 0.0);
    velocity.randomize(0.2);
    Particle* p = new TeleporterParticle(this, mover, coords, velocity, size_scalar);
   if (!base->push_back_particle(p))
      break;
  }
*/
  
//  const float radius = 0.5 * pow(2, 0.18) / 1.5;
  const float radius = 0.377628;
  for (int i = 0; i < LOD * 4; i++)
  {
    const percent_t percent = ((coord_t)i + 1) / (LOD * 4);
    capless_cylinders.push_back(new CaplessCylinder(*pos, *pos + Vec3(0.0, 10.0 / percent, 0.0), Vec3(1.0, 1.0, 1.0), (0.1 + (1.0 - percent) * 0.05) / (LOD + 2), radius * percent, (int)(25 * (percent + 0.2))));
  }
}

TeleporterEffect::~TeleporterEffect()
{
  delete mover;
  delete spawner;
  for (size_t i = 0; i < capless_cylinders.size(); i++)
    delete capless_cylinders[i];
  capless_cylinders.clear();
  if (EC_DEBUG)
    std::cout << "TeleporterEffect (" << this << ") destroyed." << std::endl;
}

bool TeleporterEffect::idle(const Uint64 usec)
{
  if ((recall) && (particles.size() == 0))
    return false;

  if (recall)
    return true;

  while (((int)particles.size() < LOD * 50) && (math_cache.powf_0_1_rough_close(randfloat(), (float)usec / 100000 * LOD) < 0.5))
  {
    const Vec3 coords = spawner->get_new_coords() + *pos + Vec3(0.0, randcoord() * randcoord() * 8.0 * sqrt_LOD, 0.0);
    Vec3 velocity;
    velocity.randomize(0.2);
    Particle* p = new TeleporterParticle(this, mover, coords, velocity, size_scalar);
    if (!base->push_back_particle(p))
      break;
  }
  
  for (int i = 0; i < (int)targets.size(); )
  {
    std::vector< std::pair<float *, Uint64> >::iterator iter = targets.begin() + i;
    Uint64 age = get_time() - iter->second;
    if (age < 500000)
    {
      *(iter->first) = 1.0 - (age / 500000.0);;
      i++;
    }
    else if (age < 1000000)
    {
      *(iter->first) = (age - 500000.0) / 500000.0;;
      i++;
    }
    else
    {
      *(iter->first) = 1.0;
      targets.erase(iter);
    }
  }

  return true;
}

void TeleporterEffect::draw(const Uint64 usec)
{
  for (std::vector<Shape*>::iterator iter = capless_cylinders.begin(); iter != capless_cylinders.end(); iter++)
    (*iter)->draw();
}

void TeleporterEffect::request_LOD(const Uint16 _LOD)
{
  if (_LOD <= desired_LOD)
    LOD = _LOD;
  else
    LOD = desired_LOD;
  
  sqrt_LOD = fastsqrt(LOD);
  size_scalar = 15 / (LOD + 5);
  
  for (size_t i = 0; i < capless_cylinders.size(); i++)
    delete capless_cylinders[i];
  capless_cylinders.clear();

//  const float radius = 0.5 * pow(2, 0.18) / 1.5;
  const float radius = 0.377628;
  for (int i = 0; i < LOD * 4; i++)
  {
    const percent_t percent = ((coord_t)i + 1) / (LOD * 4);
    capless_cylinders.push_back(new CaplessCylinder(*pos, *pos + Vec3(0.0, 10.0 / percent, 0.0), Vec3(1.0, 1.0, 1.0), (0.1 + (1.0 - percent) * 0.05) / (LOD + 2), radius * percent, (int)(25 * (percent + 0.2))));
  }
}

void TeleporterEffect::add_actor_alpha_pointer(float* ptr)
{
  targets.push_back( std::pair<float*, Uint64>(ptr, get_time()));
}

///////////////////////////////////////////////////////////////////////////////

};

#endif	// #ifdef EYE_CANDY
