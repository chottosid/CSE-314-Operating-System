#include <chrono>
#include <iostream>
#include <fstream>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <vector>
#include <random>

#define FOR(i, a, b) for (int i = a; i < b; i++)

#define lock pthread_mutex_lock
#define unlock pthread_mutex_unlock

struct visitor {
    int id;
};

int n, m, w, x, y, z;
std::vector<visitor> visitors;
pthread_mutex_t step0, step1, step2;
sem_t gall1, glass;
pthread_mutex_t check,rc_l,wc_l,photo;
int wc;
int rc;

auto start_time = std::chrono::high_resolution_clock::now();
long long get_timestamp() {
    auto current_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(current_time - start_time);
    return duration.count();
}
int get_random_number() {
  std::random_device rd;
  std::mt19937 generator(rd());

  // Lambda value for the Poisson distribution
  double lambda = 1000.234;
  std::poisson_distribution<int> poissonDist(lambda);
  return poissonDist(generator);
}

void photobooth(visitor* v)
{
    int id = v->id;
    printf("Visitor %d is about to enter the photo booth at timestamp %lld\n",id,get_timestamp());
    if(id>=2000){
        lock(&wc_l);
        wc++;
        if(wc==1){
            lock(&check);
        }
        unlock(&wc_l);
        lock(&photo);
        printf("Visitor %d is inside the photo booth at timestamp %lld\n",id,get_timestamp());
        usleep(z*1000000);
        unlock(&photo);
        lock(&wc_l);
        wc--;
        if(wc==0){
            unlock(&check);
        }
        unlock(&wc_l);
    }
    else{
        lock(&check);
        lock(&rc_l);
        rc++;
        if(rc==1){
            lock(&photo);
        }
        unlock(&rc_l);
        unlock(&check);
        printf("Visitor %d is inside the photo booth at timestamp %lld\n",id,get_timestamp());
        usleep(z*1000000);
        lock(&rc_l);
        rc--;
        if(rc==0){
            unlock(&photo);
        }
        unlock(&rc_l);
    }
}
void* work(void* arg) {
    visitor* v = (visitor*)arg;
    int id = v->id;
    usleep(get_random_number()*1000);
    printf("Visitor %d has arrived at A at timestamp %lld\n", id, get_timestamp());
    usleep(w * 1000000);  // Time spent in hallway AB

    // Step 2: Arrive at Point B
    printf("Visitor %d has arrived at B at timestamp %lld\n", id, get_timestamp());

    // Steps to Gallery 1 (C) with synchronization on each step
    pthread_mutex_lock(&step0);
    printf("Visitor %d is on step 0 at timestamp %lld\n", id, get_timestamp());
    usleep(get_random_number()*1000);  // Delay for each step
    lock(&step1);
    unlock(&step0);
    printf("Visitor %d is on step 1 at timestamp %lld\n", id, get_timestamp());
    usleep(get_random_number()*1000);
    lock(&step2);
    unlock(&step1);
    printf("Visitor %d is on step 2 at timestamp %lld\n", id, get_timestamp());
    usleep(get_random_number()*1000);
    sem_wait(&gall1);
    unlock(&step2);

    // Enter Gallery 1 (C) with limited occupancy
    printf("Visitor %d entered Gallery 1 at timestamp %lld\n", id, get_timestamp());
    usleep(x * 1000000);  // Time spent in Gallery 1
    sem_wait(&glass);  // Exit Gallery 1 and release spot

    // Move to Glass Corridor (limited to 3 visitors)
    sem_post(&gall1);
    printf("Visitor %d is in Glass Corridor at timestamp %lld ms\n", id, get_timestamp());
    usleep(get_random_number()*1000);  // Arbitrary delay for the glass corridor
    sem_post(&glass);  // Exit Glass Corridor

    // Enter Gallery 2
    printf("Visitor %d entered Gallery 2 at timestamp %lld ms\n", id, get_timestamp());
    usleep(y*1000000);
    if(id<2000){
        usleep(get_random_number()*10000);
    }
    photobooth(v);
    return NULL;
}

int main() {
    // Input for visitors and timings
    std::ifstream inputFile("input.txt");
    std::cin.rdbuf(inputFile.rdbuf());
    freopen("output.txt","w",stdout);

    std::cin >> n >> m >> w >> x >> y >> z;
    // Initialize synchronization primitives
    pthread_mutex_init(&step0, NULL);
    pthread_mutex_init(&step1, NULL);
    pthread_mutex_init(&step2, NULL);
    sem_init(&gall1, 0, 5);  // Gallery 1 can hold up to 5 visitors
    sem_init(&glass, 0, 3);  // Glass corridor can hold up to 3 visitors
    pthread_mutex_init(&check,NULL);
    pthread_mutex_init(&rc_l,NULL);
    pthread_mutex_init(&wc_l,NULL);
    pthread_mutex_init(&photo,NULL);
    rc=wc=0;
    // Generate visitors
    pthread_t visitors_t[n + m];
    visitors.clear();
    FOR(i, 0, n) {
        visitor v;
        v.id = 1001 + i;
        visitors.push_back(v);
    }
    FOR(i, 0, m) {
        visitor v;
        v.id = 2001 + i;
        visitors.push_back(v);
    }

    // Start visitor threads
    FOR(i, 0, n + m) {
        pthread_create(&visitors_t[i], NULL, work, &visitors[i]);
    }

    // Wait for all threads to finish
    FOR(i, 0, n + m) {
        pthread_join(visitors_t[i], NULL);
    }

    // Clean up
    pthread_mutex_destroy(&step0);
    pthread_mutex_destroy(&step1);
    pthread_mutex_destroy(&step2);
    pthread_mutex_destroy(&check);
    pthread_mutex_destroy(&wc_l);
    pthread_mutex_destroy(&rc_l);
    pthread_mutex_destroy(&photo);
    sem_destroy(&gall1);
    sem_destroy(&glass);

    printf("All finished\n");
    return 0;
}
