#include <iostream>
#include <filesystem>
#include <fstream>
#include <cstdint>
#include <string>

namespace fs = std::filesystem;

using namespace std;

#pragma pack(push)
#pragma pack(1)

// header_name, event_name, sensor_id, sensor_type, channel_num, data_count = unpack("10s30s h c c i", content[0:48])
extern "C" {
	struct WaveHeader {
		char header_name[10];
		char event_name[30];
		uint16_t sensor_id;
		uint8_t  sensor_type;
		uint8_t  channel_num;
		uint32_t data_count;
	};
}

#pragma pack(pop)

string source_dir;
string dest_dir;

void process_directory(const char* fullpath, const char* dir_name) {
	// ��Ŀ��λ���½�Ŀ¼
	string cmd = "mkdir \"";
	cmd = cmd + dest_dir + (fullpath + (source_dir.size())) + "\"";
	int retcode = system(cmd.c_str());
	if (retcode)
		cout << "    -- ����Ŀ¼ʧ�ܣ�" << dest_dir << (fullpath + source_dir.size());
}

void process_file(const char* fullpath, const char* file_name) {
	cout << "  �����ļ���" << fullpath << endl;

	WaveHeader header;
	fstream file(fullpath, ios_base::in | ios_base::out | ios_base::binary);
	file.read((char*)&header, 48);
	if (strcmp(header.header_name, "EVT-WAVE")) {
		cout << "    -- ����һ�������ļ���" << fullpath << endl;
		return;
	}
	fstream foutput(dest_dir + (fullpath + source_dir.size()), ios_base::out);
	if (!foutput) {
		file.close();
		cout << "    -- �޷�д���ļ���" << dest_dir << (fullpath + source_dir.size()) << endl;
		return;
	}
	foutput << scientific;
	try {
		for (int i = 0; i < header.data_count; i++) {
			double data;
			file.read((char*)&data, sizeof(data));
			foutput << data << "\n";
		}
	}
	catch (...) {}
	foutput.close();
	file.close();
}

void list_files(const char* dir_name,
	void (*cb_directory)(const char* fullpath, const char* dir_name),
	void (*cb_file)(const char* fullpath, const char* file_name)) {
	cout << "����Ŀ¼��" << dir_name << endl;
	try {
		for (const auto& entry : fs::directory_iterator(dir_name)) {
			#define FILENAME entry.path().filename().string().c_str()
			#define FULLPATH entry.path().string().c_str()
			if (entry.is_directory()) {
				if (cb_directory)
					cb_directory(FULLPATH, FILENAME);
				list_files(FULLPATH, cb_directory, cb_file);
			}
			else {
				if (cb_file)
					cb_file(FULLPATH, FILENAME);
			}
		}
	}
	catch (...) {
		cout << "�޷��򿪸�Ŀ¼��" << dir_name << endl;
	}
}

int main(int argc, const char ** argv) {
	if (argc != 3) {
		cout << "ʹ�÷���: wave2txt <Դ�������ļ�Ŀ¼> <��ŵ�Ŀ��Ŀ¼> \n\n"
			<< "  * ԴĿ¼���������Ŀ¼�������ݹ鴦��\n"
			<< "  * Ŀ��Ŀ¼�����Ѿ����ڡ�\n\n"
			<< "�˳����������г�����Ҫ��PowerShell��������ʾ�������С�";
		system("pause");
		return -1;
	}

	source_dir = argv[1];
	if ((*source_dir.rbegin()) != '/' && (*source_dir.rbegin()) != '\\')
		source_dir += '/';
	dest_dir = argv[2];
	if ((*dest_dir.rbegin()) != '/' && (*dest_dir.rbegin()) != '\\')
		dest_dir += '/';

	//cout << source_dir << endl;

	list_files(source_dir.c_str(), process_directory, process_file);
}