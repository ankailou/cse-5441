#include "lab1.h"
#include <stdio.h>
#include <cstdlib>
#include <iostream>
#include <pthread.h>
using namespace std;

/*****************************************************************************
 *********** helper functions for computing the convergence loops ************
 *****************************************************************************/

/**
 * function: convergenceLoop
 ***************************
 * loop 1: compute new dsv score for each box; loop 2: communicate dsv scores;
 *
 * param numThreads: number of threads to create
 * updates: for each @box in @Boxes: @box.dsv = computeNewDSV(@box);
 */
void convergenceLoop(int numThreads) {
    int rc, index;
    pthread_t *thread;
    thread = (pthread_t *)malloc(sizeof(*thread)*numThreads);
    for (long i = 0; i < numBoxes; i++) {
        index = i % numThreads;
        rc = pthread_create(&thread[index], NULL, computeNewDSV, (void *)i);
        if (index == numThreads - 1 || index == (numBoxes - 1)%numThreads) {
            for (long k = 0; k <= index; k++)
                rc = pthread_join(thread[k], NULL);
        }
    }
    for (int j = 0; j < numBoxes; j++)
        Boxes[j].dsv = Boxes[j].dsvNew;
}

/*****************************************************************************
 *************** helper function for computing new dsv values ****************
 *****************************************************************************/

/**
 * function: computeNewDSV
 *************************
 * for each side, average adjacent temperature = sum(intersection * dsv) / perimeter;
 *
 * param var: void pointer representing long representing @box id value;
 * updates: @box.newDSV = @box.dsv - [AFFECT_RATE * (@box.dsv - avg_adjacent_temp)];
 */
void *computeNewDSV(void *var) {
    long index = (long)var;
    Box box = Boxes[index];
    int perimeter = 0;
    float avgAdjacent = 0.0, offset = 0.0;
    int numNeighbors[NUM_SIDES] = {box.nTop, box.nLeft, box.nBottom, box.nRight};
    for (int i = 0; i < NUM_SIDES; i++) {
        if (numNeighbors[i] > 0)
            perimeter += (i % 2 == 0) ? (box).w : (box).h;
        avgAdjacent += computeSide(&(box), i, numNeighbors[i]);
    }
    if (perimeter > 0)
        offset = AFFECT_RATE * (box.dsv - (avgAdjacent / (float)perimeter));
    Boxes[index].dsvNew = box.dsv - offset;
    pthread_exit(NULL);
}

/**
 * function: computeSide
 ***********************
 * compute average adjacent temperature for one side of @box;
 *
 * param box: struct holding box information;
 * param side: int representing side := { 0 : top, 1 : left, 2 : bottom, 3 : right };
 * param numNeighbors: int representing number of neighboring boxes to iterate;
 * returns: sum_{nbox in @Boxes}( intersect(nBox,@box) * nBox.dsv );
 */
float computeSide(Box *box, int side, int numNeighbors) {
    int is;
    float temp = 0.0;
    int *sides[4] = {(*box).topNeighbor, (*box).leftNeighbor, (*box).bottomNeighbor, (*box).rightNeighbor};
    for (int i = 0; i < numNeighbors; i++) {
        Box neighbor = Boxes[sides[side][i]];
        if (side % 2 == 0)
            is = intersect((*box).x, (*box).w, neighbor.x, neighbor.w);
        else
            is = intersect((*box).y, (*box).h, neighbor.y, neighbor.h);
        temp += neighbor.dsv * is;
    }
    return temp;
}

/**
 * function: intersect
 *********************
 * compute the length of the intersection between adjacent boxes;
 *
 * param s1: start index of side of box 1;
 * param l1: length of the side of box 1;
 * param s2: start index of side of box 2;
 * param l2: length of the side of box 2;
 * returns: l1 - [ segments of s2 + l2 not intersecting with box 1 ];
 */
int intersect(int s1, int l1, int s2, int l2) {
    int intersect = l1;
    if ( s2 > s1 )
        intersect -= (s2 - s1);
    if ((s2 + l2) < (s1 + l1))
        intersect -= ((s1 + l1) - (s2 + l2));
    return intersect;
}
