/*
Modified by Adriana Rios and Kasie Kelldorf

TSP code for CS 4380 / CS 5351

Copyright (c) 2016, Texas State University. All rights reserved.

Redistribution in source or binary form, with or without modification,
is not permitted. Use in source and binary forms, with or without
modification, is only permitted for academic use in CS 4380 or CS 5351
at Texas State University.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

Author: Martin Burtscher
*/

#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <cmath>
#include <sys/time.h>
#include <pthread.h>  // #3
#include "cs43805351.h"

// # 8 may have missed something
static int cities, posx, posy, climbs, thread_count;
static int TwoOpt(int& climbs);
static const int width;

#define dist(a, b) (int)(sqrtf((px[a] - px[b]) * (px[a] - px[b]) + (py[a] - py[b]) * (py[a] - py[b])) + 0.5f)

int TwoOpt(int& climbs)   // #9
{
  // #18
  long thread;
  pthread_t* thread_handles;
  thread_handles =
          static_cast<pthread_t*>(malloc(thread_count * sizeof(phtread_t)));
  pthread_mutex_t* mutex_p;
  pthread_mutex_t_init(&mutex_p, NULL);


  // link end to beginning
  px[cities] = px[0];
  py[cities] = py[0];

  // repeat until no improvement
  long minchange;
  int iter = 0;
  do {
    iter++;

    // #11 create threads-1
    for (thread = 0; thread < thread_count - 1; thread++)
    pthread_create(&thread_handles[thread], NULL, workerThreads, (int*) thread);

    // #11 master must call same function
    workerThreads((int*) (thread_count-1));


    // apply move if it shortens the tour
    if (minchange < 0) {
      int i = (minchange >> 16) & 0xffff;
      int j = minchange & 0xffff;
      i++;
      while (i < j) {
        float t;
        t = px[i];  px[i] = px[j];  px[j] = t;
        t = py[i];  py[i] = py[j];  py[j] = t;
        i++;
        j--;
      }
    }
  } while (minchange < 0);
  climbs = iter;

  // compute tour length
  int len = 0;
  for (int i = 0; i < cities; i++) {
    len += dist(i, i + 1);
  }

  // join threads
  for (thread = 0; thread < thread_count; thread++)
    pthread_join(thread_handles[thread], NULL);

  // #18
  pthread_mutex_destroy(&mutex_p);
  pthread_exit(NULL);   //unsure

  return len;
}

int main(int argc, char *argv[])
{
  printf("TSP v1.4 [pthread]\n");  // #2

  // read input
  if (argc != 3) {fprintf(stderr, "usage: %s input_file num_threads\n", argv[0]); exit(-1);}
  FILE* f = fopen(argv[1], "rb");  if (f == NULL) {fprintf(stderr, "error: could not open file %s\n", argv[1]); exit(-1);}
  thread_count = atoi(argv[2]);   // #5
  // #6
  if (thread_count < 1) {fprintf(stderr, "error: thread_count must be at least 1\n");}
  //int cities;
  int cnt = fread(&cities, sizeof(int), 1, f);  if (cnt != 1) {fprintf(stderr, "error: failed to read cities\n"); exit(-1);}
  if (cities < 1) {fprintf(stderr, "error: cities must be greater than zero\n"); exit(-1);}
  float posx[cities + 1], posy[cities + 1];  // need an extra element later
  cnt = fread(posx, sizeof(float), cities, f);  if (cnt != cities) {fprintf(stderr, "error: failed to read posx\n"); exit(-1);}
  cnt = fread(posy, sizeof(float), cities, f);  if (cnt != cities) {fprintf(stderr, "error: failed to read posy\n"); exit(-1);}
  fclose(f);
  printf("configuration: %d cities from %s on %d threads\n", cities, argv[1], thread_count);  // #7 ?

  // start time
  struct timeval start, end;
  gettimeofday(&start, NULL);

  // find good tour
  //int climbs;
  int len = TwoOpt(climbs);

  // end time
  gettimeofday(&end, NULL);
  double runtime = end.tv_sec + end.tv_usec / 1000000.0 - start.tv_sec - start.tv_usec / 1000000.0;
  printf("compute time: %.4f s\n", runtime);
  long moves = 1LL * climbs * (cities - 2) * (cities - 1) / 2;
  printf("gigamoves/sec: %.3f\n", moves * 0.000000001 / runtime);

  // output result
  printf("tour length = %d\n", len);

  // scale and draw final tour
  width = 1024;
  unsigned char pic[width][width];
  memset(pic, 0, width * width * sizeof(unsigned char));
  float minx = posx[0], maxx = posx[0];
  float miny = posy[0], maxy = posy[0];
  for (int i = 1; i < cities; i++) {
    if (minx > posx[i]) minx = posx[i];
    if (maxx < posx[i]) maxx = posx[i];
    if (miny > posy[i]) miny = posy[i];
    if (maxy < posy[i]) maxy = posy[i];
  }
  float dist = maxx - minx;
  if (dist < (maxy - miny)) dist = maxy - miny;
  float factor = (width - 1) / dist;
  int x[cities], y[cities];
  for (int i = 0; i < cities; i++) {
    x[i] = (int)(0.5f + (posx[i] - minx) * factor);
    y[i] = (int)(0.5f + (posy[i] - miny) * factor);
  }
  for (int i = 1; i < cities; i++) {
    line(x[i - 1], y[i - 1], x[i], y[i], 127, (unsigned char*)pic, width);
  }
  line(x[cities - 1], y[cities - 1], x[0], y[0], 128, (unsigned char*)pic, width);
  for (int i = 0; i < cities; i++) {
    line(x[i], y[i], x[i], y[i], 255, (unsigned char*)pic, width);
  }
  writeBMP(width, width, (unsigned char*)pic, "tsp.bmp");

  return 0;
}

void* workerThreads(int* rank) {
  long my_rank = (long) rank;


// determine best 2-opt move
  minchange = 0;
  // #15 cyclic
  for (int i = rank; i < cities - 2; i+=thread_count) {
    for (int j = i + 2; j < cities; j++) {
      long change = dist(i, j) + dist(i + 1, j + 1) - dist(i, i + 1) - dist(j, j + 1);
      change = (change << 32) + (i << 16) + j;
      if (minchange > change) {
        minchange = change;
      }
    }
  }


  return NULL;
}
