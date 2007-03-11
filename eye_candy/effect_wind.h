#ifdef SFX

#ifndef EFFECT_WIND_H
#define EFFECT_WIND_H

// I N C L U D E S ////////////////////////////////////////////////////////////

#include "eye_candy.h"

namespace ec
{

// C L A S S E S //////////////////////////////////////////////////////////////


class WindEffect : public Effect
{
public: 
  enum WindType
  {
    LEAVES,
    FLOWER_PETALS,
    SNOW
  };
  
  struct WindNeighbor
  {
    WindEffect* neighbor;
    angle_t start_angle;
    angle_t end_angle;
  };

  WindEffect(EyeCandy* _base, bool* _dead, Vec3* _pos, const std::vector<ec::Obstruction*> _obstructions, const float _density, const std::vector<PolarCoordElement> _bounding_range, const WindType _type, const Vec3 _prevailing_wind);
  ~WindEffect(); 
  
  void set_pass_off(std::vector<WindEffect*> pass_off_to); // Required!
  void set_pass_off(std::vector<Effect*> pass_off_to); // Required!
  
  EffectEnum get_type() { return EC_WIND; };
  bool idle(const Uint64 usec);
  coord_t get_radius(const angle_t angle) const;
  virtual void request_LOD(const Uint16 _LOD)
  {
    if (_LOD <= desired_LOD)
      LOD = _LOD;
    else
      LOD = desired_LOD;
    count = LOD * max_LOD1_count;
  };

  ParticleMover* mover;
  FilledPolarCoordsSpawner* spawner;
  WindType type;
  Vec3 center;
  Vec3 prevailing_wind;
  coord_t max_adjust;
  Vec3 overall_wind_adjust;
  Vec3 overall_wind;
  int max_LOD1_count;
  int count;
  std::vector<WindNeighbor> neighbors;	// Where to pass particles off to.
  std::vector<PolarCoordElement> bounding_range;
};

class WindParticle : public Particle
{
public:
  WindParticle(Effect* _effect, ParticleMover* _mover, const Vec3 _pos, const Vec3 _velocity, const coord_t _min_height, const coord_t _max_height, const WindEffect::WindType _type);
  ~WindParticle() {}
  
  virtual bool idle(const Uint64 delta_t);
  virtual GLuint get_texture(const Uint16 res_index);
  void draw(const Uint64 usec);
  virtual light_t estimate_light_level() const { return 0.0; };	// We don't want the particle system lights to be used on the pos, since it will assumedly already have one.
  virtual light_t get_light_level() { return 0.0; };
  Vec3 get_wind_vec() const;
  
  coord_t min_height;
  coord_t max_height;
  WindEffect::WindType type;
  Uint8 subtype;
  Vec3 rotation_axes[3];
  percent_t axis_weights[3];
  Quaternion quaternion;
};

///////////////////////////////////////////////////////////////////////////////

}	// End namespace ec

#endif	// defined EFFECT_WIND_H

#endif	// #ifdef SFX
