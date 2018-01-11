
#define M3UAROUTER
#include <modules/common.h>
#include <modules/nethelper.h>
#include <modules/axe/mml/parser.h>
#undef M3UAROUTER

void signalling_loop(void)
{
	mml_init_data();

	yyinput_analyse_stop0_file1_buffer2 = 1; // parse initial data from configuration file
	yymmlparse();

	if (mml.Cmmlcnf.thread_id != 0)
		pthread_join(mml.Cmmlcnf.thread_id, NULL);

	ErrorTraceHandle(1, "signalling_loop() MML connection down, I'm down.\n");

	// wait to mml channel close down
	// or
	// wait for O&M to close down

	//while (1) {} // loop indefinetely
}
