#include "app.h"

int main(int argc, char** argv)
{
	App* hridizaApp = new App(800, 600, true);

	hridizaApp->run();
	delete hridizaApp;

	return 0;
}