#include <stdio.h>

int is_prime(unsigned long long n) {
    if (n <= 1) return 0;
    if (n <= 3) return 1;
    if (n % 2 == 0 || n % 3 == 0) return 0;
    
    for (unsigned long long i = 5; i * i <= n; i += 6) {
        if (n % i == 0 || n % (i + 2) == 0)
            return 0;
    }
    return 1;
}

unsigned long long find_nth_prime(int n) {
    int count = 0;
    unsigned long long num = 2;
    
    while (count < n) {
        if (is_prime(num)) {
            count++;
            if (count == n)
                return num;
        }
        num++;
    }
    return num;
}

int main()
{
	int target = 5000000;

    unsigned long long prime = find_nth_prime(target);
    
    printf("The %dth prime is %llu\n", target, prime);

	return 0;
}
