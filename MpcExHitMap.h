#ifndef __MPCEXHITMAP_H__
#define __MPCEXHITMAP_H__

/* class MpcExHitMap
 * Use the globle coordinate of 
 * nx,ny as the key to access
 * hit
 * Liankun Zou@08/05/2015
 *
 */

#include <map>

class MpcExRawHit;
class TMpcExHit;
class TMpcExHitContainer;

class MpcExHitMap{
  public:
  typedef std::map<unsigned int,TMpcExHit*> container;
  typedef container::const_iterator const_iterator;
  typedef container::iterator iterator;
  
  //construct
  MpcExHitMap(const MpcExRawHit* raw);

  MpcExHitMap(const TMpcExHitContainer* hits);

  MpcExHitMap();

  unsigned int get_index(const TMpcExHit*) const;
  unsigned int get_index(unsigned int,unsigned int,unsigned int, unsigned int) const;
  TMpcExHit* get_hit(unsigned int) const;

  //destructors
  virtual ~MpcExHitMap();

  const_iterator get_begin() const;
  const_iterator get_end() const;

  const_iterator get_layer_first(unsigned int,unsigned int) const;

  unsigned int get_nx(const TMpcExHit*) const;
  unsigned int get_ny(const TMpcExHit*) const;

  void remove(unsigned int index);

  void clear();


  private:
    const MpcExRawHit* _raw_hits;
    const TMpcExHitContainer* _calib_hits;


    container _hit_map;

};

#endif/*__MPCEXHITMAP_H__*/
