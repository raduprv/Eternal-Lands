
// I N C L U D E S ////////////////////////////////////////////////////////////

#include "eye_candy.h"
#include "math_cache.h"

#include "effect_ongoing.h"

namespace ec
{

// C L A S S   F U N C T I O N S //////////////////////////////////////////////

OngoingParticle::OngoingParticle(Effect* _effect, ParticleMover* _mover, const Vec3 _pos, const Vec3 _velocity, const color_t hue_adjust, const color_t saturation_adjust, const coord_t _size, const alpha_t _alpha, color_t hue, color_t saturation, color_t value, Texture* _texture, const Uint16 _LOD, const OngoingEffect::OngoingType _type) : Particle(_effect, _mover, _pos, _velocity)
{
  type = _type;
  hue += hue_adjust;
  if (hue > 1.0)
    hue -= 1.0;
  saturation *= saturation_adjust;
  if (saturation > 1.0)
    saturation = 1.0;
  hsv_to_rgb(hue, saturation, value, color[0], color[1], color[2]);
  if (type == OngoingEffect::OG_HARVEST) {
    color[0] = hue;
	color[1] = saturation;
	color[2] = value;
  }
  texture = _texture;
  size = _size;
  alpha = _alpha;
  velocity /= size;
  flare_max = 1.0;
  flare_exp = 0.1;
  flare_frequency = 50.0;
  LOD = _LOD;
  state = 0;
  angle = 0.0f;
}

OngoingParticle::OngoingParticle(Effect* _effect, ParticleMover* _mover, const Vec3 _pos, const Vec3 _velocity, const color_t hue_adjust, const color_t saturation_adjust, const coord_t _size, const alpha_t _alpha, color_t hue, color_t saturation, color_t value, Texture* _texture, const Uint16 _LOD, const OngoingEffect::OngoingType _type, const angle_t _angle) : Particle(_effect, _mover, _pos, _velocity)
{
  type = _type;
  hue += hue_adjust;
  if (hue > 1.0)
    hue -= 1.0;
  saturation *= saturation_adjust;
  if (saturation > 1.0)
    saturation = 1.0;
  hsv_to_rgb(hue, saturation, value, color[0], color[1], color[2]);
  if (type == OngoingEffect::OG_HARVEST) {
    color[0] = hue;
	color[1] = saturation;
	color[2] = value;
  }
  texture = _texture;
  size = _size;
  alpha = _alpha;
  velocity /= size;
  flare_max = 1.0;
  flare_exp = 0.1;
  flare_frequency = 50.0;
  LOD = _LOD;
  state = 0;
  angle = _angle;
}

bool OngoingParticle::idle(const Uint64 delta_t)
{
  const interval_t float_time = delta_t / 1000000.0;
  const Uint64 age = get_time() - born;
  switch(type)
  {
    case OngoingEffect::OG_MAGIC_PROTECTION:
    {
      const alpha_t scalar = 1.0 - math_cache.powf_0_1_rough_close(randfloat(), float_time * 1.0);
      alpha -= scalar;
      if (alpha < 0.02)
        return false;
      break;
    }
    case OngoingEffect::OG_SHIELD:
    {
      const alpha_t scalar = 1.0 - math_cache.powf_0_1_rough_close(randfloat(), float_time * 1.0);
      alpha -= scalar;
      if (alpha < 0.02)
        return false;
      break;
    }
    case OngoingEffect::OG_MAGIC_IMMUNITY:
    {
      const alpha_t scalar = 1.0 - math_cache.powf_0_1_rough_close(randfloat(), float_time * 0.5);
      alpha -= scalar;
      if (alpha < 0.02)
        return false;
      break;
    }
    case OngoingEffect::OG_POISON:
    {
      const alpha_t scalar = 1.0 - math_cache.powf_0_1_rough_close(randfloat(), float_time * 0.5);
      alpha -= scalar;
      if (alpha < 0.02)
        return false;
      break;
    }
    case OngoingEffect::OG_HARVEST:
    {
      const float age_f = (float)(age)/1000000;
	  pos.x = ((OngoingEffect*)effect)->pos->x + cos(angle + M_PI * age_f) * age_f / exp(age_f * 2.5);
	  pos.z = ((OngoingEffect*)effect)->pos->z + sin(angle + M_PI * age_f) * age_f / exp(age_f * 2.5);
	  pos.y = ((OngoingEffect*)effect)->pos->y - 0.0625 + pow(age_f, 2.0) * 0.25;
      const alpha_t scalar = 1.0 - math_cache.powf_0_1_rough_close(randfloat(), float_time * 0.5);
      alpha -= scalar * 0.25;
      if (alpha < 0.01)
        return false;
      size -= scalar * 0.0625;
      break;
    }
  }
   
  return true;
}

void OngoingParticle::draw(const Uint64 usec)
{
  if ((type == OngoingEffect::OG_POISON) && (state == 1))
  {
    glEnable(GL_LIGHTING);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    Vec3 normal;
    normal.randomize();
    normal.normalize();
    glNormal3f(normal.x, normal.y, normal.z);
  }
  Particle::draw(usec);

  if ((type == OngoingEffect::OG_POISON) && (state == 1))
  {
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    glDisable(GL_LIGHTING);
  }
}

GLuint OngoingParticle::get_texture(const Uint16 res_index)
{
  return texture->get_texture(res_index);
}

OngoingEffect::OngoingEffect(EyeCandy* _base, bool* _dead, Vec3* _pos, const color_t _hue_adjust, const color_t _saturation_adjust, const OngoingType _type, const Uint16 _LOD, const float _strength)
{
  if (EC_DEBUG)
    std::cout << "OngoingEffect (" << this << ") created." << std::endl;
  base = _base;
  dead = _dead;
  pos = _pos;
  hue_adjust = _hue_adjust;
  saturation_adjust = _saturation_adjust;
  effect_center = *pos;
  //effect_center.y += 0.5; // don't! it's linked to a bone position now
  type = _type;
  LOD = base->last_forced_LOD;
  desired_LOD = _LOD;
  spawner = NULL;
  bounds = NULL;
  mover = NULL;
  strength = _strength;
  
  switch(type)
  {
    case OG_MAGIC_PROTECTION:
    {
      spawner = new HollowEllipsoidSpawner(Vec3(0.45, 0.9, 0.45));
      mover = new ParticleMover(this);
      break;
    }
    case OG_SHIELD:
    {
      spawner = new HollowDiscSpawner(0.6);
      mover = new SpiralMover(this, &effect_center, 3.0, 2.9);
      break;
    }
    case OG_MAGIC_IMMUNITY:
    {
      spawner = new HollowEllipsoidSpawner(Vec3(0.6, 1.1, 0.6));
      mover = new ParticleMover(this);
      break;
    }
    case OG_POISON:
    {
      spawner = new HollowDiscSpawner(0.25);
      mover = new SpiralMover(this, &effect_center, -0.9, 0.8);
      break;
    }
    case OG_HARVEST:
    {
      spawner = new FilledSphereSpawner(0.05);
      mover = new ParticleMover(this);
      break;
    }
  }
}

OngoingEffect::~OngoingEffect()
{
  if (spawner)
    delete spawner;
  if (mover)
    delete mover;
  if (EC_DEBUG)
    std::cout << "OngoingEffect (" << this << ") destroyed." << std::endl;
}

bool OngoingEffect::idle(const Uint64 usec)
{
  if ((recall) && (particles.size() == 0))
    return false;
    
  if (recall)
    return true;
  
  effect_center = *pos;
  //effect_center.y += 0.5;
    
  const interval_t float_time = usec / 1000000.0;
  switch(type)
  {
    case OG_MAGIC_PROTECTION:
    {
      while (math_cache.powf_0_1_rough_close(randfloat(), float_time * 6.0 * LOD * strength) < 0.5)
      {
        const Vec3 coords = spawner->get_new_coords() + effect_center;
        Vec3 velocity = coords / 12.0;
        Particle * p = new OngoingParticle(this, mover, coords, velocity, hue_adjust, saturation_adjust, 0.5, 1.0, 0.93, 0.72, 0.7, &(base->TexShimmer), LOD, type);
        if (!base->push_back_particle(p))
          break;
      }
      break;
    }
    case OG_SHIELD:
    {
      while (math_cache.powf_0_1_rough_close(randfloat(), float_time * 12.0 * LOD * strength) < 0.75)
      {
        Vec3 coords = spawner->get_new_coords();
        Vec3 velocity(0.0, 0.0, 0.0);
        coords += effect_center;
        coords.y = -0.25 + randcoord(1.5);
        Particle * p = new OngoingParticle(this, mover, coords, velocity, hue_adjust, saturation_adjust, 0.5, 1.0, 0.55, 0.05, 0.9, &(base->TexShimmer), LOD, type);
        if (!base->push_back_particle(p))
          break;
      }
      break;
    }
    case OG_MAGIC_IMMUNITY:
    {
      while (math_cache.powf_0_1_rough_close(randfloat(), float_time * 6.0 * LOD * strength) < 0.5)
      {
        const Vec3 coords = spawner->get_new_coords() + effect_center;
        Vec3 velocity = coords / 18.0;
        Particle * p = new OngoingParticle(this, mover, coords, velocity, hue_adjust, saturation_adjust, 0.6, 1.0, randcolor(1.0), randcolor(1.0), randcolor(1.0), &(base->TexVoid), LOD, type);
        if (!base->push_back_particle(p))
          break;
      }
      break;
    }
    case OG_POISON:	//The odd one out.  ;)
    {
      while (math_cache.powf_0_1_rough_close(randfloat(), float_time * 4.0 * LOD * strength) < 0.5)
      {
        Vec3 coords = spawner->get_new_coords();
        Vec3 velocity;
        velocity.randomize(0.13);
        velocity.y += 0.08;
        coords += effect_center;
        coords.y = 0.3 + randcoord(0.8);
        Particle* p;
        if (randfloat() < 0.4)
        {
          p = new OngoingParticle(this, mover, coords, velocity, hue_adjust, saturation_adjust, 1.15, 0.5, 0.27 + randcolor(0.06), 0.6 + randcolor(0.15), 0.5 + randcolor(0.3), &(base->TexVoid), LOD, type);
          p->state = 1;
        }
        else
        {
          p = new OngoingParticle(this, mover, coords, velocity, hue_adjust, saturation_adjust, 0.65, 1.0, randcolor(0.5), 0.33 + randcolor(0.67), 0.2 + randcolor(0.1), &(base->TexWater), LOD, type);
          p->state = 0;
        }
        if (!base->push_back_particle(p))
          break;
      }
      break;
    }
    case OG_HARVEST:
	{
      while (math_cache.powf_0_1_rough_close(randfloat(), float_time * 2.0 * LOD * strength) < 0.6)
      {
        const Vec3 coords = spawner->get_new_coords() + effect_center;
        const Vec3 velocity = Vec3(0.0, 0.0, 0.0);
        Particle* p;
        if (randfloat() < 0.5)
        {
          p = new OngoingParticle(this, mover, coords, velocity * 0.95, hue_adjust, saturation_adjust, 0.5 + randcolor(0.75), 1.0, 0.75 + randcolor(0.25), 0.5 + randcolor(0.1), randcolor(0.1), &(base->TexFlare), LOD, type, randfloat(2.0 * M_PI));
        }
        else
        {
          p = new OngoingParticle(this, mover, coords, velocity * 1.05, hue_adjust, saturation_adjust, 0.75 + randcolor(0.5), 1.0, 0.75 + randcolor(0.25), 0.5 + randcolor(0.1), randcolor(0.1), &(base->TexShimmer), LOD, type, randfloat(2.0 * M_PI));
        }
        if (!base->push_back_particle(p))
          break;
        if (randfloat() < 0.5)
        {
          p = new OngoingParticle(this, mover, coords, velocity * 0.9, hue_adjust, saturation_adjust, 0.25 + randcolor(), 1.0, 0.75 + randcolor(0.25), 0.5 + randcolor(0.1), randcolor(0.1), &(base->TexVoid), LOD, type, randfloat(2.0 * M_PI));
        }
        else
        {
          p = new OngoingParticle(this, mover, coords, velocity * 1.1, hue_adjust, saturation_adjust, 0.5 + randcolor(0.5), 1.0, 0.75 + randcolor(0.25), 0.5 + randcolor(0.1), randcolor(0.1), &(base->TexTwinflare), LOD, type, randfloat(2.0 * M_PI));
        }
        if (!base->push_back_particle(p))
          break;
      }
      break;
    }
  }
  
  return true;
}

///////////////////////////////////////////////////////////////////////////////

};

