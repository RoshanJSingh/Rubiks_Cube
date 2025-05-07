#ifndef BFS_SOLVER_H
#define BFS_SOLVER_H

#include <queue>
#include <unordered_map>
#include <vector>
#include <omp.h>

template <typename T>
class BFSSolver
{
private:
    T rubiksCube;
    std::unordered_map<T, bool> visited;
    std::unordered_map<T, typename T::MOVE> move_done;

public:
    BFSSolver(T _rubiksCube) : rubiksCube(_rubiksCube) {}

    T bfs()
    {
        std::queue<T> q;
        q.push(rubiksCube);
        visited[rubiksCube] = true;

        while (!q.empty())
        {
            T node = q.front();
            q.pop();

            if (node.isSolved())
            {
                return node;
            }

            std::vector<std::pair<T, typename T::MOVE>> localQueue;

// Parallelize the loop using OpenMP
#pragma omp parallel for
            for (int i = 0; i < 18; i++)
            {
                T tempNode = node; // local copy
                auto curr_move = typename T::MOVE(i);
                tempNode.move(curr_move);

// Use a critical section to safely update shared resources
#pragma omp critical
                {
                    if (!visited[tempNode])
                    {
                        visited[tempNode] = true;
                        move_done[tempNode] = curr_move;
                        localQueue.emplace_back(tempNode, curr_move);
                    }
                }
            }

            // Push all new nodes into the queue
            for (auto &entry : localQueue)
            {
                q.push(entry.first);
            }
        }

        return rubiksCube; // return unsolved if no solution found
    }

    std::unordered_map<T, typename T::MOVE> getMoveDone()
    {
        return move_done;
    }
};

#endif