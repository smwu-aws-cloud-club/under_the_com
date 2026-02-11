#include <pthread.h>
#include <atomic>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <vector>
#include <chrono>
#include <iostream>

enum Mode {
    MUTEX,
    SPIN,
    ATOMIC
};

struct Config {
    Mode mode;
    int threads;
    long iters_per_thread;
};

static long global_counter_plain = 0;
static std::atomic<long> global_counter_atomic{0};

static pthread_mutex_t g_mutex;
static pthread_spinlock_t g_spin;

struct ThreadArg {
    Config cfg;
};

static void* worker(void* arg) {
    ThreadArg* a = (ThreadArg*)arg;
    auto cfg = a->cfg;

    if (cfg.mode == MUTEX) {
        for (long i = 0; i < cfg.iters_per_thread; i++) {
            pthread_mutex_lock(&g_mutex);
            global_counter_plain++;
            pthread_mutex_unlock(&g_mutex);
        }
    } else if (cfg.mode == SPIN) {
        for (long i = 0; i < cfg.iters_per_thread; i++) {
            pthread_spin_lock(&g_spin);
            global_counter_plain++;
            pthread_spin_unlock(&g_spin);
        }
    } else { // ATOMIC
        for (long i = 0; i < cfg.iters_per_thread; i++) {
            global_counter_atomic.fetch_add(1, std::memory_order_relaxed);
        }
    }

    return nullptr;
}

static Mode parse_mode(const char* s) {
    if (strcmp(s, "mutex") == 0) return MUTEX;
    if (strcmp(s, "spin") == 0) return SPIN;
    if (strcmp(s, "atomic") == 0) return ATOMIC;
    std::cerr << "Unknown mode: " << s << "\n";
    std::exit(1);
}

int main(int argc, char** argv) {
    if (argc != 4) {
        std::cerr << "Usage: " << argv[0] << " <mutex|spin|atomic> <threads> <iters_per_thread>\n";
        return 1;
    }

    Config cfg;
    cfg.mode = parse_mode(argv[1]);
    cfg.threads = std::atoi(argv[2]);
    cfg.iters_per_thread = std::atol(argv[3]);

    pthread_mutex_init(&g_mutex, nullptr);
    pthread_spin_init(&g_spin, PTHREAD_PROCESS_PRIVATE);

    global_counter_plain = 0;
    global_counter_atomic.store(0);

    std::vector<pthread_t> th(cfg.threads);
    std::vector<ThreadArg> args(cfg.threads);

    auto start = std::chrono::steady_clock::now();

    for (int i = 0; i < cfg.threads; i++) {
        args[i].cfg = cfg;
        pthread_create(&th[i], nullptr, worker, &args[i]);
    }

    for (int i = 0; i < cfg.threads; i++) {
        pthread_join(th[i], nullptr);
    }

    auto end = std::chrono::steady_clock::now();
    double sec = std::chrono::duration<double>(end - start).count();

    long expected = cfg.iters_per_thread * (long)cfg.threads;

    long actual = 0;
    if (cfg.mode == ATOMIC) actual = global_counter_atomic.load();
    else actual = global_counter_plain;

    std::cout << "mode=" << argv[1]
              << " threads=" << cfg.threads
              << " iters/thread=" << cfg.iters_per_thread
              << " expected=" << expected
              << " actual=" << actual
              << " time_sec=" << sec
              << " throughput_ops_per_sec=" << (expected / sec)
              << "\n";

    pthread_spin_destroy(&g_spin);
    pthread_mutex_destroy(&g_mutex);

    return 0;
}
