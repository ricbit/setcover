// Solve the weighted set cover problem using Large Neighbourhood Search.
// Uses EasySCIP: https://github.com/ricbit/easyscip
// Ricardo Bittencourt 2014

#include <vector>
#include <set>
#include <cstdio>
#include <iostream>
#include <sstream>
#include <fstream>
#include <algorithm>
#include <ctime>
#include "easyscip.h"

using namespace std;
using namespace easyscip;

// The description of the set cover problem.
struct SetCoverDescription {
  int items;
  int sets;
  vector<vector<int>> info;

  SetCoverDescription(int items_, int sets_) 
      : items(items_), sets(sets_), info(sets_) {}
};

// One solution of the set cover problem.
struct SetCoverSolution {
  int objective;
  bool optimal;
  vector<bool> used;
};

// A solver for the set cover problem using MIP.
class SetCoverMIP {
 public:
  SetCoverMIP(const SetCoverDescription& cover_, 
              const SetCoverSolution& initial) 
      : cover(cover_) {
    // Create one binary variable for each set.
    // This variable is "1" if the set is used in the final cover.
    // The cost for this variable is the cost defined in the input.
    for (int i = 0; i < cover.sets; i++) {
      set_used.push_back(mip.binary_variable(cover.info[i][0]));
    }

    // Create one constraint for each item. 
    // We must ensure each item is covered by at least one set.
    for (int i = 0; i < cover.items; i++) {
      Constraint cons = mip.constraint();
      for (int j = 0; j < cover.sets; j++) {
        for (int k = 1; k < int(cover.info[j].size()); k++) {
          if (cover.info[j][k] == i) {
            cons.add_variable(set_used[j], 1);
          }
        }
      }
      cons.commit(1, cover.sets);
    }

    // Force excluded sets to be out of the solution.
    set<int> excluded_sets = some_empty_sets(initial);
    for (int s : excluded_sets) {
      Constraint cons = mip.constraint();
      cons.add_variable(set_used[s], 1);
      cons.commit(0, 0);
    }

    // Include the initial solution in the model.
    Solution given = mip.empty_solution();
    for (int i = 0; i < cover.sets; i++) {
      given.set_value(set_used[i], initial.used[i] ? 1 : 0);
    }
    given.commit();
  }

  // Returns a set with 50% randomly chosen empty sets from the solution.
  set<int> some_empty_sets(const SetCoverSolution& sol) {
    vector<int> empty;
    for (int i = 0; i < int(sol.used.size()); i++) {
      if (!sol.used[i]) {
        empty.push_back(i);
      }
    }
    random_shuffle(empty.begin(), empty.end());
    int size = empty.size() * 50 / 100;
    set<int> answer;
    copy_n(empty.begin(), size, inserter(answer, answer.begin()));
    return answer;
  }

  // Solve the MIP model (with a time limit in seconds).
  SetCoverSolution solve(int time_limit) {
    mip.set_time_limit(time_limit);
    Solution sol = mip.solve();
    SetCoverSolution answer;
    answer.objective = sol.objective();
    answer.optimal = sol.is_optimal();
    for (int i = 0; i < cover.sets; i++) {
      answer.used.push_back(sol.value(set_used[i]) > 0.5);
    }
    return answer;
  }
 private:
  const SetCoverDescription cover;
  MIPSolver mip;
  vector<Variable> set_used;
};

// Read ints from the stdin until the end of the line.
vector<int> read_line() {
  string line;
  getline(cin, line);
  istringstream iss(line);
  vector<int> answer;
  int value;
  while (iss >> value) {
    answer.push_back(value);
  }
  return answer;
}

int main() {
  // Read the input from stdin.
  auto limits = read_line();
  SetCoverDescription cover(limits[0], limits[1]);
  generate_n(cover.info.begin(), cover.sets, read_line);

  // Generate an initial solution with all sets.
  SetCoverSolution best;
  best.objective = 0;
  for (auto& line : cover.info) {
    best.objective += line[0];
  }
  best.optimal = false;
  fill_n(back_inserter(best.used), cover.sets, true);

  // Solve the set cover problem.
  for (auto start = time(NULL); difftime(time(NULL), start) < 60 * 60;) {
    SetCoverMIP mip(cover, best);
    best = mip.solve(2 * 60);
  }

  // Print the solution to a file (to be read later by the python script).
  ofstream output;
  output.open("answer.txt");
  output << best.objective << " 0\n";
  for (int i = 0; i < cover.sets; i++) {
    output << (best.used[i] > 0.5 ? 1 : 0) << " ";
  }
  output << "\n";
  output.close();

  return 0;
}
