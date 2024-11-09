#pragma once

#include "Bounds.h"
#include "Intersection.h"

namespace math
{

// bool boxInFrustum( frustum3 const & fru, bound3 const & box )
// {
    // // check box outside/inside of frustum
    // for( int i=0; i<6; i++ )
    // {
    //     int out = 0;
    //     out += ((dot( fru.mPlane[i], vec4(box.mMinX, box.mMinY, box.mMinZ, 1.0f) ) < 0.0 )?1:0);
    //     out += ((dot( fru.mPlane[i], vec4(box.mMaxX, box.mMinY, box.mMinZ, 1.0f) ) < 0.0 )?1:0);
    //     out += ((dot( fru.mPlane[i], vec4(box.mMinX, box.mMaxY, box.mMinZ, 1.0f) ) < 0.0 )?1:0);
    //     out += ((dot( fru.mPlane[i], vec4(box.mMaxX, box.mMaxY, box.mMinZ, 1.0f) ) < 0.0 )?1:0);
    //     out += ((dot( fru.mPlane[i], vec4(box.mMinX, box.mMinY, box.mMaxZ, 1.0f) ) < 0.0 )?1:0);
    //     out += ((dot( fru.mPlane[i], vec4(box.mMaxX, box.mMinY, box.mMaxZ, 1.0f) ) < 0.0 )?1:0);
    //     out += ((dot( fru.mPlane[i], vec4(box.mMinX, box.mMaxY, box.mMaxZ, 1.0f) ) < 0.0 )?1:0);
    //     out += ((dot( fru.mPlane[i], vec4(box.mMaxX, box.mMaxY, box.mMaxZ, 1.0f) ) < 0.0 )?1:0);
    //     if( out==8 ) return false;
    // }
    //
    // // check frustum outside/inside box
    // int out;
    // out=0; for( int i=0; i<8; i++ ) out += ((fru.mPoints[i].x > box.mMaxX)?1:0); if( out==8 ) return false;
    // out=0; for( int i=0; i<8; i++ ) out += ((fru.mPoints[i].x < box.mMinX)?1:0); if( out==8 ) return false;
    // out=0; for( int i=0; i<8; i++ ) out += ((fru.mPoints[i].y > box.mMaxY)?1:0); if( out==8 ) return false;
    // out=0; for( int i=0; i<8; i++ ) out += ((fru.mPoints[i].y < box.mMinY)?1:0); if( out==8 ) return false;
    // out=0; for( int i=0; i<8; i++ ) out += ((fru.mPoints[i].z > box.mMaxZ)?1:0); if( out==8 ) return false;
    // out=0; for( int i=0; i<8; i++ ) out += ((fru.mPoints[i].z < box.mMinZ)?1:0); if( out==8 ) return false;
    //
    // return true;
// }


void getDirectionTriangleIntersection(const glm::vec3 &origin, const glm::vec3 &direction,
    const glm::vec3 &p0, const glm::vec3 &p1, const glm::vec3 &p2,
    SimpleIntersection &out_intersection);

// Direction must be normalized
void getDirectionTriangleIntersectionUnsafe(const glm::vec3 &origin, const glm::vec3 &dir_n,
    const glm::vec3 &p0, const glm::vec3 &p1, const glm::vec3 &p2,
    SimpleIntersection &out_intersection);


void getDirectionBoundBoxIntersection(const glm::vec3 &origin, const glm::vec3 &direction,
    const BoundBox &bb, SimpleIntersection &out_intersection);

// Direction must be normalized
void getDirectionBoundBoxIntersectionUnsafe(const glm::vec3 &origin, const glm::vec3 &dir_n,
    const BoundBox &bb, SimpleIntersection &out_intersection);


} // namespace math