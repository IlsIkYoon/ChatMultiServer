#pragma once

//#include <unordered_map>
//#include <iostream>
//#include <string>

#include <unordered_map>
#include <iostream>
#include <string>

using namespace std;

class Parser
{
private:
	unordered_map<string, string> configStringData;
	unordered_map<string, int> configIntData;
	unordered_map<string, float> configFloatData;

	//unordered_map<string, string> LeakData;
	vector<pair<string, string>> LeakData;

	bool isSpace(char c); //傍归贸府

	void skipComments(FILE* file, char& c); //林籍贸府

	int line = 1;
public:
	bool LoadFile(const char* filename);

	bool GetValue(const char* key, int& value);
	bool GetValue(const char* key, float& value);
	bool GetValue(const char* key, char* value, int valueSize);
	bool GetValue(const char* key, string& value);

	bool CheckLeak();

	////sample侩单捞磐
	//void Sample()
	//{
	//	configStringData["String"] = "abcde";
	//	configIntData["Int"] = 10;
	//	configFloatData["Float"] = 5.5;
	//}
};

class ExcelParser
{
private:
	vector<string> excelHeader;
	vector<vector<string>> excelData;
	unordered_map<string, int> headerMap;
	int currentRow;
	int currentColumn;

	void parseRow(char* line);

public:
	ExcelParser() : currentRow(0), currentColumn(0) {};

	bool LoadFile(const char* filename);

	void SelectRow(int rowIndex);
	void NextRow();
	void PrevRow();

	void MoveColumn(int colIndex);
	void NextColumn();
	void PrevColumn();

	bool GetColumn(int& value);
	bool GetColumn(float& value);
	bool GetColumn(string& value);
	bool GetColumn(char* value, int valueSize);

	int GetColumnIndex(const char* columnName);

	void Test()
	{
		cout << excelHeader.size() << '\n';
		for (int i = 0; i < excelHeader.size(); i++)
		{
			cout << excelHeader[i] << " ";
		}
		cout << '\n';
		cout << "-----------------------------\n";
		cout << excelData.size() << '\n';
		for (int i = 0; i < excelData.size(); i++)
		{
			for (int j = 0; j < excelData[0].size(); j++)
			{
				cout << excelData[i][j] << " ";
			}
			cout << '\n';
		}
		cout << "----------------------------------\n";

		cout << currentRow << " : " << currentColumn << '\n';
	}

	void Test2()
	{
		cout << currentRow << " : " << currentColumn << '\n';
	}
};

