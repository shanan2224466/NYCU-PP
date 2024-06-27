#include "bfs.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <cstddef>
#include <omp.h>

#include "../common/CycleTimer.h"
#include "../common/graph.h"

#define ROOT_NODE_ID 0
#define NOT_VISITED_MARKER -1

void vertex_set_clear(vertex_set *list)
{
    list->count = 0;
}

void vertex_set_init(vertex_set *list, int count)
{
    list->max_vertices = count;
    list->vertices = (int *)malloc(sizeof(int) * list->max_vertices);
    vertex_set_clear(list);
}

// Take one step of "top-down" BFS.  For each vertex on the frontier,
// follow all outgoing edges, and add all neighboring vertices to the
// new_frontier.
void top_down_step(
    Graph g,
    vertex_set *frontier,
    vertex_set *new_frontier,
    int *distances)
{
    // For the size of frontier is small, then serial process.
    if (frontier->count <= 900)
    {
        for (int i = 0; i < frontier->count; i++)
        {
            int node = frontier->vertices[i];

            int start_edge = g->outgoing_starts[node];
            int end_edge = (node == g->num_nodes - 1)
                            ? g->num_edges
                            : g->outgoing_starts[node + 1];

            // attempt to add all neighbors to the new frontier
            for (int neighbor = start_edge; neighbor < end_edge; neighbor++)
            {
                int outgoing = g->outgoing_edges[neighbor];

                if (distances[outgoing] == NOT_VISITED_MARKER)
                {
                    distances[outgoing] = distances[node] + 1;
                    int index = new_frontier->count++;
                    new_frontier->vertices[index] = outgoing;
                }
            }
        }
    }
	// For the size of frontier is large, then parallel process.
    else
    {
        #pragma omp parallel for schedule(dynamic, 1000)
        for (int i = 0; i < frontier->count; i++)
        {
            int node = frontier->vertices[i];

            int start_edge = g->outgoing_starts[node];
            int end_edge = (node == g->num_nodes - 1)
                            ? g->num_edges
                            : g->outgoing_starts[node + 1];

            // attempt to add all neighbors to the new frontier
            for (int neighbor = start_edge; neighbor < end_edge; neighbor++)
            {
                int outgoing = g->outgoing_edges[neighbor];

                if (distances[outgoing] == NOT_VISITED_MARKER)
                {
                    distances[outgoing] = distances[node] + 1;
                    int index, next;
                    do
                    {
                        index = new_frontier->count;
                        next = index;
                        next++;
                    } while (!__sync_bool_compare_and_swap(&new_frontier->count, index, next));
                    new_frontier->vertices[index] = outgoing;
                }
            }
        }
    }
}

// Implements top-down BFS.
//
// Result of execution is that, for each node in the graph, the
// distance to the root is stored in sol.distances.
void bfs_top_down(Graph graph, solution *sol)
{
    vertex_set list1;
    vertex_set list2;
    vertex_set_init(&list1, graph->num_nodes);
    vertex_set_init(&list2, graph->num_nodes);

    vertex_set *frontier = &list1;
    vertex_set *new_frontier = &list2;

    // initialize all nodes to NOT_VISITED
    #pragma omp parallel for
    for (int i = 0; i < graph->num_nodes; i++)
        sol->distances[i] = NOT_VISITED_MARKER;

    // setup frontier with the root node
    frontier->vertices[frontier->count++] = ROOT_NODE_ID;
    sol->distances[ROOT_NODE_ID] = 0;

    while (frontier->count != 0)
    {

#ifdef VERBOSE
        double start_time = CycleTimer::currentSeconds();
#endif

        vertex_set_clear(new_frontier);
        top_down_step(graph, frontier, new_frontier, sol->distances);

#ifdef VERBOSE
        double end_time = CycleTimer::currentSeconds();
        printf("frontier=%-10d %.4f sec\n", frontier->count, end_time - start_time);
#endif

        // swap pointers
        vertex_set *tmp = frontier;
        frontier = new_frontier;
        new_frontier = tmp;
    }
}

bool bottom_up_step(
    Graph g,
    bool *frontier,
    bool *new_frontier,
    int *distances)
{
    bool keepOn = false;
    #pragma omp parallel for schedule(dynamic, 1024)
    for (int i = 0; i < g->num_nodes; i++)
    {
		// Renew the data.
        new_frontier[i] = false;

        if (distances[i] == NOT_VISITED_MARKER)
        {
            int start_edge = g->incoming_starts[i];
            int end_edge = (i == g->num_nodes - 1)
                            ? g->num_edges
                            : g->incoming_starts[i + 1];

            for (int neighbor = start_edge; neighbor < end_edge; neighbor++)
            {
                int incoming = g->incoming_edges[neighbor];

				// If one of its incoming neighbor has already searched, then renew the info. and break.
                if (frontier[incoming])
                {
                    distances[i] = distances[incoming] + 1;
                    new_frontier[i] = true;
                    keepOn = true;
					break;
                }
            }
        }
    }
    return keepOn;
}

void bfs_bottom_up(Graph graph, solution *sol)
{
    // For PP students:
    //
    // You will need to implement the "bottom up" BFS here as
    // described in the handout.
    //
    // As a result of your code's execution, sol.distances should be
    // correctly populated for all nodes in the graph.
    //
    // As was done in the top-down case, you may wish to organize your
    // code by creating subroutine bottom_up_step() that is called in
    // each step of the BFS process.

    bool *frontier = (bool*)calloc(graph->num_nodes, sizeof(bool));
    bool *new_frontier = (bool*)calloc(graph->num_nodes, sizeof(bool));
    bool keepOn = true;

    // initialize all nodes to NOT_VISITED
    #pragma omp parallel for
    for (int i = 0; i < graph->num_nodes; i++)
        sol->distances[i] = NOT_VISITED_MARKER;

    // setup frontier with the root node
    frontier[0] = true;
    sol->distances[ROOT_NODE_ID] = 0;

    while (keepOn)
    {

#ifdef VERBOSE
        double start_time = CycleTimer::currentSeconds();
#endif

        keepOn = bottom_up_step(graph, frontier, new_frontier, sol->distances);

#ifdef VERBOSE
        double end_time = CycleTimer::currentSeconds();
        // printf("%.4f sec\n", end_time - start_time);
#endif

        // swap pointers
        bool *tmp = frontier;
        frontier = new_frontier;
        new_frontier = tmp;
    }
}

void bfs_hybrid(Graph graph, solution *sol)
{
    // For PP students:
    //
    // You will need to implement the "hybrid" BFS here as
    // described in the handout.
    
	// The same data structure as previous top_down.
    vertex_set list1;
    vertex_set list2;
    vertex_set_init(&list1, graph->num_nodes);
    vertex_set_init(&list2, graph->num_nodes);

    vertex_set *frontier_t = &list1;
    vertex_set *new_frontier_t = &list2;

    // initialize all nodes to NOT_VISITED
    #pragma omp parallel for
    for (int i = 0; i < graph->num_nodes; i++)
        sol->distances[i] = NOT_VISITED_MARKER;

    // setup frontier with the root node
    frontier_t->vertices[frontier_t->count++] = ROOT_NODE_ID;
    sol->distances[ROOT_NODE_ID] = 0;


	// The same data structure as previous bottom_up.
    bool *frontier_b = (bool*)calloc(graph->num_nodes, sizeof(bool));
    bool *new_frontier_b = (bool*)calloc(graph->num_nodes, sizeof(bool));

	// Few new data here.
	int alpha = 15, beta = 100;
	int mj, nj, mu, CBT, CTB, switchTime = 2;
    bool keepOn = true, topDown = true, change = false;

    while(topDown && frontier_t->count != 0 || !topDown && keepOn)
    {

#ifdef VERBOSE
        double start_time = CycleTimer::currentSeconds();
#endif
		
		// If it's top_down bfs.
        if (topDown)
        {
            vertex_set_clear(new_frontier_t);

            top_down_step(graph, frontier_t, new_frontier_t, sol->distances);

#ifdef VERBOSE
			double end_time = CycleTimer::currentSeconds();
			printf("frontier=%-10d %.4f sec\n", frontier_t->count, end_time - start_time);
#endif

			// Within the 2-time switches. Means you don't need to count mj and mu when already switch from bottom_up.
			if (switchTime)
			{
				mj = 0, mu = 0;

				#pragma omp parallel for reduction (+:mj)
				for (int i = 0; i < new_frontier_t->count; i++)
				{
					mj += outgoing_size(graph, new_frontier_t->vertices[i]);
				}

				#pragma omp parallel for schedule(dynamic, 1000) reduction (+:mu)
				for (int i = 0; i < graph->num_nodes; i++)
				{
					if (sol->distances[i] == NOT_VISITED_MARKER)
					{
						mu += incoming_size(graph, i);
					}
				}
				
				CTB = mu / alpha;

				if (mj > CTB)
				{
					topDown = false;
					change = true;
				}
			}

            // swap pointers
            vertex_set *tmp = frontier_t;
            frontier_t = new_frontier_t;
            new_frontier_t = tmp;
        }
        // If it's bottom_up bfs.
        else
        {
            keepOn = bottom_up_step(graph, frontier_b, new_frontier_b, sol->distances);

			nj = 0;
			for (int i = 0; i < graph->num_nodes; i++)
			{
				if (new_frontier_b[i])
				{
					nj++;
				}
				
			}

#ifdef VERBOSE
				double end_time = CycleTimer::currentSeconds();
				printf("frontier=%-10d %.4f sec\n", nj, end_time - start_time);
#endif

			CBT = graph->num_nodes / beta;

			if (nj < CBT)
			{
				topDown = true;
				change = true;
			}

            // swap pointers
            bool *tmp = frontier_b;
            frontier_b = new_frontier_b;
            new_frontier_b = tmp;
        }

		// Time to Switch.
        if (change)
        {
			// Switch the data structure from top_down to bottom_up.
            if (!topDown)
            {
				// Transfer the data structure.
				#pragma omp parallel for 
                for (int i = 0; i < frontier_t->count; i++)
                {
                    frontier_b[frontier_t->vertices[i]] = true;
                }
            }
			// Switch the data structure from bottom_up to top_down.
            else
            {
				int count = 0;
				frontier_t->count = 0;

				// Transfer the data structure.
                for (int i = 0; i < graph->num_nodes; i++)
                {
                    if (frontier_b[i])
                    {
                        frontier_t->vertices[count++] = i;
						frontier_t->count++;
                    }
                }
            }
			change = false;
			switchTime--;
        }
    }
}
