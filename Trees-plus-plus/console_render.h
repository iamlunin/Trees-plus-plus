#pragma once

#include "CA.h"

int console_ran() {

	int W = 120;
	int H = 30;

	CellularAutomation ca(W, H);


	std::vector<char> str(ca.length + 1, ' ');
	str[ca.length] = '\0';

	const char gradient[] = ".oO T";
	while (true) {




		ca.step();
		for (int x = 0; x < W; x++)
			for (int y = 0; y < H; y++) {
				str[x + y * W] = gradient[ca.get_world_cell_type(x, H - 1 - y)];
			}

		std::cout << &str[0];



		Sleep(1);
	}


	return 0;
}