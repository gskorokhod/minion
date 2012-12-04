// LIST BASED CODE WONT BE WORKING

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
* Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

#include <algorithm>
#include "boost/tuple/tuple_comparison.hpp"
#include "../constraints/constraint_checkassign.h"

// Default will be List.   
// If any special case is defined list will be switched off
// If two options given compile errors are expected to result.

#define UseElementShort false
#define UseElementLong false
#define UseLexLeqShort false
#define UseLexLeqLong false
#define UseSquarePackingShort false
#define UseSquarePackingLong false
#define UseList true
#define UseNDOneList false
#define SupportsGacNoCopyList true

#ifdef SUPPORTSGACELEMENT
#undef UseElementShort
#undef UseList
#undef UseNDOneList
#undef SupportsGacNoCopyList
#define UseElementShort true
#define UseList false
#define UseNDOneList false
#define SupportsGacNoCopyList false
#endif

#ifdef SUPPORTSGACELEMENTLONG
#undef UseElementLong
#undef UseList
#undef UseNDOneList
#undef SupportsGacNoCopyList
#define UseElementLong true
#define UseList false
#define UseNDOneList false
#define SupportsGacNoCopyList false
#endif

#ifdef SUPPORTSGACLEX
#undef UseLexLeqShort
#undef UseList
#undef UseNDOneList
#undef SupportsGacNoCopyList
#define UseLexLeqShort true
#define UseList false
#define UseNDOneList false
#define SupportsGacNoCopyList false
#endif

#ifdef SUPPORTSGACLEXLONG
#undef UseLexLeqLong
#undef UseList
#undef UseNDOneList
#undef SupportsGacNoCopyList
#define UseLexLeqLong true
#define UseList false
#define UseNDOneList false
#define SupportsGacNoCopyList false
#endif

#ifdef SUPPORTSGACSQUAREPACK
#undef UseSquarePackingShort
#undef UseList
#undef UseNDOneList
#undef SupportsGacNoCopyList
#define UseSquarePackingShort true
#define UseList false
#define UseNDOneList false
#define SupportsGacNoCopyList false
#endif

#ifdef SUPPORTSGACSQUAREPACKLONG
#undef UseSquarePackingLong
#undef UseList
#undef UseNDOneList
#undef SupportsGacNoCopyList
#define UseSquarePackingLong true
#define UseList false
#define UseNDOneList false
#define SupportsGacNoCopyList false
#endif

#ifdef SUPPORTSGACLIST
#undef UseList
#undef UseNDOneList
#undef SupportsGacNoCopyList
#define UseList true
#define UseNDOneList false
#define SupportsGacNoCopyList false
#endif

#ifdef SUPPORTSGACNDLIST
#undef UseList
#undef UseNDOneList
#undef SupportsGacNoCopyList
#define UseList false
#define UseNDOneList true
#define SupportsGacNoCopyList false
#endif

#ifdef SUPPORTSGACLISTNOCOPY
#undef UseList
#undef UseNDOneList
#undef SupportsGacNoCopyList
#define UseList true
#define UseNDOneList false
#define SupportsGacNoCopyList true
#endif

// The algorithm iGAC or short-supports-gac

// Does it place dynamic triggers for the supports.
#define SupportsGACUseDT true

// Switches on the zeroLits array. 
// This flag is a small slowdown on qg-supportsgac-7-9 -findallsols
// 
#define SupportsGACUseZeroVals true

#define CLASSNAME HaggisGAC
template<typename VarArray>
struct HaggisGAC : public AbstractConstraint, Backtrackable
{

    virtual string constraint_name()
    { return "haggisgac"; }

#include "constraint_haggisgac_common.h"

    CONSTRAINT_ARG_LIST2(vars, data);



    struct Support {
        vector<SupportCell> supportCells ;   // Size can't be more than r, but can be less.

        SysInt arity;              // could use vector.size() but don't want to destruct SupportCells when arity decreases
                                // or reconstruct existing ones when it increases.

        Support* nextFree ; // for when Support is in Free List.

        
        Support()
        {
            supportCells.resize(0);
            arity=0;
            nextFree=0;
        }
    };
    
    VarArray vars;

    vector<pair<SysInt, DomainInt> > literalsScratch;   // used instead of per-Support list, as scratch space
    
    SysInt numvals;
    SysInt numlits;
    
    // Counters
    SysInt supports;   // 0 to rd.  
    vector<SysInt> supportsPerVar;

    vector<SysInt> litsWithLostExplicitSupport;
    vector<SysInt> varsWithLostImplicitSupport;
    
    // 2d array (indexed by var then val) of sentinels,
    // at the head of list of supports. 
    // Needs a sentinel at the start so that dlx-style removals work correctly.
    vector<Literal>  literalList;
    vector<SysInt> firstLiteralPerVar;
    
    // For each variable, a vector of values with 0 supports (or had 0 supports
    // when added to the vector).
    #if SupportsGACUseZeroVals
    vector<vector<SysInt> > zeroLits;
    vector<char> inZeroLits;  // is a literal in zeroVals
    #endif
    
    // Partition of variables by number of supports.
    vector<SysInt> varsPerSupport;    // Permutation of the variables
    vector<SysInt> varsPerSupInv;   // Inverse mapping of the above.
    
    vector<SysInt> supportNumPtrs;   // rd+1 indices into varsPerSupport representing the partition
    
    Support* supportFreeList;       // singly-linked list of spare Support objects.
    
    #if UseList
    #if !SupportsGacNoCopyList
    vector<vector<vector<vector<pair<SysInt, DomainInt> > > > > tuple_lists;  // tuple_lists[var][val] is a vector 
    // of short supports for that var, val. Includes any supports that do not contain var at all.
    #else
    vector<vector<vector<vector<pair<SysInt, DomainInt> > * > > > tuple_lists;
    #endif
    #endif
    
    #if UseNDOneList
    vector<vector<tuple<SysInt,SysInt,SysInt> > > tuple_nd_list; // The inner type is var,val,next-different-pos.
    #endif
    
    vector<vector<SysInt> > tuple_list_pos;    // current position in tuple_lists (for each var and val). Wraps around.
    
    ////////////////////////////////////////////////////////////////////////////
    // Ctor
    
    ShortTupleList* data;
    
    HaggisGAC(StateObj* _stateObj, const VarArray& _var_array, ShortTupleList* tuples) : AbstractConstraint(_stateObj), 
    vars(_var_array), supportFreeList(0), data(tuples)
    {
        init();
       
        litsWithLostExplicitSupport.reserve(numlits); // max poss size, not necessarily optimal choice here
    }
    

    ////////////////////////////////////////////////////////////////////////////
    // Backtracking mechanism
    
    struct BTRecord {
        bool is_removal;   // removal or addition was made. 
        Support* sup;
        
        friend std::ostream& operator<<(std::ostream& o, const BTRecord& rec)
        {
            if(rec.sup==0) return o<<"ZeroMarker";
            o<<"BTRecord:"<<rec.is_removal<<",";
            // o<< rec.sup->literals;
            return o;
        }
    };
    
    vector<BTRecord> backtrack_stack;
    
    void mark() {
        struct BTRecord temp = { false, 0 };
        backtrack_stack.push_back(temp);  // marker.
    }
    
    void pop() {
        //cout << "BACKTRACKING:" << endl;
        //cout << backtrack_stack <<endl;
        while(backtrack_stack.back().sup != 0) {
            BTRecord temp=backtrack_stack.back();
            backtrack_stack.pop_back();
            if(temp.is_removal) {
                addSupportInternal(temp.sup);
            }
            else {
                deleteSupportInternal(temp.sup, true);
            }
        }
        
        backtrack_stack.pop_back();  // Pop the marker.
        //cout << "END OF BACKTRACKING." << endl;
    }
    
    ////////////////////////////////////////////////////////////////////////////
    // Add and delete support
    
    // don't need argument?   Just use litlist member?  
    //
    //Support* addSupport(box<pair<SysInt, DomainInt> >* litlist)
    void addSupport()
    {
       Support* newsup = getFreeSupport(); 
       vector<SupportCell>& supCells=newsup->supportCells;
       SysInt oldsize = supCells.size() ;
       SysInt newsize = literalsScratch.size() ;

       newsup->arity = newsize;

       if(newsize > oldsize) { 
               supCells.resize(newsize) ; 
               // make sure pointers to support cell are correct
               // need only be done once as will always point to
               // its own support
               for(SysInt i=oldsize; i < newsize ; i++) { 
                       supCells[i].sup = newsup; 
               }
       }

       for(SysInt i=0; i<newsize ; i++) {
            SysInt var=literalsScratch[i].first;
            DomainInt valoriginal=literalsScratch[i].second;
            DomainInt lit=firstLiteralPerVar[var]+valoriginal-vars[var].getInitialMin();
            supCells[i].literal = checked_cast<SysInt>(lit);
       }
        // now have enough supCells, and sup and literal of each is correct

        addSupportInternal(newsup);
        struct BTRecord temp;
        temp.is_removal=false;
        temp.sup=newsup;
        backtrack_stack.push_back(temp);
        // return newsup;
    }

    // these guys can be void 
    //
    //
    
    // Takes a support which has: 
    //          arity correct
    //          supCells containing at least arity elements
    //          each supCells[i[ in range has 
    //                literal correct
    //                sup correct

    void addSupportInternal(Support* sup_internal)
    {
        // add a new support given literals but not pointers in place


        //cout << "Adding support (internal) :" << litlist_internal << endl;
        //D_ASSERT(litlist_internal.size()>0);  
        //// It should be possible to deal with empty supports, but currently they wil
        // cause a memory leak. 
        
        vector<SupportCell>& supCells=sup_internal->supportCells;

        SysInt litsize = sup_internal->arity;

        for(SysInt i=0; i<litsize; i++) {

            SysInt lit=supCells[i].literal;
            SysInt var=literalList[lit].var;
            
            // Stitch it into the start of literalList.supportCellList
            
            supCells[i].prev = 0;
            supCells[i].next = literalList[lit].supportCellList;  
            if(literalList[lit].supportCellList!=0) {
                literalList[lit].supportCellList->prev = &(supCells[i]);
            }
            else { 
            // Attach trigger if this is the first support containing var,val.
                attach_trigger(var, literalList[lit].val, lit);
            }
            literalList[lit].supportCellList = &(supCells[i]);

            //update counters
            supportsPerVar[var]++;
            
            
            // Update partition
            // swap var to the end of its cell.
            partition_swap(var, varsPerSupport[supportNumPtrs[supportsPerVar[var]]-1]);
            // Move the boundary so var is now in the higher cell.
            supportNumPtrs[supportsPerVar[var]]--;
        }
        supports++;
        
        //printStructures();
        
        // return sup_internal;
    }
    
    void deleteSupport(Support* sup) {
        struct BTRecord temp;
        temp.is_removal=true;
        temp.sup=sup;
        backtrack_stack.push_back(temp);
        
        deleteSupportInternal(sup, false);
    }
    
    void deleteSupportInternal(Support* sup, bool Backtracking) {
        D_ASSERT(sup!=0);
        
        vector<SupportCell>& supCells=sup->supportCells;
        SysInt supArity = sup->arity; 
        //cout << "Removing support (internal) :" << litlist << endl;
        
        // oldIndex is where supportsPerVar = numsupports used to be 
        // Off by 1 error?

        SysInt oldIndex  = supportNumPtrs[supports];
        
        for(SysInt i=0; i<supArity; i++) {

            SupportCell& supCell = supCells[i];
            SysInt lit=supCell.literal;
            SysInt var=literalList[lit].var ;

            // D_ASSERT(prev[var]!=0);
            // decrement counters
            supportsPerVar[var]--;

            if(supCell.prev==0) {       // this was the first support in list

                    literalList[lit].supportCellList = supCell.next; 

                    if(supCell.next==0) { 
                            // We have lost the last support for lit
                            //
            // I believe that each literal can only be marked once here in a call to update_counters.
            // so we should be able to push it onto a list
            //
            // As long as we do not actually call find_new_support.
            // So probably should shove things onto a list and then call find supports later
        // Surely don't need to update lost supports on backtracking in non-backtrack-stable code?
                        if (!Backtracking && supportsPerVar[var] == (supports - 1)) {   // since supports not decremented yet
                                litsWithLostExplicitSupport.push_back(lit);
                        }
                        #if SupportsGACUseZeroVals
                                // Still need to add to zerovals even if above test is true
                                // because we might find a new implicit support, later lose it, and then it will 
                                // be essential that it is in zerovals.  Obviously if an explicit support is 
                                // found then it will later get deleted from zerovals.
                        if(!inZeroLits[lit]) {
                            inZeroLits[lit]=true;
                            zeroLits[var].push_back(lit);
                        }
                        #endif
                    // Remove trigger since this is the last support containing var,val.
                       if(SupportsGACUseDT) { detach_trigger(lit); }
                    }
                    else { 
                            supCell.next->prev=0;
                    }
            }
            else {
                    supCell.prev->next = supCell.next;
                    if(supCell.next!=0){
                            supCell.next->prev = supCell.prev;
                    }
            }
            
            

            // Update partition
            // swap var to the start of its cell.  
            // This plays a crucial role in moving together the vars which previously
            // had 1 less than numsupports and soon will have numsupports.

            partition_swap(var, varsPerSupport[supportNumPtrs[supportsPerVar[var]+1]]);
            //
            // Move the boundary so var is now in the lower cell.
            supportNumPtrs[supportsPerVar[var]+1]++;

            
        }
        supports--;
        
        // For following code it is essential that partition swaps compress 
        // vars together which used to have SupportsPerVar[i] = supports-1 and 
        // now have supportsPerVar[i] = supports (because supports has been decremented
        // 
        //
            //
            // Similarly to the above, each var can only be added to this list once per call to update_counters
            // Because it can only lose its last implicit support once since we are only deleting supports.
            //
        
        // I hope we only need to do this when NOT backtracking, at least for non backtrack-stable version
        // When we backtrack we will add supports which did support it so there is no need to find new supports

//      cout << supportNumPtrs[supports] << " " << oldIndex << endl;
        
        if (!Backtracking) {
                for(SysInt i=supportNumPtrs[supports]; i < oldIndex; i++) { 
                        varsWithLostImplicitSupport.push_back(varsPerSupport[i]);
                }
        } 
        else { 
            // We are Backtracking 
            // Can re-use the support when it is removed by BT. 
            // Stick it on the free list 
            sup->nextFree=supportFreeList;
            supportFreeList=sup;
        }
        // else can't re-use it because a ptr to it is on the BT stack. 
    }

    BOOL hasNoKnownSupport(SysInt var,SysInt lit) {
            //
            // Either implicitly supported or counter is non zero
            // Note that even if we have an explicit support which may be invalid, we can return true
            // i.e. code does not guarantee that it has a valid support, only that it has a support.
            // If we have no valid supports then (if algorithms are right) we will eventually delete
            // the last known valid support and at that time start looking for a new one.

            D_ASSERT(var == literalList[lit].var); 

            return supportsPerVar[var] == supports && (literalList[lit].supportCellList == 0);
    }
    //
    ////////////////////////////////////////////////////////////////////////////
    // 
    void printStructures()
    {
        cout << "PRINTING ALL DATA STRUCTURES" <<endl;
        cout << "supports:" << supports <<endl;
        cout << "supportsPerVar:" << supportsPerVar << endl;
        cout << "partition:" <<endl;
        for(SysInt i=0; i<supportNumPtrs.size()-1; i++) {
            cout << "supports: "<< i<< "  vars: ";
            for(SysInt j=supportNumPtrs[i]; j<supportNumPtrs[i+1]; j++) {
                cout << varsPerSupport[j]<< ", ";
            }
            cout << endl;
            if(supportNumPtrs[i+1]==vars.size()) break;
        }
        #if SupportsGACUseZeroLits
        cout << "zeroLits:" << zeroLits << endl;
        cout << "inZeroLits:" << inZeroLits << endl;
        #endif
        /*
        
        cout << "Supports for each literal:"<<endl;
        for(SysInt var=0; var<vars.size(); var++) {
            cout << "Variable: "<<var<<endl;
            for(DomainInt val=vars[var].getInitialMin(); val<=vars[var].getInitialMax(); val++) {
                cout << "Value: "<<val<<endl;
                Support* sup=supportListPerLit[var][val-vars[var].getInitialMin()].next[var];
                while(sup!=0) {
                    cout << "Support: " << sup->literals << endl;
                    bool contains_varval=false;
                    for(SysInt i=0; i<sup->literals.size(); i++) {
                        if(sup->literals[i].first==var && sup->literals[i].second==val)
                            contains_varval=true;
                    }
                    D_ASSERT(contains_varval);
                    
                    sup=sup->next[var];
                }
            }
        }
        */
    }
    
    #if !SupportsGACUseDT
        virtual triggerCollection setup_internal()
        {
            triggerCollection t;
            SysInt array_size = vars.size();
            for(SysInt i = 0; i < array_size; ++i)
              t.push_back(make_trigger(vars[i], Trigger(this, i), DomainChanged));
            return t;
        }
    #endif
    
    void partition_swap(SysInt xi, SysInt xj)
    {
        if(xi != xj) {
            varsPerSupport[varsPerSupInv[xj]]=xi;
            varsPerSupport[varsPerSupInv[xi]]=xj;
            SysInt temp=varsPerSupInv[xi];
            varsPerSupInv[xi]=varsPerSupInv[xj];
            varsPerSupInv[xj]=temp;
        }
    }

    void findSupportsIncrementalHelper(SysInt var, DomainInt val) { 

            typedef pair<SysInt,DomainInt> temptype;
            // MAKE_STACK_BOX(newsupportbox, temptype, vars.size()); 
            literalsScratch.clear(); 
            // bool foundsupport=findNewSupport(newsupportbox, var, val);
            bool foundsupport=findNewSupport<false>(var, val);
            
            if(!foundsupport) {
                vars[var].removeFromDomain(val);        
                        // note we are not doing this internally, 
                        // i.e. trying to update counters etc. 
                        // So counters won't have changed until we are retriggered on the removal
            }
            else {
                // addSupport(&newsupportbox);
                addSupport();
            }
    }
    
    void findSupportsIncremental()
    {
        // For list of vars which have lost their last implicit support
        // do this ... 
        // but don't need to redo if we have stored that list ahead of time 
        //  ... and we can check if it still has support 
        //
        // For each variable where the number of supports is equal to the total...

        for(SysInt i=litsWithLostExplicitSupport.size()-1; i >= 0; i--) { 
            SysInt lit=litsWithLostExplicitSupport[i];
            SysInt var=literalList[lit].var;
            DomainInt val=literalList[lit].val;
            
            litsWithLostExplicitSupport.pop_back(); // actually probably unnecessary - will get resized to 0 later
            
            if( hasNoKnownSupport(var,lit) &&  vars[var].inDomain(val) ) {
                    findSupportsIncrementalHelper(var,val);
            }
        }

        for(SysInt i = varsWithLostImplicitSupport.size()-1; i >= 0; i--) { 

            SysInt var= varsWithLostImplicitSupport[i];
            varsWithLostImplicitSupport.pop_back(); // actually probably unnecessary - will get resized to 0 later

            if (supportsPerVar[var] == supports) {      // otherwise has found implicit support in the meantime
                    #if !SupportsGACUseZeroVals
                    for(DomainInt val=vars[var].getMin(); val<=vars[var].getMax(); val++) {
                        SysInt lit=firstLiteralPerVar[var]+val-vars[var].getInitialMin();
                    #else
                    for(SysInt j=0; j<zeroLits[var].size() && supportsPerVar[var]==supports; j++) {
                        SysInt lit=zeroLits[var][j];
            if(literalList[lit].supportCellList != 0){
                            // No longer a zero val. remove from vector.
                            zeroLits[var][j]=zeroLits[var][zeroLits[var].size()-1];
                            zeroLits[var].pop_back();
                            inZeroLits[lit]=false;
                            j--;
                            continue;
                        }
                        DomainInt val=literalList[lit].val;
                    #endif

                    #if !SupportsGACUseZeroVals
                        if(vars[var].inDomain(val) && (literalList[lit].supportCellList] == 0)){
                    #else
                        if(vars[var].inDomain(val)) {   // tested literalList  above
                    #endif
                    D_ASSERT(hasNoKnownSupport(var, lit));
                    PROP_INFO_ADDONE(CounterA);
                            findSupportsIncrementalHelper(var,val);
                            // No longer do we remove lit from zerolits in this case if support is found.
                            // However this is correct as it can be removed lazily next time the list is traversed
                            // And we might even get lucky and save this small amount of work.
                        } // } to trick vim bracket matching
                    }    // } to trick vim bracket matching
                }
        }
    }

    inline void updateCounters(SysInt lit) {

        SupportCell* supCellList = literalList[lit].supportCellList ;

        litsWithLostExplicitSupport.resize(0);   
        varsWithLostImplicitSupport.resize(0);

        while(supCellList != 0) {
            SupportCell* next=supCellList->next;
            deleteSupport(supCellList->sup);
            supCellList=next;
        }
    }
    
    
    #if SupportsGACUseDT
        SysInt dynamic_trigger_count() { 
            return literalList.size();
        }
    #endif
    
  inline void attach_trigger(SysInt var, DomainInt val, SysInt lit)
  {
      //P("Attach Trigger: " << i);
      
      DynamicTrigger* dt = dynamic_trigger_start();
      // find the trigger for var, val.
      dt=dt+lit;
      D_ASSERT(!dt->isAttached());
      
      vars[var].addDynamicTrigger(dt, DomainRemoval, val );   //BT_CALL_BACKTRACK
  }
  
  inline void detach_trigger(SysInt lit)
  {
      //P("Detach Triggers");
      
      // D_ASSERT(supportListPerLit[var][val-vars[var].getInitialMin()].next[var] == 0);
      
      DynamicTrigger* dt = dynamic_trigger_start();
      dt=dt+lit;
      releaseTrigger(stateObj, dt);   // BT_CALL_BACKTRACK
  }
    
  virtual void propagate(SysInt prop_var, DomainDelta)
  {
  /* 
   Probably won't work
   */
    cout << "Have given up trying to make this work without dynamic triggers" << endl ;
    /*
     *
    D_ASSERT(prop_var>=0 && prop_var<vars.size());
    // Really needs triggers on each value, or on the supports. 
    
    //printStructures();
    D_ASSERT(!SupportsGACUseDT);  // Should not be here if using dynamic triggers.
    
    for(DomainInt val=vars[prop_var].getInitialMin(); val<=vars[prop_var].getInitialMax(); val++) {
        if(!vars[prop_var].inDomain(val) && supportListPerLit[prop_var][val-vars[prop_var].getInitialMin()].next[prop_var]!=0) {
            updateCounters(prop_var, val);
        }
    }
    
    findSupportsIncremental();
    */
  }
  
    virtual void propagate(DynamicTrigger* dt)
  {
      SysInt lit=dt-dynamic_trigger_start();

    //  cout << "Propagate called: var= " << var << "val = " << val << endl;
      //printStructures();

      updateCounters(lit);
      
      findSupportsIncremental();
  }

    // HERE will need to be changed for backtrack stability, i.e. added even if isAssigned. Or use FL
    
    #define ADDTOASSIGNMENT(var, val) if(keepassigned || !vars[var].isAssigned()) literalsScratch.push_back(make_pair(var,val));
    
    // For full-length support variant:
    #define ADDTOASSIGNMENTFL(var, val) if(keepassigned || !vars[var].isAssigned()) literalsScratch.push_back(make_pair(var,val));
    // #define ADDTOASSIGNMENTFL(var, val) literalsScratch.push_back(make_pair(var,val));
    
    
    // Macro to add either the lower bound or the specified value for a particular variable vartopad
    // Intended to pad out an assignment to a full-length support.
    #define PADOUT(vartopad) if(var==vartopad) { ADDTOASSIGNMENTFL(var, val);}  else { ADDTOASSIGNMENTFL(vartopad, vars[vartopad].getMin());} 
    
    ////////////////////////////////////////////////////////////////////////////
    // Methods for pair-equals. a=b or c=d.
    
    /*
    bool findNewSupport(box<pair<SysInt, DomainInt> >& assignment, SysInt var, DomainInt val) {
        // a=b or c=d
        D_ASSERT(vars[var].inDomain(val));
        D_ASSERT(vars.size()==4);
        SysInt othervar;
        if(var<=1) othervar=1-var;
        else othervar=(var==2? 3: 2);
        
        if(vars[othervar].inDomain(val)) {
            // If can satisfy the equality with var in it
            assignment.push_back(make_pair(var, val));
            assignment.push_back(make_pair(othervar, val));
            return true;
        }
        
        // Otherwise, try to satisfy the other equality.
        if(var<=1) {
            for(SysInt otherval=vars[2].getMin(); otherval<=vars[2].getMax(); otherval++) {
                if(vars[2].inDomain(otherval) && vars[3].inDomain(otherval)) {
                    assignment.push_back(make_pair(2, otherval));
                    assignment.push_back(make_pair(3, otherval));
                    return true;
                }
            }
        }
        else {
            for(SysInt otherval=vars[0].getMin(); otherval<=vars[0].getMax(); otherval++) {
                if(vars[0].inDomain(otherval) && vars[1].inDomain(otherval)) {
                    assignment.push_back(make_pair(0, otherval));
                    assignment.push_back(make_pair(1, otherval));
                    return true;
                }
            }
        }
        return false;
    }
    
    virtual BOOL check_assignment(DomainInt* v, SysInt array_size)
    {
      D_ASSERT(array_size == 4);
      
      if(v[0]==v[1] || v[2]==v[3]) return true;
      return false;
      
    }*/
    
    ////////////////////////////////////////////////////////////////////////////
    // Methods for element

#if UseElementShort
    
    // bool findNewSupport(box<pair<SysInt, DomainInt> >& assignment, SysInt var, DomainInt val) {
    bool findNewSupport(SysInt var, DomainInt val) {
        typedef typename VarArray::value_type VarRef;
        VarRef idxvar=vars[vars.size()-2];
        VarRef resultvar=vars[vars.size()-1];
        D_ASSERT(vars[var].inDomain(val));
        
        if(var<vars.size()-2) {
            // var is in the vector.
            
            for(SysInt i=idxvar.getMin(); i<=idxvar.getMax(); i++) {
                if(idxvar.inDomain(i) && i>=0 && i<vars.size()-2) {
                    for(SysInt j=resultvar.getMin(); j<=resultvar.getMax(); j++) {
                        if(resultvar.inDomain(j) && vars[i].inDomain(j) &&
                            (i!=var || j==val) ) {   // Either the support includes both var, val or neither -- if neither, it will be a support for var,val.
                            ADDTOASSIGNMENT(i,j);
                            ADDTOASSIGNMENT(vars.size()-2, i);
                            ADDTOASSIGNMENT(vars.size()-1, j);
                            return true;
                        }
                    }
                }
            }
        }
        else if(var==vars.size()-2) {
            // It's the index variable.
            if(val<0 || val>=vars.size()-2){
                return false;
            }
            
            for(SysInt i=resultvar.getMin(); i<=resultvar.getMax(); i++) {
                if(resultvar.inDomain(i) && vars[val].inDomain(i)) {
                    ADDTOASSIGNMENT(vars.size()-2, val);
                    ADDTOASSIGNMENT(vars.size()-1, i);
                    ADDTOASSIGNMENT(val, i);
                    return true;
                }
            }
            
        }
        else if(var==vars.size()-1) {
            // The result variable.
            for(SysInt i=0; i<vars.size()-2; i++) {
                if(vars[i].inDomain(val) && idxvar.inDomain(i)) {
                    ADDTOASSIGNMENT(vars.size()-2, i);
                    ADDTOASSIGNMENT(vars.size()-1, val);
                    ADDTOASSIGNMENT(i, val);
                    return true;
                }
            }
        }
        return false;
        
        
    }
    
    virtual BOOL check_assignment(DomainInt* v, SysInt array_size)
    {
        SysInt idx=v[array_size-2];
        if(idx<0 || idx>=array_size-2) return false;
        return v[v[array_size-2]] == v[array_size-1];
    }

#endif
    //
    ////////////////////////////////////////////////////////////////////////////
    // ELEMENT - FULL LENGTH TUPLES VERSION.

#if UseElementLong

    // bool findNewSupport(box<pair<SysInt, DomainInt> >& assignment, SysInt var, DomainInt val) {
    bool findNewSupport(SysInt var, DomainInt val) {
        typedef typename VarArray::value_type VarRef;
        VarRef idxvar=vars[vars.size()-2];
        VarRef resultvar=vars[vars.size()-1];
        D_ASSERT(vars[var].inDomain(val));
        
        if(var<vars.size()-2) {
            // var is in the vector.
            
            for(SysInt i=idxvar.getMin(); i<=idxvar.getMax(); i++) {
                if(idxvar.inDomain(i) && i>=0 && i<vars.size()-2) {
                    for(SysInt j=resultvar.getMin(); j<=resultvar.getMax(); j++) {
                        if(resultvar.inDomain(j) && vars[i].inDomain(j) &&
                            (i!=var || j==val) ) {   // Either the support includes both var, val or neither -- if neither, it will be a support for var,val.
                            ADDTOASSIGNMENTFL(i, j);
                            ADDTOASSIGNMENTFL(vars.size()-2, i);
                            ADDTOASSIGNMENTFL(vars.size()-1, j);
                            for(SysInt k=0; k<vars.size()-2; k++) {
                                if(k!=i) {
                                    if(k==var) { 
                                        ADDTOASSIGNMENTFL(k, val); } 
                                    else { 
                                        ADDTOASSIGNMENTFL(k, vars[k].getMin());} 
                                }
                            }
                            return true;
                        }
                    }
                }
            }
        }
        else if(var==vars.size()-2) {
            // It's the index variable.
            if(val<0 || val>=vars.size()-2){
                return false;
            }
            
            for(SysInt i=resultvar.getMin(); i<=resultvar.getMax(); i++) {
                if(resultvar.inDomain(i) && vars[val].inDomain(i)) {
                    ADDTOASSIGNMENTFL(vars.size()-2, val);
                    ADDTOASSIGNMENTFL(vars.size()-1, i);
                    ADDTOASSIGNMENTFL(val, i);
                    for(SysInt k=0; k<vars.size()-2; k++) {
                        if(k!=val) {ADDTOASSIGNMENTFL(k, vars[k].getMin()); }
                    }
                    return true;
                }
            }
            
        }
        else if(var==vars.size()-1) {
            // The result variable.
            for(SysInt i=0; i<vars.size()-2; i++) {
                if(vars[i].inDomain(val) && idxvar.inDomain(i)) {
                    ADDTOASSIGNMENTFL(vars.size()-2, i);
                    ADDTOASSIGNMENTFL(vars.size()-1, val);
                    ADDTOASSIGNMENTFL(i, val);
                    for(SysInt k=0; k<vars.size()-2; k++) {
                        if(k!=i) { ADDTOASSIGNMENTFL(k, vars[k].getMin()); } 
                    }
                    return true;
                }
            }
        }
        return false;
        
        
    }
    
    virtual BOOL check_assignment(DomainInt* v, SysInt array_size)
    {
        SysInt idx=v[array_size-2];
        if(idx<0 || idx>=array_size-2) return false;
        return v[v[array_size-2]] == v[array_size-1];
    }


#endif 
    
    ////////////////////////////////////////////////////////////////////////////
    // Methods for lexleq
    
#if UseLexLeqShort
    
    // bool findNewSupport(box<pair<SysInt, DomainInt> >& assignment, SysInt var, DomainInt val) {
    bool findNewSupport(SysInt var, DomainInt val) {
        D_ASSERT(vars[var].inDomain(val));
        D_ASSERT(vars.size()%2==0);
        // First part of vars is vector 1.
        SysInt vecsize=vars.size()/2;
        
        for(SysInt i=0; i<vecsize; i++) {
            SysInt j=i+vecsize;
            SysInt jmax=vars[j].getMax();
            SysInt imin=vars[i].getMin();
            
            // CASE 1   It is not possible for the pair to be equal or less.
            if(imin>jmax) {
                return false;
            }
            
            // CASE 2    It is only possible to make the pair equal.
            if(imin==jmax) {
                // check against var, val here.
                if(i==var && imin!=val) {
                    return false;
                }
                if(j==var && jmax!=val) {
                    return false;
                }
                
                ADDTOASSIGNMENT(i, imin);
                ADDTOASSIGNMENT(j, jmax);
                
                // Do not return, continue along the vector.
                continue;
            }
            
            // CASE 3    It is possible make the pair less.
            if(imin<jmax) {
                if(i==var) {
                    if(val==jmax) {
                        ADDTOASSIGNMENT(i, val);
                        ADDTOASSIGNMENT(j, val);
                        continue;
                    }
                    else if(val>jmax) {
                        return false;
                    }
                    else {   //  val<jmax
                        ADDTOASSIGNMENT(var, val);
                        ADDTOASSIGNMENT(j, jmax);
                        return true;
                    }
                }
                
                if(j==var) {
                    if(val==imin) {
                        ADDTOASSIGNMENT(i, val);
                        ADDTOASSIGNMENT(j, val);
                        continue;
                    }
                    else if(val<imin) {
                        return false;
                    }
                    else {   //  val>imin
                        ADDTOASSIGNMENT(var, val);
                        ADDTOASSIGNMENT(i, imin);
                        return true;
                    }
                }
                
                
                // BETTER NOT TO USE min and max here, should watch something in the middle of the domain...
                //SysInt mid=imin + (jmax-imin)/2;
                //if(vars[i].inDomain(mid-1) && vars[j].inDomain(mid)) {
                //    ADDTOASSIGNMENT(i,mid-1);
                //    ADDTOASSIGNMENT(j,mid);
                //}
                //else {
                    ADDTOASSIGNMENT(i,imin);
                    ADDTOASSIGNMENT(j,jmax);
                
                return true;
            }
            
        }
        
        // Got to end of vector without finding a pair that can satisfy
        // the ct. However this is equal....
        return true;
    }
    
    virtual BOOL check_assignment(DomainInt* v, SysInt array_size)
    {
        D_ASSERT(array_size%2==0);
        for(SysInt i=0; i<array_size/2; i++)
        {
            if(v[i]<v[i+array_size/2]) return true;
            if(v[i]>v[i+array_size/2]) return false;
        }
        return true;
    }


#endif 
    
    ////////////////////////////////////////////////////////////////////////////
    //
    //  Lexleq with full-length supports
    //

#if UseLexLeqLong
    
    // bool findNewSupport(box<pair<SysInt, DomainInt> >& assignment, SysInt var, DomainInt val) {
    bool findNewSupport(SysInt var, DomainInt val) {
        D_ASSERT(vars[var].inDomain(val));
        D_ASSERT(vars.size()%2==0);
        // First part of vars is vector 1.
        SysInt vecsize=vars.size()/2;
        
        for(SysInt i=0; i<vecsize; i++) {
            SysInt j=i+vecsize;
            SysInt jmax=vars[j].getMax();
            SysInt imin=vars[i].getMin();
            
            // CASE 1   It is not possible for the pair to be equal or less.
            if(imin>jmax) {
                return false;
            }
            
            // CASE 2    It is only possible to make the pair equal.
            if(imin==jmax) {
                // check against var, val here.
                if(i==var && imin!=val) {
                    return false;
                }
                if(j==var && jmax!=val) {
                    return false;
                }
                
                ADDTOASSIGNMENTFL(i, imin);
                ADDTOASSIGNMENTFL(j, jmax);
                
                // Do not return, continue along the vector.
                continue;
            }
            
            // CASE 3    It is possible make the pair less.
            if(imin<jmax) {
                if(i==var) {
                    if(val==jmax) {
                        ADDTOASSIGNMENTFL(i, val);
                        ADDTOASSIGNMENTFL(j, val);
                        continue;
                    }
                    else if(val>jmax) {
                        return false;
                    }
                    else {   //  val<jmax
                        ADDTOASSIGNMENTFL(var, val);
                        ADDTOASSIGNMENTFL(j, jmax);
                        for(SysInt k=i+1; k<vecsize; k++) {
                            PADOUT(k);
                            PADOUT(k+vecsize);
                        }
                        
                        return true;
                    }
                }
                
                if(j==var) {
                    if(val==imin) {
                        ADDTOASSIGNMENTFL(i, val);
                        ADDTOASSIGNMENTFL(j, val);
                        continue;
                    }
                    else if(val<imin) {
                        return false;
                    }
                    else {   //  val>imin
                        ADDTOASSIGNMENTFL(var, val);
                        ADDTOASSIGNMENTFL(i, imin);
                        for(SysInt k=i+1; k<vecsize; k++) {
                            PADOUT(k);
                            PADOUT(k+vecsize);
                        }
                        
                        return true;
                    }
                }
                
                ADDTOASSIGNMENTFL(i,imin);
                ADDTOASSIGNMENTFL(j,jmax);
                for(SysInt k=i+1; k<vecsize; k++) {
                    PADOUT(k);
                    PADOUT(k+vecsize);
                }
                
                return true;
            }
            
        }
        
        // Got to end of vector without finding a pair that can satisfy
        // the ct. However this is equal....
        return true;
    }
    
    virtual BOOL check_assignment(DomainInt* v, SysInt array_size)
    {
        D_ASSERT(array_size%2==0);
        for(SysInt i=0; i<array_size/2; i++)
        {
            if(v[i]<v[i+array_size/2]) return true;
            if(v[i]>v[i+array_size/2]) return false;
        }
        return true;
    }
    
#endif
    

#if UseList && SupportsGacNoCopyList

    ////////////////////////////////////////////////////////////////////////////
    //
    //  Table of short supports passed in.
    
    //bool findNewSupport(box<pair<SysInt, DomainInt> >& assignment, SysInt var, DomainInt val) {
    template<bool keepassigned>
    bool findNewSupport(SysInt var, DomainInt val) {
        D_ASSERT(tuple_lists.size()==vars.size());
        const SysInt val_offset = checked_cast<SysInt>(val-vars[var].getInitialMin());
        const vector<vector<pair<SysInt, DomainInt> > * >& tuplist=tuple_lists[var][val_offset]; 
        
        SysInt listsize=tuplist.size();
        for(SysInt i=tuple_list_pos[var][val_offset]; i<listsize; i++) {
            vector<pair<SysInt, DomainInt> > & tup=*(tuplist[i]);
            
            SysInt supsize=tup.size();
            bool valid=true;
            
            for(SysInt j=0; j<supsize; j++) {
                if(! vars[tup[j].first].inDomain(tup[j].second)) {
                    valid=false;
                    break;
                }
            }
            
            if(valid) {
                for(SysInt j=0; j<supsize; j++) {
                    ADDTOASSIGNMENT(tup[j].first, tup[j].second);  //assignment.push_back(tuplist[i][j]);
                }
                tuple_list_pos[var][val_offset]=i;
                return true;
            }
        }
        
        
        for(SysInt i=0; i<tuple_list_pos[var][val_offset]; i++) {
            vector<pair<SysInt, DomainInt> > & tup=*(tuplist[i]);
            
            SysInt supsize=tup.size();
            bool valid=true;
            
            for(SysInt j=0; j<supsize; j++) {
                if(! vars[tup[j].first].inDomain(tup[j].second)) {
                    valid=false;
                    break;
                }
            }
            
            if(valid) {
                for(SysInt j=0; j<supsize; j++) {
                    ADDTOASSIGNMENT(tup[j].first, tup[j].second);  //assignment.push_back(tuplist[i][j]);
                }
                tuple_list_pos[var][val_offset]=i;
                return true;
            }
        }
        return false;
    }
    
#endif
    
    virtual void full_propagate()
    {
       litsWithLostExplicitSupport.resize(0);
       varsWithLostImplicitSupport.resize(0); 

       for(SysInt i=0; i<vars.size(); i++) { 
               varsWithLostImplicitSupport.push_back(i);
       }

       findSupportsIncremental();
    }
    
    virtual vector<AnyVarRef> get_vars()
    {
      vector<AnyVarRef> ret;
      ret.reserve(vars.size());
      for(unsigned i = 0; i < vars.size(); ++i)
        ret.push_back(vars[i]);
      return ret;
    }
};  // end of class


