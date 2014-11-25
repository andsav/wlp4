#include "Loc.h"

stack<int> freeRegs;
vector<string> active;
vector<int> usedRegs;

void Loc::genLoc() {
	for(int i=28; i>13; --i) {
		freeRegs.push(i);
	}
	if(!isInit) {
		freeRegs.push(13);
		freeRegs.push(12);
	}
	if(!isPrint) {
		freeRegs.push(10);
	}

	FOREACH(program) {
		active.clear();
		current = &program[i];
		sortSymbol sortByLast(current, LAST_USE);
		sortSymbol sortByFirst(current, FIRST_DEF);

		sort(current->symbolsList.begin(), current->symbolsList.end(), sortByFirst);

		FOREACH_(current->symbolsList, j) {
			string& var = current->symbolsList[j];

			expire(var);

			if(freeRegs.empty()) {
				spill(var, sortByLast);
			}
			else {
				current->symbolsTable[var]->loc = freeRegs.top();
				freeRegs.pop();
				active.push_back(var);
				sort(active.begin(), active.end(), sortByLast);
			}
		}
	}
}

vector<int> Loc::getUsedRegs() {
	if(!usedRegs.empty())
		return usedRegs;

	int max = 0;
	FOREACH(current->symbols) {
		if(max == 28)
			break;
		if(current->symbols[i]->loc > max) 
			max = current->symbols[i]->loc;
	}

	for(int i=12; i<=max; ++i) usedRegs.push_back(i);
	if(max >= 10) usedRegs.push_back(10);

	return usedRegs;
}

void Loc::spill(string& var, sortSymbol& sortFunction) {
	string spill = active.back();

	// Spill the last element of active
	if(current->symbolsTable[spill]->use.second > current->symbolsTable[var]->use.second) {
		current->symbolsTable[var]->loc = current->symbolsTable[spill]->loc;
		++current->offset;
		current->symbolsTable[spill]->loc = -4*current->offset;
		active.pop_back();
		active.push_back(var);
		sort(active.begin(), active.end(), sortFunction);
	}
	// Spill the passed argument
	else {
		++current->offset;
		current->symbolsTable[var]->loc = -4*current->offset;
	}
}

void Loc::expire(string& var) {
	FOREACH(active) {
		if(current->symbolsTable[active[i]]->use.second >= current->symbolsTable[var]->def.first)
			break;

		freeRegs.push(current->symbolsTable[active[i]]->loc);
		active.erase(
			remove(active.begin(), active.end(), active[i]),
			active.end());
	}
}