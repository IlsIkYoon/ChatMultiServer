
#include "Parser.h"

bool Parser::isSpace(char c)
{
	if (c == '\n')
	{
		/////
		//cout << "공백줄추카 : " << line << '\n';
		line++;
	}
	return (c == ' ' || c == '\t' || c == '\n' || c == '\r');
}

void Parser::skipComments(FILE* file, char& c)
{
	if (c == '/')
	{
		c = fgetc(file);
		if (c == '/')
		{
			while (c != '\n' && c != EOF)
			{
				c = fgetc(file);
			}
			//////
			//cout << "//주석처리 line++ : " << line << '\n';
			line++;
			c = fgetc(file);
		}
		else if (c == '*')
		{
			while (true)
			{
				c = fgetc(file);
				if (c == '\n')
				{
					//////
					//cout << "/**/주석처리 line++ : " <<line<< '\n';
					line++;
				}
				if (c == '*' && (c = fgetc(file)) == '/')
				{
					c = fgetc(file);
					break;
				}
				if (c == EOF)
					break;
			}
		}
	}
}

bool Parser::LoadFile(const char* filename)
{
	FILE* file;
	errno_t err = fopen_s(&file, filename, "r");

	if (err != 0) {
		cout << "파일을 열 수 없습니다 : " << filename << '\n';
		return false;
	}

	char c = 0;
	while ((c = fgetc(file)) != EOF)
	{
		if (isSpace(c))continue;

		if (c == '/')
		{
			skipComments(file, c);
			if (c == EOF) break;
			if (isSpace(c)) continue;
		}

		string key = "";
		while (c != '=' && c != EOF)
		{
			if (!isSpace(c))
				key += c;

			c = fgetc(file);
		}

		string value = "";
		bool inQuotes = false;
		bool isString = false;
		bool semicoloni = false;
		while ((c = fgetc(file)) != ';' && c != EOF)
		{
			if (c == '\n')
			{
				semicoloni = true;
				///////
				//cout << "세미클론누락줄 공백추가++ : " << line << '\n';
				line++;
				break;
			}

			if (c == '\"')
			{
				inQuotes = !inQuotes;
				isString = true;
				continue;
			}

			if (!inQuotes && isSpace(c))continue;
			value += c;
		}

		if (semicoloni)
		{
			string leakKey = "semicoloni leaked in line" + to_string(line - 1);
			string leakValue = "( " + key + " : " + value + " )";
			LeakData.push_back({ leakKey, leakValue });
		}

		if (key.empty())
		{
			string leakKey = "nokey in line" + to_string(line);
			string leakValue = "value : " + value;
			LeakData.push_back({ leakKey, value.empty() ? "novalue" : leakValue });
		}
		else if (value.empty())
		{
			string leakKey = "novalue in line" + to_string(line);
			string leakValue = "key : " + key;
			LeakData.push_back({ leakKey, leakValue });
		}
		else if (isString)
		{
			configStringData[key] = value;
		}
		else
		{
			if (value.find('.') != string::npos)
				configFloatData[key] = stof(value);
			else
				configIntData[key] = stoi(value);
		}
	}

	fclose(file);
	return true;
}

bool Parser::GetValue(const char* key, int& value)
{
	//cout << "int GetValue\n";
	string strkey(key);
	auto it = configIntData.find(strkey);

	if (it != configIntData.end())
	{
		value = it->second;
		return true;
	}

	return false;
}

bool Parser::GetValue(const char* key, float& value)
{
	//cout << "float GetValue\n";
	string strkey(key);
	auto it = configFloatData.find(strkey);

	if (it != configFloatData.end())
	{
		value = it->second;
		return true;
	}
	return false;
}

bool Parser::GetValue(const char* key, char* value, int valueSize)
{
	//cout << "char GetValue\n";
	string strkey(key);
	auto it = configStringData.find(strkey);

	if (it != configStringData.end())
	{
		strcpy_s(value, valueSize, it->second.c_str());
		return true;
	}
	return false;

}

bool Parser::GetValue(const char* key, string& value)
{
	//cout << "string GetValue\n";
	string strkey(key);
	auto it = configStringData.find(strkey);

	if (it != configStringData.end())
	{
		value = it->second;
		return true;
	}
	return false;
}

bool Parser::CheckLeak()
{
	if (!LeakData.empty())
	{
		for (const auto& pair : LeakData)
		{
			cout << pair.first << " -> " << pair.second << '\n';
		}
		return false;
	}
	return true;
}

/// <summary>
/// -----------------------------------ExcelParser----------------------------------
/// </summary>


void ExcelParser::parseRow(char* line)
{
	vector<string> row;
	string temp;

	for (int i = 0; line[i] != '\0'; i++)
	{
		if (line[i] == ',')
		{
			row.emplace_back(temp);
			temp.clear();
		}
		else
			temp += line[i];
	}

	if (!temp.empty())
		row.emplace_back(temp);

	if (excelHeader.empty())
	{
		excelHeader = row;
		for (int i = 0; i < excelHeader.size(); i++)
		{
			headerMap[excelHeader[i]] = i;
		}
	}
	else
	{
		excelData.emplace_back(row);
	}
}


bool ExcelParser::LoadFile(const char* filename)
{
	FILE* file;
	errno_t err = fopen_s(&file, filename, "r");

	if (err != 0) {
		cout << "파일을 열 수 없습니다 : " << filename << '\n';
		return false;
	}

	char buffer[1024];
	while (fgets(buffer, sizeof(buffer), file))
	{
		parseRow(buffer);
	}

	fclose(file);

	return true;
}

void ExcelParser::SelectRow(int rowIndex)
{
	if (rowIndex >= 0 && rowIndex < excelData.size())
		currentRow = rowIndex;
}

void ExcelParser::NextRow()
{
	if (currentRow + 1 < excelData.size())
		currentRow++;
}

void ExcelParser::PrevRow()
{
	if (currentRow > 0)
		currentRow--;
}

void ExcelParser::MoveColumn(int colIndex)
{
	if (colIndex >= 0 && colIndex < excelData[0].size())
	{
		currentColumn = colIndex;
	}
}

void ExcelParser::NextColumn()
{
	if (currentColumn + 1 < excelData[0].size())
		currentColumn++;
}

void ExcelParser::PrevColumn()
{
	if (currentColumn > 0)
		currentColumn--;
}

bool ExcelParser::GetColumn(int& value)
{
	if (currentRow < excelData.size() && currentColumn < excelData[currentRow].size())
	{
		value = stoi(excelData[currentRow][currentColumn]);
		return true;
	}
	return false;
}

bool ExcelParser::GetColumn(float& value)
{
	if (currentRow < excelData.size() && currentColumn < excelData[currentRow].size()) {
		value = stof(excelData[currentRow][currentColumn]);
		return true;
	}
	return false;
}

bool ExcelParser::GetColumn(string& value)
{
	if (currentRow < excelData.size() && currentColumn < excelData[currentRow].size()) {
		value = excelData[currentRow][currentColumn];
		return true;
	}
	return false;
}

bool ExcelParser::GetColumn(char* value, int valueSize)
{
	if (currentRow < excelData.size() && currentColumn < excelData[currentRow].size()) {
		strcpy_s(value, valueSize, excelData[currentRow][currentColumn].c_str());
		return true;
	}
	return false;
}

int ExcelParser::GetColumnIndex(const char* columnName)
{
	if (headerMap.find(columnName) != headerMap.end())
	{
		currentColumn = headerMap[columnName];
		return currentColumn;
	}

	return -1;
}
