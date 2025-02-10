#include "app.h"

int main(int argc, char** argv)
{
	App* hridizaApp = new App(1800, 1000, true);

	hridizaApp->run();
	delete hridizaApp;

	return 0;
}