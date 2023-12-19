#include "csaw.h"
#include "parser/parser.h"
#include <stdio.h>
#include <fstream>

int main(int argc, const char** argv)
{
	csaw::Run();

	return 0;
}

void csaw::Shell()
{
}

void csaw::Run()
{
	std::ifstream stream("C:\\Users\\Felix Schreiber\\Documents\\GitHub\\ChainSaw\\csaw\\fib\\main.csaw");
	csaw::Parse(stream);
}
