#include <stdio.h>
#include "atarix/discovery.h"

int main(void) {
    if (sizeof(atarix_discovery_header_v1_t) != 32u) {
        printf("header size mismatch\n");
        return 1;
    }

    if (sizeof(atarix_discovery_record_header_v1_t) != 8u) {
        printf("record header size mismatch\n");
        return 1;
    }

    if (sizeof(atarix_discovery_resource_record_v1_t) != 32u) {
        printf("resource record size mismatch\n");
        return 1;
    }

    if (sizeof(atarix_discovery_service_record_v1_t) != 32u) {
        printf("service record size mismatch\n");
        return 1;
    }

    return 0;
}
