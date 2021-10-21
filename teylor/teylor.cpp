#include <iostream>
#include <cmath>
#include <cstdlib>
#include <sys/types.h>
#include <cassert>
#include <functional>
#include <cstdio>
#include <omp.h>
#include <cstring>

class Factorial;

using integer = unsigned long;
using dbl = long double;
using function = std::function<dbl(const integer k, Factorial f, const dbl x, const dbl x0)>;

void thread_info() {
    printf("info:\n");
    printf("\tthreads num set: %d\n", omp_get_max_threads());
    printf("\tnum of cores: %d\n", omp_get_num_procs());
    printf("\tdynamic threads allowed? %d\n", omp_get_dynamic());
    printf("\tnested allowed? %d\n", omp_get_nested());
    printf("\tthread limit constant: %d\n\n", omp_get_thread_limit());
}

void init_omp(int threads_num = 1) {
    // omp_set_dynamic(threads_num);     // Explicitly disable dynamic teams
    omp_set_num_threads(threads_num);
    thread_info();
}

void parse_help(int argc, char* argv[], const char* message) {
    for (int i = 0; i < argc; ++i) {
        const char* help = argv[i];
        if (!strcmp("-h", help) || !strcmp("--help", help)) {
            printf("%s\n", message);
            exit(0);
        }
    }
}

int delay() {
    int x = 0;
    for (long i = 0; i < 1000'000; ++i) {
        ++x;
    }
    return x;
}

class Factorial {
public:
    Factorial(){};
    void set(const integer new_base) noexcept;
    integer get() const noexcept;
    void dump() const noexcept;
    
private:
    Factorial(const integer n, const integer value);
    integer m_base{0};
    integer m_value{1};
};

Factorial::Factorial(const integer n, const integer val): m_base(n), m_value(val) {}

void Factorial::set(const integer new_base) noexcept {
    if (new_base == 0 || new_base == 1) {
        m_base = new_base;
        m_value = 1;
        return;
    }

    if (new_base == m_base) {
        return;
    }

    if (new_base < m_base) {
        m_base = 1;
        m_value = 1;
    }

    auto base = m_base;
    for (base = m_base; base < new_base; ++base) {
        m_value *= (base + 1);
    }
    m_base = base;
    
    return;
}

integer Factorial::get() const noexcept {
    return m_value;
}

void Factorial::dump() const noexcept {
    std::cout << "n: " << m_base << " n!: " << m_value << std::endl;
}

dbl sin_mem(const integer k, Factorial f, const dbl x, const dbl x0) {
    f.set(2*k+1);
    dbl result = powl((x - x0), 2*k+1) / f.get();
    
    if (k & 0x1 > 0)
        return -result;
    return result;
}

dbl exponent_mem(const integer k, Factorial f, const dbl x, const dbl x0) {
    return powl((x - x0), k) / f.get();
}

int main(int argc, char** argv) {
    const char* message = \
    "Calculates teylor sum of <N> with <P> threads.\n"
    "\tArgs: <N> [<P>] (default 1 thread)";

    parse_help(argc, argv, message);

    assert(argc > 1);

    uint64_t N = atoi(argv[1]);
    assert(N > 0);

    int threads = 1;
    if (argc == 3) {
        threads = atoi(argv[2]);
        assert(threads > 0 && threads < omp_get_thread_limit());
    }

    init_omp(threads);

    function teylor_mem = sin_mem; //exponent_mem;
    dbl sum = 0;

#pragma omp parallel
{
    Factorial fact{};
    #pragma omp for reduction(+:sum) schedule(static)
    for (auto k = 0; k < N; ++k) {
        fact.set(k);
        // delay();
        sum += teylor_mem(k, fact, 1, 0.);
    }
}
    printf("sum: %.32Lf\n", sum);
}