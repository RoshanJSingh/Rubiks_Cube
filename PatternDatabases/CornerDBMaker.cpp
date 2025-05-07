#include "CornerDBMaker.h"
#include <omp.h>
#include <queue>
#include <vector>

using namespace std;

CornerDBMaker::CornerDBMaker(string _fileName)
{
    fileName = _fileName;
}

CornerDBMaker::CornerDBMaker(string _fileName, uint8_t init_val)
{
    fileName = _fileName;
    cornerDB = CornerPatternDatabase(init_val);
}

bool CornerDBMaker::bfsAndStore()
{
    RubiksCubeBitboard cube;
    queue<RubiksCubeBitboard> q;
    q.push(cube);
    cornerDB.setNumMoves(cube, 0);
    int curr_depth = 0;

    while (!q.empty())
    {
        int n = q.size();
        vector<RubiksCubeBitboard> curr_level;

        for (int i = 0; i < n; ++i)
        {
            curr_level.push_back(q.front());
            q.pop();
        }

        vector<RubiksCubeBitboard> next_level;

#pragma omp parallel for
        for (int i = 0; i < (int)curr_level.size(); ++i)
        {
            RubiksCubeBitboard node = curr_level[i];

            for (int move_idx = 0; move_idx < 18; ++move_idx)
            {
                auto move = RubiksCube::MOVE(move_idx);
                node.move(move);

                bool shouldAdd = false;

#pragma omp critical(corner_db_update)
                {
                    if ((int)cornerDB.getNumMoves(node) > curr_depth + 1)
                    {
                        cornerDB.setNumMoves(node, curr_depth + 1);
                        shouldAdd = true;
                    }
                }

                if (shouldAdd)
                {
#pragma omp critical(next_level_push)
                    {
                        next_level.push_back(node);
                    }
                }

                node.invert(move);
            }
        }

        for (auto &node : next_level)
            q.push(node);

        curr_depth++;
        if (curr_depth == 9)
            break;
    }

    cornerDB.toFile(fileName);
    return true;
}
