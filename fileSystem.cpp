#include <iostream>
#include <map>
#include <string>
#include <sstream>
#include <fstream>
#include <stdlib.h>
#include <pthread.h>

using namespace std;

bool interested_cout[2];
int turn_cout;

void enterRegion_cout(int threadId) {
	int other = 1 - threadId;
	interested_cout[threadId] = true;
	turn_cout = other;

	while (turn_cout == other && interested_cout[other])
		;
}

void leaveRegion_cout(int threadId) {
	interested_cout[threadId] = false;
}

struct file {
	string data_file;
	bool interested[2];
	int turn;
};

map<string, file> system_file; //структура файловой системы

typedef std::map<string, file>::iterator it_type;

void enterRegion(int threadId, string filename) {
	int other = 1 - threadId;
	system_file[filename].interested[threadId] = true;
	system_file[filename].turn = other;
	while (system_file.count(filename) != 0
			&& system_file[filename].turn == other
			&& system_file[filename].interested[other])
		;
}

void leaveRegion(int threadId, string filename) {
	system_file[filename].interested[threadId] = false;
}

void space_split(string str, string mas[]) {
	istringstream iss(str);
	iss >> mas[0];
	iss >> mas[1];
	iss >> mas[2];
}

bool analis(string str, int th) {
	string com, fff, check;
	string mas[3];
	space_split(str, mas);
	com = mas[0];
	fff = mas[1];
	check = mas[2];
	if (check != "" || fff == "") {
		cout << "Ошибка ввода строки\n";
		return false;
	}
	if (!com.compare("touch")) {
		if (system_file.count(fff) != 0) {
			cout << "Файл " << fff << " уже создан.\n";
			return false;
		} else {
			enterRegion(th, fff);
			system_file[fff].data_file = "";
			system_file[fff].interested[0] = false;
			system_file[fff].interested[1] = false;
			cout << "Файл " << fff << " создан\n";
			leaveRegion(th, fff);
			return true;
		}
	}
	if (system_file.count(fff) == 0) {
		cout << "Файл " << fff << " не найден.\n";
		return false;
	}
	enterRegion(th, fff);
	if (!com.compare("cat")) {
		if (system_file.count(fff) == 0)
			cout << "Файл " << fff << " не найден.\n";
		else {
			cout << system_file[fff].data_file << endl;
		}
	} else if (!com.compare("write")) {
		if (system_file.count(fff) == 0)
			cout << "Файл " << fff << " не найден\n";
		else {
			char ch;
			string str = "";
			while ((ch = cin.get()) != 27) {
				if (cin.eof())
					cin.clear();
				else
					str += ch;
			}
			while ((ch = cin.get()) != '\n') {
				if (cin.eof())
					cin.clear();
			}

			system_file[fff].data_file = str;
		}
	} else if (!com.compare("delete")) {
		if (system_file.count(fff) != 0) {
			system_file.erase(fff);
			cout << "Файл " << fff << " удален\n";
		} else {
			cout << "Файл " << fff << " не найден\n";
		}
	} else {
		cout << "Ошибка. Команда " << com << " неопределена.\n";
	}
	if (system_file.count(fff) != 0)
		leaveRegion(th, fff);
	return true;
}
void* get_file_sizes(void *th_id) {
	int id = *((int*) (&th_id));
	for (it_type iterator = system_file.begin(); iterator != system_file.end();
			iterator++) {
		enterRegion(id, iterator->first);
		enterRegion_cout(id);
		cout << id << " : " << iterator->first << " : "
				<< iterator->second.data_file.length() << endl;
		leaveRegion_cout(id);
		leaveRegion(id, iterator->first);
	}
	pthread_exit(NULL);
	return 0;
}
void test_programm() {
	pthread_t thread1, thread2;
	int id = 0;

	pthread_create(&thread1, NULL, get_file_sizes, (void*) id);
	id = 1;
	pthread_create(&thread2, NULL, get_file_sizes, (void*) id);
	pthread_join(thread1, NULL);
	pthread_join(thread2, NULL);
}
void init() {
	ifstream ff;
	string in = "";
	ff.open("fs.dat");
	if (ff) {
		string f_name;
		int col;
		string mas[3];
		while (!ff.eof()) {
			getline(ff, in);
			if (!in.compare("0 0 0"))
				break;
			space_split(in, mas);
			f_name = mas[0];
			col = atoi(mas[1].c_str());
			char *data_file = new char[col];
			ff.read(data_file, col);
			data_file[col]= '\0';
			system_file[f_name].data_file = data_file;
			system_file[f_name].interested[0] = false;
			system_file[f_name].interested[1] = false;
			ff.get();
			delete [] data_file;
		}
		ff.close();
	} else
		cout << "Предупреждение! Файл fs.dat не найден\n";
}
void serializable() {
	ofstream ff;
	ff.open("fs.dat");
	for (it_type iterator = system_file.begin(); iterator != system_file.end();
			iterator++) {
		ff << iterator->first << " " << iterator->second.data_file.length()
				<< endl << iterator->second.data_file << endl;
	}
	ff << "0 0 0";
	ff.close();
}
int main() {
	string input_line;
	init();
	for (;;) {
		cout << "\n> ";
		getline(cin, input_line);
		if (cin.eof())
			cin.clear();
		if (!input_line.compare("exit"))
			break;
		if (!input_line.compare("test"))
			test_programm();
		else if (input_line.compare(""))
			analis(input_line, 0);
	}
	serializable();
	return 0;
}
