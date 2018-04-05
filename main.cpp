#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <algorithm>

using namespace std;

enum MoveType { MOVE = 0, BOMBING = 1, INC = 2};
enum EntityType { FACTORY = 0, TROOP = 1, BOMB = 2};

/* ----------------------- START UTILS   -------------------------- */
int random(int min, int max) //range : [min, max)
{
   static bool first = true;
   if ( first ) 
   {  
      srand(time(NULL)); //seeding for the first time only!
      first = false;
   }
   if(max == min) return max;
   return min + rand() % (max - min);
}

class Graph {
    public:
	std::map<std::pair<int,int>,int> dist;
	std::map<std::pair<int,int>,int> dist_opt;
	std::map<std::pair<int,int>,int> next;
	int size;
   
    Graph (std::map<std::pair<int,int>,int> pdist, int psize) {
		dist = pdist;
		size = psize;
        floyd_warshall();
    }
   
    void floyd_warshall() 
	{
        int n = size;
		cerr << "size " << n << endl;
        /*dist_opt = new int[15][15];
        next = new int[15][15];*/
        for (int i = 0; i < n; i++)
            for (int j = 0; j < n; j++) {
				next[{i,j}] = j;
                dist_opt = dist;
            }
        for (int i = 0; i < n; i++)
            for (int j = 0; j < n; j++) 
                if (i != j)
                    for (int k = 0; k < n; k++) {
                        if (k != i && k != j && dist_opt[{i,k}]+dist_opt[{k,j}] < dist_opt[{i,j}] ) {
                            dist_opt[{i,j}] = dist_opt[{i,k}]+dist_opt[{k,j}];
                            next[{i,j}] = k;
                        }
                    }
    }
 
};


class Entity{
public:    
  Entity(void){};
  Entity(int pid, EntityType ptype = FACTORY): id(pid), type(ptype){};

  ~Entity(void){};
  
  virtual void updateWithInputs(int pid, EntityType ptype = FACTORY)
  {
    id = pid;
	type = ptype;
  }

  int id;
  EntityType type;
};

class Factory : public Entity {
public:
  Factory(void){};
  Factory(int pid, int powner, int pcyborgNb, int pprod, int pstun): 
	  Entity(pid, FACTORY), owner(powner), cyborgNb(pcyborgNb), prod(pprod), stun(pstun){};

  ~Factory(void){};
  
  void updateWithInputs(int pid, int powner, int pcyborgNb, int pprod, int pstun)
  {
    Entity::updateWithInputs(pid, FACTORY);
	owner = powner;
	cyborgNb = pcyborgNb;
	prod = pprod;
	stun = pstun;
  }
  
  void print()
  {
	  switch(owner)
	  {
		  case 1: cerr << "myF ";  break;
		  case -1: cerr << "opF "; break;
		  case 0: cerr << "neuF "; break;
	  }
	cerr << id << " Prod:" << prod << " Troops:" << cyborgNb << endl;	  
  }
  
  int owner;
  int cyborgNb;
  int prod;
  int dists[15];
  int stun;
  bool bombing;
  bool def;
  bool underAttack;
  int allowedTroops;
  int helpTurn;
  int helpTroopsNeeded;
  int opInc[15];
  int supInc[15];
};

class Troop : public Entity {
public:
  Troop(void){};
  Troop(int pid, int powner, int psource, int ptarget, int pcyborgNb, int pturns): 
	  Entity(pid, TROOP), owner(powner), source(psource), target(ptarget), cyborgNb(pcyborgNb), turns(pturns){};

  ~Troop(void){};

  void updateWithInputs(int pid, int powner, int psource, int ptarget, int pcyborgNb, int pturns)
  {
    Entity::updateWithInputs(pid, TROOP);
	owner = powner;
	source = psource;
	target = ptarget;
	cyborgNb = pcyborgNb;
	turns = pturns;
  }

  int owner;
  int source;
  int target;
  int cyborgNb;
  int turns;
};


class Bomb : public Entity {
public:
  Bomb(void){};
  Bomb(int pid, int powner, int psource, int ptarget, int pturns): 
	  Entity(pid, BOMB), owner(powner), source(psource), target(ptarget), turns(pturns){};

  ~Bomb(void){};

  void updateWithInputs(int pid, int powner, int psource, int ptarget, int pturns)
  {
    Entity::updateWithInputs(pid, BOMB);
	owner = powner;
	source = psource;
	target = ptarget;
	turns = pturns;
  }

  int owner;
  int source;
  int target;
  int turns;
};


Factory *factories[15];
Factory *myFactories[15];
Factory *opFactories[15];
Factory *neutralFactories[15];
Troop *troops[200];
Troop *myTroops[200];
Troop *opTroops[200];
Bomb *myBombs[2];
Bomb *opBombs[2];

class Link {
public:
	Link(void){};
	Link(int pfact1, int pfact2, int pdist) : fact1(pfact1), fact2(pfact2), dist(pdist){};

	int fact1;
	int fact2;
	int dist;
};

Link *links[105];

class Move {
public:
	Move(void){};
	Move(int psource, int pdest, int pnb) : source(psource), dest(pdest), nb(pnb){};
	
	void set(MoveType ptype, int ps, int psind = 0, int pd = 0, int pnb = 0, int pscore = 0)
	{
		switch(ptype)
		{
			case MOVE:	nb = pnb;
			case BOMBING:  sourceInd = psind;
						dest = pd;
			case INC:
						type = ptype;
						source = ps;
						score = pscore;	
				break;
		}	
	}
	
	MoveType type;
	int source;
	int sourceInd;
	int dest;
	int nb;
	int score;
};

class Solution {
public:
	Solution(void){};
	Solution(Move moves[]);

	Move moves[50];

	void output(int movesnb)
	{
		for(int i=0; i < movesnb; ++i)
		{
			
			switch(moves[i].type)
			{
				case MOVE:	cout << "MOVE " << moves[i].source << " " << moves[i].dest << " "<< moves[i].nb ;
					break;
				case BOMBING: cout << "BOMB " << moves[i].source << " " << moves[i].dest ;
					break;
				case INC: cout << "INC " << moves[i].source ;
					break;
			}	
			
			if(i+1 < movesnb) cout << " ; " ;
		}		
	}

	void sort(int size)
	{
	   int i, j;
	   for (i = 1; i < size; ++i) {
		   Move m = moves[i];
		   for (j = i; j > 0 && moves[j-1].score < m.score; j--)
			   moves[j] = moves[j-1];
		   moves[j] = m;
	   }
	}
};

Solution solution;
Solution pool;
int bombC = 2;
int turn = 1;
int bombing = 0;
int bombNextTurnTarget = -1;
int bombNextTurnSource = -1;
int bombNextTurnIndGeneral = -1;


std::map<std::pair<int,int>,int> inputDists;

/**
 * Auto-generated code below aims at helping you parse
 * the standard input according to the problem statement.
 **/
int main()
{
	for(int i=0; i<15; ++i)
	{
		factories[i] = new Factory(i,0,0,0,0);	
		myFactories[i] = new Factory(i,0,0,0,0);
		opFactories[i] = new Factory(i,0,0,0,0);	
		neutralFactories[i] = new Factory(i,0,0,0,0);	
	}
	for(int i=0; i<200; ++i)
	{
		troops[i] = new Troop(i,0,0,0,0,0);
		myTroops[i] = new Troop(i,0,0,0,0,0);
		opTroops[i] = new Troop(i,0,0,0,0,0);
	}
	for(int i=0; i<2; ++i)
	{
		myBombs[i] = new Bomb(i,0,0,0,0);
		opBombs[i] = new Bomb(i,0,0,0,0);
	}
	for(int i=0; i<105; ++i)
	{
		links[i] = new Link(0,0,0);
	}
    int factoryCount; // the number of factories
    cin >> factoryCount; cin.ignore();
    int linkCount; // the number of links between factories
    cin >> linkCount; cin.ignore();
	int linkC = 0;
    for (int i = 0; i < linkCount; i++) {
        int factory1;
        int factory2;
        int distance;
        cin >> factory1 >> factory2 >> distance; cin.ignore();
		cerr << factory1 << " " << factory2 << " "  << distance << endl;
		inputDists[{factory1,factory2}] = distance;
		inputDists[{factory2,factory1}] = distance;
		links[linkC]->fact1 = factory1;
		links[linkC]->fact2 = factory2;
		links[linkC++]->dist = distance;
		
		factories[factory1]->dists[factory2] = distance;
		factories[factory2]->dists[factory1] = distance;
    }

    Graph graph(inputDists, factoryCount);
    
    
	cerr << graph.dist[{5,6}] << endl;
	cerr << graph.dist_opt[{5,6}] << endl;
	cerr << graph.next[{5,6}] << endl;
    // game loop
    while (1) {

		for(int i=0; i<200; ++i)
		{
			troops[i]->cyborgNb = 0;
			myTroops[i]->cyborgNb = 0;
			opTroops[i]->cyborgNb = 0;
		}
		for(int i=0; i<2; ++i)
		{
			myBombs[i]->turns = -1;
			opBombs[i]->turns = -1;
			myBombs[i]->target = -1;
			opBombs[i]->target = -1;
		}

        int entityCount; // the number of entities (e.g. factories and troops)
        cin >> entityCount; cin.ignore();
        int entityId, arg1, arg2, arg3, arg4, arg5;
        string entityType;

		int myFCount = 0, opFCount = 0, neutralFCount = 0;
        for (int i = 0; i < factoryCount; i++) 
		{
            cin >> entityId >> entityType >> arg1 >> arg2 >> arg3 >> arg4 >> arg5; cin.ignore();
			//cerr << entityId << entityType << arg1 << arg2 << arg3 << arg4 << arg5 << endl;	
			factories[i]->updateWithInputs(entityId, arg1, arg2, arg3, arg4);
			switch(arg1)
			{
			case 1:	myFactories[myFCount++]->updateWithInputs(entityId, arg1, arg2, arg3, arg4);
				break;
			case -1: opFactories[opFCount++]->updateWithInputs(entityId, arg1, arg2, arg3, arg4); 
				break;
			case 0: neutralFactories[neutralFCount++]->updateWithInputs(entityId, arg1, arg2, arg3, arg4);
				break;
			}
		}
		
		int myTCount = 0, opTCount = 0, myBCount = 0, opBCount = 0;
        for (int i = 0; i < entityCount - factoryCount; i++) 
		{
            cin >> entityId >> entityType >> arg1 >> arg2 >> arg3 >> arg4 >> arg5; cin.ignore();
			//cerr << entityId << entityType << arg1 << arg2 << arg3 << arg4 << arg5 << endl;
			if(entityType == "TROOP")
			{
				troops[i]->updateWithInputs(entityId, arg1, arg2, arg3, arg4, arg5);
				switch(arg1)
				{
				case 1:	myTroops[myTCount++]->updateWithInputs(entityId, arg1, arg2, arg3, arg4, arg5);
					break;
				case -1: opTroops[opTCount++]->updateWithInputs(entityId, arg1, arg2, arg3, arg4, arg5); 
					break;
				}
			}else 
			{
				switch(arg1)
				{
				case 1:	myBombs[myBCount++]->updateWithInputs(entityId, arg1, arg2, arg3, arg4);
					break;
				case -1: opBombs[opBCount++]->updateWithInputs(entityId, arg1, arg2, arg3, arg4); 
					break;
				}
				
			}
		}
		
		for(int i=0; i<myFCount; ++i) myFactories[i]->print();
		for(int i=0; i<opFCount; ++i) opFactories[i]->print();
		for(int i=0; i<neutralFCount; ++i) neutralFactories[i]->print();
		
		int factoryToUp = -1;
		int sourceF = -1;
		int targetF = -1;
		int sources[5];
		int sourceC = 0;
		int targets[10];
		int targetC = 0;
		int solC = 0;
				
		// RESET THINGS
		for(int i=0; i < myFCount; ++i)
		{
			for(int ti=0; ti < 15; ++ti)
			{
				myFactories[i]->opInc[ti] = 0;
				myFactories[i]->supInc[ti] = 0;
			}			
			myFactories[i]->def = false;
			myFactories[i]->underAttack = false;
			myFactories[i]->allowedTroops = 0;
			myFactories[i]->helpTurn = -1;	
			myFactories[i]->helpTroopsNeeded = 0;	
		}

		// Populate opInc and supInc for each myFactory
		for(int i=0; i < myFCount; ++i)
		{
			for(int ti=0; ti < opTCount; ++ti)
			{
				if(myFactories[i]->id == opTroops[ti]->target)
				{
					myFactories[i]->opInc[opTroops[ti]->turns] += opTroops[ti]->cyborgNb;
				}
			}
			for(int ti=0; ti < myTCount; ++ti)
			{
				if(myFactories[i]->id == myTroops[ti]->target)
				{
					myFactories[i]->supInc[myTroops[ti]->turns] += myTroops[ti]->cyborgNb;
				}
			}
		}
		for(int i=0; i < opFCount; ++i)
		{
			for(int ti=0; ti < opTCount; ++ti)
			{
				if(opFactories[i]->id == myTroops[ti]->target)
				{
					opFactories[i]->opInc[myTroops[ti]->turns] += myTroops[ti]->cyborgNb;
				}
			}
			for(int ti=0; ti < myTCount; ++ti)
			{
				if(opFactories[i]->id == opTroops[ti]->target)
				{
					opFactories[i]->supInc[opTroops[ti]->turns] += opTroops[ti]->cyborgNb;
				}
			}
		}
		
		// Check if we need help to def
		for(int i=0; i < myFCount; ++i)
		{
			int totalInc = 0, totalDef = myFactories[i]->cyborgNb;
			int troopsNeeded = 0;
			int helpNeededTurn = -1;
			// look 15 turns deep if we can def depending on troops incoming and prod
			for(int ti=1; helpNeededTurn == -1 && ti < 15; ++ti)
			{
				totalInc += myFactories[i]->opInc[ti];
				totalDef += myFactories[i]->supInc[ti];
				if(myFactories[i]->stun - ti <= 0) totalDef += myFactories[i]->prod;
				if(totalDef < totalInc)
				{
					helpNeededTurn = ti;	
					// look how much troops we need for help	
					troopsNeeded = totalInc-totalDef;
				}	
				/*if( myFactories[i]->id==5)
				{
					cerr << "f8  "<< myFactories[i]->helpTurn << endl;
					cerr << "f8 totalInc "<< totalInc << endl;
					cerr << "f8 totalDef "<< totalDef << endl;
				}*/
			}
			
			// if op troops incoming and we can def with help
			if(totalInc != 0 && helpNeededTurn != -1)
			{
				myFactories[i]->helpTurn = helpNeededTurn;	
				myFactories[i]->helpTroopsNeeded = troopsNeeded;			
			}					
		}
		for(int i=0; i<myFCount; ++i)
		{
			if(myFactories[i]->helpTurn != -1)
			{	cerr << "myFactories[nhi]->helpTurn         " << myFactories[i]->id << " " << myFactories[i]->helpTurn << endl;
				cerr << "myFactories[nhi]->helpTroopsNeeded " << myFactories[i]->id << " " << myFactories[i]->helpTroopsNeeded << endl;
			}
		}			
		// Check if we can help another factory to def
		for(int hi=0; hi < myFCount; ++hi)
		{
			for(int nhi=0; nhi < myFCount; ++nhi)
			{
				if(myFactories[hi]->id == myFactories[nhi]->id) continue;
				// Are we close enough with troopsNb needed 
				int distHelper = factories[myFactories[hi]->id]->dists[myFactories[nhi]->id];
				if( distHelper+1 == myFactories[nhi]->helpTurn &&
					myFactories[hi]->cyborgNb >= myFactories[nhi]->helpTroopsNeeded)
				{
					cerr << " helper    id: " <<myFactories[hi]->id << " troops: " <<  myFactories[hi]->cyborgNb << endl;
					cerr << " need help id: " <<myFactories[nhi]->id << " troops: " <<  myFactories[nhi]->helpTroopsNeeded << endl;
					cerr << " dist " << factories[myFactories[hi]->id]->dists[myFactories[nhi]->id] << " " << myFactories[nhi]->helpTurn << endl;
					
					pool.moves[solC++].set(MOVE, myFactories[hi]->id, hi, myFactories[nhi]->id,
					myFactories[nhi]->helpTroopsNeeded +1, myFactories[nhi]->prod*20);
					
					myFactories[hi]->cyborgNb -= myFactories[nhi]->helpTroopsNeeded +1;
					myFactories[nhi]->supInc[distHelper] +=  myFactories[nhi]->helpTroopsNeeded +1;
				}
			}	
		}
		// Check if we need to def (if possible set def to true)
		for(int i=0; i < myFCount; ++i)
		{
			int totalInc = 0, totalDef = myFactories[i]->cyborgNb;
			int allowedTroops = 500;
			bool canWeDefThisTurn = true;
			// look 15 turns deep if we can def depending on troops incoming and prod
			for(int ti=1; ti < 15; ++ti)
			{

				totalInc += myFactories[i]->opInc[ti];
				totalDef += myFactories[i]->supInc[ti];
				if(myFactories[i]->stun - ti <= 0) totalDef += myFactories[i]->prod;
				if(totalDef < totalInc)
				{
					canWeDefThisTurn = false;					
				}	
				// look each turn how much troops we can send
				// save the lowest number, its the allowed troops
				else if(totalDef-totalInc < allowedTroops ) allowedTroops = totalDef-totalInc;
				if( myFactories[i]->id==5)
				{
					cerr << "f5 totalInc "<< totalInc << endl;
					cerr << "f5 totalDef "<< totalDef << endl;
				}
			}
			
			// if op troops incoming and we can def each turn defend this factory
			if(totalInc != 0)
			{
				myFactories[i]->underAttack = true;
				factories[i]->underAttack = true;
				cerr << "myFactories underAttack " << myFactories[i]->id << endl;
				if(canWeDefThisTurn) myFactories[i]->def = true;			
			}		
			// We may have more troops than needed to def, set allowed troops to attack
			if(myFactories[i]->def || myFactories[i]->underAttack)
			{		
				myFactories[i]->allowedTroops = allowedTroops;				
			}
			else myFactories[i]->allowedTroops = 0;
			myFactories[i]->allowedTroops = allowedTroops;	

			
		}
		
		for(int i=0; i<myFCount; ++i)
		{
			cerr << "myFac id:" << myFactories[i]->id << " allowedTroops:" << myFactories[i]->allowedTroops << endl;
		}			
			

		for(int i=0; i<myFCount; ++i)
		{
			if(myFactories[i]->cyborgNb > 10 && myFactories[i]->prod != 3) factoryToUp = i;
		}
		if( factoryToUp != -1) cerr << "factoryToUp " << myFactories[factoryToUp]->id  << endl;
		else cerr << "factoryToUp -1" << endl;

		for(int i=0; solC <= 50 && i < myFCount; ++i)
		{
			if(myFactories[i]->def == false)				
			{
				// Prod loop
				for(int prod=3; solC <= 50 && prod >= 1; --prod)
				{
					for(int ni=0;  solC <= 50 && ni<neutralFCount; ++ni)
					{
						if(neutralFactories[ni]->prod == prod && myFactories[i]->cyborgNb > neutralFactories[ni]->cyborgNb)
						{
							int lscore = prod*20 - neutralFactories[ni]->cyborgNb;
							lscore -= factories[myFactories[i]->id]->dists[neutralFactories[ni]->id] * 2;
							
							pool.moves[solC++].set(MOVE, myFactories[i]->id, i, 
														 neutralFactories[ni]->id,
														 neutralFactories[ni]->cyborgNb + 1, 
														 lscore);
						}
					}
				}
				for(int prod=3; solC <= 50 && prod >= 1; --prod)
				{
					for(int oi=0; solC <= 50 && oi<opFCount; ++oi)
					{
						/*int opTotalSup = 0;
						for(int supi=0; supi < factories[myFactories[i]->id]->dists[opFactories[oi]->id] ; ++supi)
						{
							opTotalSup += opFactories[oi]->supInc[supi];
						}*/
						int troopsNeeded =  opFactories[oi]->cyborgNb + opFactories[oi]->prod*(factories[myFactories[i]->id]->dists[opFactories[oi]->id]+2);	
						if(opFactories[oi]->prod == prod && myFactories[i]->cyborgNb > troopsNeeded)
						{
							int lscore = prod*20 ;
							lscore -= factories[myFactories[i]->id]->dists[opFactories[oi]->id] * 2;
							
							pool.moves[solC++].set(MOVE, myFactories[i]->id, i, 
														 opFactories[oi]->id,
														 troopsNeeded + 1, 
														 lscore);
						}
					}
				}
			}
			else
			{				
				for(int prod=3; solC <= 50 && prod >= 1; --prod)
				{
					for(int ni=0;  solC <= 50 && ni<neutralFCount; ++ni)
					{
						if(neutralFactories[ni]->prod == prod && myFactories[i]->allowedTroops > neutralFactories[ni]->cyborgNb)
						{
							cerr << " fa  " <<i << " " <<  myFactories[i]->allowedTroops << endl;
							
							int lscore = prod*20 - neutralFactories[ni]->cyborgNb;
							lscore -= factories[myFactories[i]->id]->dists[neutralFactories[ni]->id] * 2;
							
							pool.moves[solC++].set(MOVE, myFactories[i]->id, i, 
														 neutralFactories[ni]->id,
														 neutralFactories[ni]->cyborgNb + 1, 
														 lscore);														 
						}
					}
				}
				for(int prod=3; solC <= 50 && prod >= 1; --prod)
				{
					for(int oi=0; solC <= 50 && oi<opFCount; ++oi)
					{
						int troopsNeeded = opFactories[oi]->cyborgNb + opFactories[oi]->prod*(factories[myFactories[i]->id]->dists[opFactories[oi]->id]+2);	
						if(opFactories[oi]->prod == prod && myFactories[i]->allowedTroops > troopsNeeded)
						{
							cerr << " f  " << myFactories[i]->id << " " <<  myFactories[i]->cyborgNb << endl;
							cerr << " op " <<opFactories[oi]->id << " " <<  troopsNeeded << endl;
							cerr << " dist " <<opFactories[oi]->id << " " <<  factories[myFactories[i]->id]->dists[opFactories[oi]->id] << endl;
							int lscore = prod*20 ;
							lscore -= factories[myFactories[i]->id]->dists[opFactories[oi]->id] * 2;
							
							pool.moves[solC++].set(MOVE, myFactories[i]->id, i, 
														 opFactories[oi]->id,
														 troopsNeeded + 1, 
														 lscore);
						}
					}
				}
				
			}
		}
		
		
		--bombing;
		if(bombing == 1 && bombNextTurnTarget != -1 && factories[bombNextTurnIndGeneral]->owner == 1)
		{
			cerr << "bombNextTurnTarget " <<bombNextTurnTarget << endl;
			cerr << "bombNextTurnSource " <<bombNextTurnSource << endl;
			pool.moves[solC++].set(MOVE, bombNextTurnSource, bombNextTurnIndGeneral, 
										 bombNextTurnTarget,
										 1, 
										 500);
			bombNextTurnTarget = -1;
			bombNextTurnSource = -1;
			bombNextTurnIndGeneral = -1;
		}
		// BOMBING balourd
		for(int oi=0; bombC > 0 && oi<opFCount && myFCount != 0; ++oi)
		{			
			if(opFactories[oi]->prod * 20 + opFactories[oi]->cyborgNb >= 50 && 
					opFactories[oi]->id != myBombs[0]->target && 
					opFactories[oi]->id != myBombs[1]->target)
			{
				int bestDist = 100;
				int bestId = -1;
				int bestInd = -1;
				for(int lf=0; bombC > 0 && lf<factoryCount; ++lf)
				{
					if(factories[lf]->owner == 1 && factories[lf]->dists[opFactories[oi]->id] < bestDist)
					{
						bestDist = factories[lf]->dists[opFactories[oi]->id];
						bestId = factories[lf]->id;
						bestInd = lf;
						//cerr << "bestDist " << bestDist << " from " << bestId << endl;
					}
				}
				
				// Check if the conditions are still ok when the bomb will land
				int totalInc = 0, totalDef = opFactories[oi]->cyborgNb;
				// look bestDist turns deep
				for(int ti=1; ti < bestDist; ++ti)
				{
					if(opFactories[oi]->stun - ti <= 0) totalDef += opFactories[oi]->prod;
					if(myBombs[0]->turns == ti || myBombs[1]->turns == ti)
					{
						if(totalDef <=10 )totalDef = 0;
						else totalDef /= 2;
					}					
					totalInc += opFactories[oi]->opInc[ti];
					totalDef += opFactories[oi]->supInc[ti];
				}				
				if(totalDef - totalInc < 10 ) continue; // Abort because there will be not enough troops
				
				/*pool.moves[solC++].set(BOMBING, bestId, bestInd, 
										 opFactories[oi]->id,
										 0, 
										 opFactories[oi]->prod * 20 - bestDist*2);*/
				cout << "BOMB " << bestId << " " << opFactories[oi]->id << " ; ";

				bombNextTurnTarget = opFactories[oi]->id;
				bombNextTurnSource = bestId;
				bombNextTurnIndGeneral = bestInd;
				
				bombing = 5;
				--bombC;
			}
		}

		pool.sort(solC);	
		
		int finalSolutionC = 0;
		// Decrease cyborgNb in factories and copy possible solutions
		for(int i=0; i < solC; ++i)
		{
			if(pool.moves[i].score <= 0) continue;

			/*// Attack
			if(factories[pool.moves[i].dest]->owner == -1)
			{
				if(myFactories[pool.moves[i].source]->cyborgNb >= pool.moves[i].nb)
				{					
					solution.moves[finalSolutionC].source = pool.moves[i].source;
					solution.moves[finalSolutionC].dest = pool.moves[i].dest;
					solution.moves[finalSolutionC++].nb = pool.moves[i].nb;
					myFactories[pool.moves[i].source]->cyborgNb -= pool.moves[i].nb;
					opFactories[pool.moves[i].dest]->cyborgNb -= pool.moves[i].nb;
				}
			}
			// Defense
			else if(factories[pool.moves[i].dest]->owner == 1)
			{
				if(myFactories[pool.moves[i].source]->cyborgNb >= pool.moves[i].nb)
				{					
					solution.moves[finalSolutionC].source = pool.moves[i].source;
					solution.moves[finalSolutionC].dest = pool.moves[i].dest;
					solution.moves[finalSolutionC++].nb = pool.moves[i].nb;
					myFactories[pool.moves[i].source]->cyborgNb -= pool.moves[i].nb;
					myFactories[pool.moves[i].dest]->cyborgNb += pool.moves[i].nb;
				}
			}
			// Attack neutral
			else if(factories[pool.moves[i].dest]->owner == 0) neutralFactories[pool.moves[i].dest]->cyborgNb -= pool.moves[i].nb;
			*/
			if(myFactories[pool.moves[i].sourceInd]->cyborgNb >= pool.moves[i].nb || pool.moves[i].score == 500)
			{
				solution.moves[finalSolutionC].source = pool.moves[i].source;
				solution.moves[finalSolutionC].dest = pool.moves[i].dest;
				solution.moves[finalSolutionC].score = pool.moves[i].score;
				solution.moves[finalSolutionC++].nb = pool.moves[i].nb;
				myFactories[pool.moves[i].sourceInd]->cyborgNb -= pool.moves[i].nb;
				
				if(factories[pool.moves[i].dest]->owner == -1) opFactories[pool.moves[i].dest]->cyborgNb -= pool.moves[i].nb;
				else if(factories[pool.moves[i].dest]->owner == 1) myFactories[pool.moves[i].dest]->cyborgNb += pool.moves[i].nb;
				else if(factories[pool.moves[i].dest]->owner == 0) neutralFactories[pool.moves[i].dest]->cyborgNb -= pool.moves[i].nb;
			}
		}		
		
		if(factoryToUp != -1)
		{
			if(myFactories[factoryToUp]->underAttack) cerr << "myFactories underAttack " <<factoryToUp << endl;
			else  cerr << "myFactories NOT underAttack " << myFactories[factoryToUp]->id << endl;
		}
			

		if(factoryToUp != -1 && (!myFactories[factoryToUp]->underAttack && myFactories[factoryToUp]->allowedTroops > 10  ) )
		{
			if(myFactories[factoryToUp]->underAttack) cerr << "myFactories underAttack " << myFactories[factoryToUp]->id << endl;
			cout << "INC " << myFactories[factoryToUp]->id << " ; ";
		}
		for(int i=0; i < finalSolutionC; ++i)
		{
			if(solution.moves[i].score == 500) continue; // do not change the troop after the bomb
			
			int nextId = graph.next[{solution.moves[i].source,solution.moves[i].dest}];
			switch(factories[nextId]->owner)
			{
				case 1: if(!factories[nextId]->underAttack) solution.moves[i].dest = nextId;
					break;
				case 0: if(factories[nextId]->cyborgNb == 0) solution.moves[i].dest = nextId;
					break;
				case -1:
					break;
			}
		}	

		if(finalSolutionC > 0)
		{
			solution.output(finalSolutionC);
		}
		else cout << "WAIT";
		
		cout << endl;
		++turn;
	}
}
