#ifndef	__FRUSTUM_H
#define	__FRUSTUM_H

void NormalizePlane(float frustum[8][4], int side);
void CalculateFrustum();
int PointInFrustum( float x, float y, float z );
int SphereInFrustum( float x, float y, float z, float radius );
int CubeInFrustum(float x, float y, float z, float x_size, float y_size, float z_size);
int check_tile_in_frustrum(float x,float y);

#endif	//__FRUSTUM_H
