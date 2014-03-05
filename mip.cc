// Solve the weighted set cover problem using MIP.
// Uses EasySCIP: https://github.com/ricbit/easyscip
// Ricardo Bittencourt 2014

#include <vector>
#include <cstdio>
#include <iostream>
#include <sstream>
#include <fstream>
#include <algorithm>
#include "easyscip.h"

using namespace std;
using namespace easyscip;

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
  int items = limits[0];
  int sets = limits[1];
  vector<vector<int>> cover_info(sets);
  generate_n(cover_info.begin(), sets, read_line);

  // Create the MIP model.
  MIPSolver mip;

  // Create one binary variable for each set.
  // This variable is "1" if the set is used in the final cover.
  // The cost for this variable is the cost defined in the input.
  vector<Variable> set_used;
  for (int i = 0; i < sets; i++) {
    set_used.push_back(mip.binary_variable(cover_info[i][0]));
  }

  // Create one constraint for each item. 
  // We must ensure each item is covered by at least one set.
  for (int i = 0; i < items; i++) {
    Constraint cons = mip.constraint();
    for (int j = 0; j < sets; j++) {
      for (int k = 1; k < int(cover_info[j].size()); k++) {
        if (cover_info[j][k] == i) {
          cons.add_variable(set_used[j], 1);
        }
      }
    }
    cons.commit(1, sets);
  }

  // Solve the MIP model (with a time limit in seconds).
  mip.set_time_limit(2 * 60 * 60);
  Solution sol = mip.solve();

  // Print the solution to a file (to be read later by the python script).
  ofstream output;
  output.open("answer.txt");
  output << sol.objective() << " " << (sol.is_optimal() ? 1 : 0) << "\n";
  for (int i = 0; i < sets; i++) {
    output << (sol.value(set_used[i]) > 0.5 ? 1 : 0) << " ";
  }
  output << "\n";
  output.close();

  return 0;
}
