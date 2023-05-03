#pragma once

// TODO
// 
// сделать возраст клетки и ген отвечающий за то в каком возрасте она должна размножитс¤
// сделать настраиваемую консоль
// сделать разным деревьям разный цвет
// сделать энергию
// сделать свет
// сделать вероятность мутации генетически детерменированную
// сделать генетически дерерменированное время жизни
// настроить кодировку кириллицы +

Randomaizer gRAND;

inline uint32_t color_u32(ivec4 c) {
	return uint8_t(c[0]) + (uint8_t(c[1]) << 8) + (uint8_t(c[2]) << 16) + (uint8_t(c[3]) << 24);
}


template<typename T>
class PoolContainer {
public:
	std::vector<T> storage;

	int get_new() {
		if (disabled.empty()) {
			enabled.push_back(storage.size());
			storage.push_back(T());
		}
		else {
			enabled.push_back(disabled.back());
			disabled.pop_back();
		}
		return enabled.back();
	}

	int push(T obj) {
		int index = get_new();
		storage[index] = obj;
		return index;
	}

	void erase(int index) {
		if (index >= 0 && index < enabled.size()) {
			disabled.push_back(enabled[index]), enabled[index] = -1;
		}
	}

	// убирает мусор из enabled и возвращает в disabled
	void erase() {
		enabled.erase(std::remove_if(enabled.begin(), enabled.end(),
			[&](const int& a) { return a < 0; }
		), enabled.end());
	}

	std::vector<int> enabled;
	std::vector<int> disabled;
};

template <typename T>
void remove(std::vector<T>& v, size_t index) {
	v.erase(v.begin() + index);
}


enum {
	semen = 0,
	green = 1,
	wood = 2,
	air = 3,
	ground = 4,
};

uint8_t r() {
	return gRAND.u8q() % 15;
}

uint8_t r2() {
	return gRAND.u8() % 2;
}


class Tree {
public:
	int age;
	int energy;
	bool alive;
	int cell_counter;

	std::vector<std::vector<uint8_t>> genom;
	Tree() {
		init();
	}

	Tree(std::vector<std::vector<uint8_t>> &from_this) {
		init(from_this);
	}

	Tree(std::vector<std::vector<uint8_t>> &from_this, int mutation) {
		init(from_this, mutation);
	}

	void init(std::vector<std::vector<uint8_t>> &from_this, int mutation) {
		genom = from_this;
		for (int i = 0; i < mutation; i++) {
			int index1 = gRAND.u16() % genom.size();
			int index2 = gRAND.u16() % genom[index1].size();
			if (index2 == (genom[index1].size() - 1))
				genom[index1][index2] = r2();
			else
				genom[index1][index2] = r();
		}
		_init();
	}

	void init(std::vector<std::vector<uint8_t>> &from_this) {
		genom = from_this;
		_init();
	}

	void init() {
		genom = { // что отрастить в стороны + тип клетки
			{r(),r(),r(),r(),semen},
			{r(),r(),r(),r(),r2(),},
			{r(),r(),r(),r(),r2(),},
			{r(),r(),r(),r(),r2(),},
			{r(),r(),r(),r(),r2(),},
			{r(),r(),r(),r(),r2(),},
			{r(),r(),r(),r(),r2(),},
			{r(),r(),r(),r(),r2(),},
			{r(),r(),r(),r(),r2(),},
			{r(),r(),r(),r(),r2(),},
		};
		_init();
	}



private:
	void _init() {
		age = 0;
		energy = 0;
		cell_counter = 0;
		alive = true;
	}
};



class Cell {
public:
	uint8_t type;
	uint8_t gen_index;
	int index_tree = -1;


	Cell(int t) {
		type = t;
	}


};


class CellularAutomation {
public:
	std::vector<uint8_t> color_map;
	uint32_t* color_map_u32;

	PoolContainer<Tree> trees;
	std::vector<int> index_live_arr;
	int max_age;
	int width;
	int height, extended_height;
	int length;
	std::vector<Cell> world_map;
	int frame_count = 0;
	enum { up, right, down, left };
	std::vector<int> directions;

	

	CellularAutomation(int w, int h) {

		color_map.resize(w * h * 4, 255);
		color_map_u32 = (uint32_t*)&color_map[0];

		gRAND.ini();
		max_age = h;
		width = w;
		height = h, extended_height = h + 1;
		length = w * h;
		world_map.resize(width * extended_height, Cell(air));


		for (int ind = 0; ind < world_map.size(); ind++) {

			int x = (ind % width);
			int y = (ind / width);

			if (y < 1 )
				become(world_map[ind], ind, ground); 

			if (y >= height)
				world_map[ind].type = ground;
		}

		directions = { width , 1, -width, -1 };
	}

	

	void spawn(int x, int y) {
		try_spawn_semen(x + y * width);
	}

	int get_world_cell_type(int x, int y) {
		return world_map[x + y * width].type;
	}

	void step() {
		spawn_starting_seeds_if_needed();
		cells_handler();
		clean_up_index_live_arr();
		trees_handler();
		frame_count++;
	}

private:





	void cells_handler() {
		int old_length = index_live_arr.size();
		for (int ind = 0; ind < old_length; ind++) {
			int index = index_live_arr[ind];
			auto& c = world_map[index];

			if (c.type == semen) {
				// ПОЛУЧИТЬ ИНДЕКС КЛЕТКИ СНИЗУ
				int index_d = index + directions[down];


				switch (world_map[index_d].type) {

					// СЕМЕНЬ ПАДАЕТ
				case air:
					index_live_arr[ind] = -index_live_arr[ind];
					become(c, index, air);                                                      
					index_live_arr.push_back(index_d);
					become_live(world_map[index_d], index_d, semen, c.index_tree, c.gen_index);        
					break;

					// СЕМЕНЬ ПРОРОСТАЕТ
				case ground:
					trees.storage[c.index_tree].cell_counter--;
					become_live(c, index, green, trees.push(Tree(trees.storage[c.index_tree].genom, 1))); 
					trees.storage[c.index_tree].cell_counter++;
					break;

					// СЕМЕНЬ УМИРАЕТ
				default:
					kill_cell(ind, c, index);
				}
			}
			else {
				// КЛЕТКА УМИРАЕТ
				if (trees.storage[c.index_tree].alive == false) {
					kill_cell(ind, c, index);
					continue;
				}

				// КЛЕТКА ПРОРОСТАЕТ
				if (c.type == green) {
					try_grow(c.index_tree, index, c.gen_index);
				}
			}
		}
	}

	void become(Cell &cell, int index, int type){
		static const uint32_t color_arr[] = {
				color_u32(ivec4(0, 0, 0, 255)),
				color_u32(ivec4(255, 103, 61, 255)),
				color_u32(ivec4(128, 143, 255, 255)),
				color_u32(ivec4(255)),
				color_u32(ivec4(60, 50, 40, 250))
		};

		color_map_u32[index] = color_arr[type];
		cell.type = type;

		
	}

	void become_live(Cell &cell, int index, int type, int index_tree) {
		become(cell, index, type);
		cell.index_tree = index_tree;
	}

	void become_live(Cell &cell, int index, int type, int index_tree, int gen_index) {
		become(cell, index, type);
		cell.gen_index = gen_index;
		cell.index_tree = index_tree;
	}


	void spawn_starting_seeds_if_needed() {
		// спавн новых семечек при пустом поле
		if (index_live_arr.size() == 0) {
			for (int i = 0; i < width / 2; i++)
				spawn(rand() % width, rand() % height / 2);
		}
	}

	void clean_up_index_live_arr() {
		// убрать умершие клетки из списка живых
		index_live_arr.erase(std::remove_if(index_live_arr.begin(), index_live_arr.end(),
			// если клетка не жива¤, в ней будет отрицательный индекс, равный положительному эквиваленту
			[&](const auto& a) { if (a < 0) { return true; } return false; }
		), index_live_arr.end());
	}


	void trees_handler() {
		for (int i = 0; i < trees.enabled.size(); i++) {
			auto& tree = trees.storage[trees.enabled[i]];
			if (tree.age >= max_age || tree.cell_counter <= 0) {
				tree.alive = false;
				if (tree.cell_counter <= 0)
					trees.erase(i);
			}
			else
				tree.age++;
		}
		trees.erase();
	}


	void kill_cell(int index, Cell& cell, int index_world_map) {
		index_live_arr[index] = -index_live_arr[index];
		become(cell, index_world_map, air); 
		trees.storage[cell.index_tree].cell_counter--;
	}

	void try_spawn_semen(int ind) {
		if (world_map[ind].type != air)
			return;
		// спавним виртуальное дерево родитель
		spawn(semen, ind, trees.push(Tree()));
	}



	void spawn(int type, int ind, int ind_tree, int ind_gen = 0) {
		index_live_arr.push_back(ind);
		become_live(world_map[ind], ind, type, ind_tree, ind_gen); 
		trees.storage[ind_tree].cell_counter++;
	}

	void try_grow(int index_tree, int index, int gen_index) {
		try {
			const auto& tree = trees.storage.at(index_tree);
			const auto& gen = tree.genom.at(gen_index);
			// ПО КАЖДОМУ ИЗ НАПРАВЛЕНИЙ
			for (int i = 0; i < 4; i++) {
				// ЕСЛИ ЕСТЬ ЖЕЛАНИЕ РАЗМНОЖИТСЯ
				const auto& g = gen.at(i);
				if (g < tree.genom.size()) {
					// ЕСЛИ ЕСТЬ ВОЗМОЖНОСТЬ РАЗМНОЖИТСЯ
					int index_n = index + directions[i];
					if (world_map[index_n].type == air) {
						// РАЗМНОЖИТСЯ
						spawn(tree.genom[g][4], index_n, index_tree, g);
					}
				}
			}


		}
		catch (std::exception &e) {
			std::cout << e.what();
		}

		become(world_map[index], index, wood); // одеревенение 

	}


};