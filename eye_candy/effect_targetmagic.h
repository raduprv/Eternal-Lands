
#ifndef EFFECT_TARGETMAGIC_H
#define EFFECT_TARGETMAGIC_H

// I N C L U D E S ////////////////////////////////////////////////////////////

#include "eye_candy.h"

namespace ec
{

// C L A S S E S //////////////////////////////////////////////////////////////

class TargetMagicEffect : public Effect
{
public: 
  enum TargetMagicType
  {
    REMOTE_HEAL,
    POISON,
    TELEPORT_TO_RANGE,
    HARM,
    LIFE_DRAIN,
    HEAL_SUMMONED,
    SMITE_SUMMONED,
    DRAIN_MANA
  };

  TargetMagicEffect(EyeCandy* _base, bool* _dead, Vec3* _pos, Vec3* _target, const TargetMagicType _type, const std::vector<ec::Obstruction*>& _obstructions, const u_int16_t _LOD);
  TargetMagicEffect(EyeCandy* _base, bool* _dead, Vec3* _pos, const std::vector<Vec3*> _targets, const TargetMagicType _type, const std::vector<ec::Obstruction*>& _obstructions, const u_int16_t _LOD);
  ~TargetMagicEffect(); 
  
  void initialize(EyeCandy* _base, bool* _dead, Vec3* _pos, const std::vector<Vec3*> _targets, const TargetMagicType _type, const std::vector<ec::Obstruction*>& _obstructions, const u_int16_t _LOD);
  
  virtual EffectEnum get_type() { return EC_TARGETMAGIC; };
  bool idle(const u_int64_t usec);
  void draw(const u_int64_t usec);
  static u_int64_t get_max_end_time() { return 6000000; };
  virtual u_int64_t get_expire_time() { return 6000000 + born; };

  ParticleSpawner* spawner;
  ParticleMover* mover;
  ParticleSpawner* spawner2;
  ParticleMover* mover2;
  std::vector<Vec3*> targets;
  std::vector<Vec3> effect_centers;
  Vec3 target;
  u_int16_t effect_count;
  TargetMagicType type;
  std::vector<Shape*> capless_cylinders;
  float* target_alpha;
};

class TargetMagicEffect2 : public Effect
{
public:
  TargetMagicEffect2(EyeCandy* _base, TargetMagicEffect* _effect, Vec3* _pos, const TargetMagicEffect::TargetMagicType _type, ParticleSpawner* _spawner, ParticleMover* _mover, float* _target_alpha, u_int16_t _effect_id, const u_int16_t _LOD);
  ~TargetMagicEffect2(); 
  
  virtual EffectEnum get_type() { return EC_TARGETMAGIC; };
  bool idle(const u_int64_t usec);
  virtual u_int64_t get_max_end_time() { return 6000000; };

  TargetMagicEffect* effect;
  ParticleSpawner* spawner;
  ParticleMover* mover;
  Vec3 center;
  Vec3 gravity_center;
  u_int16_t LOD;
  TargetMagicEffect::TargetMagicType type;
  std::vector<Shape*> capless_cylinders;
  u_int16_t effect_id;
  float* target_alpha;
};

class TargetMagicParticle : public Particle
{
public:
  TargetMagicParticle(Effect* _effect, ParticleMover* _mover, const Vec3 _pos, const Vec3 _velocity, const coord_t _size, const alpha_t _alpha, const color_t red, const color_t green, const color_t blue, Texture* _texture, const u_int16_t _LOD, const TargetMagicEffect::TargetMagicType _type, ParticleSpawner* _spawner2, ParticleMover* _mover2, Vec3* _target, u_int16_t _effect_id, u_int16_t _state);
  ~TargetMagicParticle() {}
  
  virtual bool idle(const u_int64_t delta_t);
  void draw(const u_int64_t usec);
  virtual GLuint get_texture(const u_int16_t res_index);
  virtual light_t estimate_light_level() const { return 0.002; };
  
  Texture* texture;
  u_int16_t LOD;
  TargetMagicEffect::TargetMagicType type;
  ParticleSpawner* spawner2;
  ParticleMover* mover2;
  Vec3* target;
  u_int16_t effect_id;
};


///////////////////////////////////////////////////////////////////////////////

}	// End namespace ec

#endif	// defined EFFECT_TARGETMAGIC_H
