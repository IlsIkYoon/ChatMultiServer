
#include "pch.h"
#include "TextParser.h"

bool TextParser::GetData(const char* fileName) {

	FILE* fpRead;

	if (fopen_s(&fpRead, fileName, "rt")) {
		printf("%s 파일 오픈 실패\n", fileName);
		return false;
	}

	fseek(fpRead, 0, SEEK_END);
	_bufferSize = ftell(fpRead);
	fseek(fpRead, 0, SEEK_SET);

	_dataBuf = new char[_bufferSize];

	fread(_dataBuf, 1, _bufferSize, fpRead);

	fclose(fpRead);

	_dataPos = _dataBuf;

	return true;
}

void TextParser::CloseData() {

	delete _dataBuf;

}



TextParser txParser;
