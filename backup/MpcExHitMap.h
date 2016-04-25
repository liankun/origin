#ifndef __MPCEXHITMAP_H__
#define __MPCEXHITMAP_H__

/* class MpcExHitMap
 * Use the globle coordinate of 
 * nx,ny as the key to access
 * hit
 * Liankun Zou@08/05/2015
 *
 */

#include <Map>
#include "MpcExRawHit.h"
#include "TMpcExHit.h"

class MpcExHitMap{
  typedef typename std::map<unsigned int,TMpcExHit*> container;
  typedef typename container::const_iterator const_iterator;
  
  //construct
  MpcExHitMap(const MpcExRawHit* raw)

  //destructors
  virtual ~MpcExHitMap();

  
}

#endif/*__MPCEXHITMAP_H__*/
