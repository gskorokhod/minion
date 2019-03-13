/*
 * Minion http://minion.sourceforge.net
 * Copyright (C) 2006-09
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301,
 * USA.
 */

#ifndef _TRIES_H_INCLUDE_15243
#define _TRIES_H_INCLUDE_15243

#include <numeric>

#include <algorithm>
#include <cassert>
#include <vector>

using namespace std;

struct TupleComparator {
  const SysInt significantIndex;
  const SysInt arity;

  TupleComparator(DomainInt i, DomainInt a)
      : significantIndex(checked_cast<SysInt>(i)), arity(checked_cast<SysInt>(a)) {}

  // returns tuple1 <= tuple2 under our ordering.
  bool operator()(const vector<DomainInt>& tuple1, const vector<DomainInt>& tuple2) {
    if(tuple1[significantIndex] != tuple2[significantIndex])
      return tuple1[significantIndex] < tuple2[significantIndex];
    for(SysInt tupleIndex = 0; tupleIndex < arity; ++tupleIndex) {
      if(tuple1[tupleIndex] != tuple2[tupleIndex])
        return tuple1[tupleIndex] < tuple2[tupleIndex];
    }
    return false;
  }
};

struct TrieObj {
  DomainInt val;
  TrieObj* offsetPtr;
};

struct TupleTrie {
  static const SysInt max_arity = 100;
  const SysInt arity;
  const SysInt sigIndex;
  TupleList* tuplelist;
  // A temporary tuple to store the most recently found
  // complete assignment.
  DomainInt current_tuple[max_arity];

  vector<vector<DomainInt>> tuples_vector;

  SysInt map_depth(DomainInt depth) {
    if(depth == 0)
      return sigIndex;
    if(depth <= sigIndex)
      return checked_cast<SysInt>(depth - 1);
    return checked_cast<SysInt>(depth);
  }

  DomainInt tuples(DomainInt num, DomainInt depth) {
    return tuples_vector[checked_cast<SysInt>(num)][map_depth(depth)];
  }

  TupleTrie(DomainInt _significantIndex, TupleList* tuplelist)
      : arity(checked_cast<SysInt>(tuplelist->tupleSize())),
        sigIndex(checked_cast<SysInt>(_significantIndex)) {
    // TODO : Fix this hard limit.
    D_ASSERT(arity < 100);
    trie_data = NULL;
    tuples_vector.resize(checked_cast<SysInt>(tuplelist->size()));

    // Need a copy so we can sort it and such things.
    for(SysInt i = 0; i < tuplelist->size(); ++i)
      tuples_vector[i] = tuplelist->getVector(i);

    std::stable_sort(tuples_vector.begin(), tuples_vector.end(), TupleComparator(sigIndex, arity));
    if(tuplelist->size() > 0) {
      buildTrie(0, tuplelist->size());
      buildFinalTrie();
    }
  }

  struct EarlyTrieObj {
    DomainInt val;
    DomainInt depth;
    DomainInt offsetPtr;
  };

  vector<EarlyTrieObj> initial_trie;

  TrieObj* trie_data; // This is the actual trie used during search.

  void buildFinalTrie() {
    const SysInt size = checked_cast<SysInt>(initial_trie.size());
    trie_data = new TrieObj[size];
    for(SysInt i = 0; i < size; ++i) {
      trie_data[i].val = initial_trie[i].val;
      if(initial_trie[i].offsetPtr == -1)
        trie_data[i].offsetPtr = NULL;
      else
        trie_data[i].offsetPtr = trie_data + checked_cast<SysInt>(initial_trie[i].offsetPtr);
    }

    // Little C++ trick to remove all the memory used by the initial_trie
    // vector.
    vector<EarlyTrieObj> v;
    initial_trie.swap(v);
  }

  void print_trie() {
    for(UnsignedSysInt i = 0; i < initial_trie.size(); ++i) {
      printf("%ld,%ld,%ld\n", checked_cast<long>(initial_trie[i].depth),
             checked_cast<long>(initial_trie[i].val),
             checked_cast<long>(initial_trie[i].offsetPtr));
    }
  }

  void buildTrie(const DomainInt start_pos, const DomainInt end_pos, DomainInt depth = 0) {
    const bool last_stage = (depth == arity - 1);
    if(depth == arity)
      return;

    assert(start_pos <= end_pos);
    DomainInt values = get_distinctValues(start_pos, end_pos, depth);

    SysInt start_section = initial_trie.size();
    // Make space for this list of values.
    // '+1' is for end marker.
    initial_trie.resize(checked_cast<SysInt>(initial_trie.size() + values + 1));

    DomainInt currentVal = tuples(start_pos, depth);
    DomainInt currentStart = start_pos;
    // I trust this not to overflow, and can't be bothered with all the required
    // casts
    SysInt num_ofVal = 0;

    for(DomainInt i = start_pos; i < end_pos; ++i) {
      if(currentVal != tuples(i, depth)) {
        initial_trie[start_section + num_ofVal].val = currentVal;
        initial_trie[start_section + num_ofVal].depth = depth;
        if(last_stage) {
          initial_trie[start_section + num_ofVal].offsetPtr = -1;
        } else {
          initial_trie[start_section + num_ofVal].offsetPtr = initial_trie.size();
          buildTrie(currentStart, i, depth + 1);
        }
        currentVal = tuples(i, depth);
        currentStart = i;
        num_ofVal++;
      }
    }

    // Also have to cover last stretch of values.
    initial_trie[start_section + num_ofVal].val = currentVal;
    initial_trie[start_section + num_ofVal].depth = depth;
    if(last_stage)
      initial_trie[start_section + num_ofVal].offsetPtr = -1;
    else
      initial_trie[start_section + num_ofVal].offsetPtr = initial_trie.size();

    buildTrie(currentStart, end_pos, depth + 1);

    assert(num_ofVal + 1 == values);
    SysInt checkValues = checked_cast<SysInt>(values);
    initial_trie[start_section + checkValues].val = DomainInt_Max;
    initial_trie[start_section + checkValues].depth = depth;
    initial_trie[start_section + checkValues].offsetPtr = -1;
  }

  // Find how many values there are for index 'depth' between tuples
  // start_pos and end_pos.
  DomainInt get_distinctValues(const DomainInt start_pos, const DomainInt end_pos,
                                DomainInt depth) {
    DomainInt currentVal = tuples(start_pos, depth);
    DomainInt foundValues = 1;
    for(DomainInt i = start_pos; i < end_pos; ++i) {
      if(currentVal != tuples(i, depth)) {
        currentVal = tuples(i, depth);
        ++foundValues;
      }
    }
    return foundValues;
  }

  // Starting from the start of an array of TrieObjs, find the
  // values which is findVal
  TrieObj* get_nextPtr(TrieObj* obj, DomainInt findVal) {
    while(obj->val < findVal)
      ++obj;
    if(obj->val == findVal)
      return obj;
    else
      return NULL;
  }

  template <typename VarArray>
  bool search_trie(const VarArray& _vars, TrieObj** obj_list, DomainInt depth_in) {
    const SysInt depth = checked_cast<SysInt>(depth_in);
    CON_INFO_ADDONE(SearchTrie);
    VarArray& vars = const_cast<VarArray&>(_vars);
    if(depth == arity)
      return true;

    obj_list[depth] = obj_list[depth - 1]->offsetPtr;
    while(obj_list[depth]->val != DomainInt_Max) {
      if(vars[map_depth(depth)].inDomain(obj_list[depth]->val)) {
        if(search_trie(_vars, obj_list, depth + 1))
          return true;
      }
      obj_list[depth]++;
    }
    return false;
  }

  // Variant for the lightweight table constraint.
  template <typename VarArray>
  bool search_trie_nostate(DomainInt domainVal, const VarArray& _vars) {
    MAKE_STACK_BOX(obj_list, TrieObj*, _vars.size());
    if(trie_data == NULL)
      return false;
    TrieObj* firstPtr = get_nextPtr(trie_data, domainVal);
    if(firstPtr == NULL)
      return false;
    obj_list.resize(_vars.size());
    obj_list[0] = firstPtr;

    return search_trie_nostate_internal(_vars, obj_list, 1);
  }

  // Same as search_trie
  template <typename VarArray>
  bool search_trie_nostate_internal(const VarArray& _vars, box<TrieObj*> obj_list,
                                    DomainInt depth_in) {
    const SysInt depth = checked_cast<SysInt>(depth_in);
    CON_INFO_ADDONE(SearchTrie);
    VarArray& vars = const_cast<VarArray&>(_vars);
    if(depth == arity)
      return true;

    obj_list[depth] = obj_list[depth - 1]->offsetPtr;
    while(obj_list[depth]->val != DomainInt_Max) {
      if(vars[map_depth(depth)].inDomain(obj_list[depth]->val)) {
        if(search_trie_nostate_internal(_vars, obj_list, depth + 1))
          return true;
      }
      obj_list[depth]++;
    }
    return false;
  }

  // search_trie_negative searches for a tuple which is not in the trie.
  // For the negative table constraint.
  // WARNING does not fill in the value at position map_depth(0) in returnTuple
  template <typename VarArray>
  bool search_trie_negative(const VarArray& _vars, TrieObj** obj_list, DomainInt depth_in,
                            DomainInt* returnTuple) {
    const SysInt depth = checked_cast<SysInt>(depth_in);
    CON_INFO_ADDONE(SearchTrie);
    VarArray& vars = const_cast<VarArray&>(_vars);
    if(depth == arity)
      return false;

    obj_list[depth] = obj_list[depth - 1]->offsetPtr;

    SysInt dep = map_depth(depth);
    for(DomainInt i = vars[dep].min(); i <= vars[dep].max(); ++i) {
      if(vars[dep].inDomain(i)) {
        while(obj_list[depth]->val < i)
          obj_list[depth]++;
        returnTuple[dep] = i;
        if(obj_list[depth]->val > i) { // includes case where val is maxint.
          // if the value is in the domain but not in the trie, we are nearly
          // finished.
          // Just need to fill in the rest of returnTuple.
          // Is there any need to search from the previous position?? Yes. But
          // not doing so yet.
          for(SysInt depth2 = checked_cast<SysInt>(depth) + 1; depth2 < arity; depth2++)
            returnTuple[map_depth(depth2)] = vars[map_depth(depth2)].min();
          return true;
        } else {
          if(search_trie_negative(_vars, obj_list, depth + 1, returnTuple))
            return true;
        }
      }
    }
    return false;
  }

  void reconstructTuple(DomainInt* array, TrieObj** obj_list) {
    // D_ASSERT(check == obj_list[arity - 1] - obj_list[0]);
    for(SysInt i = 0; i < checked_cast<SysInt>(arity); ++i)
      array[map_depth(i)] = obj_list[i]->val;
  }

  template <typename VarArray>
  bool loopSearch_trie(const VarArray& _vars, TrieObj** obj_list, DomainInt depth_in) {
    const SysInt depth = checked_cast<SysInt>(depth_in);
    CON_INFO_ADDONE(LoopSearchTrie);
    VarArray& vars = const_cast<VarArray&>(_vars);
    if(depth == arity)
      return true;

    if(obj_list[depth]->val == DomainInt_Max)
      return search_trie(_vars, obj_list, depth);

    if(vars[map_depth(depth)].inDomain(obj_list[depth]->val)) {
      if(loopSearch_trie(_vars, obj_list, depth + 1))
        return true;
    }

    TrieObj* initial_pos = obj_list[depth];

    obj_list[depth]++;
    while(obj_list[depth]->val != DomainInt_Max) {
      if(vars[map_depth(depth)].inDomain(obj_list[depth]->val)) {
        if(search_trie(_vars, obj_list, depth + 1))
          return true;
      }
      obj_list[depth]++;
    }

    obj_list[depth] = obj_list[depth - 1]->offsetPtr;

    while(obj_list[depth] != initial_pos) {
      if(vars[map_depth(depth)].inDomain(obj_list[depth]->val)) {
        if(search_trie(_vars, obj_list, depth + 1))
          return true;
      }
      obj_list[depth]++;
    }
    return false;
  }

  // Find support for domain value i. This will be the value used by
  // the first variable.
  template <typename VarArray>
  DomainInt nextSupportingTuple(DomainInt domainVal, const VarArray& _vars, TrieObj** obj_list) {
    if(trie_data == NULL)
      return -1;

    VarArray& vars = const_cast<VarArray&>(_vars);

    if(obj_list[0] == NULL) {
      TrieObj* firstPtr = get_nextPtr(trie_data, domainVal);
      if(firstPtr == NULL)
        return -1;

      obj_list[0] = firstPtr;
      if(search_trie(vars, obj_list, 1))
        return obj_list[arity - 1] - obj_list[0];
      else
        return -1;
    } else {
      if(loopSearch_trie(vars, obj_list, 1))
        return obj_list[arity - 1] - obj_list[0];
      else
        return -1;
      /*    D_ASSERT(obj_list[0] == get_nextPtr(trie_data, domainVal));
            SysInt OK_depth = 1;
            while(OK_depth < arity &&
         vars[map_depth(OK_depth)].inDomain(obj_list[OK_depth]->val))
              OK_depth++;
            if(search_trie(vars,obj_list, OK_depth))
              return obj_list[arity-1] - obj_list[0];
            else
            {
              if(search_trie(vars, obj_list, 1))
                return obj_list[arity-1] - obj_list[0];
              else
                return -1;
            }*/
    }
  }

  // Find support for domain value i. This will be the value used by
  // the first variable.
  template <typename VarArray>
  DomainInt nextSupportingTupleNegative(DomainInt domainVal, const VarArray& _vars,
                                        TrieObj** obj_list, DomainInt* recycTuple) {
    VarArray& vars = const_cast<VarArray&>(_vars);

    // Starts from scratch each time
    TrieObj* firstPtr = get_nextPtr(trie_data, domainVal);

    recycTuple[map_depth(0)] = domainVal;
    if(firstPtr == NULL) {
      // Hang on a minute. How do we ever get here? Should only be at root node.
      for(DomainInt depth2 = 1; depth2 < arity; ++depth2)
        recycTuple[map_depth(depth2)] = vars[map_depth(depth2)].min();
      return 0;
    }

    obj_list[0] = firstPtr;

    bool flag = search_trie_negative(vars, obj_list, 1, recycTuple);

    if(!flag)
      return -1;
    return 0;
  }
};

// template<typename VarArray>
class TupleTrieArray {
public:
  TupleList* tuplelist;

  // Would make this const if I could, but it cannot be setup until after
  // 'finalise_tuples'
  SysInt arity;
  TupleTrie* tupleTries;

  TupleTrie& getTrie(DomainInt varIndex) {
    return tupleTries[checked_cast<SysInt>(varIndex)];
  };

  /* %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
    Constructor
    %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% */

  TupleTrieArray(TupleList* _tuplelist) : tuplelist(_tuplelist) {
    tuplelist->finalise_tuples();
    arity = checked_cast<SysInt>(tuplelist->tupleSize());
    vector<DomainInt> domSize(arity);
    vector<DomainInt> offset(arity);

    // create one trie for each element of scope.
    tupleTries = (TupleTrie*)checked_malloc(sizeof(TupleTrie) * arity);
    if(!tupleTries) {
      outputFatalError("Out of memory in TupleTrie construction");
    }
    for(SysInt varIndex = 0; varIndex < arity; varIndex++)
      new(tupleTries + varIndex) TupleTrie(varIndex, tuplelist);
  }
};

#endif
