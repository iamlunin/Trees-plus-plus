#pragma once

// TODO
// ������� ������� ������ � ��� ���������� �� �� � ����� �������� ��� ������ �����������
// ������� ������������� �������
// ������� ������ �������� ������ ����
// ������� �������
// ������� ����
// ������� ����������� ������� ����������� �����������������
// ������� ����������� ����������������� ����� �����

Randomaizer gRAND;

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

	void erase(int index) {
		if (index >= 0 && index < enabled.size()) {
			disabled.push_back(enabled[index]), enabled[index] = -1;
		}
	}

	// ������� ����� �� enabled � ���������� � disabled
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
		genom = { // ��� ��������� � ������� + ��� ������
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
		become(t);
	}


	void become_live(int t, int i_t, int g) {
		type = t;
		index_tree = i_t;
		gen_index = g;
	}

	void become_live(int t, int i_t) {
		type = t;
		index_tree = i_t;
	}

	void become(int t) {
		type = t;
	}



};


class CellularAutomation {
public:
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
		gRAND.ini();
		max_age = h;
		width = w;
		height = h, extended_height = h + 1;
		length = w * h;
		world_map.resize(width * extended_height, Cell(air));

		for (int ind = 0; ind < world_map.size(); ind++) {

			int x = (ind % width);
			int y = (ind / width);

			if (y < 1 || y >= height)
				world_map[ind] = Cell(ground);

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

		// ����� ����� ������� ��� ������ ����
		if (index_live_arr.size() == 0) {
			for (int i = 0; i < width / 2; i++)
				spawn(rand() % width, rand()%height/2);
		}

		// �������� ����. �������. �����������.
		int old_length = index_live_arr.size();
		for (int ind = 0; ind < old_length; ind++) {
			int index = index_live_arr[ind];
			auto& c = world_map[index];

			if (c.type == semen) {
				// �������� ������ ������ �����
				int index_d = index + directions[down];

				int old_index_tree;

				switch (world_map[index_d].type) {

					// ��� ������� ���� �������. ������ ������
				case air:
					index_live_arr[ind] = -index_live_arr[ind];
					c.become(air);
					index_live_arr.push_back(index_d);
					world_map[index_d].become_live(semen, c.index_tree, c.gen_index);
					break;

					// ��� ������� �����. ������ ����������
				case ground:
					old_index_tree = c.index_tree; 
					trees.storage[old_index_tree].cell_counter--;
					c.become_live(green, trees.get_new());
					trees.storage[c.index_tree] = Tree(trees.storage[old_index_tree].genom, 1);
					trees.storage[c.index_tree].cell_counter++;
					break;

					// ��� ������� ������������ �����, ������ �������
				default:
					index_live_arr[ind] = -index_live_arr[ind];
					c.become(air);
					trees.storage[c.index_tree].cell_counter--;

				}
			}
			else {
				// ���� �� ������
				if (trees.storage[c.index_tree].alive == false) {
					index_live_arr[ind] = -index_live_arr[ind];
					c.become(air);
					trees.storage[c.index_tree].cell_counter--;
					continue;
				}

				// ������� � �����������
				if (c.type == green) {
					try_grow(c.index_tree, index, c.gen_index);
				}
			}
		}


		// ������ ������� ������ �� ������ �����
		index_live_arr.erase(std::remove_if(index_live_arr.begin(), index_live_arr.end(),
			// ���� ������ �� �����, � ��� ����� ������������� ������, ������ �������������� �����������
			[&](const auto& a) { if (a < 0) { return true; } return false; }
		), index_live_arr.end());

		// ���������� ������ �������� �� ������� ������
		for (int i = 0; i < trees.enabled.size(); i++) {
			int index = trees.enabled[i];
			auto& t = trees.storage[index];

			// ��� ����� ������ ����� ���� ��� ��������� ��� ������ ���
			if (t.cell_counter < 0) { 
				throw "cell_counter < 0";
			}
			// ������� ���������� ������ �������
			if (t.age >= max_age) {
				t.alive = false;

				// ������� �������� ������ �� ������
				if (t.cell_counter <= 0)
					trees.erase(i);

			// ��������� ���� ������ ����
			} else 
				t.age++;

		
			
			
		}

		// ����� ����������� �������� �� ������ �����
		trees.erase();

		
		frame_count++;
	}

private:

	void try_spawn_semen(int ind) {
		if (world_map[ind].type != air)
			return;
		// ������� ����������� ������ ��������
		int index_new_tree = trees.get_new();
		trees.storage[index_new_tree] = Tree();
		spawn(semen, ind, index_new_tree);
	}



	void spawn(int type, int ind, int ind_tree, int ind_gen = 0) {
		index_live_arr.push_back(ind);
		world_map[ind].become_live(type, ind_tree, ind_gen);
		trees.storage[ind_tree].cell_counter++;
	}

	void try_grow(int index_tree, int index, int gen_index) {

		try {


			const auto& tree = trees.storage.at(index_tree);
			const auto& gen = tree.genom.at(gen_index);
			// �� ������� �� �����������
			for (int i = 0; i < 4; i++) {
				// ���� ���� ������� �����������
				const auto& g = gen.at(i);
				if (g < tree.genom.size()) {
					// ���� ���� ����������� �����������
					int index_n = index + directions[i];
					if (world_map[index_n].type == air) {
						// �����������
						spawn(tree.genom[g][4], index_n, index_tree, g);
					}
				}
			}


		}
		catch (std::exception &e) {
			std::cout << e.what();
		}

		world_map[index].become(wood); // ������������

	}


};