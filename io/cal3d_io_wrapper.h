/**
 * @file
 * @ingroup wrapper
 * @brief file i/o functions with support for zip and gzip files
 */
#ifndef UUID_ff56ac18_4412_418c_b07e_b67a8634ec2c
#define UUID_ff56ac18_4412_418c_b07e_b67a8634ec2c

#include "../platform.h"
#include "../cal3d_wrapper.h"

#ifdef __cplusplus
extern "C"
{
#endif

extern struct CalCoreAnimation *CalLoader_ELLoadCoreAnimation(struct CalLoader *self, const char *strFilename);
extern struct CalCoreMaterial *CalLoader_ELLoadCoreMaterial(struct CalLoader *self, const char *strFilename);
extern struct CalCoreMesh *CalLoader_ELLoadCoreMesh(struct CalLoader *self, const char *strFilename);
extern struct CalCoreSkeleton *CalLoader_ELLoadCoreSkeleton(struct CalLoader *self, const char *strFilename);
extern int CalCoreModel_ELLoadCoreAnimation(struct CalCoreModel *self, const char *strFilename, float scale);
extern int CalCoreModel_ELLoadCoreMaterial(struct CalCoreModel *self, const char *strFilename);
extern int CalCoreModel_ELLoadCoreMesh(struct CalCoreModel *self, const char *strFilename);
extern enum CalBoolean CalCoreModel_ELLoadCoreSkeleton(struct CalCoreModel *self, const char *strFilename);
extern void set_invert_v_coord();

#ifdef __cplusplus
}
#endif


#endif	/* UUID_ff56ac18_4412_418c_b07e_b67a8634ec2c */

