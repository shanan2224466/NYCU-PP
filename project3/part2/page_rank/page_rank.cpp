#include "page_rank.h"

#include <stdlib.h>
#include <cmath>
#include <omp.h>
#include <utility>

#include "../common/CycleTimer.h"
#include "../common/graph.h"

// pageRank --
//
// g:           graph to process (see common/graph.h)
// solution:    array of per-vertex vertex scores (length of array is num_nodes(g))
// damping:     page-rank algorithm's damping parameter
// convergence: page-rank algorithm's convergence threshold
//
void pageRank(Graph g, double *solution, double damping, double convergence)
{

  // initialize vertex weights to uniform probability. Double
  // precision scores are used to avoid underflow for large graphs

  int numNodes = num_nodes(g);
  double equal_prob = 1.0 / numNodes;
  for (int i = 0; i < numNodes; ++i)
  {
    solution[i] = equal_prob;
  }
  /*
     For PP students: Implement the page rank algorithm here.  You
     are expected to parallelize the algorithm using openMP.  Your
     solution may need to allocate (and free) temporary arrays.

     Basic page rank pseudocode is provided below to get you started:

     // initialization: see example code above
     score_old[vi] = 1/numNodes;

     while (!converged) {

       // compute score_new[vi] for all nodes vi:
       score_new[vi] = sum over all nodes vj reachable from incoming edges
                          { score_old[vj] / number of edges leaving vj  }
       score_new[vi] = (damping * score_new[vi]) + (1.0-damping) / numNodes;

       score_new[vi] += sum over all nodes v in graph with no outgoing edges
                          { damping * score_old[v] / numNodes }

       // compute how much per-node scores have changed
       // quit once algorithm has converged

       global_diff = sum over all nodes vi { abs(score_new[vi] - score_old[vi]) };
       converged = (global_diff < convergence)
     }

   */
  int converged = 0, i, j;
  double *newScore = (double*)malloc(sizeof(double) * g->num_nodes), global_diff;

  while (!converged)
  {
    // Compute score_new[vi] for all nodes vi this round.
    #pragma omp parallel for private(j)
    for (i = 0; i < g->num_nodes; i++)
    {
      double weight = 0.0;

      // Sum over all nodes vj reachable from incoming edges.
      for (j = 0; j < incoming_size(g, i); j++)
      {
        // weight = score_old[vj] / number of edges leaving vj
        weight += solution[g->incoming_edges[g->incoming_starts[i] + j]] / outgoing_size(g, g->incoming_edges[g->incoming_starts[i] + j]);
      }

      // score_new[vi] = (damping * weight) + (1.0 - damping) / numNodes
      newScore[i] = (damping * weight) + (1.0 - damping) / g->num_nodes;
    }

    // score_new[vi] += sum over all nodes v in graph with no outgoing edges
    double tmp = 0.0;
    for (j = 0; j < g->num_nodes; j++)
    {
      // tmp is the total summary of all score_old[v] when v has no outgoing edges.
      if (outgoing_begin(g, j) == outgoing_end(g, j))
        tmp += solution[j];
    }

    // Renew tmp.
    tmp = tmp * damping / g->num_nodes;

    // Sum the tmp to every vi score_new[vi].
    #pragma omp parallel for
    for (i = 0; i < g->num_nodes; i++)
      newScore[i] += tmp;

    // global_diff = sum over all nodes vi { abs(score_new[vi] - score_old[vi]) };
    global_diff = 0.0;
    for(i = 0; i <g->num_nodes; i++)
    {
      double diff = solution[i] - newScore[i];
      global_diff += abs(diff);
    }

    // Test whether the algorithm has converged.
    converged = (global_diff < convergence);

    // Replace the score_old[vi] last round with the score_new[vi] this round.
    #pragma omp parallel for
    for(i = 0; i <g->num_nodes; i++)
    {
      solution[i] = newScore[i];
    }
  }
  free(newScore);
}
