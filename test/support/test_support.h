#ifndef TEST_SUPPORT_H
#define TEST_SUPPORT_H

#include <stdbool.h>

void setUp(void);
void tearDown(void);

bool test_state_request_queue_empty(void);

#endif /* TEST_SUPPORT_H */