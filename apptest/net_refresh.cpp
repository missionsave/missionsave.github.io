#include <resolv.h>
#include <iostream>

void flush_dns_cache() {
    if (res_init() != 0) {
        std::cerr << "Falha ao reinicializar resolver\n";
    } else {
        std::cout << "Cache DNS interno reinicializado\n";
    }
}

int main() {
    // After re-bringing up the interface
    flush_dns_cache();
    // Agora as próximas chamadas de getaddrinfo/gethostbyname vão buscar de novo
    return 0;
}
