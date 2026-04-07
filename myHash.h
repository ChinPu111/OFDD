/****************************************************************************
  FileName     [ myHash.h ]
  PackageName  [ util ]
  Synopsis     [ Define Hash and Cache ADT ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2009-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#ifndef MY_HASH_H
#define MY_HASH_H

#include <vector>

using namespace std;

template <class HashKey, class HashData>
class Hash
{
typedef pair<HashKey, HashData> HashNode;

public:
   Hash() : _numBuckets(0), _buckets(0) {}
   Hash(size_t b) : _numBuckets(0), _buckets(0) { init(b); }
   ~Hash() { reset(); }

   class iterator
   {
      friend class Hash<HashKey, HashData>;

   public:
      iterator(Hash<HashKey, HashData>* h = 0, size_t b = 0, size_t bn = 0)
      : _hash(h), _bId(b), _bnId(bn) {}
      iterator(const iterator& i)
      : _hash(i._hash), _bId(i._bId), _bnId(i._bnId) {}
      ~iterator() {}

      const HashNode& operator * () const { return (*_hash)[_bId][_bnId]; }
      HashNode& operator * () { return (*_hash)[_bId][_bnId]; }
      iterator& operator ++ () {
         if (_hash == 0) return (*this);
         if (_bId >= _hash->_numBuckets) return (*this);
         if (++_bnId >= (*_hash)[_bId].size()) {
            while ((++_bId < _hash->_numBuckets) && (*_hash)[_bId].empty());
            _bnId = 0;
         }
         return (*this);
      }
      iterator& operator -- () {
         if (_hash == 0) return (*this);
         if (_bnId == 0) {
            if (_bId == 0) return (*this);
            while ((*_hash)[--_bId].empty())
               if (_bId == 0) return (*this);
            _bnId = (*_hash)[_bId].size() - 1;
         }
         else
            --_bnId;
         return (*this);
      }
      iterator operator ++ (int) { iterator li=(*this); ++(*this); return li; }
      iterator operator -- (int) { iterator li=(*this); --(*this); return li; }

      iterator& operator = (const iterator& i) {
         _hash = i._hash; _bId = i._bId; _bnId = i._bnId; return (*this); }

      bool operator != (const iterator& i) const { return !(*this == i); }
      bool operator == (const iterator& i) const {
         return (_hash == i._hash && _bId == i._bId && _bnId == i._bnId); }

   private:
      Hash<HashKey, HashData>*   _hash;
      size_t                     _bId;
      size_t                     _bnId;
   };

   iterator begin() const {
      size_t i = 0;
      if (_buckets == 0) return end();
      while (_buckets[i].empty()) ++i;
      return iterator(const_cast<Hash<HashKey, HashData>*>(this), i, 0);
   }
   iterator end() const {
      return iterator(const_cast<Hash<HashKey, HashData>*>(this),
             _numBuckets, 0);
   }
   bool empty() const {
      for (size_t i = 0; i < _numBuckets; ++i)
         if (_buckets[i].size() != 0) return false;
      return true;
   }
   size_t size() const {
      size_t s = 0;
      for (size_t i = 0; i < _numBuckets; ++i) s += _buckets[i].size();
      return s;
   }
   size_t numBuckets() const { return _numBuckets; }

   vector<HashNode>& operator [] (size_t i) { return _buckets[i]; }
   const vector<HashNode>& operator [](size_t i) const { return _buckets[i]; }

   void init(size_t b) {
      reset(); _numBuckets = b; _buckets = new vector<HashNode>[b]; }
   void reset() {
      _numBuckets = 0;
      if (_buckets) { delete [] _buckets; _buckets = 0; }
   }

   bool check(const HashKey& k, HashData& n) const {
      size_t b = bucketNum(k);
      for (size_t i = 0, bn = _buckets[b].size(); i < bn; ++i)
         if (_buckets[b][i].first == k) {
            n = _buckets[b][i].second;
            return true;
         }
      return false;
   }

   bool insert(const HashKey& k, const HashData& d) {
      size_t b = bucketNum(k);
      for (size_t i = 0, bn = _buckets[b].size(); i < bn; ++i)
         if (_buckets[b][i].first == k)
            return false;
      _buckets[b].push_back(HashNode(k, d));
      return true;
   }

   bool replaceInsert(const HashKey& k, const HashData& d) {
      size_t b = bucketNum(k);
      for (size_t i = 0, bn = _buckets[b].size(); i < bn; ++i)
         if (_buckets[b][i].first == k) {
            _buckets[b][i].second = d;
            return false;
         }
      _buckets[b].push_back(HashNode(k, d));
      return true;
   }

   void forceInsert(const HashKey& k, const HashData& d) {
      _buckets[bucketNum(k)].push_back(HashNode(k, d)); }

private:
   size_t                   _numBuckets;
   vector<HashNode>*        _buckets;

   size_t bucketNum(const HashKey& k) const {
      return (k() % _numBuckets); }

};


template <class CacheKey, class CacheData>
class Cache
{
typedef pair<CacheKey, CacheData> CacheNode;

public:
   Cache() : _size(0), _cache(0) {}
   Cache(size_t s) : _size(0), _cache(0) { init(s); }
   ~Cache() { reset(); }

   void init(size_t s) { reset(); _size = s; _cache = new CacheNode[s]; }
   void reset() { _size = 0; if (_cache) { delete [] _cache; _cache = 0; } }

   size_t size() const { return _size; }

   CacheNode& operator [] (size_t i) { return _cache[i]; }
   const CacheNode& operator [](size_t i) const { return _cache[i]; }

   bool read(const CacheKey& k, CacheData& d) const {
      size_t i = k() % _size;
      if (k == _cache[i].first) {
         d = _cache[i].second;
         return true;
      }
      return false;
   }
   void write(const CacheKey& k, const CacheData& d) {
      size_t i = k() % _size;
      _cache[i].first = k;
      _cache[i].second = d;
   }

private:
   size_t         _size;
   CacheNode*     _cache;
};


#endif // MY_HASH_H
