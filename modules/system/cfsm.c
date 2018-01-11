#define CFSM
#include <modules/common.h>
#include "cfsm.h"
#undef CFSM


void cfsm(void) {
	int error_code;

	if (error_code = pthread_create(&cfsm_thread_id, NULL,
									cfsm_thread,
									NULL) != 0) {
		ErrorTraceHandle(0, "cfsm() cfsm_thread() thread failed in pthread_create() !.\n\n");
	}
}

void cfsm_loop() {

	while (1) {





	}
}


void cfsm_thread(void *arg) {
	cfsm_loop();
}


